/***************************************************************************
 * ROM Properties Page shell extension. (librpbase)                        *
 * IconAnimHelper.hpp: Icon animation helper.                              *
 *                                                                         *
 * Copyright (c) 2016-2019 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __ROMPROPERTIES_LIBRPBASE_IMG_ICONANIMHELPER_HPP__
#define __ROMPROPERTIES_LIBRPBASE_IMG_ICONANIMHELPER_HPP__

#include "common.h"
#include "IconAnimData.hpp"

namespace LibRpBase {

class IconAnimHelper
{
	public:
		IconAnimHelper()
			: m_iconAnimData(nullptr)
			, m_seq_idx(0)
			, m_frame(0)
			, m_delay(0)
			, m_last_valid_frame(0)
		{ }

		explicit IconAnimHelper(const IconAnimData *iconAnimData)
			: m_iconAnimData(iconAnimData)
			, m_seq_idx(0)
			, m_frame(0)
			, m_delay(0)
			, m_last_valid_frame(0)
		{
			reset();
		}

		~IconAnimHelper()
		{
			UNREF(m_iconAnimData);
		}

	private:
		RP_DISABLE_COPY(IconAnimHelper);

	public:
		/**
		 * Set the IconAnimData.
		 * The iconAnimData() will be ref()'d.
		 * @param iconAnimData New iconAnimData.
		 */
		void setIconAnimData(const IconAnimData *iconAnimData)
		{
			UNREF(m_iconAnimData);
			m_iconAnimData = iconAnimData->ref();
			reset();
		}

		/**
		 * Get the IconAnimData.
		 * The caller must take a ref() to the IconAnimData.
		 * @return IconAnimData.
		 */
		const IconAnimData *iconAnimData(void) const
		{
			return m_iconAnimData;
		}

		/**
		 * Is this an animated icon?
		 *
		 * This checks if iconAnimData is set and has an animation
		 * sequence that refers to more than one frame.
		 *
		 * @return True if this is an animated icon; false if not.
		 */
		bool isAnimated(void) const
		{
			// TODO: Verify that the sequence references more than one frame?
			return (m_iconAnimData &&
				m_iconAnimData->count > 0 &&
				m_iconAnimData->seq_count > 0);
		}

		/**
		 * Get the current frame number.
		 *
		 * Note that this is actually the last valid frame
		 * that had a valid image.
		 *
		 * @return Frame number.
		 */
		int frameNumber(void) const
		{
			return m_last_valid_frame;
		}

		/**
		 * Get the current frame's delay.
		 * @return Current frame's delay, in milliseconds.
		 */
		int frameDelay(void) const
		{
			return m_delay;
		}

		/**
		 * Reset the animation.
		 */
		void reset(void);

		/**
		 * Advance the animation by one frame.
		 * @param pDelay	[out] Pointer to int to store the frame delay, in milliseconds.
		 * @return Next frame number. (Returns 0 if there's no animation.)
		 */
		int nextFrame(int *pDelay);

	protected:
		const IconAnimData *m_iconAnimData;
		int m_seq_idx;		// Current sequence index.
		int m_frame;		// Current frame.
		int m_delay;		// Current frame delay. (ms)
		int m_last_valid_frame;	// Last frame that had a valid image.
};

}

#endif /* __ROMPROPERTIES_LIBRPBASE_IMG_ICONANIMHELPER_HPP__ */
