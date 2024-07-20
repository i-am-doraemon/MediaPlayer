#ifndef _MYPLAY_H_
#define _MYPLAY_H_

#include <windows.h>

#define MYVIDEO_API __declspec(dllexport)
#define MYVIDEO_API_CALL __stdcall

#define OK 0
#define NG_FAILED_MAKE_ELEMENT          -1000
#define NG_FAILED_ALLOCATE_PIPELINE     -1002
#define NG_FAILED_LINK_ELEMENT          -1003
#define NG_FAILED_FIND_ELEMENT          -1004
#define NG_FAILED_PLAYBACK              -1005

MYVIDEO_API int MYVIDEO_API_CALL myvideo_initialize();

MYVIDEO_API int MYVIDEO_API_CALL myvideo_create_m2ts_pipeline(int** signal_chain);

MYVIDEO_API int MYVIDEO_API_CALL myvideo_create_h264_pipeline(int** signal_chain);

MYVIDEO_API int MYVIDEO_API_CALL myvideo_create_h265_pipeline(int** signal_chain);

MYVIDEO_API int MYVIDEO_API_CALL myvideo_delete_pipeline(int** signal_chain);

MYVIDEO_API int MYVIDEO_API_CALL myvideo_playback(int* signal_chain, char* filename, HWND handle);

MYVIDEO_API int MYVIDEO_API_CALL myvideo_stop(int* signal_chain);

#endif