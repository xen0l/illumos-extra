/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").  
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/*
 * Copyright 2007 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#pragma	ident	"@(#)utf8_to_cp720_generator.c	1.0	07/12/03 SMI"


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <strings.h>
#include "../common_defs.h"

int
main(int ac, char **av)
{
	to_utf8_table_component_t tbl[256];
	register int i, j;
	char buf[BUFSIZ], num[100];
	unsigned int l, k;
	char ascii_only = 0;

	if (ac > 1 && strcmp(av[1], "-ascii") == 0)
		ascii_only = 1;

	for (i = 0; i < 256; i++) {
		if (i <= 0x1f || i == 0x7f || (ascii_only && i <= 0x7f)) {
			tbl[i].size = (signed char)1;
			tbl[i].u8 = (unsigned int)i;
		} else if (!ascii_only && (i >= 0x80 && i <= 0x9f)) {
			tbl[i].size = (signed char)2;
			tbl[i].u8 = (unsigned int)i;
		} else {
			tbl[i].size = (signed char)ICV_TYPE_ILLEGAL_CHAR;
			tbl[i].u8 = 0;
		}
	}


	while (fgets(buf, BUFSIZ, stdin)) {
		i = 0;
		while (buf[i] && isspace(buf[i]))
			i++;
		if (strncmp(buf + i, "<U", 2) != 0)
			continue;

		i += 2;
		for (j = 0; isxdigit(buf[i]); i++, j++)
			num[j] = buf[i];
		num[j] = '\0';

		l = strtol(num, (char **)NULL, 16);

		while (!isxdigit(buf[i]))
			i++;

		for (j = 0; isxdigit(buf[i]); i++, j++)
			num[j] = buf[i];
		num[j] = '\0';

		k = strtol(num, (char **)NULL, 16);

		while (buf[i] == ' ' || buf[i] == '\t')
			i++;

		if (strncmp(buf + i, "|0", 2) != 0)
			continue;

		tbl[k].u8 = l;
		if (l < 0x80)
			tbl[k].size = (signed char)1;
		else if (l < 0x800)
			tbl[k].size = (signed char)2;
		else if (l < 0x10000)
			tbl[k].size = (signed char)3;
		else if (l < 0x200000)
			tbl[k].size = (signed char)4;
		else if (l < 0x4000000)
			tbl[k].size = (signed char)5;
		else
			tbl[k].size = (signed char)6;

	}

	for (i = 0; i < 256; i++) {
		if (tbl[i].u8 < 0x80)
			l = tbl[i].u8;
		else if (tbl[i].u8 < 0x800) {
			l = 0xc080 |
				(((tbl[i].u8 >> 6) & 0x1f) << 8) | 
				(tbl[i].u8 & 0x3f);
		} else if (tbl[i].u8 < 0x10000) {
			l = 0xe08080 |
				(((tbl[i].u8 >> 12) & 0x0f) << 16) |
				(((tbl[i].u8 >> 6) & 0x3f) << 8) | 
				(tbl[i].u8 & 0x3f);
		} else if (tbl[i].u8 < 0x200000) {
			l = 0xf0808080 |
				(((tbl[i].u8 >> 18) & 0x07) << 24) |
				(((tbl[i].u8 >> 12) & 0x3f) << 16) |
				(((tbl[i].u8 >> 6) & 0x3f) << 8) |
				(tbl[i].u8 & 0x3f);
		} /* We only support characters in range of UTF-16  
		else if (tbl[i].u8 < 0x4000000) {
			l = 0xf880808080 |
				(((tbl[i].u8 >> 24) & 0x03) << 32) |
				(((tbl[i].u8 >> 18) & 0x3f) << 24) |
				(((tbl[i].u8 >> 12) & 0x3f) << 16) |
				(((tbl[i].u8 >> 6) & 0x3f) << 8) |
				(tbl[i].u8 & 0x3f);
		} else {
			l = 0xfc8080808080 |
				(((tbl[i].u8 >> 30) & 0x01) << 40) |
				(((tbl[i].u8 >> 24) & 0x3f) << 32) |
				(((tbl[i].u8 >> 18) & 0x3f) << 24) |
				(((tbl[i].u8 >> 12) & 0x3f) << 16) |
				(((tbl[i].u8 >> 6) & 0x3f) << 8) |
				(tbl[i].u8 & 0x3f);
		}
		*/

		if (i > 0x7f && l != 0)
			printf("\t{  0x%08X, 0x%02X  },\n", l, i);
	}
	
	if (ascii_only)
		printf("\t{  0x%08X, 0x%02X  },\n", 0, 0);

        fprintf(stderr, "%s: make sure you sort the result by using\n\n\
\tsort -k 1 -t ',' result_file\n\n\
since iconv module that will include the result table uses binary search.\n",
av[0]);
}
