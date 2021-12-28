/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Palmtop Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef MPEG3CSS_H
#define MPEG3CSS_H


#include "mpeg3private.inc"

struct mpeg3_block 
{
	unsigned char b[5];
};

struct mpeg3_playkey {
	int offset;
	unsigned char key[5];
};

typedef struct
{
	int encrypted;
	char device_path[MPEG3_STRLEN];    /* Device the file is located on */
	unsigned char disk_key[MPEG3_DVD_PACKET_SIZE];
	unsigned char title_key[5];
	unsigned char challenge[10];
	struct mpeg3_block key1;
	struct mpeg3_block key2;
	struct mpeg3_block keycheck;
	int varient;
	int fd;
	char path[MPEG3_STRLEN];
} mpeg3_css_t;

#endif
