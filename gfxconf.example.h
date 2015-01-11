/**
 * This file has a different license to the rest of the uGFX system.
 * You can copy, modify and distribute this file as you see fit.
 * You do not need to publish your source modifications to this file.
 * The only thing you are not permitted to do is to relicense it
 * under a different license.
 */

/**
 * Copy this file into your project directory and rename it as gfxconf.h
 * Edit your copy to turn on the uGFX features you want to use.
 * The values below are the defaults. You should delete anything
 * you are leaving as default.
 *
 * Please use spaces instead of tabs in this file.
 */

#ifndef _GFXCONF_H
#define _GFXCONF_H


///////////////////////////////////////////////////////////////////////////
// GOS - One of these must be defined, preferably in your Makefile       //
///////////////////////////////////////////////////////////////////////////
//#define GFX_USE_OS_CHIBIOS                           TRUE
#define GFX_USE_OS_FREERTOS                          TRUE
//#define GFX_FREERTOS_USE_TRACE                   TRUE
//#define GFX_USE_OS_WIN32                             TRUE
//#define GFX_USE_OS_LINUX                             TRUE
//#define GFX_USE_OS_OSX                               TRUE


///////////////////////////////////////////////////////////////////////////
// GDISP                                                                 //
///////////////////////////////////////////////////////////////////////////
#define GFX_USE_GDISP                                TRUE

#define GDISP_NEED_AUTOFLUSH                         TRUE
#define GDISP_NEED_TIMERFLUSH                        TRUE
#define GDISP_NEED_VALIDATION                        TRUE
#define GDISP_NEED_CLIP                              TRUE
#define GDISP_NEED_CIRCLE                            TRUE
#define GDISP_NEED_ELLIPSE                           TRUE
#define GDISP_NEED_ARC                               TRUE
#define GDISP_NEED_CONVEX_POLYGON                    TRUE
#define GDISP_NEED_SCROLL                            TRUE
#define GDISP_NEED_PIXELREAD                         TRUE
#define GDISP_NEED_CONTROL                           TRUE
#define GDISP_NEED_QUERY                             TRUE
#define GDISP_NEED_MULTITHREAD                       TRUE
#define GDISP_NEED_STREAMING                         TRUE
#define GDISP_NEED_TEXT                              TRUE
    #define GDISP_NEED_ANTIALIAS                     TRUE
    #define GDISP_NEED_UTF8                          TRUE
    #define GDISP_NEED_TEXT_KERNING                  TRUE
    #define GDISP_INCLUDE_FONT_UI1                   TRUE
    #define GDISP_INCLUDE_FONT_UI2                   TRUE
    #define GDISP_INCLUDE_FONT_LARGENUMBERS          TRUE
    #define GDISP_INCLUDE_FONT_DEJAVUSANS10          TRUE
    #define GDISP_INCLUDE_FONT_DEJAVUSANS12          TRUE
    #define GDISP_INCLUDE_FONT_DEJAVUSANS16          TRUE
    #define GDISP_INCLUDE_FONT_DEJAVUSANS24          TRUE
    #define GDISP_INCLUDE_FONT_DEJAVUSANS32          TRUE
    #define GDISP_INCLUDE_FONT_DEJAVUSANSBOLD12      TRUE
    #define GDISP_INCLUDE_FONT_FIXED_10X20           TRUE
    #define GDISP_INCLUDE_FONT_FIXED_7X14            TRUE
    #define GDISP_INCLUDE_FONT_FIXED_5X8             TRUE
    #define GDISP_INCLUDE_FONT_DEJAVUSANS12_AA       TRUE
    #define GDISP_INCLUDE_FONT_DEJAVUSANS16_AA       TRUE
    #define GDISP_INCLUDE_FONT_DEJAVUSANS24_AA       TRUE
    #define GDISP_INCLUDE_FONT_DEJAVUSANS32_AA       TRUE
    #define GDISP_INCLUDE_FONT_DEJAVUSANSBOLD12_AA   TRUE
    #define GDISP_INCLUDE_USER_FONTS                 TRUE

