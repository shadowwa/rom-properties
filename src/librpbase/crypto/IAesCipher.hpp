/***************************************************************************
 * ROM Properties Page shell extension. (librpbase)                        *
 * IAesCipher.hpp: AES decryption interface.                               *
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

#ifndef __ROMPROPERTIES_LIBRPBASE_CRYPTO_IAESCIPHER_HPP__
#define __ROMPROPERTIES_LIBRPBASE_CRYPTO_IAESCIPHER_HPP__

#include "librpbase/config.librpbase.h"
#include "librpbase/common.h"

// C includes.
#include <stdint.h>

namespace LibRpBase {

class IAesCipher
{
	protected:
		IAesCipher() { }
	public:
		virtual ~IAesCipher() = 0;

	private:
		RP_DISABLE_COPY(IAesCipher)

	public:
		/**
		 * Get the name of the AesCipher implementation.
		 * @return Name.
		 */
		virtual const char *name(void) const = 0;

		/**
		 * Has the cipher been initialized properly?
		 * @return True if initialized; false if not.
		 */
		virtual bool isInit(void) const = 0;

		/**
		 * Set the encryption key.
		 * @param key Key data.
		 * @param len Key length, in bytes.
		 * @return 0 on success; negative POSIX error code on error.
		 */
		virtual int setKey(const uint8_t *RESTRICT key, unsigned int len) = 0;

		enum ChainingMode {
			CM_ECB,
			CM_CBC,
			CM_CTR,
		};

		/**
		 * Set the cipher chaining mode.
		 *
		 * Note that the IV/counter must be set *after* setting
		 * the chaining mode; otherwise, setIV() will fail.
		 *
		 * @param mode Cipher chaining mode.
		 * @return 0 on success; negative POSIX error code on error.
		 */
		virtual int setChainingMode(ChainingMode mode) = 0;

		/**
		 * Set the IV (CBC mode) or counter (CTR mode).
		 * @param iv IV/counter data.
		 * @param len IV/counter length, in bytes.
		 * @return 0 on success; negative POSIX error code on error.
		 */
		virtual int setIV(const uint8_t *RESTRICT iv, unsigned int len) = 0;

		/**
		 * Decrypt a block of data.
		 * @param data Data block.
		 * @param data_len Length of data block.
		 * @return Number of bytes decrypted on success; 0 on error.
		 */
		virtual unsigned int decrypt(uint8_t *RESTRICT data, unsigned int data_len) = 0;

		/**
		 * Decrypt a block of data using the specified IV (CBC mode) or counter (CTR mode).
		 * @param data Data block.
		 * @param data_len Length of data block.
		 * @param iv IV/counter for the data block.
		 * @param iv_len Length of the IV/counter.
		 * @return Number of bytes decrypted on success; 0 on error.
		 */
		virtual unsigned int decrypt(uint8_t *RESTRICT data, unsigned int data_len,
			const uint8_t *RESTRICT iv, unsigned int iv_len) = 0;
};

/**
 * Both gcc and MSVC fail to compile unless we provide
 * an empty implementation, even though the function is
 * declared as pure-virtual.
 */
inline IAesCipher::~IAesCipher() { }

}

#endif /* __ROMPROPERTIES_LIBRPBASE_CRYPTO_IAESCIPHER_HPP__ */
