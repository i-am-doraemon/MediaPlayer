#include <gst/gst.h>
#include <gst/video/videooverlay.h>

#include "myvideo.h"

#define ELEMENT_FILE_M2TS    "filesrc"
#define ELEMENT_DMUX_M2TS    "tsdemux"
#define ELEMENT_DECODER_M2TS "avdec_mpeg2video"
#define ELEMENT_DISPLAY_M2TS "d3dvideosink"

MYVIDEO_API int MYVIDEO_API_CALL myvideo_initialize()
{
	int  argc       =   00;
	char argv[][16] = { "" };

	gst_init(&argc, &argv);

	return 0;
}

static void on_can_connect_demultiplexer_and_decoder(GstElement* dmux, GstPad* data_output_door, gpointer data)
{
	GstElement* decoder = GST_ELEMENT_CAST(data);
	GstPad* data_input_door = gst_element_get_static_pad(decoder, "sink");

	GstPadLinkReturn why_returned = gst_pad_link(data_output_door, data_input_door);
	g_object_unref(data_input_door);
}

MYVIDEO_API int MYVIDEO_API_CALL myvideo_create_m2ts_pipeline(int** signal_chain)
{
	int failed = OK;

	GstElement* filesrc = gst_element_factory_make(ELEMENT_FILE_M2TS, "filesrc");

	if (!filesrc) {
		failed = NG_FAILED_MAKE_ELEMENT;
		goto FAILED_CREATE_ELEMENTS;
	}

	GstElement* dmulple = gst_element_factory_make(ELEMENT_DMUX_M2TS, "dmulple");

	if (!dmulple) {
		failed = NG_FAILED_MAKE_ELEMENT;
		goto FAILED_CREATE_ELEMENTS;
	}

	GstElement* decoder = gst_element_factory_make(ELEMENT_DECODER_M2TS, "decoder");

	if (!decoder) {
		failed = NG_FAILED_MAKE_ELEMENT;
		goto FAILED_CREATE_ELEMENTS;
	}

	GstElement* monitor = gst_element_factory_make(ELEMENT_DISPLAY_M2TS, "monitor");

	if (!monitor) {
		failed = NG_FAILED_MAKE_ELEMENT;
		goto FAILED_CREATE_ELEMENTS;
	}

	GstElement* pipeline = gst_pipeline_new("m2ts-pipeline");

	if (!pipeline) {
		failed = NG_FAILED_ALLOCATE_PIPELINE;
		goto FAILED_CREATE_ELEMENTS;
	}

	gst_bin_add_many(GST_BIN(pipeline), filesrc, dmulple, decoder, monitor, NULL);

	gboolean ok = 0;
	ok = gst_element_link(filesrc, dmulple);
	if (!ok) {
		failed = NG_FAILED_LINK_ELEMENT;
		goto FAILED_CREATE_PIPELINE;
	}

	ok = gst_element_link(decoder, monitor);
	if (!ok) {
		failed = NG_FAILED_LINK_ELEMENT;
		goto FAILED_CREATE_PIPELINE;
	}

	*signal_chain = (int*)pipeline;
	return failed;

FAILED_CREATE_PIPELINE:
	gst_object_unref(pipeline);

FAILED_CREATE_ELEMENTS:
	*signal_chain = NULL;
	return failed;
}

MYVIDEO_API int MYVIDEO_API_CALL myvideo_create_h264_pipeline(int** signal_chain)
{
	return -1;
}

MYVIDEO_API int MYVIDEO_API_CALL myvideo_create_h265_pipeline(int** signal_chain)
{
	return -1;
}

MYVIDEO_API int MYVIDEO_API_CALL myvideo_delete_pipeline(int** signal_chain)
{
	GstPipeline* pipeline = GST_PIPELINE_CAST(signal_chain);
	gst_object_unref(pipeline);

	return OK;
}

MYVIDEO_API int MYVIDEO_API_CALL myvideo_playback(int* signal_chain, char* filename, HWND handle)
{
	GstPipeline* pipeline = GST_PIPELINE_CAST(signal_chain);

	GstElement* filesrc = gst_bin_get_by_name(GST_BIN_CAST(pipeline), "filesrc");
	if (!filesrc) {
		return NG_FAILED_FIND_ELEMENT;
	}

	GstElement* dmulple = gst_bin_get_by_name(GST_BIN_CAST(pipeline), "dmulple");

	if (!dmulple) {
		return NG_FAILED_FIND_ELEMENT;
	}

	GstElement* decoder = gst_bin_get_by_name(GST_BIN_CAST(pipeline), "decoder");
	if (!decoder) {
		return NG_FAILED_FIND_ELEMENT;
	}

	GstElement* monitor = gst_bin_get_by_name(GST_BIN_CAST(pipeline), "monitor");
	if (!monitor) {
		return NG_FAILED_FIND_ELEMENT;
	}

	g_object_set(G_OBJECT(filesrc), "location", filename, NULL);
	g_signal_connect(dmulple, "pad-added", G_CALLBACK(on_can_connect_demultiplexer_and_decoder), decoder);

	gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(monitor), handle);

	GstStateChangeReturn state_changed = gst_element_set_state(GST_ELEMENT_CAST(pipeline), GST_STATE_PLAYING);
	if (state_changed == GST_STATE_CHANGE_FAILURE) {
		return NG_FAILED_PLAYBACK;
	}

	return OK;
}

MYVIDEO_API int MYVIDEO_API_CALL myvideo_stop(int* signal_chain)
{
	GstPipeline* pipeline = GST_PIPELINE_CAST(signal_chain);
	gst_element_set_state(GST_ELEMENT_CAST(pipeline), GST_STATE_READY);

	return OK;
}