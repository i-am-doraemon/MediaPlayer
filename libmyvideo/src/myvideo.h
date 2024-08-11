#ifndef _MYPLAY_H_
#define _MYPLAY_H_

#include <glib.h>
#include <windows.h>

#define MYVIDEO_API __declspec(dllexport)
#define MYVIDEO_API_CALL __stdcall

#define OK 0
#define NG_GST_FAILED_MAKE_ELEMENT       -1000
#define NG_GST_FAILED_ALLOCATE_PIPELINE  -1002
#define NG_GST_FAILED_LINK_ELEMENT       -1003
#define NG_GST_FAILED_FIND_ELEMENT       -1004
#define NG_GST_FAILED_PLAYBACK           -1005

#define NG_FFMPEG_FAILED_CREATE_AVFORMAT   -2000
#define NG_FFMPEG_FAILED_FIND_STREAMINFO   -2001
#define NG_FFMPEG_FAILED_FIND_VIDEO_STREAM -2002

MYVIDEO_API int MYVIDEO_API_CALL myvideo_initialize();

MYVIDEO_API int MYVIDEO_API_CALL myvideo_get_codec_id(const char* filename);

MYVIDEO_API int MYVIDEO_API_CALL myvideo_create_m2ts_pipeline(guint64** signal_chain);

MYVIDEO_API int MYVIDEO_API_CALL myvideo_create_h264_pipeline(guint64** signal_chain);

MYVIDEO_API int MYVIDEO_API_CALL myvideo_create_h265_pipeline(guint64** signal_chain);

MYVIDEO_API int MYVIDEO_API_CALL myvideo_delete_pipeline(guint64* signal_chain);

MYVIDEO_API int MYVIDEO_API_CALL myvideo_playback(guint64* signal_chain, const char* filename, HWND handle);

MYVIDEO_API int MYVIDEO_API_CALL myvideo_stop(guint64* signal_chain);

MYVIDEO_API int MYVIDEO_API_CALL myvideo_get_duration(guint64* signal_chain);

MYVIDEO_API int MYVIDEO_API_CALL myvideo_get_position(guint64* signal_chain);

#endif