#define GDISP_NEED_IMAGE                             TRUE
    #define GDISP_NEED_IMAGE_NATIVE                  TRUE
    #define GDISP_NEED_IMAGE_GIF                     TRUE
    #define GDISP_NEED_IMAGE_BMP                     TRUE
        #define GDISP_NEED_IMAGE_BMP_1               TRUE
        #define GDISP_NEED_IMAGE_BMP_4               TRUE
        #define GDISP_NEED_IMAGE_BMP_4_RLE           TRUE
        #define GDISP_NEED_IMAGE_BMP_8               TRUE
        #define GDISP_NEED_IMAGE_BMP_8_RLE           TRUE
        #define GDISP_NEED_IMAGE_BMP_16              TRUE
        #define GDISP_NEED_IMAGE_BMP_24              TRUE
        #define GDISP_NEED_IMAGE_BMP_32              TRUE
    #define GDISP_NEED_IMAGE_JPG                     TRUE
    #define GDISP_NEED_IMAGE_PNG                     TRUE
    #define GDISP_NEED_IMAGE_ACCOUNTING              TRUE

#define GDISP_NEED_STARTUP_LOGO                      TRUE

#define GDISP_DEFAULT_ORIENTATION                    GDISP_ROTATE_LANDSCAPE
#define GDISP_LINEBUF_SIZE                           128

#define GDISP_TOTAL_DISPLAYS                         1
    #if GDISP_TOTAL_DISPLAYS > 1
        #define GDISP_HARDWARE_STREAM_WRITE          TRUE
        #define GDISP_HARDWARE_STREAM_READ           TRUE
        #define GDISP_HARDWARE_STREAM_POS            TRUE
        #define GDISP_HARDWARE_DRAWPIXEL             TRUE
        #define GDISP_HARDWARE_CLEARS                TRUE
        #define GDISP_HARDWARE_FILLS                 TRUE
        #define GDISP_HARDWARE_BITFILLS              TRUE
        #define GDISP_HARDWARE_SCROLL                TRUE
        #define GDISP_HARDWARE_PIXELREAD             TRUE
        #define GDISP_HARDWARE_CONTROL               TRUE
        #define GDISP_HARDWARE_QUERY                 TRUE
        #define GDISP_HARDWARE_CLIP                  TRUE
    #endif

#define GDISP_TOTAL_CONTROLLERS                      1
    #if GDISP_TOTAL_CONTROLLERS > 1
        #define GDISP_CONTROLLER_LIST                GDISPVMT_Win32, GDISPVMT_Win32
        #define GDISP_CONTROLLER_DISPLAYS            1, 1
        #define GDISP_PIXELFORMAT                    GDISP_PIXELFORMAT_RGB888
    #endif

#define GDISP_USE_GFXNET                             TRUE
    #define GDISP_GFXNET_PORT                        13001
    #define GDISP_GFXNET_CUSTOM_LWIP_STARTUP         TRUE
    #define GDISP_DONT_WAIT_FOR_NET_DISPLAY          TRUE
    #define GDISP_GFXNET_UNSAFE_SOCKETS              TRUE


///////////////////////////////////////////////////////////////////////////
// GWIN                                                                  //
///////////////////////////////////////////////////////////////////////////
#define GFX_USE_GWIN                                 TRUE

#define GWIN_NEED_WINDOWMANAGER                      TRUE

#define GWIN_NEED_CONSOLE                            TRUE
    #define GWIN_CONSOLE_USE_HISTORY                 TRUE
        #define GWIN_CONSOLE_HISTORY_AVERAGING       TRUE
        #define GWIN_CONSOLE_HISTORY_ATCREATE        TRUE
    #define GWIN_CONSOLE_ESCSEQ                      TRUE
	#define GWIN_CONSOLE_USE_BASESTREAM              TRUE
    #define GWIN_CONSOLE_USE_FLOAT                   TRUE
#define GWIN_NEED_GRAPH                              TRUE

#define GWIN_NEED_WIDGET                             TRUE
    #define GWIN_NEED_LABEL                          TRUE
        #define GWIN_LABEL_ATTRIBUTE                 TRUE
    #define GWIN_NEED_BUTTON                         TRUE
        #define GWIN_BUTTON_LAZY_RELEASE             TRUE
    #define GWIN_NEED_SLIDER                         TRUE
    #define GWIN_NEED_CHECKBOX                       TRUE
    #define GWIN_NEED_IMAGE                          TRUE
        #define GWIN_NEED_IMAGE_ANIMATION            TRUE
    #define GWIN_NEED_RADIO                          TRUE
    #define GWIN_NEED_LIST                           TRUE
        #define GWIN_NEED_LIST_IMAGES                TRUE
    #define GWIN_NEED_PROGRESSBAR                    TRUE
        #define GWIN_PROGRESSBAR_AUTO                TRUE
	#define GWIN_FLAT_STYLING                        TRUE


