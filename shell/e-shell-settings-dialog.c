/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* e-shell-settings-dialog.c
 *
 * Copyright (C) 2002  Ximian, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Author: Ettore Perazzoli <ettore@ximian.com>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "e-shell-settings-dialog.h"

#include "e-corba-config-page.h"

#include "e-util/e-lang-utils.h"

#include <gal/util/e-util.h>

#include <bonobo/bonobo-widget.h>
#include <bonobo/bonobo-exception.h>
#include <liboaf/liboaf.h>

#include <string.h>


#define PARENT_TYPE e_multi_config_dialog_get_type ()
static EMultiConfigDialogClass *parent_class = NULL;



struct _EShellSettingsDialogPrivate {
	GHashTable *types;
};


/* FIXME ugly hack to work around that sizing of invisible widgets is broken
   with Bonobo.  */

static void
set_dialog_size (EShellSettingsDialog *dialog)
{
	GdkFont *font;
	int width, height;

	font = GTK_WIDGET (dialog)->style->font;
	width = gdk_string_width (font, "M") * 66;
	height = (font->ascent + font->descent) * 35;

	gtk_widget_set_usize (GTK_WIDGET (dialog), width, height);
}


/* Page handling.  */

struct _Page {
	char *title;
	char *description;
	GdkPixbuf *icon;
	OAF_Property *type;
	int priority;
	EConfigPage *page_widget;
};
typedef struct _Page Page;

static Page *
page_new (const char *title,
	  const char *description,
	  GdkPixbuf *icon,
	  OAF_Property *type,
	  int priority,
	  EConfigPage *page_widget)
{
	Page *page;

	if (icon != NULL)
		gdk_pixbuf_ref (icon);

	page = g_new (Page, 1);
	page->title       = g_strdup (_(title));
	page->description = g_strdup (_(description));
	page->icon        = icon;
	page->type        = type;
	page->priority    = priority;
	page->page_widget = page_widget;

	return page;
}

static void
page_free (Page *page)
{
	g_free (page->title);
	g_free (page->description);

	if (page->icon != NULL)
		gdk_pixbuf_unref (page->icon);

	g_free (page);
}

static int
compare_page_func (const void *a,
		   const void *b)
{
	const Page *page_a;
	const Page *page_b;

	page_a = (const Page *) a;
	page_b = (const Page *) b;

	if (page_a->priority == page_b->priority)
		return strcmp (page_a->title, page_b->title);

	return page_a->priority - page_b->priority;
}

static GList *
sort_page_list (GList *list)
{
	return g_list_sort (list, compare_page_func);
}

static void
load_pages (EShellSettingsDialog *dialog)
{
	EShellSettingsDialogPrivate *priv;
	OAF_ServerInfoList *control_list;
	CORBA_Environment ev;
	GSList *language_list;
	GList *page_list;
	GList *p;
	int i, j;

	priv = dialog->priv;
	
	CORBA_exception_init (&ev);

	control_list = oaf_query ("defined(evolution:config_item:title)", NULL, &ev);
	if (ev._major != CORBA_NO_EXCEPTION || control_list == NULL) {
		g_warning ("Cannot load configuration pages -- %s", ev._repo_id);
		CORBA_exception_free (&ev);
		return;
	}

	language_list = e_get_language_list ();

	page_list = NULL;
	for (i = 0; i < control_list->_length; i ++) {
		CORBA_Object corba_object;
		OAF_ServerInfo *info;
		const char *title;
		const char *description;
		const char *icon_path;
		const char *priority_string;
		OAF_Property *type;
		int priority;
		GdkPixbuf *icon;

		info = & control_list->_buffer[i];

		title       	= oaf_server_info_prop_lookup (info, "evolution:config_item:title", language_list);
		description 	= oaf_server_info_prop_lookup (info, "evolution:config_item:description", language_list);
		icon_path   	= oaf_server_info_prop_lookup (info, "evolution:config_item:icon_name", NULL);
		type            = oaf_server_info_prop_find (info, "evolution:config_item:type");
		priority_string = oaf_server_info_prop_lookup (info, "evolution:config_item:priority", NULL);

		if (icon_path == NULL) {
			icon = NULL;
		} else {
			if (g_path_is_absolute (icon_path)) {
				icon = gdk_pixbuf_new_from_file (icon_path);
			} else {
				char *real_icon_path;

				real_icon_path = g_concat_dir_and_file (EVOLUTION_IMAGES, icon_path);
				icon = gdk_pixbuf_new_from_file (real_icon_path);
				g_free (real_icon_path);
			}
		}

		if (type != NULL && type->v._d != OAF_P_STRINGV)
			type = NULL;
		if (priority_string == NULL)
			priority = 0xffff;
		else
			priority = atoi (priority_string);

		corba_object = oaf_activate_from_id ((char *) info->iid, 0, NULL, &ev);

		if (! BONOBO_EX (&ev)) {
			Page *page;

			page = page_new (title, description, icon, type, priority,
					 E_CONFIG_PAGE (e_corba_config_page_new_from_objref (corba_object)));

			page_list = g_list_prepend (page_list, page);
		} else {
			g_warning ("Cannot activate %s -- %s", info->iid, BONOBO_EX_ID (&ev));
		}

		if (icon != NULL)
			gdk_pixbuf_unref (icon);
	}

	page_list = sort_page_list (page_list);
	for (p = page_list, i = 0; p != NULL; p = p->next, i++) {
		Page *page;

		page = (Page *) p->data;

		e_multi_config_dialog_add_page (E_MULTI_CONFIG_DIALOG (dialog),
						page->title,
						page->description,
						page->icon,
						page->page_widget);

		if (page->type != NULL) {
			GNOME_stringlist list = page->type->v._u.value_stringv;
			
			for (j = 0; j < list._length; j++) {
				if (g_hash_table_lookup (priv->types, list._buffer[j]) == NULL)
					g_hash_table_insert (priv->types, g_strdup (list._buffer[j]), 
							     GINT_TO_POINTER (i));
			}
		}
		
		
		page_free (page);
	}

	g_list_free (page_list);
	e_free_language_list (language_list);
	CORBA_free (control_list);

	CORBA_exception_free (&ev);
}


