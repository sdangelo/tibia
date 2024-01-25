/*
 * Copyright (C) 2023, 2024 Orastron Srl unipersonale
 */

/*
#include <algorithm>
#include <mutex>
#include <vector>
#include <jni.h>
#define MINIAUDIO_IMPLEMENTATION
#define MA_ENABLE_ONLY_SPECIFIC_BACKENDS
#define MA_ENABLE_AAUDIO
#include <miniaudio.h>
#include <amidi/AMidi.h>
#include "config.h"

#define BLOCK_SIZE	32
#define NUM_BUFS	(NUM_CHANNELS_IN > NUM_CHANNELS_OUT ? NUM_CHANNELS_IN : NUM_CHANNELS_OUT)

ma_device device;
P_TYPE instance;
float paramValues[NUM_PARAMETERS];
float bufs[NUM_BUFS][BLOCK_SIZE];
#if NUM_CHANNELS_IN != 0
const float *inBufs[NUM_CHANNELS_IN];
#endif
float *outBufs[NUM_CHANNELS_OUT];
std::mutex mutex;
#ifdef P_MEM_REQ
void *mem;
#endif
#ifdef P_NOTE_ON
struct PortData {
	AMidiDevice	*device;
	int		 portNumber;
	AMidiOutputPort	*port;
};
std::vector<PortData> midiPorts;
#define MIDI_BUFFER_SIZE 1024
uint8_t midiBuffer[MIDI_BUFFER_SIZE];
#endif

static void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
	(void)pDevice;
#if NUM_CHANNELS_IN == 0
	(void)pInput;
#else
	const float *x = reinterpret_cast<const float *>(pInput);
#endif
	float *y = reinterpret_cast<float *>(pOutput);

	if (mutex.try_lock()) {
		for (int i = 0; i < NUM_PARAMETERS; i++)
			if (config_parameters[i].out)
				paramValues[i] = P_GET_PARAMETER(&instance, i);
			else
				P_SET_PARAMETER(&instance, i, paramValues[i]);
#ifdef P_NOTE_ON
	for (std::vector<PortData>::iterator it = midiPorts.begin(); it != midiPorts.end(); it++) {
		int32_t opcode;
		size_t numBytes;
		while (AMidiOutputPort_receive(it->port, &opcode, midiBuffer, MIDI_BUFFER_SIZE, &numBytes, NULL) > 0) {
			if (opcode != AMIDI_OPCODE_DATA)
				continue;
			switch (midiBuffer[0] & 0xf0) {
			case 0x90:
				P_NOTE_ON(&instance, midiBuffer[1], midiBuffer[2]);
				break;
			case 0x80:
				P_NOTE_OFF(&instance, midiBuffer[1]);
				break;
#ifdef P_PITCH_BEND
			case 0xe0:
				P_PITCH_BEND(&instance, midiBuffer[2] << 7 | midiBuffer[1]);
				break;
#endif
#ifdef P_MOD_WHEEL
			case 0xb0:
				if (midiBuffer[1] == 1)
					P_MOD_WHEEL(&instance, midiBuffer[2]);
				break;
#endif
			}
		}
	}
#endif
		mutex.unlock();
	}

	ma_uint32 i = 0;
	while (i < frameCount) {
		ma_uint32 n = std::min(frameCount - i, static_cast<ma_uint32>(BLOCK_SIZE));

		int l;
#if NUM_CHANNELS_IN != 0
		l = NUM_CHANNELS_IN * i;
		for (ma_uint32 j = 0; j < n; j++)
			for (int k = 0;  k < NUM_CHANNELS_IN; k++, l++)
				bufs[k][j] = x[l];
#endif

#if NUM_CHANNELS_IN != 0
		P_PROCESS(&instance, inBufs, outBufs, n);
#else
		P_PROCESS(&instance, NULL, outBufs, n);
#endif

		l = NUM_CHANNELS_OUT * i;
		for (ma_uint32 j = 0; j < n; j++)
			for (int k = 0;  k < NUM_CHANNELS_OUT; k++, l++)
				y[l] = bufs[k][j];

		i += n;
	}
}

#ifdef P_NOTE_ON
extern "C"
JNIEXPORT void JNICALL
Java_com_orastron_@JNI_NAME@_MainActivity_addMidiPort(JNIEnv* env, jobject thiz, jobject d, jint p) {
	(void)thiz;

	PortData data;
	AMidiDevice_fromJava(env, d, &data.device);
	data.portNumber = p;
	mutex.lock();
	if (AMidiOutputPort_open(data.device, p, &data.port) == AMEDIA_OK)
		midiPorts.push_back(data);
	mutex.unlock();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_orastron_@JNI_NAME@_MainActivity_removeMidiPort(JNIEnv* env, jobject thiz, jobject d, jint p) {
	(void)thiz;

	AMidiDevice *device;
	AMidiDevice_fromJava(env, d, &device);
	mutex.lock();
	for (std::vector<PortData>::iterator it = midiPorts.begin(); it != midiPorts.end(); ) {
		PortData data = *it;
		if (data.device != device || data.portNumber != p) {
			it++;
			continue;
		}
		AMidiOutputPort_close(data.port);
		it = midiPorts.erase(it);
	}
	mutex.unlock();
}
#endif
*/

