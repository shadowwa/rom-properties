/***************************************************************************
 * ROM Properties Page shell extension. (librptexture)                     *
 * rp_image_p.hpp: Image class. (Private class)                            *
 *                                                                         *
 * Copyright (c) 2016-2019 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __ROMPROPERTIES_LIBRPTEXTURE_IMG_RP_IMAGE_P_HPP__
#define __ROMPROPERTIES_LIBRPTEXTURE_IMG_RP_IMAGE_P_HPP__

#include "rp_image.hpp"

namespace LibRpTexture {

class rp_image_backend;
class rp_image_private
{
	public:
		/**
		 * Create an rp_image_private.
		 *
		 * If an rp_image_backend has been registered, that backend
		 * will be used; otherwise, the defaul tbackend will be used.
		 *
		 * @param width Image width.
		 * @param height Image height.
		 * @param format Image format.
		 */
		rp_image_private(int width, int height, rp_image::Format format);

		/**
		 * Create an rp_image_private using the specified rp_image_backend.
		 *
		 * NOTE: This rp_image will take ownership of the rp_image_backend.
		 *
		 * @param backend rp_image_backend.
		 */
		explicit rp_image_private(rp_image_backend *backend);

		~rp_image_private();

	private:
		rp_image_private(const rp_image_private &other);
		rp_image_private &operator=(const rp_image_private &other);

	public:
		static rp_image::rp_image_backend_creator_fn backend_fn;

	public:
		// Image backend.
		rp_image_backend *backend;

		// Metadata.
		bool has_sBIT;
		rp_image::sBIT_t sBIT;
};

}

#endif /* __ROMPROPERTIES_LIBRPTEXTURE_IMG_RP_IMAGE_P_HPP__ */