///////////////////////////////////////////////////////////////////////////
// GEVENT                                                                //
///////////////////////////////////////////////////////////////////////////
#define GFX_USE_GEVENT                               TRUE

#define GEVENT_ASSERT_NO_RESOURCE                    TRUE
#define GEVENT_MAXIMUM_SIZE                          32
#define GEVENT_MAX_SOURCE_LISTENERS                  32


///////////////////////////////////////////////////////////////////////////
// GTIMER                                                                //
///////////////////////////////////////////////////////////////////////////
#define GFX_USE_GTIMER                               TRUE

#define GTIMER_THREAD_PRIORITY                       HIGH_PRIORITY
#define GTIMER_THREAD_WORKAREA_SIZE                  2048


///////////////////////////////////////////////////////////////////////////
// GQUEUE                                                                //
///////////////////////////////////////////////////////////////////////////
#define GFX_USE_GQUEUE                               TRUE

#define GQUEUE_NEED_ASYNC                            TRUE
#define GQUEUE_NEED_GSYNC                            TRUE
#define GQUEUE_NEED_FSYNC                            TRUE
#define GQUEUE_NEED_BUFFERS                          TRUE

///////////////////////////////////////////////////////////////////////////
// GINPUT                                                                //
///////////////////////////////////////////////////////////////////////////
#define GFX_USE_GINPUT                               TRUE

#define GINPUT_NEED_MOUSE                            TRUE
#define GINPUT_NEED_KEYBOARD                         TRUE
#define GINPUT_NEED_TOGGLE                           FALSE
#define GINPUT_NEED_DIAL                             TRUE


///////////////////////////////////////////////////////////////////////////
// GFILE                                                                 //
///////////////////////////////////////////////////////////////////////////
#define GFX_USE_GFILE                                TRUE

#define GFILE_NEED_PRINTG                            TRUE
#define GFILE_NEED_SCANG                             TRUE
#define GFILE_NEED_STRINGS                           TRUE
#define GFILE_NEED_STDIO                             TRUE
    #define GFILE_ALLOW_FLOATS                       TRUE
    #define GFILE_ALLOW_DEVICESPECIFIC               TRUE
    #define GFILE_MAX_GFILES                         3

#define GFILE_NEED_MEMFS                             FALSE
#define GFILE_NEED_ROMFS                             FALSE
#define GFILE_NEED_RAMFS                             FALSE
#define GFILE_NEED_FATFS                             FALSE
#define GFILE_NEED_NATIVEFS                          FALSE
#define GFILE_NEED_CHBIOSFS                          FALSE


///////////////////////////////////////////////////////////////////////////
// GADC                                                                  //
///////////////////////////////////////////////////////////////////////////
#define GFX_USE_GADC                                 FALSE

#define GADC_MAX_LOWSPEED_DEVICES                    4


///////////////////////////////////////////////////////////////////////////
// GAUDIO                                                                //
///////////////////////////////////////////////////////////////////////////
#define GFX_USE_GAUDIO                               FALSE
    #define GAUDIO_NEED_PLAY                         TRUE
    #define GAUDIO_NEED_RECORD                       TRUE


///////////////////////////////////////////////////////////////////////////
// GMISC                                                                 //
///////////////////////////////////////////////////////////////////////////
#define GFX_USE_GMISC                                TRUE

#define GMISC_NEED_ARRAYOPS                          TRUE
#define GMISC_NEED_FASTTRIG                          TRUE
#define GMISC_NEED_FIXEDTRIG                         TRUE
#define GMISC_NEED_INVSQRT                           TRUE
    #define GMISC_INVSQRT_MIXED_ENDIAN               TRUE
    #define GMISC_INVSQRT_REAL_SLOW                  TRUE


#endif /* _GFXCONF_H */
