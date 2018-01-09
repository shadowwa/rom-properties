/***************************************************************************
 * ROM Properties Page shell extension. (libromdata)                       *
 * CtrKeyScrambler.cpp: Nintendo 3DS key scrambler.                        *
 *                                                                         *
 * Copyright (c) 2016-2017 by David Korth.                                 *
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

#include "librpbase/config.librpbase.h"
#ifndef ENABLE_DECRYPTION
#error This file should only be compiled if decryption is enabled.
#endif /* ENABLE_DECRYPTION */

#include "CtrKeyScrambler.hpp"

// librpbase
#include "librpbase/byteswap.h"
#include "librpbase/crypto/KeyManager.hpp"
using LibRpBase::KeyManager;

// C includes. (C++ namespace)
#include <cassert>
#include <cerrno>

namespace LibRomData {

class CtrKeyScramblerPrivate
{
	private:
		CtrKeyScramblerPrivate();
		~CtrKeyScramblerPrivate();
		RP_DISABLE_COPY(CtrKeyScramblerPrivate);

	public:
		// Verification key names.
		static const char *const EncryptionKeyNames[CtrKeyScrambler::Key_Max];

		// Verification key data.
		static const uint8_t EncryptionKeyVerifyData[CtrKeyScrambler::Key_Max][16];
};

// Verification key names.
const char *const CtrKeyScramblerPrivate::EncryptionKeyNames[CtrKeyScrambler::Key_Max] = {
	"twl-scrambler",
	"ctr-scrambler",
};

const uint8_t CtrKeyScramblerPrivate::EncryptionKeyVerifyData[CtrKeyScrambler::Key_Max][16] = {
	// twl-scrambler
	{0x65,0xCF,0x82,0xC5,0xDB,0x79,0x93,0x8C,
	 0x01,0x33,0x65,0x87,0x72,0xDF,0x60,0x94},
	// ctr-scrambler
	{0xEF,0x4F,0x47,0x3C,0x04,0xAD,0xAA,0xAE,
	 0x66,0x98,0x29,0xCB,0xC2,0x4D,0x9D,0xB0},
};

/**
 * Get the total number of encryption key names.
 * @return Number of encryption key names.
 */
int CtrKeyScrambler::encryptionKeyCount_static(void)
{
	return Key_Max;
}

/**
 * Get an encryption key name.
 * @param keyIdx Encryption key index.
 * @return Encryption key name (in ASCII), or nullptr on error.
 */
const char *CtrKeyScrambler::encryptionKeyName_static(int keyIdx)
{
	assert(keyIdx >= 0);
	assert(keyIdx < Key_Max);
	if (keyIdx < 0 || keyIdx >= Key_Max)
		return nullptr;
	return CtrKeyScramblerPrivate::EncryptionKeyNames[keyIdx];
}

/**
 * Get the verification data for a given encryption key index.
 * @param keyIdx Encryption key index.
 * @return Verification data. (16 bytes)
 */
const uint8_t *CtrKeyScrambler::encryptionVerifyData_static(int keyIdx)
{
	assert(keyIdx >= 0);
	assert(keyIdx < Key_Max);
	if (keyIdx < 0 || keyIdx >= Key_Max)
		return nullptr;
	return CtrKeyScramblerPrivate::EncryptionKeyVerifyData[keyIdx];
}

/**
 * Byteswap a 128-bit key for use with 32/64-bit addition.
 * @param dest Destination.
 * @param src Source.
 */
static inline void bswap_u128_t(u128_t &dest, const u128_t &src)
{
	dest.u64[0] = __swab64(src.u64[0]);
	dest.u64[1] = __swab64(src.u64[1]);
}

/**
 * CTR key scrambler. (for keyslots 0x04-0x3F)
 * @param keyNormal	[out] Normal key.
 * @param keyX		[in] KeyX.
 * @param keyY		[in] KeyY.
 * @param ctr_scrambler	[in] Scrambler constant.
 * @return 0 on success; negative POSIX error code on error.
 */
int CtrKeyScrambler::CtrScramble(u128_t *keyNormal,
	const u128_t *keyX, const u128_t *keyY,
	const u128_t *ctr_scrambler)
{
	// CTR key scrambler: KeyNormal = (((KeyX <<< 2) ^ KeyY) + constant) <<< 87
	// NOTE: Since C doesn't have 128-bit types, we'll operate on
	// 64-bit types. This requires some byteswapping, since the
	// key is handled as if it's big-endian.

	assert(keyNormal != nullptr);
	assert(keyX != nullptr);
	assert(keyY != nullptr);
	assert(ctr_scrambler != nullptr);
	if (!keyNormal || !keyX || !keyY || !ctr_scrambler) {
		// Invalid parameters.
		return -EINVAL;
	}

#if SYS_BYTEORDER == SYS_LIL_ENDIAN
	u128_t keyXtmp, ctr_scrambler_tmp;
	bswap_u128_t(keyXtmp, *keyX);
	bswap_u128_t(ctr_scrambler_tmp, *ctr_scrambler);
#else /* SYS_BYTEORDER == SYS_BIG_ENDIAN */
	const u128_t &keyXtmp = *keyX;
	const u128_t &ctr_scrambler_tmp = *ctr_scrambler;
#endif

	// Rotate KeyX left by two.
	u128_t keyTmp;
	keyTmp.u64[0] = (keyXtmp.u64[0] << 2) | (keyXtmp.u64[1] >> 62);
	keyTmp.u64[1] = (keyXtmp.u64[1] << 2) | (keyXtmp.u64[0] >> 62);

	// XOR by KeyY.
	keyTmp.u64[0] ^= be64_to_cpu(keyY->u64[0]);
	keyTmp.u64[1] ^= be64_to_cpu(keyY->u64[1]);

	// Add the constant.
	// Reference for carry functionality: https://accu.org/index.php/articles/1849
	keyTmp.u64[1] += ctr_scrambler_tmp.u64[1];
	keyTmp.u64[0] += ctr_scrambler_tmp.u64[0] + (keyTmp.u64[1] < ctr_scrambler_tmp.u64[1]);

	// Rotate left by 87.
	// This is effectively "rotate left by 23" with adjusted DWORD indexes.
	keyNormal->u64[1] = cpu_to_be64((keyTmp.u64[0] << 23) | (keyTmp.u64[1] >> 41));
	keyNormal->u64[0] = cpu_to_be64((keyTmp.u64[1] << 23) | (keyTmp.u64[0] >> 41));

	// We're done here.
	return 0;
}

/**
 * CTR key scrambler. (for keyslots 0x04-0x3F)
 *
 * "ctr-scrambler" is retrieved from KeyManager and is
 * used as the scrambler constant.
 *
 * @param keyNormal	[out] Normal key.
 * @param keyX		[in] KeyX.
 * @param keyY		[in] KeyY.
 * @return 0 on success; negative POSIX error code on error.
 */
int CtrKeyScrambler::CtrScramble(u128_t *keyNormal,
	const u128_t *keyX, const u128_t *keyY)
{
	assert(keyNormal != nullptr);
	assert(keyX != nullptr);
	assert(keyY != nullptr);
	if (!keyNormal || !keyX || !keyY) {
		// Invalid parameters.
		return -EINVAL;
	}

	// Load the key scrambler constant.
	KeyManager *const keyManager = KeyManager::instance();
	if (!keyManager) {
		// Unable to initialize the KeyManager.
		return -EIO;
	}

	KeyManager::KeyData_t keyData;
	KeyManager::VerifyResult res = keyManager->getAndVerify(
		CtrKeyScramblerPrivate::EncryptionKeyNames[Key_Ctr_Scrambler], &keyData,
		CtrKeyScramblerPrivate::EncryptionKeyVerifyData[Key_Ctr_Scrambler], 16);
	if (res != KeyManager::VERIFY_OK) {
		// Key error.
		// TODO: Return the key error?
		return -ENOENT;
	}

	if (!keyData.key || keyData.length != 16) {
		// Key is not valid.
		return -EIO;	// TODO: Better error code?
	}

	return CtrScramble(keyNormal, keyX, keyY,
		reinterpret_cast<const u128_t*>(keyData.key));
}

}
