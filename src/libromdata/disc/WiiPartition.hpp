/***************************************************************************
 * ROM Properties Page shell extension. (libromdata)                       *
 * WiiPartition.hpp: Wii partition reader.                                 *
 *                                                                         *
 * Copyright (c) 2016-2020 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __ROMPROPERTIES_LIBROMDATA_DISC_WIIPARTITION_HPP__
#define __ROMPROPERTIES_LIBROMDATA_DISC_WIIPARTITION_HPP__

#include "librpbase/config.librpbase.h"
#include "GcnPartition.hpp"
#include "../Console/wii_structs.h"

// librpbase
#include "librpbase/crypto/KeyManager.hpp"

namespace LibRomData {

class WiiPartitionPrivate;
class WiiPartition : public GcnPartition
{
	public:
		// Bitfield enum indicating the encryption type.
		enum CryptoMethod {
			CM_ENCRYPTED = 0,	// Data is encrypted.
			CM_UNENCRYPTED = 1,	// Data is not encrypted.
			CM_MASK_ENCRYPTED = 1,

			CM_1K_31K = 0,		// 1k hashes, 31k data
			CM_32K = 2,		// 32k data
			CM_MASK_SECTOR = 2,

			CM_STANDARD = (CM_ENCRYPTED | CM_1K_31K),	// Standard encrypted Wii disc
			CM_RVTH = (CM_UNENCRYPTED | CM_32K),		// Unencrypted RVT-H disc image
			CM_NASOS = (CM_UNENCRYPTED | CM_1K_31K),	// NASOS compressed retail disc image
		};

		/**
		 * Construct a WiiPartition with the specified IDiscReader.
		 *
		 * NOTE: The IDiscReader *must* remain valid while this
		 * WiiPartition is open.
		 *
		 * @param discReader		[in] IDiscReader.
		 * @param partition_offset	[in] Partition start offset.
		 * @param partition_size	[in] Calculated partition size. Used if the size in the header is 0.
		 * @param cryptoMethod		[in] Crypto method.
		 */
		WiiPartition(IDiscReader *discReader, off64_t partition_offset,
			off64_t partition_size, CryptoMethod crypto = CM_STANDARD);
		~WiiPartition();

	private:
		typedef GcnPartition super;
		RP_DISABLE_COPY(WiiPartition)

	protected:
		friend class WiiPartitionPrivate;
		// d_ptr is used from the subclass.
		//WiiPartitionPrivate *const d_ptr;

	public:
		/** IDiscReader **/

		/**
		 * Read data from the partition.
		 * @param ptr Output data buffer.
		 * @param size Amount of data to read, in bytes.
		 * @return Number of bytes read.
		 */
		ATTR_ACCESS_SIZE(write_only, 2, 3)
		size_t read(void *ptr, size_t size) final;

		/**
		 * Set the partition position.
		 * @param pos Partition position.
		 * @return 0 on success; -1 on error.
		 */
		int seek(off64_t pos) final;

		/**
		 * Get the partition position.
		 * @return Partition position on success; -1 on error.
		 */
		off64_t tell(void) final;

	public:
		/**
		 * Get the used partition size.
		 * This size includes the partition header and hashes,
		 * but does not include "empty" sectors.
		 * @return Used partition size, or -1 on error.
		 */
		off64_t partition_size_used(void) const final;

	public:
		/** WiiPartition **/

		/**
		 * Encryption key verification result.
		 * @return Encryption key verification result.
		 */
		LibRpBase::KeyManager::VerifyResult verifyResult(void) const;

		// Encryption key in use.
		// TODO: Merge with EncryptionKeys.
		enum class EncKey {
			Unknown = -1,

			RVL_Common = 0,		// Common key
			RVL_Korean = 1,		// Korean key
			WUP_vWii = 2,		// vWii key

			RVT_Debug = 3,		// Common key (debug)
			RVT_Korean = 4,		// Korean key (debug)
			CAT_vWii = 5,		// vWii key (debug)

			None = 6,		// No encryption (RVT-H)

			Max
		};

		/**
		 * Get the encryption key in use.
		 * @return Encryption key in use.
		 */
		EncKey encKey(void) const;

		/**
		 * Get the encryption key that would be in use if the partition was encrypted.
		 * This is only needed for NASOS images.
		 * @return "Real" encryption key in use.
		 */
		EncKey encKeyReal(void) const;

		/**
		 * Get the ticket.
		 * @return Ticket, or nullptr if unavailable.
		 */
		const RVL_Ticket *ticket(void) const;

		/**
		 * Get the TMD header.
		 * @return TMD header, or nullptr if unavailable.
		 */
		const RVL_TMD_Header *tmdHeader(void) const;

		/**
		 * Get the title ID. (NOT BYTESWAPPED)
		 * @return Title ID. (0-0 if unavailable)
		 */
		Nintendo_TitleID_BE_t titleID(void) const;

	public:
		// Encryption key indexes.
		enum EncryptionKeys {
			// Retail
			Key_Rvl_Common,
			Key_Rvl_Korean,
			Key_Wup_Starbuck_vWii_Common,

			// Debug
			Key_Rvt_Debug,
			Key_Rvt_Korean,
			Key_Cat_Starbuck_vWii_Common,

			// SD card (TODO: Retail vs. Debug?)
			Key_Rvl_SD_AES,
			Key_Rvl_SD_IV,
			Key_Rvl_SD_MD5,

			Key_Max
		};

#ifdef ENABLE_DECRYPTION
	public:
		/**
		 * Get the total number of encryption key names.
		 * @return Number of encryption key names.
		 */
		static int encryptionKeyCount_static(void);

		/**
		 * Get an encryption key name.
		 * @param keyIdx Encryption key index.
		 * @return Encryption key name (in ASCII), or nullptr on error.
		 */
		static const char *encryptionKeyName_static(int keyIdx);

		/**
		 * Get the verification data for a given encryption key index.
		 * @param keyIdx Encryption key index.
		 * @return Verification data. (16 bytes)
		 */
		static const uint8_t *encryptionVerifyData_static(int keyIdx);
#endif /* ENABLE_DECRYPTION */
};

}

#endif /* __ROMPROPERTIES_LIBROMDATA_DISC_WIIPARTITION_HPP__ */
