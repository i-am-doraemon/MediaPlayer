#include <gst/gst.h>
#include <gst/video/videooverlay.h>

#include "myvideo.h"

#define ELEMENT_FILE_M2TS    "filesrc"
#define ELEMENT_FILE_H264    "filesrc"
#define ELEMENT_FILE_H265    "filesrc"
#define ELEMENT_DMUX_M2TS    "tsdemux"
#define ELEMENT_DMUX_H264    "qtdemux"
#define ELEMENT_DMUX_H265    "qtdemux"
#define ELEMENT_DECODER_M2TS "avdec_mpeg2video"
#define ELEMENT_DECODER_H264 "avdec_h264"
#define ELEMENT_DECODER_H265 "avdec_h265"
#define ELEMENT_DISPLAY_M2TS "d3dvideosink"
#define ELEMENT_DISPLAY_H264 "d3dvideosink"
#define ELEMENT_DISPLAY_H265 "d3dvideosink"

#define ONE_MILLISECONDS (1000 * 1000) // nanoseconds

MYVIDEO_API int MYVIDEO_API_CALL myvideo_initialize()
{
	int    argc    =  00;
	char  *argv[1] = {""};

	gst_init(&argc, &argv);

	return 0;
}

static void on_can_connect_dmux_and_decoder(GstElement* dmux, GstPad* source, gpointer data)
{
	GstElement* decoder = GST_ELEMENT_CAST(data);
	GstPad* sink = gst_element_get_static_pad(decoder, "sink");

	GstPadLinkReturn why_returned = gst_pad_link(source, sink);
	g_object_unref(sink);
}

static int create_pipeline(GstElement **element, const char* name, const char* file_element_name, const char* dmux_element_name, const char* decoder_element_name, const char* monitor_element_name)
{
	int failed = NO_ERROR;
	*element = NULL;

	GstElement* file = gst_element_factory_make(file_element_name, "file");

	if (!file) {
		failed = NG_FAILED_MAKE_ELEMENT;
		goto FAILED_CREATE_ELEMENTS;
	}

	GstElement* dmux = gst_element_factory_make(dmux_element_name, "dmux");

	if (!dmux) {
		failed = NG_FAILED_MAKE_ELEMENT;
		goto FAILED_CREATE_ELEMENTS;
	}

	GstElement* decoder = gst_element_factory_make(decoder_element_name, "decoder");

	if (!decoder) {
		failed = NG_FAILED_MAKE_ELEMENT;
		goto FAILED_CREATE_ELEMENTS;
	}

	GstElement* monitor = gst_element_factory_make(monitor_element_name, "monitor");

	if (!monitor) {
		failed = NG_FAILED_MAKE_ELEMENT;
		goto FAILED_CREATE_ELEMENTS;
	}

	GstElement* pipeline = gst_pipeline_new(name);

	if (!pipeline) {
		failed = NG_FAILED_ALLOCATE_PIPELINE;
		goto FAILED_CREATE_ELEMENTS;
	}

	gst_bin_add_many(GST_BIN(pipeline), file, dmux, decoder, monitor, NULL);

	gboolean ok = 0;
	ok = gst_element_link(file, dmux);
	if (!ok) {
		failed = NG_FAILED_LINK_ELEMENT;
		goto FAILED_CREATE_PIPELINE;
	}

	ok = gst_element_link(decoder, monitor);
	if (!ok) {
		failed = NG_FAILED_LINK_ELEMENT;
		goto FAILED_CREATE_PIPELINE;
	}

	*element = pipeline;
	return failed;

FAILED_CREATE_PIPELINE:
	gst_object_unref(pipeline);

FAILED_CREATE_ELEMENTS:
	return failed;
}

