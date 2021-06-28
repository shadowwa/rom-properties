/***************************************************************************
 * ROM Properties Page shell extension. (GTK+ common)                      *
 * MessageWidget.hpp: Message widget. (Similar to KMessageWidget)          *
 *                                                                         *
 * Copyright (c) 2017-2020 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "stdafx.h"
#include "MessageWidget.hpp"

/* Property identifiers */
typedef enum {
	PROP_0,

	PROP_TEXT,
	PROP_MESSAGE_TYPE,

	PROP_LAST
} MessageWidgetPropID;

static void	message_widget_dispose	(GObject	*object);
static void	message_widget_finalize	(GObject	*object);

static void	message_widget_get_property	(GObject	*object,
						 guint		 prop_id,
						 GValue		*value,
						 GParamSpec	*pspec);
static void	message_widget_set_property	(GObject	*object,
						 guint		 prop_id,
						 const GValue	*value,
						 GParamSpec	*pspec);

/** Signal handlers. **/
static void	closeButton_clicked_handler	(GtkButton	*button,
						 gpointer	 user_data);
static gboolean	timeout_hide_source_func	(gpointer	 user_data);

#if GTK_CHECK_VERSION(3,0,0)
typedef GtkBoxClass superclass;
typedef GtkBox super;
#define GTK_TYPE_SUPER GTK_TYPE_BOX
#define USE_GTK_GRID 1	// Use GtkGrid instead of GtkTable.
#else
// Top class is GtkEventBox, since we can't
// set the background color of GtkHBox.
typedef GtkEventBoxClass superclass;
typedef GtkEventBox super;
#define GTK_TYPE_SUPER GTK_TYPE_EVENT_BOX
#endif

// MessageWidget class.
struct _MessageWidgetClass {
	superclass __parent__;

	GParamSpec *properties[PROP_LAST];
};

// MessageWidget instance.
struct _MessageWidget {
	super __parent__;

#if !GTK_CHECK_VERSION(3,0,0)
	GtkWidget *evbox_inner;
	GtkWidget *hbox;
#endif /* !GTK_CHECK_VERSION(3,0,0) */

	GtkWidget *image;
	GtkWidget *label;
	GtkWidget *closeButton;

	GtkMessageType messageType;
	guint timeout_hide;
};

// NOTE: G_DEFINE_TYPE() doesn't work in C++ mode with gcc-6.2
// due to an implicit int to GTypeFlags conversion.
G_DEFINE_TYPE_EXTENDED(MessageWidget, message_widget,
	GTK_TYPE_SUPER, static_cast<GTypeFlags>(0), {});

