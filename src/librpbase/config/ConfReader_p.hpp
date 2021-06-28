/***************************************************************************
 * ROM Properties Page shell extension. (librpbase)                        *
 * ConfReader_p.hpp: Configuration reader base class.(Private class)       *
 *                                                                         *
 * Copyright (c) 2016-2020 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __ROMPROPERTIES_LIBRPBASE_CONFIG_CONFREADER_P_HPP__
#define __ROMPROPERTIES_LIBRPBASE_CONFIG_CONFREADER_P_HPP__

#include "librpbase/config.librpbase.h"
#include "common.h"

// librpthreads
#include "librpthreads/Mutex.hpp"

// INI parser.
#include "ini.h"

// C++ includes.
#include <string>

namespace LibRpBase {

class ConfReader;
class ConfReaderPrivate
{
	public:
		/**
		 * Configuration reader.
		 * @param filename Configuration filename. Relative to ~/.config/rom-properties
		 */
		explicit ConfReaderPrivate(const char *filename);
		virtual ~ConfReaderPrivate();

	private:
		RP_DISABLE_COPY(ConfReaderPrivate)

	public:
		// load() mutex.
		LibRpThreads::Mutex mtxLoad;

		// Configuration filename.
		const char *const conf_rel_filename;	// from ctor
		std::string conf_filename;		// alloc()'d in load()

		// rom-properties.conf status.
		bool conf_was_found;
		time_t conf_mtime;
		time_t conf_last_checked;

	public:
		/**
		 * Reset the configuration to the default values.
		 */
		virtual void reset(void) = 0;

		/**
		 * Process a configuration line.
		 * Static function; used by inih as a C-style callback function.
		 *
		 * @param user ConfReaderPrivate object.
		 * @param section Section.
		 * @param name Key.
		 * @param value Value.
		 * @return 1 on success; 0 on error.
		 */
		static int processConfigLine_static(void *user,
			const char *section, const char *name, const char *value);

		/**
		 * Process a configuration line.
		 * Virtual function; must be reimplemented by subclasses.
		 *
		 * @param section Section.
		 * @param name Key.
		 * @param value Value.
		 * @return 1 on success; 0 on error.
		 */
		virtual int processConfigLine(const char *section,
			const char *name, const char *value) = 0;
};

}

#endif /* __ROMPROPERTIES_LIBRPBASE_CONFIG_CONFREADER_P_HPP__ */
