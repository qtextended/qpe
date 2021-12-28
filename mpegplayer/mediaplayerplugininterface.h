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
#ifndef MEDIA_PLAYER_PLUGIN_INTERFACE_H
#define MEDIA_PLAYER_PLUGIN_INTERFACE_H

#include <qcom.h>

#ifndef QT_NO_COMPONENT
// ### this needs to be properly generated right?
// {7CA34D0C-C637-4865-A667-7D4CD8A70408}
# ifndef IID_MediaPlayerPlugin
#  define IID_MediaPlayerPlugin QUuid( 0x7ca34d0c, 0xc637, 0x4865, 0xa6, 0x67, 0x7d, 0x4c, 0xd8, 0xa7, 0x04, 0x08 )
# endif
#endif

class MediaPlayerPlugin {

public:
    virtual ~MediaPlayerPlugin() { };
    
    virtual bool isFileSupported( QString& file ) = 0;
    virtual bool open( QString& file ) = 0;
    virtual bool close() = 0;
    virtual bool isOpen() = 0;

    // If decoder doesn't support audio then return 0 here
    virtual int audioStreams() = 0;
    virtual int audioChannels( int stream ) = 0;
    virtual int audioFrequency( int stream ) = 0;
    virtual int audioSamples( int stream ) = 0;
    virtual bool audioSetSample( long sample, int stream ) = 0;
    virtual long audioGetSample( int stream ) = 0;
    virtual bool audioReadSamples( short *samples, int channel, long samples, int stream ) = 0;
    virtual bool audioReReadSamples( short *samples, int channel, long samples, int stream ) = 0;
    virtual bool audioReadStereoSamples( short *samples, long samples, int stream ) = 0;

    // If decoder doesn't support video then return 0 here
    virtual int videoStreams() = 0;
    virtual int videoWidth( int stream ) = 0;
    virtual int videoHeight( int stream ) = 0;
    virtual double videoFrameRate( int stream ) = 0;
    virtual int videoFrames( int stream ) = 0;
    virtual bool videoSetFrame( long sample, int stream ) = 0;
    virtual long videoGetFrame( int stream ) = 0;
    virtual bool videoReadFrame( unsigned char **output_rows, int in_x, int in_y, int in_w, int in_h, int color_model, int stream ) = 0;
    virtual bool videoReadScaledFrame( unsigned char **output_rows, int in_x, int in_y, int in_w, int in_h, int out_w, int out_h, int color_model, int stream ) = 0;
    virtual bool videoReadYUVFrame( char *y_output, char *u_output, char *v_output, int in_x, int in_y, int in_w, int in_h, int stream ) = 0;

    // Profiling
    virtual double getTime() = 0;

    // Ignore if these aren't supported
    virtual bool setSMP( int cpus ) = 0;
    virtual bool setMMX( bool useMMX ) = 0;

    // Capabilities
    virtual bool supportsAudio() = 0;
    virtual bool supportsVideo() = 0;
    virtual bool supportsYUV() = 0;
    virtual bool supportsMMX() = 0;
    virtual bool supportsSMP() = 0;
    virtual bool supportsStereo() = 0;
    virtual bool supportsScaling() = 0;

};

struct MediaPlayerPluginInterface : public QUnknownInterface
{
    virtual MediaPlayerPlugin *plugin() = 0;
};

#endif

