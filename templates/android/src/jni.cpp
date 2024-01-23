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

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_orastron_@JNI_NAME@_MainActivity_nativeAudioStart(JNIEnv* env, jobject thiz) {
	(void)env;
	(void)thiz;

#if NUM_CHANNELS_IN == 0
	ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
#else
	ma_device_config deviceConfig = ma_device_config_init(ma_device_type_duplex);
#endif
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

	P_INIT(&instance);
	P_SET_SAMPLE_RATE(&instance, (float)device.sampleRate);
#ifdef P_MEM_REQ
	size_t req = P_MEM_REQ(&instance);
	if (req) {
		mem = malloc(req);
		if (mem == NULL) {
			ma_device_uninit(&device);
			return false;
		}
		P_MEM_SET(&instance, mem);
	} else
		mem = NULL;
#endif

	for (int i = 0; i < NUM_PARAMETERS; i++) {
		paramValues[i] = config_parameters[i].defaultValueUnmapped;
		if (!config_parameters[i].out)
			P_SET_PARAMETER(&instance, i, paramValues[i]);
	}

	P_RESET(&instance);

#if NUM_CHANNELS_IN != 0
	for (int i = 0; i < NUM_CHANNELS_IN; i++)
		inBufs[i] = bufs[i];
#endif
	for (int i = 0; i < NUM_CHANNELS_OUT; i++)
		outBufs[i] = bufs[i];

	if (ma_device_start(&device) != MA_SUCCESS) {
#ifdef P_MEM_REQ
		free(mem);
#endif
		ma_device_uninit(&device);
		return false;
	}

	return true;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_orastron_@JNI_NAME@_MainActivity_nativeAudioStop(JNIEnv* env, jobject thiz) {
	(void)env;
	(void)thiz;

#ifdef P_MEM_REQ
	free(mem);
#endif
#ifdef P_FINI
	P_FINI(&instance);
#endif
	ma_device_stop(&device);
	ma_device_uninit(&device);
}

extern "C"
JNIEXPORT jfloat JNICALL
Java_com_orastron_@JNI_NAME@_MainActivity_nativeGetParameter(JNIEnv* env, jobject thiz, jint i) {
	(void)env;
	(void)thiz;

	mutex.lock();
	float v = paramValues[i];
	mutex.unlock();
	return v;
}


extern "C"
JNIEXPORT void JNICALL
Java_com_orastron_@JNI_NAME@_MainActivity_nativeSetParameter(JNIEnv* env, jobject thiz, jint i, jfloat v) {
	(void)env;
	(void)thiz;

	mutex.lock();
	paramValues[i] = v;
	mutex.unlock();
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
#if NUM_CHANNELS_IN + NUM_CHANNELS_OUT > 0
# define MINIAUDIO_IMPLEMENTATION
# define MA_ENABLE_ONLY_SPECIFIC_BACKENDS
# define MA_ENABLE_AAUDIO
# include <miniaudio.h>

# define BLOCK_SIZE	32
# define NUM_BUFS	(NUM_CHANNELS_IN > NUM_CHANNELS_OUT ? NUM_CHANNELS_IN : NUM_CHANNELS_OUT)

static ma_device	device;
#endif
static plugin		instance;
static void *		mem;

static void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
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
	for (size_t i = 0; i < PARAMETERS_N; i++)
		if (!param_data[i].out)
			plugin_set_parameter(&instance, i, param_data[i].def);
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

	return true;
}

extern "C"
JNIEXPORT void JNICALL
JNI_FUNC(nativeAudioStop)(JNIEnv* env, jobject thiz) {
	(void)env;
	(void)thiz;

	plugin_fini(&instance);

#if NUM_CHANNELS_IN + NUM_CHANNELS_OUT > 0
	ma_device_stop(&device);
	ma_device_uninit(&device);
#endif
}

extern "C"
JNIEXPORT jfloat JNICALL
JNI_FUNC(nativeGetParameter)(JNIEnv* env, jobject thiz, jint i) {
	return 0.f;
}

extern "C"
JNIEXPORT void JNICALL
JNI_FUNC(nativeSetParameter)(JNIEnv* env, jobject thiz, jint i, jfloat v) {
}

extern "C"
JNIEXPORT void JNICALL
JNI_FUNC(addMidiPort)(JNIEnv* env, jobject thiz, jobject d, jint p) {
}

extern "C"
JNIEXPORT void JNICALL
JNI_FUNC(removeMidiPort)(JNIEnv* env, jobject thiz, jobject d, jint p) {
}
