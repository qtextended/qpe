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
#ifndef LIBMPEG3_H
#define LIBMPEG3_H

#include "mpeg3private.h"

#if defined(__cplusplus)
extern "C" {
#endif

/* Supported color models for mpeg3_read_frame */
#define MPEG3_RGB565 2
#define MPEG3_BGR888 0
#define MPEG3_BGRA8888 1
#define MPEG3_RGB888 3
#define MPEG3_RGBA8888 4
#define MPEG3_RGBA16161616 5

/* Color models for the 601 to RGB conversion */
/* 601 not implemented for scalar code */
#define MPEG3_601_RGB565 11
#define MPEG3_601_BGR888 7
#define MPEG3_601_BGRA8888 8
#define MPEG3_601_RGB888 9
#define MPEG3_601_RGBA8888 10

/* Check for file compatibility.  Return 1 if compatible. */
LIBMPEG_EXPORT int mpeg3_check_sig(char *path);

/* Open the MPEG3 stream. */
LIBMPEG_EXPORT mpeg3_t* mpeg3_open(char *path);

/* Open the MPEG3 stream and copy the tables from an already open stream. */
/* Eliminates the initial timecode search. */
LIBMPEG_EXPORT mpeg3_t* mpeg3_open_copy(char *path, mpeg3_t *old_file);
LIBMPEG_EXPORT int mpeg3_close(mpeg3_t *file);

/* Performance */
LIBMPEG_EXPORT int mpeg3_set_cpus(mpeg3_t *file, int cpus);
LIBMPEG_EXPORT int mpeg3_set_mmx(mpeg3_t *file, int use_mmx);

/* Query the MPEG3 stream about audio. */
LIBMPEG_EXPORT int mpeg3_has_audio(mpeg3_t *file);
LIBMPEG_EXPORT int mpeg3_total_astreams(mpeg3_t *file);             /* Number of multiplexed audio streams */
LIBMPEG_EXPORT int mpeg3_audio_channels(mpeg3_t *file, int stream);
LIBMPEG_EXPORT int mpeg3_sample_rate(mpeg3_t *file, int stream);

/* Total length obtained from the timecode. */
/* For DVD files, this is unreliable. */
LIBMPEG_EXPORT long mpeg3_audio_samples(mpeg3_t *file, int stream); 
LIBMPEG_EXPORT int mpeg3_set_sample(mpeg3_t *file, long sample, int stream);    /* Seek to a sample */
LIBMPEG_EXPORT long mpeg3_get_sample(mpeg3_t *file, int stream);    /* Tell current position */

/* Read a PCM buffer of audio from 1 channel and advance the position. */
/* Return a 1 if error. */
/* Stream defines the number of the multiplexed stream to read. */
LIBMPEG_EXPORT int mpeg3_read_audio(mpeg3_t *file, 
		mpeg3_real_t *output_f,      /* Pointer to pre-allocated buffer of floats */
		short *output_i,      /* Pointer to pre-allocated buffer of int16's */
		int channel,          /* Channel to decode */
		long samples,         /* Number of samples to decode */
		int stream);          /* Stream containing the channel */

/* Reread the last PCM buffer from a different channel and advance the position */
LIBMPEG_EXPORT int mpeg3_reread_audio(mpeg3_t *file, 
		mpeg3_real_t *output_f,      /* Pointer to pre-allocated buffer of floats */
		short *output_i,      /* Pointer to pre-allocated buffer of int16's */
		int channel,          /* Channel to decode */
		long samples,         /* Number of samples to decode */
		int stream);          /* Stream containing the channel */

/* Read the next compressed audio chunk.  Store the size in size and return a  */
/* 1 if error. */
/* Stream defines the number of the multiplexed stream to read. */
LIBMPEG_EXPORT int mpeg3_read_audio_chunk(mpeg3_t *file, 
		unsigned char *output, 
		long *size, 
		long max_size,
		int stream);

/* Query the stream about video. */
LIBMPEG_EXPORT int mpeg3_has_video(mpeg3_t *file);
LIBMPEG_EXPORT int mpeg3_total_vstreams(mpeg3_t *file);            /* Number of multiplexed video streams */
LIBMPEG_EXPORT int mpeg3_video_width(mpeg3_t *file, int stream);
LIBMPEG_EXPORT int mpeg3_video_height(mpeg3_t *file, int stream);
LIBMPEG_EXPORT float mpeg3_frame_rate(mpeg3_t *file, int stream);  /* Frames/sec */

/* Total length.   */
/* For DVD files, this is 1 indicating only percentage seeking is available. */
LIBMPEG_EXPORT long mpeg3_video_frames(mpeg3_t *file, int stream);
LIBMPEG_EXPORT int mpeg3_set_frame(mpeg3_t *file, long frame, int stream); /* Seek to a frame */
LIBMPEG_EXPORT int mpeg3_skip_frames();
LIBMPEG_EXPORT long mpeg3_get_frame(mpeg3_t *file, int stream);            /* Tell current position */

/* Seek all the tracks based on a percentage of the total bytes in the  */
/* file or the total */
/* time in a toc if one exists.  Percentage is a 0 to 1 double. */
/* This eliminates the need for tocs and 64 bit longs but doesn't  */
/* give frame accuracy. */
LIBMPEG_EXPORT int mpeg3_seek_percentage(mpeg3_t *file, double percentage);
LIBMPEG_EXPORT double mpeg3_tell_percentage(mpeg3_t *file);
LIBMPEG_EXPORT int mpeg3_previous_frame(mpeg3_t *file, int stream);
LIBMPEG_EXPORT int mpeg3_end_of_audio(mpeg3_t *file, int stream);
LIBMPEG_EXPORT int mpeg3_end_of_video(mpeg3_t *file, int stream);

/* Give the seconds time in the last packet read */
LIBMPEG_EXPORT double mpeg3_get_time(mpeg3_t *file);

/* Read a frame.  The dimensions of the input area and output frame must be supplied. */
/* The frame is taken from the input area and scaled to fit the output frame in 1 step. */
/* Stream defines the number of the multiplexed stream to read. */
/* The last row of **output_rows must contain 4 extra bytes for scratch work. */
LIBMPEG_EXPORT int mpeg3_read_frame(mpeg3_t *file, 
		unsigned char **output_rows, /* Array of pointers to the start of each output row */
		int in_x,                    /* Location in input frame to take picture */
		int in_y, 
		int in_w, 
		int in_h, 
		int out_w,                   /* Dimensions of output_rows */
		int out_h, 
		int color_model,             /* One of the color model #defines */
		int stream);

/* Read a YUV frame.  The 3 planes are copied into the y, u, and v buffers provided. */
/* The input is cropped to the dimensions given but not scaled. */
LIBMPEG_EXPORT int mpeg3_read_yuvframe(mpeg3_t *file,
		char *y_output,
		char *u_output,
		char *v_output,
		int in_x,
		int in_y,
		int in_w,
		int in_h,
		int stream);

LIBMPEG_EXPORT int mpeg3_drop_frames(mpeg3_t *file, long frames, int stream);

/* Read the next compressed frame including headers. */
/* Store the size in size and return a 1 if error. */
/* Stream defines the number of the multiplexed stream to read. */
LIBMPEG_EXPORT int mpeg3_read_video_chunk(mpeg3_t *file, 
		unsigned char *output, 
		long *size, 
		long max_size,
		int stream);

/* Master control */
LIBMPEG_EXPORT int mpeg3_total_programs();
LIBMPEG_EXPORT int mpeg3_set_program(int program);

#if defined(__cplusplus)
}
#endif

#endif