#include <stdlib.h>
#include <stdint.h>

#include "data.h"
#include "plugin.h"

#include <jni.h>
#if PARAMETERS_N + NUM_MIDI_INPUTS > 0
# include <mutex>
#endif
#if PARAMETERS_N > 0
# include <algorithm>
#endif
#if NUM_CHANNELS_IN + NUM_CHANNELS_OUT > 0
# define MINIAUDIO_IMPLEMENTATION
# define MA_ENABLE_ONLY_SPECIFIC_BACKENDS
# define MA_ENABLE_AAUDIO
# include <miniaudio.h>

# define BLOCK_SIZE	32
#endif

#if NUM_CHANNELS_IN + NUM_CHANNELS_OUT > 0
static ma_device	device;
#endif
static plugin		instance;
static void *		mem;
#if NUM_CHANNELS_IN > 0
float			x_buf[NUM_CHANNELS_IN * BLOCK_SIZE];
#endif
#if NUM_CHANNELS_OUT > 0
float			y_buf[NUM_CHANNELS_OUT * BLOCK_SIZE];
#endif
const float **		x;
float **		y;
#if PARAMETERS_N > 0
std::mutex		mutex;
float			param_values[PARAMETERS_N];
float			param_values_prev[PARAMETERS_N];
#endif

static void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
	(void)pDevice;

#if PARAMETERS_N + NUM_MIDI_INPUTS > 0
	if (mutex.try_lock()) {
# if PARAMETERS_N > 0
		for (size_t i = 0; i < PARAMETERS_N; i++) {
			if (param_data[i].out)
				param_values_prev[i] = param_values[i] = plugin_get_parameter(&instance, i);
			else if (param_values_prev[i] != param_values[i]) {
				plugin_set_parameter(&instance, i, param_values[i]);
				param_values_prev[i] = param_values[i];
			}
			// TODO: midi
		}
# endif
		mutex.unlock();
	}
#endif

	const float * in_buf = reinterpret_cast<const float *>(pInput);
	float * out_buf = reinterpret_cast<float *>(pOutput);
	ma_uint32 i = 0;
	while (i < frameCount) {
		ma_uint32 n = std::min(frameCount - i, static_cast<ma_uint32>(BLOCK_SIZE));

#if NUM_CHANNELS_IN > 0
		size_t ix = NUM_CHANNELS_IN * i;
		for (ma_uint32 j = 0; j < n; j++)
			for (size_t k = 0;  k < NUM_CHANNELS_IN; k++, ix++)
				x_buf[BLOCK_SIZE * k + j] = in_buf[ix];
#endif

		plugin_process(&instance, x, y, n);

#if NUM_CHANNELS_OUT > 0
		size_t iy = NUM_CHANNELS_OUT * i;
		for (ma_uint32 j = 0; j < n; j++)
			for (size_t k = 0;  k < NUM_CHANNELS_OUT; k++, iy++)
				y_buf[BLOCK_SIZE * k + j] = out_buf[ix];
#endif

		i += n;
	}
}