static void
message_widget_class_init(MessageWidgetClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
	gobject_class->dispose = message_widget_dispose;
	gobject_class->finalize = message_widget_finalize;
	gobject_class->get_property = message_widget_get_property;
	gobject_class->set_property = message_widget_set_property;

	/** Properties **/

	klass->properties[PROP_TEXT] = g_param_spec_string(
		"text", "Text", "Text displayed on the MessageWidget.",
		nullptr,
		(GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	klass->properties[PROP_MESSAGE_TYPE] = g_param_spec_enum(
		"message-type", "Message Type", "Message type.",
		GTK_TYPE_MESSAGE_TYPE, GTK_MESSAGE_OTHER,
		(GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	// Install the properties.
	g_object_class_install_properties(gobject_class, PROP_LAST, klass->properties);

#if GTK_CHECK_VERSION(3,0,0)
	// Initialize MessageWidget CSS.
	GtkCssProvider *const provider = gtk_css_provider_new();
	GdkDisplay *const display = gdk_display_get_default();
	GdkScreen *const screen = gdk_display_get_default_screen(display);

	gtk_style_context_add_provider_for_screen(screen,
		GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);

	static const char css_MessageWidget[] =
		"@define-color gsrp_color_info rgb(61,174,233);\n"
		"@define-color gsrp_color_warning rgb(246,116,0);\n"
		"@define-color gsrp_color_error rgb(218,68,83);\n"
		".gsrp_msgw_info {\n"
		"\tbackground-color: lighter(@gsrp_color_info);\n"
		"\tborder: 2px solid @gsrp_color_info;\n"
		"}\n"
		".gsrp_msgw_warning {\n"
		"\tbackground-color: lighter(@gsrp_color_warning);\n"
		"\tborder: 2px solid @gsrp_color_warning;\n"
		"}\n"
		".gsrp_msgw_question {\n"
		"\tbackground-color: lighter(@gsrp_color_info);\n"	// NOTE: Same as INFO.
		"\tborder: 2px solid @gsrp_color_info;\n"
		"}\n"
		".gsrp_msgw_error {\n"
		"\tbackground-color: lighter(@gsrp_color_error);\n"
		"\tborder: 2px solid @gsrp_color_error;\n"
		"}\n";

	gtk_css_provider_load_from_data(GTK_CSS_PROVIDER(provider), css_MessageWidget, -1, nullptr);
	g_object_unref(provider);
#endif /* GTK_CHECK_VERSION(3,0,0) */
}

static void
message_widget_init(MessageWidget *widget)
{
	GtkBox *hbox;
#if GTK_CHECK_VERSION(3,0,0)
	// Make this an HBox.
	gtk_orientable_set_orientation(GTK_ORIENTABLE(widget), GTK_ORIENTATION_HORIZONTAL);
	hbox = GTK_BOX(widget);
#else /* !GTK_CHECK_VERSION(3,0,0) */
	// Add a GtkEventBox for the inner color.
	widget->evbox_inner = gtk_event_box_new();
	gtk_widget_show(widget->evbox_inner);
	gtk_container_add(GTK_CONTAINER(widget), widget->evbox_inner);

	// Add a GtkHBox for all the other widgets.
	widget->hbox = gtk_hbox_new(0, 0);
	gtk_widget_show(widget->hbox);
	gtk_container_add(GTK_CONTAINER(widget->evbox_inner), widget->hbox);
	hbox = GTK_BOX(widget->hbox);
#endif /* GTK_CHECK_VERSION(3,0,0) */

	widget->messageType = GTK_MESSAGE_OTHER;

	widget->image = gtk_image_new();
	gtk_box_pack_start(hbox, widget->image, FALSE, FALSE, 4);

	widget->label = gtk_label_new(nullptr);
	gtk_widget_show(widget->label);
	gtk_box_pack_start(hbox, widget->label, FALSE, FALSE, 0);

	// TODO: Prpoer alignment.

	widget->closeButton = gtk_button_new();
	GtkWidget *const imageClose = gtk_image_new_from_icon_name("dialog-close", GTK_ICON_SIZE_BUTTON);
	gtk_button_set_image(GTK_BUTTON(widget->closeButton), imageClose);
	gtk_button_set_relief(GTK_BUTTON(widget->closeButton), GTK_RELIEF_NONE);
	gtk_widget_show(widget->closeButton);
	gtk_box_pack_end(hbox, widget->closeButton, FALSE, FALSE, 0);

	g_signal_connect(widget->closeButton, "clicked",
		G_CALLBACK(closeButton_clicked_handler), widget);
}

static void
message_widget_dispose(GObject *object)
{
	//MessageWidget *const widget = MESSAGE_WIDGET(object);

	// Nothing to do here right now...

	// Call the superclass dispose() function.
	G_OBJECT_CLASS(message_widget_parent_class)->dispose(object);
}

static void
message_widget_finalize(GObject *object)
{
	//MessageWidget *const widget = MESSAGE_WIDGET(object);

	// Nothing to do here right now...

	// Call the superclass finalize() function.
	G_OBJECT_CLASS(message_widget_parent_class)->finalize(object);
}

GtkWidget*
message_widget_new(void)
{
	return static_cast<GtkWidget*>(g_object_new(TYPE_MESSAGE_WIDGET, nullptr));
}

/** Properties **/

static void
message_widget_get_property(GObject	*object,
			   guint	 prop_id,
			   GValue	*value,
			   GParamSpec	*pspec)
{
	MessageWidget *const widget = MESSAGE_WIDGET(object);

	switch (prop_id) {
		case PROP_TEXT:
			g_value_set_string(value, gtk_label_get_text(GTK_LABEL(widget->label)));
			break;

		case PROP_MESSAGE_TYPE:
			g_value_set_enum(value, widget->messageType);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
			break;
	}
}

static void
message_widget_set_property(GObject	*object,
			   guint	 prop_id,
			   const GValue	*value,
			   GParamSpec	*pspec)
{
	MessageWidget *const widget = MESSAGE_WIDGET(object);

	switch (prop_id) {
		case PROP_TEXT:
			gtk_label_set_text(GTK_LABEL(widget->label), g_value_get_string(value));
			break;

		case PROP_MESSAGE_TYPE:
			message_widget_set_message_type(widget, static_cast<GtkMessageType>(g_value_get_enum(value)));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
			break;
	}
}

void
message_widget_set_text(MessageWidget *widget, const gchar *str)
{
	gtk_label_set_text(GTK_LABEL(widget->label), str);

	// FIXME: If called from rom_data_view_set_property(), this might
	// result in *two* notifications.
	MessageWidgetClass *const klass = MESSAGE_WIDGET_GET_CLASS(widget);
	g_object_notify_by_pspec(G_OBJECT(widget), klass->properties[PROP_TEXT]);
}

const gchar*
message_widget_get_text(MessageWidget *widget)
{
	return gtk_label_get_text(GTK_LABEL(widget->label));
}

void
message_widget_set_message_type(MessageWidget *widget, GtkMessageType messageType)
{
	// Update the icon.
	// Background colors based on KMessageWidget.
	struct IconInfo_t {
		const char *icon_name;	// XDG icon name
		const char *css_class;	// CSS class (GTK+ 3.x only)
		uint32_t border_color;	// from KMessageWidget
		uint32_t bg_color;	// lightened version of border_color
	};
	static const IconInfo_t iconInfo[] = {
		{"dialog-information",	"gsrp_msgw_info",	0x3DAEE9, 0x7FD3FF},	// INFO
		{"dialog-warning",	"gsrp_msgw_warning",	0xF67400, 0xFF9B41},	// WARNING
		{"dialog-question",	"gsrp_msgw_question",	0x3DAEE9, 0x7FD3FF},	// QUESTION (same as INFO)
		{"dialog-error",	"gsrp_msgw_error",	0xDA4453, 0xF77E8A},	// ERROR
		{nullptr,		nullptr,		0,        0},		// OTHER
	};

	if (messageType < 0 || messageType > ARRAY_SIZE(iconInfo)) {
		// Default to OTHER.
		messageType = GTK_MESSAGE_OTHER;
	}
	if (widget->messageType == messageType)
		return;
	widget->messageType = messageType;

	const IconInfo_t *const pIconInfo = &iconInfo[messageType];

	gtk_widget_set_visible(widget->image, (pIconInfo->icon_name != nullptr));
	if (pIconInfo->icon_name) {
		// TODO: Better icon size?
		gtk_image_set_from_icon_name(GTK_IMAGE(widget->image), pIconInfo->icon_name, GTK_ICON_SIZE_BUTTON);

#if GTK_CHECK_VERSION(3,0,0)
		// Remove all of our CSS classes first.
		GtkStyleContext *const context = gtk_widget_get_style_context(GTK_WIDGET(widget));
		gtk_style_context_remove_class(context, "gsrp_msgw_info");
		gtk_style_context_remove_class(context, "gsrp_msgw_warning");
		gtk_style_context_remove_class(context, "gsrp_msgw_question");
		gtk_style_context_remove_class(context, "gsrp_msgw_error");

		// Add the new CSS class.
		gtk_style_context_add_class(context, pIconInfo->css_class);
#else /* !GTK_CHECK_VERSION(3,0,0) */
		GdkColor color;
		color.pixel	 = pIconInfo->border_color;
		color.red	 = (color.pixel >> 16) & 0xFF;
		color.green	 = (color.pixel >>  8) & 0xFF;
		color.blue	 =  color.pixel & 0xFF;
		color.red	|= (color.red << 8);
		color.green	|= (color.green << 8);
		color.blue	|= (color.blue << 8);
		gtk_widget_modify_bg(GTK_WIDGET(widget), GTK_STATE_NORMAL, &color);

		color.pixel	 = pIconInfo->bg_color;
		color.red	 = (color.pixel >> 16) & 0xFF;
		color.green	 = (color.pixel >>  8) & 0xFF;
		color.blue	 =  color.pixel & 0xFF;
		color.red	|= (color.red << 8);
		color.green	|= (color.green << 8);
		color.blue	|= (color.blue << 8);
		gtk_widget_modify_bg(GTK_WIDGET(widget->evbox_inner), GTK_STATE_NORMAL, &color);

		gtk_container_set_border_width(GTK_CONTAINER(widget->evbox_inner), 2);
#endif /* GTK_CHECK_VERSION(3,0,0) */
	} else {
#if !GTK_CHECK_VERSION(3,0,0)
		gtk_container_set_border_width(GTK_CONTAINER(widget->evbox_inner), 0);
		gtk_widget_modify_bg(GTK_WIDGET(widget), GTK_STATE_NORMAL, nullptr);
		gtk_widget_modify_bg(GTK_WIDGET(widget->evbox_inner), GTK_STATE_NORMAL, nullptr);
#endif /* !GTK_CHECK_VERSION(3,0,0) */
	}

	// FIXME: If called from rom_data_view_set_property(), this might
	// result in *two* notifications.
	MessageWidgetClass *const klass = MESSAGE_WIDGET_GET_CLASS(widget);
	g_object_notify_by_pspec(G_OBJECT(widget), klass->properties[PROP_MESSAGE_TYPE]);
}

GtkMessageType
message_widget_get_message_type(MessageWidget *widget)
{
	return widget->messageType;
}

/** Other functions **/

void
message_widget_show_with_timeout(MessageWidget *widget)
{
	g_return_if_fail(widget != nullptr || IS_MESSAGE_WIDGET(widget));
	gtk_widget_show(GTK_WIDGET(widget));

	// TODO: Animation like KMessageWidget.
	if (widget->timeout_hide) {
		// Remove the current timeout.
		g_source_remove(widget->timeout_hide);
	}
	// Set the timeout.
	widget->timeout_hide = g_timeout_add_seconds(10, timeout_hide_source_func, widget);
}

/** Signal handlers **/

/**
 * The Close button was clicked.
 * @param button Close button
 * @param user_data MessageWidget*
 */
static void
closeButton_clicked_handler(GtkButton	*button,
			    gpointer	 user_data)
{
	MessageWidget *const widget = MESSAGE_WIDGET(user_data);

	// TODO: Animation like KMessageWidget.
	RP_UNUSED(button);
	if (widget->timeout_hide != 0) {
		g_source_remove(widget->timeout_hide);
		widget->timeout_hide = 0;
	}
	gtk_widget_hide(GTK_WIDGET(widget));
}

/**
 * Message hide timeout has elapsed.
 * @param user_data MessageWidget*
 * @return G_SOURCE_CONTINUE to continue the timer; G_SOURCE_REMOVE to remove the timer.
 */
static gboolean
timeout_hide_source_func(gpointer user_data)
{
	MessageWidget *const widget = MESSAGE_WIDGET(user_data);
	widget->timeout_hide = 0;
	gtk_widget_hide(GTK_WIDGET(widget));
	return G_SOURCE_REMOVE;
}