/* GtkWidget methods.  */

static void
impl_realize (GtkWidget *widget)
{
	EShellSettingsDialog *dialog;

	dialog = E_SHELL_SETTINGS_DIALOG (widget);

	set_dialog_size (dialog);

	(* GTK_WIDGET_CLASS (parent_class)->realize) (widget);
}


/* GtkObject methods.  */

static gboolean
destroy_type_entry (gpointer key, gpointer value, gpointer data)
{
	g_free (key);
	
	return TRUE;
}
		
static void
impl_destroy (GtkObject *object)
{
	EShellSettingsDialog *dialog;
	EShellSettingsDialogPrivate *priv;

	dialog = E_SHELL_SETTINGS_DIALOG (object);
	priv = dialog->priv;
	
	g_hash_table_foreach_remove (priv->types, destroy_type_entry, NULL);
	g_hash_table_destroy (priv->types);

	g_free (priv);
	
	(* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}


static void
class_init (EShellSettingsDialog *class)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = GTK_OBJECT_CLASS (class);
	object_class->destroy = impl_destroy;

	widget_class = GTK_WIDGET_CLASS (class);
	widget_class->realize = impl_realize;

	parent_class = gtk_type_class (PARENT_TYPE);
}

static void
init (EShellSettingsDialog *dialog)
{
	EShellSettingsDialogPrivate *priv;

	priv = g_new (EShellSettingsDialogPrivate, 1);
	priv->types = g_hash_table_new (g_str_hash, g_str_equal);

	dialog->priv = priv;

	load_pages (dialog);

	gtk_window_set_title (GTK_WINDOW (dialog), _("Evolution Settings"));
}


GtkWidget *
e_shell_settings_dialog_new ()
{
	EShellSettingsDialog *new;

	new = gtk_type_new (e_shell_settings_dialog_get_type ());

	return GTK_WIDGET (new);
}

void
e_shell_settings_dialog_show_type (EShellSettingsDialog *dialog, const char *type)
{
	EShellSettingsDialogPrivate *priv;
	gpointer value;
	int page;
	
	g_return_if_fail (dialog != NULL);
	g_return_if_fail (E_IS_SHELL_SETTINGS_DIALOG (dialog));
	g_return_if_fail (type != NULL);

	priv = dialog->priv;
	
	value = g_hash_table_lookup (priv->types, type);
	page = GPOINTER_TO_INT (value);
	
	e_multi_config_dialog_show_page (E_MULTI_CONFIG_DIALOG (dialog), page);
}


E_MAKE_TYPE (e_shell_settings_dialog, "EShellSettingsDialog", EShellSettingsDialog,
	     class_init, init, PARENT_TYPE)

