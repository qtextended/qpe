#include "qconfig-qpe.h"
#define QT_NO_NETWORK
#define NO_CHECK

//#define QT_DEMO_SINGLE_FLOPPY


#ifdef QT_DEMO_SINGLE_FLOPPY 
// VGA16 is all we need
#define QT_NO_QWS_TRANSFORMED
#define QT_NO_QWS_MACH64
#define QT_NO_QWS_VOODOO3
#define QT_NO_QWS_MATROX
#define QT_NO_QWS_DEPTH_16
#define QT_NO_QWS_DEPTH_32
#define QT_NO_SOUND
#endif

#ifdef QT_NO_QWS_DEPTH_16
// Needed for mpegplayer on 16bpp
#define QT_NO_IMAGE_16_BIT
#endif