extern "C"
JNIEXPORT jboolean JNICALL
JNI_FUNC(nativeAudioStart)(JNIEnv* env, jobject thiz) {
	(void)env;
	(void)thiz;

#if NUM_CHANNELS_IN + NUM_CHANNELS_OUT > 0
# if NUM_CHANNELS_IN == 0
	ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
# elif NUM_CHANNELS_OUT == 0
	ma_device_config deviceConfig = ma_device_config_init(ma_device_type_capture);
# else
	ma_device_config deviceConfig = ma_device_config_init(ma_device_type_duplex);
# endif
	deviceConfig.periodSizeInFrames		= BLOCK_SIZE;
	deviceConfig.periods			= 1;
	deviceConfig.performanceProfile		= ma_performance_profile_low_latency;
	deviceConfig.noPreSilencedOutputBuffer	= 1;
	deviceConfig.noClip			= 0;
	deviceConfig.noDisableDenormals		= 0;
	deviceConfig.noFixedSizedCallback	= 1;
	deviceConfig.dataCallback		= data_callback;
	deviceConfig.capture.pDeviceID		= NULL;
	deviceConfig.capture.format		= ma_format_f32;
	deviceConfig.capture.channels		= NUM_CHANNELS_IN;
	deviceConfig.capture.shareMode		= ma_share_mode_shared;
	deviceConfig.playback.pDeviceID		= NULL;
	deviceConfig.playback.format		= ma_format_f32;
	deviceConfig.playback.channels		= NUM_CHANNELS_OUT;
	deviceConfig.playback.shareMode		= ma_share_mode_shared;

	if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS)
		return false;
#endif

	plugin_init(&instance);

#if PARAMETERS_N > 0
	for (size_t i = 0; i < PARAMETERS_N; i++) {
		if (!param_data[i].out)
			plugin_set_parameter(&instance, i, param_data[i].def);
		param_values_prev[i] = param_values[i] = param_data[i].def;
	}
#endif

	plugin_set_sample_rate(&instance, (float)device.sampleRate);
	size_t req = plugin_mem_req(&instance);
	if (req != 0) {
		mem = malloc(req);
		if (mem == NULL) {
			plugin_fini(&instance);
#if NUM_CHANNELS_IN + NUM_CHANNELS_OUT > 0
			ma_device_uninit(&device);
#endif
			return false;
		}
		plugin_mem_set(&instance, mem);
	} else
		mem = NULL;

	plugin_reset(&instance);

#if NUM_CHANNELS_IN > 0
	for (size_t i = 0; i < NUM_CHANNELS_IN; i++)
		x[i] = x_buf + BLOCK_SIZE * i;
#else
	x = NULL;
#endif
#if NUM_CHANNELS_OUT > 0
	for (size_t i = 0; i < NUM_CHANNELS_OUT; i++)
		y[i] = y_buf + BLOCK_SIZE * i;
#else
	y = NULL;
#endif

#if NUM_CHANNELS_IN + NUM_CHANNELS_OUT > 0
	if (ma_device_start(&device) != MA_SUCCESS) {
		if (mem != NULL)
			free(mem);
		ma_device_uninit(&device);
		return false;
	}
#endif

	return true;
}

extern "C"
JNIEXPORT void JNICALL
JNI_FUNC(nativeAudioStop)(JNIEnv* env, jobject thiz) {
	(void)env;
	(void)thiz;

	if (mem != NULL)
		free(mem);
	plugin_fini(&instance);
#if NUM_CHANNELS_IN + NUM_CHANNELS_OUT > 0
	ma_device_stop(&device);
	ma_device_uninit(&device);
#endif
}

extern "C"
JNIEXPORT jfloat JNICALL
JNI_FUNC(nativeGetParameter)(JNIEnv* env, jobject thiz, jint i) {
	(void)env;
	(void)thiz;

#if PARAMETERS_N > 0
	mutex.lock();
	float v = param_values[i];
	mutex.unlock();
	return v;
#else
	return 0.f;
#endif
}

extern "C"
JNIEXPORT void JNICALL
JNI_FUNC(nativeSetParameter)(JNIEnv* env, jobject thiz, jint i, jfloat v) {
	(void)env;
	(void)thiz;

	if (param_data[i].flags & (PARAM_BYPASS | PARAM_TOGGLED))
		v = v > 0.5f ? 1.f : 0.f;
	else if (param_data[i].flags & PARAM_INTEGER)
		v = (int32_t)(v + 0.5f);
	v = std::min(std::max(v, param_data[i].min), param_data[i].max);
#if PARAMETERS_N > 0
	mutex.lock();
	param_values[i] = v;
	mutex.unlock();
#endif
}

extern "C"
JNIEXPORT void JNICALL
JNI_FUNC(addMidiPort)(JNIEnv* env, jobject thiz, jobject d, jint p) {
	(void)env;
	(void)thiz;

	//TBD
}

extern "C"
JNIEXPORT void JNICALL
JNI_FUNC(removeMidiPort)(JNIEnv* env, jobject thiz, jobject d, jint p) {
	(void)env;
	(void)thiz;

	//TBD
}
