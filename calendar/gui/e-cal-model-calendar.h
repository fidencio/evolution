/*
 *
 * Evolution calendar - Data model for ETable
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with the program; if not, see <http://www.gnu.org/licenses/>  
 *
 *
 * Authors:
 *		Rodrigo Moya <rodrigo@ximian.com>
 *
 * Copyright (C) 1999-2008 Novell, Inc. (www.novell.com)
 *
 */

#ifndef E_CAL_MODEL_CALENDAR_H
#define E_CAL_MODEL_CALENDAR_H

#include "e-cal-model.h"

G_BEGIN_DECLS

#define E_TYPE_CAL_MODEL_CALENDAR            (e_cal_model_calendar_get_type ())
#define E_CAL_MODEL_CALENDAR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), E_TYPE_CAL_MODEL_CALENDAR, ECalModelCalendar))
#define E_CAL_MODEL_CALENDAR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), E_TYPE_CAL_MODEL_CALENDAR, ECalModelCalendarClass))
#define E_IS_CAL_MODEL_CALENDAR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), E_TYPE_CAL_MODEL_CALENDAR))
#define E_IS_CAL_MODEL_CALENDAR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), E_TYPE_CAL_MODEL_CALENDAR))

typedef struct _ECalModelCalendarPrivate ECalModelCalendarPrivate;

typedef enum {
	/* If you add new items here or reorder them, you have to update the
	   .etspec files for the tables using this model */
	E_CAL_MODEL_CALENDAR_FIELD_DTEND = E_CAL_MODEL_FIELD_LAST,
	E_CAL_MODEL_CALENDAR_FIELD_LOCATION,
	E_CAL_MODEL_CALENDAR_FIELD_TRANSPARENCY,
	E_CAL_MODEL_CALENDAR_FIELD_LAST
} ECalModelCalendarField;

typedef struct {
	ECalModel model;
	ECalModelCalendarPrivate *priv;
} ECalModelCalendar;

typedef struct {
	ECalModelClass parent_class;
} ECalModelCalendarClass;

GType              e_cal_model_calendar_get_type (void);
ECalModelCalendar *e_cal_model_calendar_new (void);

G_END_DECLS

#endif
