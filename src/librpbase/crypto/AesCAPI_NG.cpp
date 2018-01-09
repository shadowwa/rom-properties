/***************************************************************************
 * ROM Properties Page shell extension. (librpbase)                        *
 * AesCAPI_NG.cpp: AES decryption class using Win32 CryptoAPI NG.          *
 *                                                                         *
 * Copyright (c) 2016 by David Korth.                                      *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation; either version 2 of the License, or (at your  *
 * option) any later version.                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc., *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.           *
 ***************************************************************************/

#include "AesCAPI_NG.hpp"
#include "../common.h"

// C includes. (C++ namespace)
#include <cassert>
#include <cerrno>

// libwin32common
#include "libwin32common/RpWin32_sdk.h"

// Atomic reference counter.
#include "../threads/Atomics.h"

// References:
// - https://msdn.microsoft.com/en-us/library/windows/desktop/aa376234%28v=vs.85%29.aspx?f=255&MSPPError=-2147217396
#include <bcrypt.h>
#include <winternl.h>

// Function pointer macro.
#define DECL_FUNCPTR(f) typeof(f) * p##f
#define DEF_FUNCPTR(f) typeof(f) * AesCAPI_NG_Private::p##f = nullptr
#define LOAD_FUNCPTR(f) AesCAPI_NG_Private::p##f = (typeof(f)*)GetProcAddress(hBcryptDll, #f)

// Workaround for RP_D() expecting the no-underscore naming convention.
#define AesCAPI_NGPrivate AesCAPI_NG_Private

namespace LibRpBase {

class AesCAPI_NG_Private
{
	public:
		AesCAPI_NG_Private();
		~AesCAPI_NG_Private();

	private:
		RP_DISABLE_COPY(AesCAPI_NG_Private)

	public:
		// Reference counter.
		static volatile int ref_cnt;

		/**
		 * Load bcrypt.dll and associated function pointers.
		 * @return 0 on success; non-zero on error.
		 */
		static int load_bcrypt(void);

		/** bcrypt.dll handle and function pointers. **/
		static HMODULE hBcryptDll;
		static DECL_FUNCPTR(BCryptOpenAlgorithmProvider);
		static DECL_FUNCPTR(BCryptGetProperty);
		static DECL_FUNCPTR(BCryptSetProperty);
		static DECL_FUNCPTR(BCryptCloseAlgorithmProvider);
		static DECL_FUNCPTR(BCryptGenerateSymmetricKey);
		static DECL_FUNCPTR(BCryptDecrypt);
		static DECL_FUNCPTR(BCryptDestroyKey);
		static DECL_FUNCPTR(BCryptEncrypt);

	public:
		// NOTE: While the provider is shared in AesCAPI,
		// it can't be shared here because properties like
		// chaining mode and IV are set on the algorithm
		// handle, not the key.
		BCRYPT_ALG_HANDLE hAesAlg;
		BCRYPT_KEY_HANDLE hKey;

		// Key object.
		uint8_t *pbKeyObject;
		ULONG cbKeyObject;

		// Key data.
		// If the cipher mode is changed, the key
		// has to be reinitialized.
		uint8_t key[32];
		unsigned int key_len;

		// Chaining mode.
		IAesCipher::ChainingMode chainingMode;

