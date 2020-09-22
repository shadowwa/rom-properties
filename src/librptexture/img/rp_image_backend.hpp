/***************************************************************************
 * ROM Properties Page shell extension. (librptexture)                     *
 * rp_image_backend.hpp: Image backend and storage classes.                *
 *                                                                         *
 * Copyright (c) 2016-2019 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __ROMPROPERTIES_LIBRPTEXTURE_IMG_RP_IMAGE_BACKEND_HPP__
#define __ROMPROPERTIES_LIBRPTEXTURE_IMG_RP_IMAGE_BACKEND_HPP__

#include "rp_image.hpp"

namespace LibRpTexture {

/**
 * rp_image data storage class.
 * This can be overridden for e.g. QImage or GDI+.
 */
class rp_image_backend
{
	public:
		rp_image_backend(int width, int height, rp_image::Format format);
		virtual ~rp_image_backend();

	private:
		RP_DISABLE_COPY(rp_image_backend)
	public:
		bool isValid(void) const;

	protected:
		/**
		 * Clear the width, height, stride, and format properties.
		 * Used in error paths.
		 * */
		void clear_properties(void);

		/**
		 * Check if the palette contains alpha values other than 0 and 255.
		 * @return True if an alpha value other than 0 and 255 was found; false if not, or if ARGB32.
		 */
		bool has_translucent_palette_entries(void) const;

	public:
		/**
		 * Shrink image dimensions.
		 * @param width New width.
		 * @param height New height.
		 * @return 0 on success; negative POSIX error code on error.
		 */
		virtual int shrink(int width, int height) = 0;

	public:
		int width;
		int height;
		int stride;
		rp_image::Format format;

		// Image data.
		virtual void *data(void) = 0;
		virtual const void *data(void) const = 0;
		virtual size_t data_len(void) const = 0;

		// Image palette.
		virtual uint32_t *palette(void) = 0;
		virtual const uint32_t *palette(void) const = 0;
		virtual int palette_len(void) const = 0;
		int tr_idx;

	public:
		// Subclasses can have other stuff here.
		// Use dynamic_cast<> to access it.
};

}

#endif /* __ROMPROPERTIES_LIBRPTEXTURE_IMG_RP_IMAGE_BACKEND_HPP__ */