MYVIDEO_API int MYVIDEO_API_CALL myvideo_create_m2ts_pipeline(guint64** signal_chain)
{
	GstElement* pipeline = NULL;
	int failed = create_pipeline(&pipeline, "m2ts-pipeline", ELEMENT_FILE_M2TS, 
		                                                     ELEMENT_DMUX_M2TS, 
		                                                     ELEMENT_DECODER_M2TS, ELEMENT_DISPLAY_M2TS);
	
	*signal_chain = pipeline;
	return failed;
}

MYVIDEO_API int MYVIDEO_API_CALL myvideo_create_h264_pipeline(guint64** signal_chain)
{
	return -1;
}

MYVIDEO_API int MYVIDEO_API_CALL myvideo_create_h265_pipeline(guint64** signal_chain)
{
	GstElement* pipeline = NULL;
	int failed = create_pipeline(&pipeline, "h265-pipeline", ELEMENT_FILE_H265, 
		                                                     ELEMENT_DMUX_H265, 
															 ELEMENT_DECODER_H265, ELEMENT_DISPLAY_H265);
	*signal_chain = pipeline;
	return failed;
}

MYVIDEO_API int MYVIDEO_API_CALL myvideo_delete_pipeline(guint64* signal_chain)
{
	GstPipeline* pipeline = GST_PIPELINE_CAST(signal_chain);
	gst_object_unref(pipeline);

	return OK;
}

MYVIDEO_API int MYVIDEO_API_CALL myvideo_playback(guint64* signal_chain, char* filename, HWND handle)
{
	GstPipeline* pipeline = GST_PIPELINE_CAST(signal_chain);

	GstElement* file = gst_bin_get_by_name(GST_BIN_CAST(pipeline), "file");
	if (!file) {
		return NG_FAILED_FIND_ELEMENT;
	}

	GstElement* dmux = gst_bin_get_by_name(GST_BIN_CAST(pipeline), "dmux");

	if (!dmux) {
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

	g_object_set(G_OBJECT(file), "location", filename, NULL);
	g_signal_connect(dmux, "pad-added", G_CALLBACK(on_can_connect_dmux_and_decoder), decoder);

	gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(monitor), handle);

	GstStateChangeReturn state_changed = gst_element_set_state(GST_ELEMENT_CAST(pipeline), GST_STATE_PLAYING);
	if (state_changed == GST_STATE_CHANGE_FAILURE) {
		return NG_FAILED_PLAYBACK;
	}

	return OK;
}

MYVIDEO_API int MYVIDEO_API_CALL myvideo_stop(guint64* signal_chain)
{
	GstPipeline* pipeline = GST_PIPELINE_CAST(signal_chain);
	gst_element_set_state(GST_ELEMENT_CAST(pipeline), GST_STATE_READY);

	return OK;
}

MYVIDEO_API int MYVIDEO_API_CALL myvideo_get_duration(guint64* signal_chain)
{
	GstElement* pipeline = GST_ELEMENT_CAST(signal_chain);

	gint64 duration = -1;

	GstState state = GST_STATE_NULL;
	GstStateChangeReturn why_returned = gst_element_get_state(
		                       pipeline, &state, NULL, ONE_MILLISECONDS);
	if (why_returned == GST_STATE_CHANGE_SUCCESS) {
		if (state > GST_STATE_READY) {
			gboolean ok = gst_element_query_duration(
				                   pipeline, GST_FORMAT_TIME, &duration);
			if (ok) {
				duration /= 1000; // ns -> us
				duration /= 1000; // us -> ms
				duration /= 1000; // ms -> •b
			} else {
				duration = -1;
			}
		}
	}

	return (int)duration;
}

MYVIDEO_API int MYVIDEO_API_CALL myvideo_get_position(guint64* signal_chain)
{
	GstElement* pipeline = GST_ELEMENT_CAST(signal_chain);

	gint64 position = -1;

	gboolean ok = gst_element_query_position(pipeline, GST_FORMAT_TIME, &position);
	if (ok) {
		position /= 1000; // ns -> us
		position /= 1000; // us -> ms
		position /= 1000; // ms -> •b
	} else {
		position = -1;
	}

	return (int)position;
}