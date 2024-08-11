#include "myvideo.h"
#include <libavformat/avformat.h>

#define NOTFOUND -1

MYVIDEO_API int MYVIDEO_API_CALL myvideo_get_codec_id(const char* filename)
{
	int failed = NO_ERROR;
	int result = NOTFOUND;

	AVFormatContext *fmt_ctx = NULL;

	failed = avformat_open_input(&fmt_ctx, filename, NULL, NULL);
	if (failed < 0) {
		failed = NG_FFMPEG_FAILED_CREATE_AVFORMAT;
		goto FAILED_CREATE_AVFORMAT;
	}

	failed = avformat_find_stream_info(fmt_ctx, NULL);
	if (failed < 0) {
		failed = NG_FFMPEG_FAILED_FIND_STREAMINFO;
		goto FAILED_FIND_STREAMINFO;
	}

	for (int i = 0; i < fmt_ctx->nb_streams; i++) {
		AVStream* stream = fmt_ctx->streams[i];
		if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			result = stream->codecpar->codec_id;
			break;
		}
	}

FAILED_FIND_VIDEO_STREAM:
FAILED_FIND_STREAMINFO:

	avformat_close_input(&fmt_ctx);

FAILED_CREATE_AVFORMAT:

	return result;
}