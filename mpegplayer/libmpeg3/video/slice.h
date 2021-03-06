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
#ifndef SLICE_H
#define SLICE_H

#ifndef _WIN32
#include <pthread.h>
#endif

/* Array of these feeds the slice decoders */
typedef struct
{
	unsigned char *data;   /* Buffer for holding the slice data */
	int buffer_size;         /* Size of buffer */
	int buffer_allocation;   /* Space allocated for buffer  */
	int current_position;    /* Position in buffer */
	unsigned MPEG3_INT32 bits;
	int bits_size;
	pthread_mutex_t completion_lock; /* Lock slice until completion */
	int done;           /* Signal for slice decoder to skip */
} mpeg3_slice_buffer_t;

/* Each slice decoder */
typedef struct
{
	struct mpeg3video_rec *video;
	mpeg3_slice_buffer_t *slice_buffer;

	int thread_number;      /* Number of this thread */
	int current_buffer;     /* Buffer this slice decoder is on */
	int buffer_step;        /* Number of buffers to skip */
	int last_buffer;        /* Last buffer this decoder should process */
	int fault;
	int done;
	int quant_scale;
	int pri_brk;                  /* slice/macroblock */
	short block[12][64];
	int sparse[12];
	pthread_t tid;   /* ID of thread */
	pthread_mutex_t input_lock, output_lock;
} mpeg3_slice_t;

#define mpeg3slice_fillbits(buffer, nbits) \
	while(((mpeg3_slice_buffer_t*)(buffer))->bits_size < (nbits)) \
	{ \
		if(((mpeg3_slice_buffer_t*)(buffer))->current_position < ((mpeg3_slice_buffer_t*)(buffer))->buffer_size) \
		{ \
			((mpeg3_slice_buffer_t*)(buffer))->bits <<= 8; \
			((mpeg3_slice_buffer_t*)(buffer))->bits |= ((mpeg3_slice_buffer_t*)(buffer))->data[((mpeg3_slice_buffer_t*)(buffer))->current_position++]; \
		} \
		((mpeg3_slice_buffer_t*)(buffer))->bits_size += 8; \
	}

#define mpeg3slice_flushbits(buffer, nbits) \
	{ \
		mpeg3slice_fillbits((buffer), (nbits)); \
		((mpeg3_slice_buffer_t*)(buffer))->bits_size -= (nbits); \
	}

#define mpeg3slice_flushbit(buffer) \
{ \
	if(((mpeg3_slice_buffer_t*)(buffer))->bits_size) \
		((mpeg3_slice_buffer_t*)(buffer))->bits_size--; \
	else \
	if(((mpeg3_slice_buffer_t*)(buffer))->current_position < ((mpeg3_slice_buffer_t*)(buffer))->buffer_size) \
	{ \
		((mpeg3_slice_buffer_t*)(buffer))->bits = \
			((mpeg3_slice_buffer_t*)(buffer))->data[((mpeg3_slice_buffer_t*)(buffer))->current_position++]; \
		((mpeg3_slice_buffer_t*)(buffer))->bits_size = 7; \
	} \
}

extern inline unsigned int mpeg3slice_getbit(mpeg3_slice_buffer_t *buffer)
{
	if(buffer->bits_size)
		return (buffer->bits >> (--buffer->bits_size)) & 0x1;
	else
	if(buffer->current_position < buffer->buffer_size)
	{
		buffer->bits = buffer->data[buffer->current_position++];
		buffer->bits_size = 7;
		return (buffer->bits >> 7) & 0x1;
	}
	return 0; // WWA - stop warn
}

extern inline unsigned int mpeg3slice_getbits2(mpeg3_slice_buffer_t *buffer)
{
	if(buffer->bits_size >= 2)
		return (buffer->bits >> (buffer->bits_size -= 2)) & 0x3;
	else
	if(buffer->current_position < buffer->buffer_size)
	{
		buffer->bits <<= 8;
		buffer->bits |= buffer->data[buffer->current_position++];
		buffer->bits_size += 6;
		return (buffer->bits >> buffer->bits_size)  & 0x3;
	}
	return 0; // WWA - stop warn
}

extern inline unsigned int mpeg3slice_getbyte(mpeg3_slice_buffer_t *buffer)
{
	if(buffer->bits_size >= 8)
		return (buffer->bits >> (buffer->bits_size -= 8)) & 0xff;
	else
	if(buffer->current_position < buffer->buffer_size)
	{
		buffer->bits <<= 8;
		buffer->bits |= buffer->data[buffer->current_position++];
		return (buffer->bits >> buffer->bits_size) & 0xff;
	}
	return 0; // WWA - stop warn
}


extern inline unsigned int mpeg3slice_getbits(mpeg3_slice_buffer_t *slice_buffer, int bits)
{
	if(bits == 1) return mpeg3slice_getbit(slice_buffer);
	mpeg3slice_fillbits(slice_buffer, bits);
	return (slice_buffer->bits >> (slice_buffer->bits_size -= bits)) & (0xffffffff >> (32 - bits));
}

extern inline unsigned int mpeg3slice_showbits16(mpeg3_slice_buffer_t *buffer)
{
	if(buffer->bits_size >= 16)
		return (buffer->bits >> (buffer->bits_size - 16)) & 0xffff;
	else
	if(buffer->current_position < buffer->buffer_size)
	{
		buffer->bits <<= 16;
		buffer->bits_size += 16;
		buffer->bits |= (unsigned int)buffer->data[buffer->current_position++] << 8;
		buffer->bits |= buffer->data[buffer->current_position++];
		return (buffer->bits >> (buffer->bits_size - 16)) & 0xffff;
	}
	return 0; // WWA - stop warn
}

extern inline unsigned int mpeg3slice_showbits9(mpeg3_slice_buffer_t *buffer)
{
	if(buffer->bits_size >= 9)
		return (buffer->bits >> (buffer->bits_size - 9)) & 0x1ff;
	else
	if(buffer->current_position < buffer->buffer_size)
	{
		buffer->bits <<= 16;
		buffer->bits_size += 16;
		buffer->bits |= (unsigned int)buffer->data[buffer->current_position++] << 8;
		buffer->bits |= buffer->data[buffer->current_position++];
		return (buffer->bits >> (buffer->bits_size - 9)) & 0x1ff;
	}
	return 0; // WWA - stop warn
}

extern inline unsigned int mpeg3slice_showbits5(mpeg3_slice_buffer_t *buffer)
{
	if(buffer->bits_size >= 5)
		return (buffer->bits >> (buffer->bits_size - 5)) & 0x1f;
	else
	if(buffer->current_position < buffer->buffer_size)
	{
		buffer->bits <<= 8;
		buffer->bits_size += 8;
		buffer->bits |= buffer->data[buffer->current_position++];
		return (buffer->bits >> (buffer->bits_size - 5)) & 0x1f;
	}
	return 0; // WWA - stop warn
}

extern inline unsigned int mpeg3slice_showbits(mpeg3_slice_buffer_t *slice_buffer, int bits)
{
	mpeg3slice_fillbits(slice_buffer, bits);
	return (slice_buffer->bits >> (slice_buffer->bits_size - bits)) & (0xffffffff >> (32 - bits));
}

#endif
