/* Evolution calendar - Utilities for tagging ECalendar widgets
 *
 * Copyright (C) 2000 Ximian, Inc.
 *
 * Authors: Damon Chaplin <damon@ximian.com>
 *          Federico Mena-Quintero <federico@ximian.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <cal-util/timeutil.h>
#include "calendar-config.h"
#include "tag-calendar.h"



struct calendar_tag_closure {
	ECalendarItem *calitem;
	icaltimezone *zone;
	time_t start_time;
	time_t end_time;

	gboolean skip_transparent_events;
};

/* Clears all the tags in a calendar and fills a closure structure with the
 * necessary information for iterating over occurrences.
 * Returns FALSE if the calendar has no dates shown.
 */
static gboolean
prepare_tag (ECalendar *ecal, struct calendar_tag_closure *c, gboolean clear_first)
{
	gint start_year, start_month, start_day;
	gint end_year, end_month, end_day;
	struct icaltimetype start_tt = icaltime_null_time ();
	struct icaltimetype end_tt = icaltime_null_time ();
	char *location;

	if (clear_first)
		e_calendar_item_clear_marks (ecal->calitem);

	if (!e_calendar_item_get_date_range (ecal->calitem,
					     &start_year, &start_month,
					     &start_day,
					     &end_year, &end_month, &end_day))
	    return FALSE;

	start_tt.year = start_year;
	start_tt.month = start_month + 1;
	start_tt.day = start_day;

	end_tt.year = end_year;
	end_tt.month = end_month + 1;
	end_tt.day = end_day;

	icaltime_adjust (&end_tt, 1, 0, 0, 0);

	c->calitem = ecal->calitem;

	/* FIXME. It may be better if the timezone is passed in. */
	location = calendar_config_get_timezone ();
	c->zone = icaltimezone_get_builtin_timezone (location);

	c->start_time = icaltime_as_timet_with_zone (start_tt, c->zone);
	c->end_time = icaltime_as_timet_with_zone (end_tt, c->zone);

	return TRUE;
}

/* Marks the specified range in an ECalendar; called from cal_client_generate_instances() */
static gboolean
tag_calendar_cb (CalComponent *comp,
		 time_t istart,
		 time_t iend,
		 gpointer data)
{
	struct calendar_tag_closure *c = data;
	struct icaltimetype start_tt, end_tt;
	CalComponentTransparency transparency;

	/* If we are skipping TRANSPARENT events, return if the event is
	   transparent. */
	if (c->skip_transparent_events) {
		cal_component_get_transparency (comp, &transparency);
		if (transparency == CAL_COMPONENT_TRANSP_TRANSPARENT)
			return TRUE;
	}

	start_tt = icaltime_from_timet_with_zone (istart, FALSE, c->zone);
	end_tt = icaltime_from_timet_with_zone (iend - 1, FALSE, c->zone);
	e_calendar_item_mark_days (c->calitem,
				   start_tt.year, start_tt.month - 1,
				   start_tt.day,
				   end_tt.year, end_tt.month - 1,
				   end_tt.day,
				   E_CALENDAR_ITEM_MARK_BOLD);

	return TRUE;
}

/**
 * tag_calendar_by_client:
 * @ecal: Calendar widget to tag.
 * @client: A calendar client object.
 * 
 * Tags an #ECalendar widget with the events that occur in its current time
 * range.  The occurrences are extracted from the specified calendar @client.
 **/
void
tag_calendar_by_client (ECalendar *ecal, CalClient *client)
{
	struct calendar_tag_closure c;

	g_return_if_fail (ecal != NULL);
	g_return_if_fail (E_IS_CALENDAR (ecal));
	g_return_if_fail (client != NULL);
	g_return_if_fail (IS_CAL_CLIENT (client));

	/* If the ECalendar isn't visible, we just return. */
	if (!GTK_WIDGET_VISIBLE (ecal))
		return;

	if (cal_client_get_load_state (client) != CAL_CLIENT_LOAD_LOADED)
		return;

	if (!prepare_tag (ecal, &c, TRUE))
		return;

	c.skip_transparent_events = TRUE;

#if 0
	g_print ("DateNavigator generating instances\n");
#endif
	cal_client_generate_instances (client, CALOBJ_TYPE_EVENT,
				       c.start_time, c.end_time,
				       tag_calendar_cb, &c);
}

/**
 * tag_calendar_by_comp:
 * @ecal: Calendar widget to tag.
 * @comp: A calendar component object.
 * @clear_first: Whether the #ECalendar should be cleared of any marks first.
 * 
 * Tags an #ECalendar widget with any occurrences of a specific calendar
 * component that occur within the calendar's current time range.
 * Note that TRANSPARENT events are also tagged here.
 **/
void
tag_calendar_by_comp (ECalendar *ecal, CalComponent *comp, CalClient *client, gboolean clear_first)
{
	struct calendar_tag_closure c;

	g_return_if_fail (ecal != NULL);
	g_return_if_fail (E_IS_CALENDAR (ecal));
	g_return_if_fail (comp != NULL);
	g_return_if_fail (IS_CAL_COMPONENT (comp));

	/* If the ECalendar isn't visible, we just return. */
	if (!GTK_WIDGET_VISIBLE (ecal))
		return;

	if (!prepare_tag (ecal, &c, clear_first))
		return;

	c.skip_transparent_events = FALSE;

#if 0
	g_print ("DateNavigator generating instances\n");
#endif
	cal_recur_generate_instances (comp, c.start_time, c.end_time,
				      tag_calendar_cb, &c,
				      cal_client_resolve_tzid_cb, client);
}
