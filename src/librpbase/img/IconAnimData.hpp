/***************************************************************************
 * ROM Properties Page shell extension. (librpbase)                        *
 * IconAnimData.hpp: Icon animation data.                                  *
 *                                                                         *
 * Copyright (c) 2016 by David Korth.                                      *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __ROMPROPERTIES_LIBRPBASE_IMG_ICONANIMDATA_HPP__
#define __ROMPROPERTIES_LIBRPBASE_IMG_ICONANIMDATA_HPP__

#include "common.h"
#include "RefBase.hpp"

// C includes.
#include <stdint.h>

// C includes. (C++ namespace)
#include <cstring>

// C++ includes.
#include <algorithm>
#include <array>
#include <cstdio>

// librptexture
#include "librptexture/img/rp_image.hpp"

namespace LibRpBase {

struct IconAnimData : public RefBase
{
	static const int MAX_FRAMES = 64;
	static const int MAX_SEQUENCE = 64;

	int count;	// Frame count.
	int seq_count;	// Sequence count.

	// Array of icon sequence indexes.
	// Each entry indicates which frame to use.
	// Check the seq_count field to determine
	// how many indexes are actually here.
	std::array<uint8_t, MAX_SEQUENCE> seq_index;

	struct delay_t {
		uint16_t numer;	// Numerator.
		uint16_t denom;	// Denominator.
		// TODO: Keep precalculated milliseconds here?
		int ms;		// Precalculated milliseconds.
	};

	// Array of icon delays.
	// NOTE: These are associated with sequence indexes,
	// not the individual icon frames.
	std::array<delay_t, MAX_SEQUENCE> delays;

	// Array of icon frames.
	// Check the count field to determine
	// how many frames are actually here.
	// NOTE: Frames may be nullptr, in which case
	// the previous frame should be used.
	// NOTE 2: Frames stored here must be ref()'d.
	// They will be automatically unref()'d in the destructor.
	std::array<LibRpTexture::rp_image*, MAX_FRAMES> frames;

	IconAnimData()
		: count(0)
		, seq_count(0)
	{
		seq_index.fill(0);
		frames.fill(0);

		// MSVC 2010 doesn't support initializer lists,
		// so create a dummy struct.
		static const delay_t zero_delay = {0, 0, 0};
		delays.fill(zero_delay);
	}

protected:
	~IconAnimData()	// call unref() instead
	{
		std::for_each(frames.begin(), frames.end(),
			[](LibRpTexture::rp_image *img) {
				UNREF(img);
			}
		);
	}

private:
	RP_DISABLE_COPY(IconAnimData);

public:
	inline IconAnimData *ref(void)
	{
		return RefBase::ref<IconAnimData>();
	}

	/**
	 * Special case unref() function to allow
	 * const IconAnimData* to be ref'd.
	 */
	inline const IconAnimData *ref(void) const
	{
		return const_cast<IconAnimData*>(this)->RefBase::ref<IconAnimData>();
	}

	/**
	 * Special case unref() function to allow
	 * const IconAnimData* to be unref'd.
	 */
	inline void unref(void) const
	{
		const_cast<IconAnimData*>(this)->RefBase::unref();
	}
};

}

#endif /* __ROMPROPERTIES_LIBRPBASE_IMG_ICONANIMDATA_HPP__ */
