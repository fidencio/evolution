/*
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
 *		Srinivasa Ragavan <sragavan@gnome.org>
 *
 *
 */

#ifndef E_MAIL_UTILS_H
#define E_MAIL_UTILS_H

#include <camel/camel.h>
#include <libedataserver/e-account.h>

gboolean	em_utils_folder_is_drafts	(CamelFolder *folder);
gboolean	em_utils_folder_is_templates	(CamelFolder *folder);
gboolean	em_utils_folder_is_sent		(CamelFolder *folder);
gboolean	em_utils_folder_is_outbox	(CamelFolder *folder);
gboolean	em_utils_in_addressbook		(CamelInternetAddress *addr,
						 gboolean local_only);
CamelMimePart *	em_utils_contact_photo		(CamelInternetAddress *addr,
						 gboolean local);
EAccount *	em_utils_guess_account		(CamelMimeMessage *message,
						 CamelFolder *folder);
EAccount *	em_utils_guess_account_with_recipients
						(CamelMimeMessage *message,
						 CamelFolder *folder);
void		emu_remove_from_mail_cache	(const GSList *addresses);
void		emu_remove_from_mail_cache_1	(const gchar *address);
void		emu_free_mail_cache		(void);
void		em_utils_uids_free		(GPtrArray *uids);
gboolean	em_utils_is_local_delivery_mbox_file
						(CamelURL *url);

#endif /* E_MAIL_UTILS_H */
