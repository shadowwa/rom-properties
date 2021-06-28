/***************************************************************************
 * ROM Properties Page shell extension. (GTK+ 3.x)                         *
 * RpNautilusPlugin.cpp: Nautilus (and forks) Plugin Definition.           *
 *                                                                         *
 * Copyright (c) 2017-2020 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "stdafx.h"
#include "config.gtk3.h"
#include "plugin-helper.h"

#include "RpNautilusPlugin.hpp"
#include "RpNautilusProvider.hpp"
#include "AchGDBus.hpp"

static GType type_list[1];

// C includes.
#include <assert.h>

// Function pointers.
static void *libextension_so;
PFN_NAUTILUS_FILE_INFO_GET_TYPE pfn_nautilus_file_info_get_type;
PFN_NAUTILUS_FILE_INFO_GET_URI pfn_nautilus_file_info_get_uri;
PFN_NAUTILUS_PROPERTY_PAGE_PROVIDER_GET_TYPE pfn_nautilus_property_page_provider_get_type;
PFN_NAUTILUS_PROPERTY_PAGE_NEW pfn_nautilus_property_page_new;

static void
rp_nautilus_register_types(GTypeModule *module)
{
	/* Register the types provided by this module */
	// NOTE: G_DEFINE_DYNAMIC_TYPE() marks the *_register_type()
	// functions as static, so we're using wrapper functions here.
	rp_nautilus_provider_register_type_ext(module);

	/* Setup the plugin provider type list */
	type_list[0] = TYPE_RP_NAUTILUS_PROVIDER;
}

/** Per-frontend initialization functions. **/

#ifdef ENABLE_ACHIEVEMENTS
#  define REGISTER_ACHDBUS() AchGDBus::instance()
#else /* !ENABLE_ACHIEVEMENTS */
#  define REGISTER_ACHDBUS() do { } while (0)
#endif /* ENABLE_ACHIEVEMENTS */

#define NAUTILUS_MODULE_INITIALIZE_FUNC(prefix) \
extern "C" G_MODULE_EXPORT void \
prefix##_module_initialize(GTypeModule *module) \
{ \
	CHECK_UID(); \
\
	assert(libextension_so == NULL); \
	if (libextension_so != NULL) { \
		/* TODO: Reference count? */ \
		g_critical("*** " G_LOG_DOMAIN ": " #prefix "_module_initialize() called twice?"); \
		return; \
	} \
\
	/* dlopen() the extension library. */ \
	libextension_so = dlopen("lib" #prefix "-extension.so", RTLD_LAZY | RTLD_LOCAL); \
	if (!libextension_so) { \
		g_critical("*** " G_LOG_DOMAIN ": dlopen() failed: %s\n", dlerror()); \
		return; \
	} \
\
	/* Load symbols. */ \
	DLSYM(nautilus_file_info_get_type,		prefix##_file_info_get_type); \
	DLSYM(nautilus_file_info_get_uri,		prefix##_file_info_get_uri); \
	DLSYM(nautilus_property_page_provider_get_type,	prefix##_property_page_provider_get_type); \
	DLSYM(nautilus_property_page_new,		prefix##_property_page_new); \
\
	/* Symbols loaded. Register our types. */ \
	rp_nautilus_register_types(module); \
\
	/* Register AchGDBus if it's available. */ \
	REGISTER_ACHDBUS(); \
}

NAUTILUS_MODULE_INITIALIZE_FUNC(nautilus)
NAUTILUS_MODULE_INITIALIZE_FUNC(caja)
NAUTILUS_MODULE_INITIALIZE_FUNC(nemo)

/** Common shutdown and list_types functions. **/

extern "C" G_MODULE_EXPORT void
nautilus_module_shutdown(void)
{
#ifdef G_ENABLE_DEBUG
	g_message("Shutting down " G_LOG_DOMAIN " extension");
#endif

	if (libextension_so) {
		dlclose(libextension_so);
		libextension_so = NULL;
	}
}

extern "C" G_MODULE_EXPORT void
nautilus_module_list_types(const GType	**types,
			   gint		 *n_types)
{
	*types = type_list;
	*n_types = G_N_ELEMENTS(type_list);
}

extern "C" {

/** Symbol aliases for MATE (Caja) **/

G_MODULE_EXPORT void caja_module_shutdown		(void)
	__attribute__((alias("nautilus_module_shutdown")));
G_MODULE_EXPORT void caja_module_list_types		(const GType	**types,
							 gint		 *n_types)
	__attribute__((alias("nautilus_module_list_types")));

/** Symbol aliases for Cinnamon (Nemo) **/

G_MODULE_EXPORT void nemo_module_shutdown		(void)
	__attribute__((alias("nautilus_module_shutdown")));
G_MODULE_EXPORT void nemo_module_list_types		(const GType	**types,
							 gint		 *n_types)
	__attribute__((alias("nautilus_module_list_types")));

}
