/**********************************************************************
** Copyright (C) 2001 Trolltech AS.  All rights reserved.
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
#include "libmpeg3plugin.h"


bool LibMpeg3Plugin::audioReadSamples( short *output, int channel, long samples, int stream ) {
    return file ? mpeg3_read_audio( file, 0, output, channel, samples, stream ) == 1 : FALSE; 
}


bool LibMpeg3Plugin::audioReReadSamples( short *output, int channel, long samples, int stream ) {
    return file ? mpeg3_reread_audio( file, 0, output, channel, samples, stream ) == 1 : FALSE; 
}


bool LibMpeg3Plugin::audioReadStereoSamples( short *output, long samples, int stream ) {
    bool err = FALSE;
    if ( file ) {
	short left[samples];
	short right[samples];
	err = mpeg3_read_audio  ( file, 0,  left, 0, samples, stream ) == 1;
	if ( err == FALSE ) {
	    err = mpeg3_reread_audio( file, 0, right, 1, samples, stream ) == 1;
	}
	for ( int j = 0; j < samples; j++ ) {
	    output[j*2+0] =  left[j];
	    output[j*2+1] = right[j];
	}
    }
    return err;
}


bool LibMpeg3Plugin::videoReadFrame( unsigned char **output_rows, int in_x, int in_y, int in_w, int in_h, int color_model, int stream ) {
    return file ? mpeg3_read_frame( file, output_rows, in_x, in_y, in_w, in_h, in_w, in_h, color_model, stream ) == 1 : FALSE;
}


bool LibMpeg3Plugin::videoReadScaledFrame( unsigned char **output_rows, int in_x, int in_y, int in_w, int in_h, int out_w, int out_h, int color_model, int stream ) {
    return file ? mpeg3_read_frame( file, output_rows, in_x, in_y, in_w, in_h, out_w, out_h, color_model, stream ) == 1 : FALSE;
}


bool LibMpeg3Plugin::videoReadYUVFrame( char *y_output, char *u_output, char *v_output, int in_x, int in_y, int in_w, int in_h, int stream ) {
    return file ? mpeg3_read_yuvframe( file, y_output, u_output, v_output, in_x, in_y, in_w, in_h, stream ) == 1 : FALSE;
}


