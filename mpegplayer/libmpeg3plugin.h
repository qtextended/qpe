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
#ifndef LIBMPEG3_PLUGIN_H 
#define LIBMPEG3_PLUGIN_H


#include <qstring.h>
#include "libmpeg3/libmpeg3.h"
#include "mediaplayerplugininterface.h"


class LibMpeg3Plugin : public MediaPlayerPlugin {

public:
    LibMpeg3Plugin() { file = NULL; }
    ~LibMpeg3Plugin() { close(); }

    bool isFileSupported( QString& ) { return FALSE; }
    bool open( QString& ) { return FALSE; }

    bool isFileSupported( char *path ) { return mpeg3_check_sig( path ) == 1; }
    bool open( char *path ) { file = mpeg3_open( path ); return file != NULL; }
    bool close() { if ( file ) { int r = mpeg3_close( file ); file = NULL; return r == 1; } return FALSE; }
    bool isOpen() { return file != NULL; }

    // If decoder doesn't support audio then return 0 here
    int audioStreams() { return file ? mpeg3_total_astreams( file ) : 0; }
    int audioChannels( int stream ) { return file ? mpeg3_audio_channels( file, stream ) : 0; }
    int audioFrequency( int stream ) { return file ? mpeg3_sample_rate( file, stream ) : 0; }
    int audioSamples( int stream ) { return file ? mpeg3_audio_samples( file, stream ) : 0; } 
    bool audioSetSample( long sample, int stream ) { return file ? mpeg3_set_sample( file, sample, stream) == 1 : FALSE; }
    long audioGetSample( int stream ) { return file ? mpeg3_get_sample( file, stream ) : 0; }
    bool audioReadSamples( short *output, int channel, long samples, int stream );
    bool audioReReadSamples( short *output, int channel, long samples, int stream );
    bool audioReadStereoSamples( short *output, long samples, int stream );

    // If decoder doesn't support video then return 0 here
    int videoStreams() { return file ? mpeg3_total_vstreams( file ) : 0; }
    int videoWidth( int stream ) { return file ? mpeg3_video_width( file, stream ) : 0; }
    int videoHeight( int stream ) { return file ? mpeg3_video_height( file, stream ) : 0; }
    double videoFrameRate( int stream ) { return file ? mpeg3_frame_rate( file, stream ) : 0.0; }
    int videoFrames( int stream ) { return file ? mpeg3_video_frames( file, stream ) : 0; }
    bool videoSetFrame( long frame, int stream ) { return file ? mpeg3_set_frame( file, frame, stream) == 1 : FALSE; }
    long videoGetFrame( int stream ) { return file ? mpeg3_get_frame( file, stream ) : 0; }
    bool videoReadFrame( unsigned char **output_rows, int in_x, int in_y, int in_w, int in_h, int color_model, int stream );
    bool videoReadScaledFrame( unsigned char **output_rows, int in_x, int in_y, int in_w, int in_h, int out_w, int out_h, int color_model, int stream );
    bool videoReadYUVFrame( char *y_output, char *u_output, char *v_output, int in_x, int in_y, int in_w, int in_h, int stream );

    // Profiling
    double getTime() { return file ? mpeg3_get_time( file ) : 0.0; }

    // Ignore if these aren't supported
    bool setSMP( int cpus ) { return file ? mpeg3_set_cpus( file, cpus ) == 1 : FALSE; }
    bool setMMX( bool useMMX ) { return file ? mpeg3_set_mmx( file, useMMX ) == 1 : FALSE; }

    // Capabilities
    bool supportsAudio() { return TRUE; }
    bool supportsVideo() { return TRUE; }
    bool supportsYUV() { return TRUE; }
    bool supportsMMX() { return TRUE; }
    bool supportsSMP() { return TRUE; }
    bool supportsStereo() { return TRUE; }
    bool supportsScaling() { return TRUE; }

private:
    mpeg3_t *file;

};


#endif