		// CBC: Initialization vector.
		// CTR: Counter.
		uint8_t iv[16];
};

/** AesCAPI_NG_Private **/

// Reference counter.
volatile int AesCAPI_NG_Private::ref_cnt = 0;

/** bcrypt.dll handle and function pointers. **/
HMODULE AesCAPI_NG_Private::hBcryptDll = nullptr;
DEF_FUNCPTR(BCryptOpenAlgorithmProvider);
DEF_FUNCPTR(BCryptGetProperty);
DEF_FUNCPTR(BCryptSetProperty);
DEF_FUNCPTR(BCryptCloseAlgorithmProvider);
DEF_FUNCPTR(BCryptGenerateSymmetricKey);
DEF_FUNCPTR(BCryptDecrypt);
DEF_FUNCPTR(BCryptDestroyKey);
DEF_FUNCPTR(BCryptEncrypt);

AesCAPI_NG_Private::AesCAPI_NG_Private()
	: hAesAlg(nullptr)
	, hKey(nullptr)
	, pbKeyObject(nullptr)
	, cbKeyObject(0)
	, key_len(0)
	, chainingMode(IAesCipher::CM_ECB)
{
	// Clear the key and IV.
	memset(key, 0, sizeof(key));
	memset(iv, 0, sizeof(iv));

	// Increment the reference counter.
	assert(ref_cnt >= 0);
	if (ATOMIC_INC_FETCH(&ref_cnt) == 1) {
		// First AesCAPI_NG reference.
		// Attempt to load bcrypt.dll.
		if (load_bcrypt() != 0 || !hBcryptDll) {
			// Error loading bcrypt.dll.
			// NOTE: Not resetting the reference count here.
			// It will be decremented in the destructor.
			return;
		}
	}

	BCRYPT_ALG_HANDLE hAesAlg;
	NTSTATUS status = pBCryptOpenAlgorithmProvider(
		&hAesAlg, BCRYPT_AES_ALGORITHM, nullptr, 0);
	if (NT_SUCCESS(status)) {
		// Default to ECB chaining.
		NTSTATUS status = pBCryptSetProperty(
					hAesAlg, 
					BCRYPT_CHAINING_MODE, 
					(PBYTE)BCRYPT_CHAIN_MODE_ECB,
					sizeof(BCRYPT_CHAIN_MODE_ECB), 0);
		if (NT_SUCCESS(status)) {
			// Save the algorithm.
			this->hAesAlg = hAesAlg;
		} else {
			// Error setting the chaining mode.
			pBCryptCloseAlgorithmProvider(hAesAlg, 0);
		}
	}
}

AesCAPI_NG_Private::~AesCAPI_NG_Private()
{
	if (hBcryptDll) {
		if (hKey) {
			pBCryptDestroyKey(hKey);
		}
		if (hAesAlg) {
			pBCryptCloseAlgorithmProvider(hAesAlg, 0);
		}
	}

	assert(ref_cnt > 0);
	if (ATOMIC_DEC_FETCH(&ref_cnt) == 0) {
		// Unload bcrypt.dll.
		// TODO: Clear the function pointers?
		if (hBcryptDll) {
			FreeLibrary(hBcryptDll);
			hBcryptDll = nullptr;
		}
	}
}

/**
 * Load bcrypt.dll and associated function pointers.
 * @return 0 on success; non-zero on error.
 */
int AesCAPI_NG_Private::load_bcrypt(void)
{
	if (hBcryptDll) {
		// bcrypt.dll has already been loaded.
		return 0;
	}

	// Attempt to load bcrypt.dll.
	hBcryptDll = LoadLibrary(L"bcrypt.dll");
	if (!hBcryptDll) {
		// bcrypt.dll not found.
		return -ENOENT;
	}

	// Load the function pointers.
	LOAD_FUNCPTR(BCryptOpenAlgorithmProvider);
	LOAD_FUNCPTR(BCryptGetProperty);
	LOAD_FUNCPTR(BCryptSetProperty);
	LOAD_FUNCPTR(BCryptCloseAlgorithmProvider);
	LOAD_FUNCPTR(BCryptGenerateSymmetricKey);
	LOAD_FUNCPTR(BCryptDecrypt);
	LOAD_FUNCPTR(BCryptDestroyKey);
	LOAD_FUNCPTR(BCryptEncrypt);

	// Make sure all of the function pointers are valid.
	if (!AesCAPI_NG_Private::pBCryptOpenAlgorithmProvider ||
	    !AesCAPI_NG_Private::pBCryptGetProperty ||
	    !AesCAPI_NG_Private::pBCryptSetProperty ||
	    !AesCAPI_NG_Private::pBCryptCloseAlgorithmProvider ||
	    !AesCAPI_NG_Private::pBCryptGenerateSymmetricKey ||
	    !AesCAPI_NG_Private::pBCryptDecrypt ||
	    !AesCAPI_NG_Private::pBCryptDestroyKey ||
	    !AesCAPI_NG_Private::pBCryptEncrypt)
	{
		// At least one function pointer is missing.
		AesCAPI_NG_Private::pBCryptOpenAlgorithmProvider = nullptr;
		AesCAPI_NG_Private::pBCryptGetProperty = nullptr;
		AesCAPI_NG_Private::pBCryptSetProperty = nullptr;
		AesCAPI_NG_Private::pBCryptCloseAlgorithmProvider = nullptr;
		AesCAPI_NG_Private::pBCryptGenerateSymmetricKey = nullptr;
		AesCAPI_NG_Private::pBCryptDecrypt = nullptr;
		AesCAPI_NG_Private::pBCryptDestroyKey = nullptr;
		AesCAPI_NG_Private::pBCryptEncrypt = nullptr;

		FreeLibrary(hBcryptDll);
		return -ENOENT;
	}

	// bcrypt.dll has been loaded.
	return 0;
}

/** AesCAPI_NG **/

AesCAPI_NG::AesCAPI_NG()
	: d_ptr(new AesCAPI_NG_Private())
{ }

AesCAPI_NG::~AesCAPI_NG()
{
	delete d_ptr;
}

/**
 * Is CryptoAPI NG usable on this system?
 *
 * If CryptoAPI NG is usable, this function will load
 * bcrypt.dll and all required function pointers.
 *
 * @return True if this system supports CryptoAPI NG.
 */
bool AesCAPI_NG::isUsable(void)
{
	if (AesCAPI_NG_Private::hBcryptDll) {
		// bcrypt.dll is loaded.
		return true;
	}

	// NOTE: We can't call load_bcrypt() due to reference counting,
	// so assume it works as long as bcrypt.dll is present and
	// BCryptOpenAlgorithmProvider exists.
	bool bRet = false;
	HMODULE hBcryptDll = LoadLibrary(L"bcrypt.dll");
	if (hBcryptDll) {
		bRet = (GetProcAddress(hBcryptDll, "BCryptOpenAlgorithmProvider") != nullptr);
		FreeLibrary(hBcryptDll);
	}
	return bRet;
}

/**
 * Get the name of the AesCipher implementation.
 * @return Name.
 */
const char *AesCAPI_NG::name(void) const
{
	return "CryptoAPI NG";
}

/**
 * Has the cipher been initialized properly?
 * @return True if initialized; false if not.
 */
bool AesCAPI_NG::isInit(void) const
{
	RP_D(const AesCAPI_NG);
	return (d->hBcryptDll != nullptr &&
		d->hAesAlg != nullptr);
}

/**
 * Set the encryption key.
 * @param key Key data.
 * @param len Key length, in bytes.
 * @return 0 on success; negative POSIX error code on error.
 */
int AesCAPI_NG::setKey(const uint8_t *RESTRICT key, unsigned int len)
{
	// Acceptable key lengths:
	// - 16 (AES-128)
	// - 24 (AES-192)
	// - 32 (AES-256)
	RP_D(AesCAPI_NG);
	if (!key) {
		// No key specified.
		return -EINVAL;
	} else if (!d->hBcryptDll || !d->hAesAlg) {
		// Algorithm is not available.
		return -EBADF;
	}

	if (len != 16 && len != 24 && len != 32) {
		// Invalid key length.
		return -EINVAL;
	}

	// Reference: https://msdn.microsoft.com/en-us/library/windows/desktop/aa376234%28v=vs.85%29.aspx?f=255&MSPPError=-2147217396

	// Calculate the buffer size for the key object.
	ULONG cbKeyObject;
	ULONG cbData;
	NTSTATUS status = d->pBCryptGetProperty(
		d->hAesAlg,
		BCRYPT_OBJECT_LENGTH,
		(PBYTE)&cbKeyObject, sizeof(cbKeyObject),
		&cbData, 0);
	if (!NT_SUCCESS(status) || cbData != sizeof(cbKeyObject)) {
		// Failed to get the key object length.
		// TODO: Better error?
		return -ENOMEM;
	}

	uint8_t *pbKeyObject = static_cast<uint8_t*>(malloc(cbKeyObject));
	if (!pbKeyObject) {
		return -ENOMEM;
	}

	// Generate the key.
	BCRYPT_KEY_HANDLE hKey;
	status = d->pBCryptGenerateSymmetricKey(
		d->hAesAlg,
		&hKey,
		pbKeyObject, cbKeyObject,
		(PBYTE)key, len, 0);
	if (!NT_SUCCESS(status)) {
		// Error generating the key.
		free(pbKeyObject);
		// TODO: Better error?
		return -ENOMEM;
	}

	// Key loaded successfully.
	BCRYPT_KEY_HANDLE hOldKey = d->hKey;
	uint8_t *pbOldKeyObject = d->pbKeyObject;
	d->hKey = hKey;
	d->pbKeyObject = pbKeyObject;
	d->cbKeyObject = cbKeyObject;
	if (hOldKey != nullptr) {
		// Destroy the old key.
		d->pBCryptDestroyKey(hOldKey);
	}
	if (pbOldKeyObject != nullptr) {
		// Delete the old key blob.
		free(pbOldKeyObject);
	}

	// Save the key data.
	if (d->key != key) {
		memcpy(d->key, key, len);
		d->key_len = len;
	}
	return 0;
}

/**
 * Set the cipher chaining mode.
 *
 * Note that the IV/counter must be set *after* setting
 * the chaining mode; otherwise, setIV() will fail.
 *
 * @param mode Cipher chaining mode.
 * @return 0 on success; negative POSIX error code on error.
 */
int AesCAPI_NG::setChainingMode(ChainingMode mode)
{
	RP_D(AesCAPI_NG);
	if (!d->hBcryptDll || !d->hAesAlg) {
		// Algorithm is not available.
		return -EBADF;
	} else if (d->chainingMode == mode) {
		// No change necessary.
		return 0;
	}

	const wchar_t *szMode;
	ULONG cbMode;
	switch (mode) {
		case CM_ECB:
		case CM_CTR:	// implemented using ECB
			szMode = BCRYPT_CHAIN_MODE_ECB;
			cbMode = sizeof(BCRYPT_CHAIN_MODE_ECB);
			break;
		case CM_CBC:
			szMode = BCRYPT_CHAIN_MODE_CBC;
			cbMode = sizeof(BCRYPT_CHAIN_MODE_CBC);
			break;
		default:
			return -EINVAL;
	}

	// Set the cipher chaining mode on the algorithm.
	NTSTATUS status = d->pBCryptSetProperty(
				d->hAesAlg, 
                                BCRYPT_CHAINING_MODE, 
                                (PBYTE)szMode, cbMode, 0);
	if (!NT_SUCCESS(status)) {
		// Error setting the cipher chaining mode.
		// TODO: Specific error?
		return -EIO;
	}

	d->chainingMode = mode;

	// Re-apply the key.
	// Otherwise, the chaining mode won't take effect.
	setKey(d->key, d->key_len);
	return 0;
}

/**
 * Set the IV (CBC mode) or counter (CTR mode).
 * @param iv IV/counter data.
 * @param len IV/counter length, in bytes.
 * @return 0 on success; negative POSIX error code on error.
 */
int AesCAPI_NG::setIV(const uint8_t *RESTRICT iv, unsigned int len)
{
	RP_D(AesCAPI_NG);
	if (!iv || len != 16) {
		return -EINVAL;
	} else if (!d->hBcryptDll || !d->hAesAlg) {
		// Algorithm is not available.
		return -EBADF;
	} else if (d->chainingMode < CM_CBC || d->chainingMode > CM_CTR) {
		// This chaining mode doesn't have an IV or counter.
		return -EINVAL;
	}

	// Verify the block length.
	ULONG cbBlockLen;
	ULONG cbData;
	NTSTATUS status = d->pBCryptGetProperty(
				d->hAesAlg, 
				BCRYPT_BLOCK_LENGTH, 
				(PBYTE)&cbBlockLen, sizeof(DWORD),
				&cbData, 0);
	if (!NT_SUCCESS(status) || cbData != sizeof(cbBlockLen)) {
		// Failed to get the block length.
		// TODO: Better error?
		return -EIO;
	}

	if (cbBlockLen != 16) {
		// Block length is incorrect...
		return -EIO;
	}

	// Set the IV.
	assert(sizeof(d->iv) == 16);
	memcpy(d->iv, iv, sizeof(d->iv));
	return 0;
}

/**
 * Decrypt a block of data.
 * @param data Data block.
 * @param data_len Length of data block.
 * @return Number of bytes decrypted on success; 0 on error.
 */
unsigned int AesCAPI_NG::decrypt(uint8_t *RESTRICT data, unsigned int data_len)
{
	RP_D(AesCAPI_NG);
	if (!d->hBcryptDll || !d->hAesAlg || !d->hKey) {
		// Algorithm is not available,
		// or the key hasn't been set.
		return 0;
	}

	// Get the block length.
	ULONG cbBlockLen;
	ULONG cbData;
	NTSTATUS status = d->pBCryptGetProperty(
				d->hAesAlg, 
				BCRYPT_BLOCK_LENGTH, 
				(PBYTE)&cbBlockLen, sizeof(DWORD),
				&cbData, 0);
	if (!NT_SUCCESS(status) || cbData != sizeof(cbBlockLen)) {
		// Failed to get the block length.
		return 0;
	}

	if (cbBlockLen != 16) {
		// Block length is incorrect...
		return 0;
	}

	// data_len must be a multiple of the block length.
	assert(data_len % cbBlockLen == 0);
	if (data_len % cbBlockLen != 0) {
		// Invalid data length.
		return 0;
	}

	ULONG cbResult;
	switch (d->chainingMode) {
		case CM_ECB:
			status = d->pBCryptDecrypt(d->hKey,
						data, data_len,
						nullptr,
						nullptr, 0,
						data, data_len,
						&cbResult, 0);
			break;

		case CM_CBC:
			status = d->pBCryptDecrypt(d->hKey,
						data, data_len,
						nullptr,
						d->iv, sizeof(d->iv),
						data, data_len,
						&cbResult, 0);
			break;

		case CM_CTR: {
			// CTR isn't supported by CryptoAPI-NG directly.
			// Need to decrypt each block manually.

			// Using a union to allow 64-bit XORs
			// for improved performance.
			// NOTE: Can't use u128_t, since that's in libromdata. (N3DSVerifyKeys.hpp)
			union ctr_block {
				uint8_t u8[16];
				uint64_t u64[2];
			};

			// TODO: Verify data alignment.
			ctr_block ctr_crypt;
			ctr_block *ctr_data = reinterpret_cast<ctr_block*>(data);
			ULONG cbTmpResult;
			cbResult = 0;
			for (; data_len > 0; data_len -= 16, ctr_data++) {
				// Encrypt the current counter.
				memcpy(ctr_crypt.u8, d->iv, sizeof(ctr_crypt.u8));
				status = d->pBCryptEncrypt(d->hKey,
						ctr_crypt.u8, sizeof(ctr_crypt.u8),
						nullptr,
						nullptr, 0,
						ctr_crypt.u8, sizeof(ctr_crypt.u8),
						&cbTmpResult, 0);
				if (!NT_SUCCESS(status)) {
					// Encryption failed.
					return 0;
				}
				cbResult += cbTmpResult;

				// XOR with the ciphertext.
				ctr_data->u64[0] ^= ctr_crypt.u64[0];
				ctr_data->u64[1] ^= ctr_crypt.u64[1];

				// Increment the counter.
				for (int i = 15; i >= 0; i--) {
					if (++d->iv[i] != 0) {
						// No carry needed.
						break;
					}
				}
			}
			break;
		}

		default:
			// Invalid chaining mode.
			return 0;
	}
	
	return (NT_SUCCESS(status) ? cbResult : 0);
}

/**
 * Decrypt a block of data using the specified IV (CBC mode) or counter (CTR mode).
 * @param data Data block.
 * @param data_len Length of data block.
 * @param iv IV/counter for the data block.
 * @param iv_len Length of the IV/counter.
 * @return Number of bytes decrypted on success; 0 on error.
 */
unsigned int AesCAPI_NG::decrypt(uint8_t *RESTRICT data, unsigned int data_len,
	const uint8_t *RESTRICT iv, unsigned int iv_len)
{
	RP_D(AesCAPI_NG);
	if (!d->hBcryptDll || !d->hAesAlg || !d->hKey) {
		// Algorithm is not available,
		// or the key hasn't been set.
		return 0;
	} else if (!iv || iv_len != 16) {
		// Invalid IV.
		return 0;
	}

	// Set the IV.
	assert(sizeof(d->iv) == 16);
	memcpy(d->iv, iv, sizeof(d->iv));

	// Use the regular decrypt() function.
	return decrypt(data, data_len);
}

}
