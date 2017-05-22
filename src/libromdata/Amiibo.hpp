/***************************************************************************
 * ROM Properties Page shell extension. (libromdata)                       *
 * Amiibo.hpp: Nintendo amiibo NFC dump reader.                            *
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

#ifndef __ROMPROPERTIES_LIBROMDATA_AMIIBO_HPP__
#define __ROMPROPERTIES_LIBROMDATA_AMIIBO_HPP__

#include <stdint.h>
#include <string>
#include "librpbase/TextFuncs.hpp"

#include "librpbase/RomData.hpp"

namespace LibRomData {

class AmiiboPrivate;
class Amiibo : public LibRpBase::RomData
{
	public:
		/**
		 * Read a Nintendo amiibo NFC dump.
		 *
		 * An NFC dump must be opened by the caller. The file handle
		 * will be dup()'d and must be kept open in order to load
		 * data from the NFC dump.
		 *
		 * To close the file, either delete this object or call close().
		 *
		 * NOTE: Check isValid() to determine if this is a valid ROM.
		 *
		 * @param file Open NFC dump.
		 */
		explicit Amiibo(LibRpBase::IRpFile *file);

	protected:
		/**
		 * RomData destructor is protected.
		 * Use unref() instead.
		 */
		virtual ~Amiibo() { }

	private:
		typedef RomData super;
		friend class AmiiboPrivate;
		RP_DISABLE_COPY(Amiibo)

	public:
		/** ROM detection functions. **/

		/**
		 * Is a ROM image supported by this class?
		 * @param info DetectInfo containing ROM detection information.
		 * @return Class-specific system ID (>= 0) if supported; -1 if not.
		 */
		static int isRomSupported_static(const DetectInfo *info);

		/**
		 * Is a ROM image supported by this object?
		 * @param info DetectInfo containing ROM detection information.
		 * @return Class-specific system ID (>= 0) if supported; -1 if not.
		 */
		virtual int isRomSupported(const DetectInfo *info) const override final;

		/**
		 * Get the name of the system the loaded ROM is designed for.
		 * @param type System name type. (See the SystemName enum.)
		 * @return System name, or nullptr if type is invalid.
		 */
		virtual const rp_char *systemName(uint32_t type) const override final;

	public:
		/**
		 * Get a list of all supported file extensions.
		 * This is to be used for file type registration;
		 * subclasses don't explicitly check the extension.
		 *
		 * NOTE: The extensions include the leading dot,
		 * e.g. ".bin" instead of "bin".
		 *
		 * NOTE 2: The array and the strings in the array should
		 * *not* be freed by the caller.
		 *
		 * @return NULL-terminated array of all supported file extensions, or nullptr on error.
		 */
		static const rp_char *const *supportedFileExtensions_static(void);

		/**
		 * Get a list of all supported file extensions.
		 * This is to be used for file type registration;
		 * subclasses don't explicitly check the extension.
		 *
		 * NOTE: The extensions include the leading dot,
		 * e.g. ".bin" instead of "bin".
		 *
		 * NOTE 2: The array and the strings in the array should
		 * *not* be freed by the caller.
		 *
		 * @return NULL-terminated array of all supported file extensions, or nullptr on error.
		 */
		virtual const rp_char *const *supportedFileExtensions(void) const override final;

		/**
		 * Get a bitfield of image types this class can retrieve.
		 * @return Bitfield of supported image types. (ImageTypesBF)
		 */
		static uint32_t supportedImageTypes_static(void);

		/**
		 * Get a bitfield of image types this class can retrieve.
		 * @return Bitfield of supported image types. (ImageTypesBF)
		 */
		virtual uint32_t supportedImageTypes(void) const override final;

		/**
		 * Get a list of all available image sizes for the specified image type.
		 *
		 * The first item in the returned vector is the "default" size.
		 * If the width/height is 0, then an image exists, but the size is unknown.
		 *
		 * @param imageType Image type.
		 * @return Vector of available image sizes, or empty vector if no images are available.
		 */
		static std::vector<RomData::ImageSizeDef> supportedImageSizes_static(ImageType imageType);

		/**
		 * Get a list of all available image sizes for the specified image type.
		 *
		 * The first item in the returned vector is the "default" size.
		 * If the width/height is 0, then an image exists, but the size is unknown.
		 *
		 * @param imageType Image type.
		 * @return Vector of available image sizes, or empty vector if no images are available.
		 */
		virtual std::vector<RomData::ImageSizeDef> supportedImageSizes(ImageType imageType) const override final;

		/**
		 * Get image processing flags.
		 *
		 * These specify post-processing operations for images,
		 * e.g. applying transparency masks.
		 *
		 * @param imageType Image type.
		 * @return Bitfield of ImageProcessingBF operations to perform.
		 */
		virtual uint32_t imgpf(ImageType imageType) const override final;

	protected:
		/**
		 * Load field data.
		 * Called by RomData::fields() if the field data hasn't been loaded yet.
		 * @return 0 on success; negative POSIX error code on error.
		 */
		virtual int loadFieldData(void) override final;

	public:
		/**
		 * Get a list of URLs for an external image type.
		 *
		 * A thumbnail size may be requested from the shell.
		 * If the subclass supports multiple sizes, it should
		 * try to get the size that most closely matches the
		 * requested size.
		 *
		 * @param imageType	[in]     Image type.
		 * @param pExtURLs	[out]    Output vector.
		 * @param size		[in,opt] Requested image size. This may be a requested
		 *                               thumbnail size in pixels, or an ImageSizeType
		 *                               enum value.
		 * @return 0 on success; negative POSIX error code on error.
		 */
		virtual int extURLs(ImageType imageType, std::vector<ExtURL> *pExtURLs, int size = IMAGE_SIZE_DEFAULT) const override final;
};

}

#endif /* __ROMPROPERTIES_LIBROMDATA_AMIIBO_HPP__ */
