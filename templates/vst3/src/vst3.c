#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include "vst3_c_api.h"
#pragma GCC diagnostic pop
#include "data.h"
#include "plugin.h"

#if defined(__i386__) || defined(__x86_64__)
#include <xmmintrin.h>
#include <pmmintrin.h>
#endif

// COM in C doc:
//   https://github.com/rubberduck-vba/Rubberduck/wiki/COM-in-plain-C
//   https://devblogs.microsoft.com/oldnewthing/20040205-00/?p=40733

#ifdef TIBIA_TRACE
# include <stdio.h>
# define TRACE(...)	printf(__VA_ARGS__)
#else
# define TRACE(...)	/* do nothing */
#endif

#if defined(__i386__) || defined(__x86_64__)
#include <xmmintrin.h>
#include <pmmintrin.h>
#endif

static double clamp(double x, double m, double M) {
	return x < m ? m : (x > M ? M : x);
}

static int parameterGetIndexById(Steinberg_Vst_ParamID id) {
	for (int i = 0; i < DATA_PRODUCT_PARAMETERS_N + 3 * DATA_PRODUCT_BUSES_MIDI_INPUT_N; i++)
		if (parameterInfo[i].id == id)
			return i;
	return -1;
}

static double parameterMap(int i, double v) {
	return parameterData[i].flags & DATA_PARAM_MAP_LOG ? parameterData[i].min * exp(parameterData[i].mapK * v) : parameterData[i].min + (parameterData[i].max - parameterData[i].min) * v;
}

static double parameterUnmap(int i, double v) {
	return parameterData[i].flags & DATA_PARAM_MAP_LOG ? log(v / parameterData[i].min) / parameterData[i].mapK : (v - parameterData[i].min) / (parameterData[i].max - parameterData[i].min);
}

static double parameterAdjust(int i, double v) {
	v = parameterData[i].flags & (DATA_PARAM_BYPASS | DATA_PARAM_TOGGLED) ? (v >= 0.5 ? 1.0 : 0.0)
		: (parameterData[i].flags & DATA_PARAM_INTEGER ? (int32_t)(v + (v >= 0.0 ? 0.5 : -0.5)) : v);
	return clamp(v, parameterData[i].min, parameterData[i].max);
}

typedef struct pluginInstance {
	Steinberg_Vst_IComponentVtbl *			vtblIComponent;
	Steinberg_Vst_IAudioProcessorVtbl *		vtblIAudioProcessor;
	Steinberg_Vst_IProcessContextRequirementsVtbl *	vtblIProcessContextRequirements;
	Steinberg_uint32				refs;
	Steinberg_FUnknown *				context;
	plugin						p;
	float						sampleRate;
#if DATA_PRODUCT_PARAMETERS_N > 0
	float						parameters[DATA_PRODUCT_PARAMETERS_N];
#endif
#if DATA_PRODUCT_CHANNELS_AUDIO_INPUT_N > 0
	const float *					inputs[DATA_PRODUCT_CHANNELS_AUDIO_INPUT_N];
#endif
#if DATA_PRODUCT_CHANNELS_AUDIO_OUTPUT_N > 0
	float *						outputs[DATA_PRODUCT_CHANNELS_AUDIO_OUTPUT_N];
#endif
#if DATA_PRODUCT_BUSES_AUDIO_INPUT_N > 0
	char						inputsActive[DATA_PRODUCT_BUSES_AUDIO_INPUT_N];
#endif
#if DATA_PRODUCT_BUSES_AUDIO_OUTPUT_N > 0
	char						outputsActive[DATA_PRODUCT_BUSES_AUDIO_OUTPUT_N];
#endif
#if DATA_PRODUCT_BUSES_MIDI_INPUT_N > 0
	char						midiInputsActive[DATA_PRODUCT_BUSES_MIDI_INPUT_N];
#endif
#if DATA_PRODUCT_BUSES_MIDI_OUTPUT_N > 0
	char						midiOutputsActive[DATA_PRODUCT_BUSES_MIDI_OUTPUT_N];
#endif
	void *						mem;
} pluginInstance;

static Steinberg_Vst_IComponentVtbl pluginVtblIComponent;
static Steinberg_Vst_IAudioProcessorVtbl pluginVtblIAudioProcessor;

static Steinberg_tresult pluginQueryInterface(pluginInstance *p, const Steinberg_TUID iid, void ** obj) {
	// This seems to violate the way multiple inheritance should work in COM, but hosts like it, so what do I know...
	size_t offset;
	if (memcmp(iid, Steinberg_FUnknown_iid, sizeof(Steinberg_TUID)) == 0
	    || memcmp(iid, Steinberg_IPluginBase_iid, sizeof(Steinberg_TUID)) == 0
	    || memcmp(iid, Steinberg_Vst_IComponent_iid, sizeof(Steinberg_TUID)) == 0)
		offset = offsetof(pluginInstance, vtblIComponent);
	else if (memcmp(iid, Steinberg_Vst_IAudioProcessor_iid, sizeof(Steinberg_TUID)) == 0)
		offset = offsetof(pluginInstance, vtblIAudioProcessor);
	else if (memcmp(iid, Steinberg_Vst_IProcessContextRequirements_iid, sizeof(Steinberg_TUID)) == 0)
		offset = offsetof(pluginInstance, vtblIProcessContextRequirements);
	else {
		TRACE(" not supported\n");
		for (int i = 0; i < 16; i++)
			TRACE(" %x", iid[i]);
		TRACE("\n");
		*obj = NULL;
		return Steinberg_kNoInterface;
	}
	*obj = (void *)((char *)p + offset);
	p->refs++;
	return Steinberg_kResultOk;
}

static Steinberg_uint32 pluginAddRef(pluginInstance *p) {
	p->refs++;
	return p->refs;
}

static Steinberg_uint32 pluginRelease(pluginInstance *p) {
	p->refs--;
	if (p->refs == 0) {
		TRACE(" free %p\n", (void *)p);
		free(p);
		return 0;
	}
	return p->refs;
}

static Steinberg_tresult pluginIComponentQueryInterface(void *thisInterface, const Steinberg_TUID iid, void ** obj) {
	TRACE("plugin IComponent queryInterface %p\n", thisInterface);
	return pluginQueryInterface((pluginInstance *)((char *)thisInterface - offsetof(pluginInstance, vtblIComponent)), iid, obj);
}

static Steinberg_uint32 pluginIComponentAddRef(void *thisInterface) {
	TRACE("plugin IComponent addRef %p\n", thisInterface);
	return pluginAddRef((pluginInstance *)((char *)thisInterface - offsetof(pluginInstance, vtblIComponent)));
}

static Steinberg_uint32 pluginIComponentRelease(void *thisInterface) {
	TRACE("plugin IComponent release %p\n", thisInterface);
	return pluginRelease((pluginInstance *)((char *)thisInterface - offsetof(pluginInstance, vtblIComponent)));
}

static Steinberg_tresult pluginInitialize(void *thisInterface, struct Steinberg_FUnknown *context) {
	TRACE("plugin initialize\n");
	pluginInstance *p = (pluginInstance *)((char *)thisInterface - offsetof(pluginInstance, vtblIComponent));
	if (p->context != NULL)
		return Steinberg_kResultFalse;
	p->context = context;
	plugin_init(&p->p);
#if DATA_PRODUCT_PARAMETERS_N > 0
	for (size_t i = 0; i < DATA_PRODUCT_PARAMETERS_N; i++) {
		p->parameters[i] = parameterData[i].def;
		if (!(parameterInfo[i].flags & Steinberg_Vst_ParameterInfo_ParameterFlags_kIsReadOnly))
			plugin_set_parameter(&p->p, parameterData[i].index, parameterData[i].def);
	}
#endif
#if DATA_PRODUCT_BUSES_AUDIO_INPUT_N > 0
	for (size_t i = 0; i < DATA_PRODUCT_BUSES_AUDIO_INPUT_N; i++)
		p->inputsActive[i] = 0;
#endif
#if DATA_PRODUCT_BUSES_AUDIO_OUTPUT_N > 0
	for (size_t i = 0; i < DATA_PRODUCT_BUSES_AUDIO_OUTPUT_N; i++)
		p->outputsActive[i] = 0;
#endif
#if DATA_PRODUCT_BUSES_MIDI_INPUT_N > 0
	for (size_t i = 0; i < DATA_PRODUCT_BUSES_MIDI_INPUT_N; i++)
		p->midiInputsActive[i] = 0;
#endif
#if DATA_PRODUCT_BUSES_MIDI_OUTPUT_N > 0
	for (size_t i = 0; i < DATA_PRODUCT_BUSES_MIDI_OUTPUT_N; i++)
		p->midiOutputsActive[i] = 0;
#endif
	p->mem = NULL;
	return Steinberg_kResultOk;
}

static Steinberg_tresult pluginTerminate(void *thisInterface) {
	TRACE("plugin terminate\n");
	pluginInstance *p = (pluginInstance *)((char *)thisInterface - offsetof(pluginInstance, vtblIComponent));
	p->context = NULL;
	plugin_fini(&p->p);
	if (p->mem)
		free(p->mem);
	return Steinberg_kResultOk;
}

static Steinberg_tresult pluginGetControllerClassId(void *thisInterface, Steinberg_TUID classId) {
	(void)thisInterface;

	TRACE("plugin get controller class id %p %p\n", thisInterface, classId);
	if (classId != NULL) {
		memcpy(classId, dataControllerCID, sizeof(Steinberg_TUID));
		return Steinberg_kResultTrue;
	}
	return Steinberg_kResultFalse;
}

static Steinberg_tresult pluginSetIoMode(void *thisInterface, Steinberg_Vst_IoMode mode) {
	(void)thisInterface;
	(void)mode;

	TRACE("plugin set io mode\n");
	return Steinberg_kNotImplemented;
}

static Steinberg_int32 pluginGetBusCount(void *thisInterface, Steinberg_Vst_MediaType type, Steinberg_Vst_BusDirection dir) {
	(void)thisInterface;

	TRACE("plugin get bus count\n");
	if (type == Steinberg_Vst_MediaTypes_kAudio) {
		if (dir == Steinberg_Vst_BusDirections_kInput)
			return DATA_PRODUCT_BUSES_AUDIO_INPUT_N;
		else if (dir == Steinberg_Vst_BusDirections_kOutput)
			return DATA_PRODUCT_BUSES_AUDIO_OUTPUT_N;
	} else if (type == Steinberg_Vst_MediaTypes_kEvent) {
		if (dir == Steinberg_Vst_BusDirections_kInput)
			return DATA_PRODUCT_BUSES_MIDI_INPUT_N;
		else if (dir == Steinberg_Vst_BusDirections_kOutput)
			return DATA_PRODUCT_BUSES_MIDI_OUTPUT_N;
	}
	return 0;
}

static Steinberg_tresult pluginGetBusInfo(void* thisInterface, Steinberg_Vst_MediaType type, Steinberg_Vst_BusDirection dir, Steinberg_int32 index, struct Steinberg_Vst_BusInfo* bus) {
	(void)thisInterface;

	TRACE("plugin get bus info\n");
	if (index < 0)
		return Steinberg_kInvalidArgument;
	if (type == Steinberg_Vst_MediaTypes_kAudio) {
		if (dir == Steinberg_Vst_BusDirections_kInput) {
#if DATA_PRODUCT_BUSES_AUDIO_INPUT_N > 0
			if (index >= DATA_PRODUCT_BUSES_AUDIO_INPUT_N)
				return Steinberg_kInvalidArgument;
			*bus = busInfoAudioInput[index];
			return Steinberg_kResultTrue;
#endif
		} else if (dir == Steinberg_Vst_BusDirections_kOutput) {
#if DATA_PRODUCT_BUSES_AUDIO_OUTPUT_N > 0
			if (index >= DATA_PRODUCT_BUSES_AUDIO_OUTPUT_N)
				return Steinberg_kInvalidArgument;
			*bus = busInfoAudioOutput[index];
			return Steinberg_kResultTrue;
#endif
		}
	} else if (type == Steinberg_Vst_MediaTypes_kEvent) {
		if (dir == Steinberg_Vst_BusDirections_kInput) {
#if DATA_PRODUCT_BUSES_MIDI_INPUT_N > 0
			if (index >= DATA_PRODUCT_BUSES_MIDI_INPUT_N)
				return Steinberg_kInvalidArgument;
			*bus = busInfoMidiInput[index];
			return Steinberg_kResultTrue;
#endif
		} else if (dir == Steinberg_Vst_BusDirections_kOutput) {
#if DATA_PRODUCT_BUSES_MIDI_OUTPUT_N > 0
			if (index >= DATA_PRODUCT_BUSES_MIDI_OUTPUT_N)
				return Steinberg_kInvalidArgument;
			*bus = busInfoMidiOutput[index];
			return Steinberg_kResultTrue;
#endif
		}
	}
	return Steinberg_kInvalidArgument;
}

static Steinberg_tresult pluginGetRoutingInfo(void* thisInterface, struct Steinberg_Vst_RoutingInfo* inInfo, struct Steinberg_Vst_RoutingInfo* outInfo) {
	(void)thisInterface;
	(void)inInfo;
	(void)outInfo;

	TRACE("plugin get routing info\n");
	return Steinberg_kNotImplemented;
}

static Steinberg_tresult pluginActivateBus(void* thisInterface, Steinberg_Vst_MediaType type, Steinberg_Vst_BusDirection dir, Steinberg_int32 index, Steinberg_TBool state) {
	TRACE("plugin activate bus\n");
	if (index < 0)
		return Steinberg_kInvalidArgument;
	pluginInstance *p = (pluginInstance *)((char *)thisInterface - offsetof(pluginInstance, vtblIComponent));
	if (type == Steinberg_Vst_MediaTypes_kAudio) {
		if (dir == Steinberg_Vst_BusDirections_kInput) {
#if DATA_PRODUCT_BUSES_AUDIO_INPUT_N > 0
			if (index >= DATA_PRODUCT_BUSES_AUDIO_INPUT_N)
				return Steinberg_kInvalidArgument;
			p->inputsActive[index] = state;
			return Steinberg_kResultTrue;
#endif
		} else if (dir == Steinberg_Vst_BusDirections_kOutput) {
#if DATA_PRODUCT_BUSES_AUDIO_OUTPUT_N > 0
			if (index >= DATA_PRODUCT_BUSES_AUDIO_OUTPUT_N)
				return Steinberg_kInvalidArgument;
			p->outputsActive[index] = state;
			return Steinberg_kResultTrue;
#endif
		}
	} else if (type == Steinberg_Vst_MediaTypes_kEvent) {
		if (dir == Steinberg_Vst_BusDirections_kInput) {
#if DATA_PRODUCT_BUSES_MIDI_INPUT_N > 0
			if (index >= DATA_PRODUCT_BUSES_MIDI_INPUT_N)
				return Steinberg_kInvalidArgument;
			p->midiInputsActive[index] = state;
			return Steinberg_kResultTrue;
#endif
		} else if (dir == Steinberg_Vst_BusDirections_kOutput) {
#if DATA_PRODUCT_BUSES_MIDI_OUTPUT_N > 0
			if (index >= DATA_PRODUCT_BUSES_MIDI_OUTPUT_N)
				return Steinberg_kInvalidArgument;
			p->midiOutputsActive[index] = state;
			return Steinberg_kResultTrue;
#endif
		}
	}
	return Steinberg_kInvalidArgument;
}

static Steinberg_tresult pluginSetActive(void* thisInterface, Steinberg_TBool state) {
	TRACE("plugin set active\n");
	pluginInstance *p = (pluginInstance *)((char *)thisInterface - offsetof(pluginInstance, vtblIComponent));
	if (p->mem != NULL) {
		free(p->mem);
		p->mem = NULL;
	}
	if (state) {
#if DATA_PRODUCT_BUSES_AUDIO_INPUT_N > 0
		for (size_t i = 0; i < DATA_PRODUCT_BUSES_AUDIO_INPUT_N; i++)
			if (!p->inputsActive[i] && (busInfoAudioInput[i].flags & Steinberg_Vst_BusInfo_BusFlags_kDefaultActive))
				return Steinberg_kResultFalse;
#endif
#if DATA_PRODUCT_BUSES_AUDIO_OUTPUT_N > 0
		for (size_t i = 0; i < DATA_PRODUCT_BUSES_AUDIO_OUTPUT_N; i++)
			if (!p->outputsActive[i] && (busInfoAudioOutput[i].flags & Steinberg_Vst_BusInfo_BusFlags_kDefaultActive))
				return Steinberg_kResultFalse;
#endif
#if DATA_PRODUCT_BUSES_MIDI_INPUT_N > 0
		for (size_t i = 0; i < DATA_PRODUCT_BUSES_MIDI_INPUT_N; i++)
			if (!p->midiInputsActive[i] && (busInfoMidiInput[i].flags & Steinberg_Vst_BusInfo_BusFlags_kDefaultActive))
				return Steinberg_kResultFalse;
#endif
#if DATA_PRODUCT_BUSES_MIDI_OUTPUT_N > 0
		for (size_t i = 0; i < DATA_PRODUCT_BUSES_MIDI_OUTPUT_N; i++)
			if (!p->midiOutputsActive[i] && (busInfoMidiOutput[i].flags & Steinberg_Vst_BusInfo_BusFlags_kDefaultActive))
				return Steinberg_kResultFalse;
#endif
		plugin_set_sample_rate(&p->p, p->sampleRate);
		size_t req = plugin_mem_req(&p->p);
		if (req != 0) {
			p->mem = malloc(req);
			if (p->mem == NULL)
				return Steinberg_kOutOfMemory;
			plugin_mem_set(&p->p, p->mem);
		}
		plugin_reset(&p->p);
	}
	return Steinberg_kResultOk;
}

// https://stackoverflow.com/questions/2100331/macro-definition-to-determine-big-endian-or-little-endian-machine
#define IS_BIG_ENDIAN (!(union { uint16_t u16; unsigned char c; }){ .u16 = 1 }.c)
// https://stackoverflow.com/questions/2182002/how-to-convert-big-endian-to-little-endian-in-c-without-using-library-functions
#define SWAP_UINT32(x) (((x) >> 24) | (((x) & 0x00FF0000) >> 8) | (((x) & 0x0000FF00) << 8) | ((x) << 24))

static Steinberg_tresult pluginSetState(void* thisInterface, struct Steinberg_IBStream* state) {
	TRACE("plugin set state\n");
	if (state == NULL)
		return Steinberg_kResultFalse;
#if DATA_PRODUCT_PARAMETERS_N > 0
	pluginInstance *p = (pluginInstance *)((char *)thisInterface - offsetof(pluginInstance, vtblIComponent));
	for (size_t i = 0; i < DATA_PRODUCT_PARAMETERS_N; i++) {
		if (parameterInfo[i].flags & Steinberg_Vst_ParameterInfo_ParameterFlags_kIsReadOnly)
			continue;
		union { float f; uint32_t u; } v;
		Steinberg_int32 n;
		state->lpVtbl->read(state, &v, 4, &n);
		if (n != 4)
			return Steinberg_kResultFalse;
		if (IS_BIG_ENDIAN)
			v.u = SWAP_UINT32(v.u);
		p->parameters[i] = parameterAdjust(i, v.f);
		plugin_set_parameter(&p->p, parameterData[i].index, p->parameters[i]);
	}
#endif
	return Steinberg_kResultOk;
}

static Steinberg_tresult pluginGetState(void* thisInterface, struct Steinberg_IBStream* state) {
	TRACE("plugin get state\n");
	if (state == NULL)
		return Steinberg_kResultFalse;
#if DATA_PRODUCT_PARAMETERS_N > 0
	pluginInstance *p = (pluginInstance *)((char *)thisInterface - offsetof(pluginInstance, vtblIComponent));
	for (size_t i = 0; i < DATA_PRODUCT_PARAMETERS_N; i++) {
		if (parameterInfo[i].flags & Steinberg_Vst_ParameterInfo_ParameterFlags_kIsReadOnly)
			continue;
		union { float f; uint32_t u; } v;
		v.f = p->parameters[i];
		if (IS_BIG_ENDIAN)
			v.u = SWAP_UINT32(v.u);
		Steinberg_int32 n;
		state->lpVtbl->write(state, &v, 4, &n);
		if (n != 4)
			return Steinberg_kResultFalse;
	}
#endif
	return Steinberg_kResultOk;
}

static Steinberg_Vst_IComponentVtbl pluginVtblIComponent = {
	/* FUnknown */
	/* .queryInterface		= */ pluginIComponentQueryInterface,
	/* .addRef			= */ pluginIComponentAddRef,
	/* .release			= */ pluginIComponentRelease,

	/* IPluginBase */
	/* .initialize			= */ pluginInitialize,
	/* .terminate			= */ pluginTerminate,

	/* IComponent */
	/* .getControllerClassId	= */ pluginGetControllerClassId,
	/* .setIoMode			= */ pluginSetIoMode,
	/* .getBusCount			= */ pluginGetBusCount,
	/* .getBusInfo			= */ pluginGetBusInfo,
	/* .getRoutingInfo		= */ pluginGetRoutingInfo,
	/* .activateBus			= */ pluginActivateBus,
	/* .setActive			= */ pluginSetActive,
	/* .setState			= */ pluginSetState,
	/* .getState			= */ pluginGetState
};

static Steinberg_tresult pluginIAudioProcessorQueryInterface(void *thisInterface, const Steinberg_TUID iid, void ** obj) {
	TRACE("plugin IAudioProcessor queryInterface %p\n", thisInterface);
	return pluginQueryInterface((pluginInstance *)((char *)thisInterface - offsetof(pluginInstance, vtblIAudioProcessor)), iid, obj);
}

static Steinberg_uint32 pluginIAudioProcessorAddRef(void *thisInterface) {
	TRACE("plugin IAudioProcessor addRef %p\n", thisInterface);
	return pluginAddRef((pluginInstance *)((char *)thisInterface - offsetof(pluginInstance, vtblIAudioProcessor)));
}

static Steinberg_uint32 pluginIAudioProcessorRelease(void *thisInterface) {
	TRACE("plugin IAudioProcessor release %p\n", thisInterface);
	return pluginRelease((pluginInstance *)((char *)thisInterface - offsetof(pluginInstance, vtblIAudioProcessor)));
}

static Steinberg_tresult pluginSetBusArrangements(void* thisInterface, Steinberg_Vst_SpeakerArrangement* inputs, Steinberg_int32 numIns, Steinberg_Vst_SpeakerArrangement* outputs, Steinberg_int32 numOuts) {
	(void)thisInterface;

	TRACE("plugin IAudioProcessor set bus arrangements\n");
	if (numIns < 0 || numOuts < 0)
		return Steinberg_kInvalidArgument;
	if (numIns != DATA_PRODUCT_BUSES_AUDIO_INPUT_N || numOuts != DATA_PRODUCT_BUSES_AUDIO_OUTPUT_N)
		return Steinberg_kResultFalse;

#if DATA_PRODUCT_BUSES_AUDIO_INPUT_N > 0
	for (Steinberg_int32 i = 0; i < numIns; i++)
		if ((busInfoAudioInput[i].channelCount == 1 && inputs[i] != Steinberg_Vst_SpeakerArr_kMono)
		    || (busInfoAudioInput[i].channelCount == 2 && inputs[i] != Steinberg_Vst_SpeakerArr_kStereo))
			return Steinberg_kResultFalse;
#else
	(void)inputs;
#endif

#if DATA_PRODUCT_BUSES_AUDIO_OUTPUT_N > 0
	for (Steinberg_int32 i = 0; i < numOuts; i++)
		if ((busInfoAudioOutput[i].channelCount == 1 && outputs[i] != Steinberg_Vst_SpeakerArr_kMono)
		    || (busInfoAudioOutput[i].channelCount == 2 && outputs[i] != Steinberg_Vst_SpeakerArr_kStereo))
			return Steinberg_kResultFalse;
#else
	(void)outputs;
#endif

	return Steinberg_kResultTrue;
}

static Steinberg_tresult pluginGetBusArrangement(void* thisInterface, Steinberg_Vst_BusDirection dir, Steinberg_int32 index, Steinberg_Vst_SpeakerArrangement* arr) {
	(void)thisInterface;

	TRACE("plugin IAudioProcessor get bus arrangement\n");

#if DATA_PRODUCT_BUSES_AUDIO_INPUT_N > 0
	if (dir == Steinberg_Vst_BusDirections_kInput && index >= 0 && index < DATA_PRODUCT_BUSES_AUDIO_INPUT_N) {
		*arr = busInfoAudioInput[index].channelCount == 1 ? Steinberg_Vst_SpeakerArr_kMono : Steinberg_Vst_SpeakerArr_kStereo;
		return Steinberg_kResultTrue;
	}
#endif

#if DATA_PRODUCT_BUSES_AUDIO_OUTPUT_N > 0
	if (dir == Steinberg_Vst_BusDirections_kOutput && index >= 0 && index < DATA_PRODUCT_BUSES_AUDIO_OUTPUT_N) {
		*arr = busInfoAudioOutput[index].channelCount == 1 ? Steinberg_Vst_SpeakerArr_kMono : Steinberg_Vst_SpeakerArr_kStereo;
		return Steinberg_kResultTrue;
	}
#endif

	return Steinberg_kInvalidArgument;
}

static Steinberg_tresult pluginCanProcessSampleSize(void* thisInterface, Steinberg_int32 symbolicSampleSize) {
	(void)thisInterface;

	TRACE("plugin IAudioProcessor can process sample size\n");
	return symbolicSampleSize == Steinberg_Vst_SymbolicSampleSizes_kSample32 ? Steinberg_kResultOk : Steinberg_kNotImplemented;
}

static Steinberg_uint32 pluginGetLatencySamples(void* thisInterface) {
	(void)thisInterface;

	TRACE("plugin IAudioProcessor get latency samples\n");
	//TBD
	return 0;
}

static Steinberg_tresult pluginSetupProcessing(void* thisInterface, struct Steinberg_Vst_ProcessSetup* setup) {
	TRACE("plugin IAudioProcessor setup processing\n");
	pluginInstance *p = (pluginInstance *)((char *)thisInterface - offsetof(pluginInstance, vtblIAudioProcessor));
	p->sampleRate = (float)setup->sampleRate;
	return Steinberg_kResultOk;
}

static Steinberg_tresult pluginSetProcessing(void* thisInterface, Steinberg_TBool state) {
	(void)thisInterface;
	(void)state;

	TRACE("plugin IAudioProcessor set processing\n");
	return Steinberg_kNotImplemented;
}

static void processParams(pluginInstance *p, struct Steinberg_Vst_ProcessData *data, char before) {
#if DATA_PRODUCT_PARAMETERS_N + DATA_PRODUCT_BUSES_MIDI_INPUT_N > 0
	if (data->inputParameterChanges == NULL)
		return;
	Steinberg_int32 n = data->inputParameterChanges->lpVtbl->getParameterCount(data->inputParameterChanges);
	for (Steinberg_int32 i = 0; i < n; i++) {
		struct Steinberg_Vst_IParamValueQueue *q = data->inputParameterChanges->lpVtbl->getParameterData(data->inputParameterChanges, i);
		if (q == NULL)
			continue;
		Steinberg_int32 c = q->lpVtbl->getPointCount(q);
		if (c <= 0)
			continue;
		Steinberg_int32 o;
		Steinberg_Vst_ParamValue v;
		if (q->lpVtbl->getPoint(q, before ? 0 : c - 1, &o, &v) != Steinberg_kResultTrue)
			continue;
		if (before ? o != 0 : o <= 0)
			continue;
		Steinberg_Vst_ParamID id = q->lpVtbl->getParameterId(q);
		int pi = parameterGetIndexById(id);
# if DATA_PRODUCT_BUSES_MIDI_INPUT_N > 0
		if (pi >= DATA_PRODUCT_PARAMETERS_N) {
			size_t j = pi - DATA_PRODUCT_PARAMETERS_N;
			size_t index = j / 3;
			uint8_t data[3];
			switch (j & 0x3) {
			case 0:
				// channel pressure
				data[0] = 0xd0 /* | channel */;
				data[1] = (uint8_t)(127.0 * v);
				data[2] = 0;
				break;
			case 1:
			{
				// pitch bend change
				uint16_t x = (uint16_t)(16382.0 * v) + 1;
				data[0] = 0xe0 /* | channel */;
				data[1] = (x >> 7) & 0x7f;
				data[2] = x & 0x7f;
			}
				break;
			case 2:
				// mod wheel
				data[0] = 0xb0 /* | channel */;
				data[1] = 1;
				data[2] = (uint8_t)(127.0 * v);
				break;
			default:
				continue;
			}
			plugin_midi_msg_in(&p->p, midiInIndex[index], data);
			continue;
		}
# endif
		v = parameterAdjust(pi, parameterMap(pi, v));
		if (v != p->parameters[pi]) {
			p->parameters[pi] = v;
			plugin_set_parameter(&p->p, parameterData[pi].index, p->parameters[pi]);
		}
	}
#endif
}

static Steinberg_tresult pluginProcess(void* thisInterface, struct Steinberg_Vst_ProcessData* data) {
	TRACE("plugin IAudioProcessor process\n");

#if defined(__aarch64__)
	uint64_t fpcr;
	__asm__ __volatile__ ("mrs %0, fpcr" : "=r"(fpcr));
	__asm__ __volatile__ ("msr fpcr, %0" :: "r"(fpcr | 0x1000000)); // enable FZ
#elif defined(__i386__) || defined(__x86_64__)
	const unsigned int flush_zero_mode = _MM_GET_FLUSH_ZERO_MODE();
	const unsigned int denormals_zero_mode = _MM_GET_DENORMALS_ZERO_MODE();

	_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
	_MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
#endif

	pluginInstance *p = (pluginInstance *)((char *)thisInterface - offsetof(pluginInstance, vtblIAudioProcessor));

	processParams(p, data, 1); 

#if DATA_PRODUCT_BUSES_MIDI_INPUT_N > 0
	if (data->inputEvents != NULL) {
		Steinberg_int32 n = data->inputEvents->lpVtbl->getEventCount(data->inputEvents);
		for (Steinberg_int32 i = 0; i < n; i++) {
			struct Steinberg_Vst_Event ev;
			if (data->inputEvents->lpVtbl->getEvent(data->inputEvents, i, &ev) != Steinberg_kResultOk)
				continue;
			switch (ev.type) {
			case Steinberg_Vst_Event_EventTypes_kNoteOnEvent:
			{
				const uint8_t data[3] = { 0x90 | ev.Steinberg_Vst_Event_noteOn.channel, ev.Steinberg_Vst_Event_noteOn.pitch, (uint8_t)(127.f * ev.Steinberg_Vst_Event_noteOn.velocity) };
				plugin_midi_msg_in(&p->p, midiInIndex[ev.busIndex], data);
			}
				break;
			case Steinberg_Vst_Event_EventTypes_kNoteOffEvent:
			{
				const uint8_t data[3] = { 0x80 | ev.Steinberg_Vst_Event_noteOff.channel, ev.Steinberg_Vst_Event_noteOff.pitch, (uint8_t)(127.f * ev.Steinberg_Vst_Event_noteOff.velocity) };
				plugin_midi_msg_in(&p->p, midiInIndex[ev.busIndex], data);
			}
				break;
			case Steinberg_Vst_Event_EventTypes_kPolyPressureEvent:
			{
				const uint8_t data[3] = { 0xa0 | ev.Steinberg_Vst_Event_polyPressure.channel, ev.Steinberg_Vst_Event_polyPressure.pitch, (uint8_t)(127.f * ev.Steinberg_Vst_Event_polyPressure.pressure) };
				plugin_midi_msg_in(&p->p, midiInIndex[ev.busIndex], data);
			}
				break;
			}
		}
	}
#endif

#if DATA_PRODUCT_CHANNELS_AUDIO_INPUT_N > 0
	const float **inputs = p->inputs;
	Steinberg_int32 ki = 0;
	for (Steinberg_int32 i = 0; i < data->numInputs; i++)
		for (Steinberg_int32 j = 0; j < data->inputs[i].numChannels; j++, ki++)
			inputs[ki] = data->inputs[i].Steinberg_Vst_AudioBusBuffers_channelBuffers32[j];
#else
	const float **inputs = NULL;
#endif

#if DATA_PRODUCT_CHANNELS_AUDIO_OUTPUT_N > 0
	float **outputs = p->outputs;
	Steinberg_int32 ko = 0;
	for (Steinberg_int32 i = 0; i < data->numOutputs; i++)
		for (Steinberg_int32 j = 0; j < data->outputs[i].numChannels; j++, ko++)
			outputs[ko] = data->outputs[i].Steinberg_Vst_AudioBusBuffers_channelBuffers32[j];
#else
	float **outputs = NULL;
#endif

	plugin_process(&p->p, inputs, outputs, data->numSamples);

	processParams(p, data, 0); 

#if DATA_PRODUCT_PARAMETERS_N > 0
	for (Steinberg_int32 i = 0; i < DATA_PRODUCT_PARAMETERS_N; i++) {
		if (!(parameterInfo[i].flags & Steinberg_Vst_ParameterInfo_ParameterFlags_kIsReadOnly))
			continue;
		float v = plugin_get_parameter(&p->p, parameterData[i].index);
		if (v == p->parameters[i])
			continue;
		p->parameters[i] = v;
		if (data->outputParameterChanges == NULL)
			continue;
		Steinberg_Vst_ParamID id = parameterInfo[i].id;
		Steinberg_int32 index;
		struct Steinberg_Vst_IParamValueQueue *q = data->outputParameterChanges->lpVtbl->addParameterData(data->outputParameterChanges, &id, &index);
		if (q == NULL)
			continue;
		q->lpVtbl->addPoint(q, data->numSamples - 1, parameterUnmap(i, v), &index);
	}
#endif

	// TBD: latency + IComponentHandler::restartComponent (kLatencyChanged), see https://steinbergmedia.github.io/vst3_dev_portal/pages/Technical+Documentation/Workflow+Diagrams/Get+Latency+Call+Sequence.html

#if defined(__aarch64__)
	__asm__ __volatile__ ("msr fpcr, %0" : : "r"(fpcr));
#elif defined(__i386__) || defined(__x86_64__)
	_MM_SET_FLUSH_ZERO_MODE(flush_zero_mode);
	_MM_SET_DENORMALS_ZERO_MODE(denormals_zero_mode);
#endif

	return Steinberg_kResultOk;
}

static Steinberg_uint32 pluginGetTailSamples(void* thisInterface) {
	(void)thisInterface;

	TRACE("plugin IAudioProcessor get tail samples\n");
	//TBD
	return 0;
}

static Steinberg_Vst_IAudioProcessorVtbl pluginVtblIAudioProcessor = {
	/* FUnknown */
	/* .queryInterface		= */ pluginIAudioProcessorQueryInterface,
	/* .addRef			= */ pluginIAudioProcessorAddRef,
	/* .release			= */ pluginIAudioProcessorRelease,

	/* IAudioProcessor */
	/* .setBusArrangements		= */ pluginSetBusArrangements,
	/*. getBusArrangement		= */ pluginGetBusArrangement,
	/* .canProcessSampleSize	= */ pluginCanProcessSampleSize,
	/* .getLatencySamples		= */ pluginGetLatencySamples,
	/* .setupProcessing		= */ pluginSetupProcessing,
	/* .setProcessing		= */ pluginSetProcessing,
	/* .process			= */ pluginProcess,
	/* .getTailSamples		= */ pluginGetTailSamples
};

static Steinberg_tresult pluginIProcessContextRequirementsQueryInterface(void *thisInterface, const Steinberg_TUID iid, void ** obj) {
	TRACE("plugin IProcessContextRequirements queryInterface %p\n", thisInterface);
	return pluginQueryInterface((pluginInstance *)((char *)thisInterface - offsetof(pluginInstance, vtblIProcessContextRequirements)), iid, obj);
}

static Steinberg_uint32 pluginIProcessContextRequirementsAddRef(void *thisInterface) {
	TRACE("plugin IComponent addRef %p\n", thisInterface);
	return pluginAddRef((pluginInstance *)((char *)thisInterface - offsetof(pluginInstance, vtblIProcessContextRequirements)));
}

static Steinberg_uint32 pluginIProcessContextRequirementsRelease(void *thisInterface) {
	TRACE("plugin IComponent release %p\n", thisInterface);
	return pluginRelease((pluginInstance *)((char *)thisInterface - offsetof(pluginInstance, vtblIProcessContextRequirements)));
}
static Steinberg_uint32 pluginGetProcessContextRequirements(void* thisInterface) {
	(void)thisInterface;

	// TBD
	return 0; 
}

static Steinberg_Vst_IProcessContextRequirementsVtbl pluginVtblIProcessContextRequirements = {
	/* FUnknown */
	/* .queryInterface			= */ pluginIProcessContextRequirementsQueryInterface,
	/* .addRef				= */ pluginIProcessContextRequirementsAddRef,
	/* .release				= */ pluginIProcessContextRequirementsRelease,

	/* IProcessContextRequirements */
	/* .getProcessContextRequirements	= */ pluginGetProcessContextRequirements
};

typedef struct controller {
	Steinberg_Vst_IEditControllerVtbl *	vtblIEditController;
	Steinberg_Vst_IMidiMappingVtbl *	vtblIMidiMapping;
	Steinberg_uint32			refs;
	Steinberg_FUnknown *			context;
#if DATA_PRODUCT_PARAMETERS_N + DATA_PRODUCT_BUSES_MIDI_INPUT_N > 0
	double					parameters[DATA_PRODUCT_PARAMETERS_N + 3 * DATA_PRODUCT_BUSES_MIDI_INPUT_N];
#endif
	struct Steinberg_Vst_IComponentHandler* componentHandler;
} controller;

static Steinberg_Vst_IEditControllerVtbl controllerVtblIEditController;
static Steinberg_Vst_IMidiMappingVtbl controllerVtblIMidiMapping;

static Steinberg_tresult controllerQueryInterface(controller *c, const Steinberg_TUID iid, void ** obj) {
	// Same as above (pluginQueryInterface)
	size_t offset;
	if (memcmp(iid, Steinberg_FUnknown_iid, sizeof(Steinberg_TUID)) == 0
	    || memcmp(iid, Steinberg_IPluginBase_iid, sizeof(Steinberg_TUID)) == 0
	    || memcmp(iid, Steinberg_Vst_IEditController_iid, sizeof(Steinberg_TUID)) == 0)
		offset = offsetof(controller, vtblIEditController);
	else if (memcmp(iid, Steinberg_Vst_IMidiMapping_iid, sizeof(Steinberg_TUID)) == 0)
		offset = offsetof(controller, vtblIMidiMapping);
	else {
		TRACE(" not supported\n");
		for (int i = 0; i < 16; i++)
			TRACE(" %x", iid[i]);
		TRACE("\n");
		*obj = NULL;
		return Steinberg_kNoInterface;
	}
	*obj = (void *)((char *)c + offset);
	c->refs++;
	return Steinberg_kResultOk;
}

static Steinberg_uint32 controllerAddRef(controller *c) {
	c->refs++;
	return c->refs;
}

static Steinberg_uint32 controllerRelease(controller *c) {
	c->refs--;
	if (c->refs == 0) {
		TRACE(" free %p\n", (void *)c);
		free(c);
		return 0;
	}
	return c->refs;
}

static Steinberg_tresult controllerIEditControllerQueryInterface(void* thisInterface, const Steinberg_TUID iid, void** obj) {
	TRACE("controller IEditController queryInterface %p\n", thisInterface);
	return controllerQueryInterface((controller *)((char *)thisInterface - offsetof(controller, vtblIEditController)), iid, obj);
}

static Steinberg_uint32 controllerIEditControllerAddRef(void* thisInterface) {
	TRACE("controller IEditController addRef %p\n", thisInterface);
	return controllerAddRef((controller *)((char *)thisInterface - offsetof(controller, vtblIEditController)));
}

static Steinberg_uint32 controllerIEditControllerRelease (void* thisInterface) {
	TRACE("controller IEditController release %p\n", thisInterface);
	return controllerRelease((controller *)((char *)thisInterface - offsetof(controller, vtblIEditController)));
}

static Steinberg_tresult controllerInitialize(void* thisInterface, struct Steinberg_FUnknown* context) {
	TRACE("controller initialize\n");
	controller *c = (controller *)((char *)thisInterface - offsetof(controller, vtblIEditController));
	if (c->context != NULL)
		return Steinberg_kResultFalse;
	c->context = context;
#if DATA_PRODUCT_PARAMETERS_N > 0
	for (int i = 0; i < DATA_PRODUCT_PARAMETERS_N; i++)
		c->parameters[i] = parameterData[i].def;
#endif
#if DATA_PRODUCT_BUSES_MIDI_INPUT_N > 0
	for (int i = DATA_PRODUCT_PARAMETERS_N; i < DATA_PRODUCT_PARAMETERS_N + 3 * DATA_PRODUCT_BUSES_MIDI_INPUT_N; i += 3) {
		c->parameters[i] = 0.0;
		c->parameters[i + 1] = 0.5;
		c->parameters[i + 2] = 0.0;
	}
#endif
	return Steinberg_kResultOk;
}

static Steinberg_tresult controllerTerminate(void* thisInterface) {
	TRACE("controller terminate\n");
	controller *c = (controller *)((char *)thisInterface - offsetof(controller, vtblIEditController));
	c->context = NULL;
	return Steinberg_kResultOk;
}

static Steinberg_tresult controllerSetComponentState(void* thisInterface, struct Steinberg_IBStream* state) {
	TRACE("controller set component state %p %p\n", thisInterface, (void *)state);
	if (state == NULL)
		return Steinberg_kResultFalse;
#if DATA_PRODUCT_PARAMETERS_N > 0
	controller *c = (controller *)((char *)thisInterface - offsetof(controller, vtblIEditController));
	for (size_t i = 0; i < DATA_PRODUCT_PARAMETERS_N; i++) {
		if (parameterInfo[i].flags & Steinberg_Vst_ParameterInfo_ParameterFlags_kIsReadOnly)
			continue;
		union { float f; uint32_t u; } v;
		Steinberg_int32 n;
		state->lpVtbl->read(state, &v, 4, &n);
		if (n != 4)
			return Steinberg_kResultFalse;
		if (IS_BIG_ENDIAN)
			v.u = SWAP_UINT32(v.u);
		c->parameters[i] = parameterAdjust(i, v.f);
	}
#endif
	TRACE(" ok\n");
	return Steinberg_kResultTrue;
}

static Steinberg_tresult controllerSetState(void* thisInterface, struct Steinberg_IBStream* state) {
	(void)thisInterface;
	(void)state;

	TRACE("controller set state\n");
	return Steinberg_kNotImplemented;
}

static Steinberg_tresult controllerGetState(void* thisInterface, struct Steinberg_IBStream* state) {
	(void)thisInterface;
	(void)state;

	TRACE("controller get state\n");
	return Steinberg_kNotImplemented;
}

static Steinberg_int32 controllerGetParameterCount(void* thisInterface) {
	(void)thisInterface;

	TRACE("controller get parameter count\n");
	return DATA_PRODUCT_PARAMETERS_N + 3 * DATA_PRODUCT_BUSES_MIDI_INPUT_N;
}

static Steinberg_tresult controllerGetParameterInfo(void* thisInterface, Steinberg_int32 paramIndex, struct Steinberg_Vst_ParameterInfo* info) {
	(void)thisInterface;

	TRACE("controller get parameter info\n");
	if (paramIndex < 0 || paramIndex >= DATA_PRODUCT_PARAMETERS_N + 3 * DATA_PRODUCT_BUSES_MIDI_INPUT_N)
		return Steinberg_kResultFalse;
	*info = parameterInfo[paramIndex];
	return Steinberg_kResultTrue;
}

static void dToStr(double v, Steinberg_Vst_String128 s, int precision) {
	int i = 0;

	if (v < 0.0) {
		s[0] = '-';
		v = -v;
		i++;
	}

	if (v < 1.0) {
		s[i] = '0';
		i++;
	} else {
		double x = 1.0;
		while (x <= v)
			x *= 10.0;
		x *= 0.1;
		while (x >= 1.0) {
			char c = v / x;
			s[i] = c + '0';
			i++;
			v -= c * x;
			x *= 0.1;
		}
	}

	s[i] = '.';
	i++;

	double x = 0.1;
	while (precision != 0) {
		char c = v / x;
		s[i] = c + '0';
		i++;
		v -= c * x;
		x *= 0.1;
		precision--;
	}
	
	s[i] = '\0';
}

static Steinberg_tresult controllerGetParamStringByValue(void* thisInterface, Steinberg_Vst_ParamID id, Steinberg_Vst_ParamValue valueNormalized, Steinberg_Vst_String128 string) {
	(void)thisInterface;

	TRACE("controller get param string by value\n");
	int pi = parameterGetIndexById(id);
	if (pi >= DATA_PRODUCT_PARAMETERS_N + 3 * DATA_PRODUCT_BUSES_MIDI_INPUT_N || pi < 0)
		return Steinberg_kResultFalse;
	dToStr(pi >= DATA_PRODUCT_PARAMETERS_N ? valueNormalized : parameterMap(pi, valueNormalized), string, 2);
	return Steinberg_kResultTrue;
}

void TCharToD(Steinberg_Vst_TChar* s, double *v) {
	int i = 0;
	*v = 0.0;

	if (s[0] == '-') {
		*v = -0.0;
		i++;
	}

	while (s[i] >= '0' && s[i] <= '9') {
		char d = s[i] - '0';
		i++;
		*v = 10.0 * *v + d;
	}

	if (s[i] != '.')
		return;
	i++;

	double x = 1.0;
	while (s[i] >= '0' && s[i] <= '9') {
		char d = s[i] - '0';
		i++;
		x *= 0.1;
		*v = *v + d * x;
	}
}

static Steinberg_tresult controllerGetParamValueByString(void* thisInterface, Steinberg_Vst_ParamID id, Steinberg_Vst_TChar* string, Steinberg_Vst_ParamValue* valueNormalized) {
	(void)thisInterface;

	TRACE("controller get param value by string\n");
	int pi = parameterGetIndexById(id);
	if (pi >= DATA_PRODUCT_PARAMETERS_N + 3 * DATA_PRODUCT_BUSES_MIDI_INPUT_N || pi < 0)
		return Steinberg_kResultFalse;
	double v;
	TCharToD(string, &v);
	*valueNormalized = pi >= DATA_PRODUCT_PARAMETERS_N ? v : parameterUnmap(pi, v);
	return Steinberg_kResultTrue;
}

static Steinberg_Vst_ParamValue controllerNormalizedParamToPlain(void* thisInterface, Steinberg_Vst_ParamID id, Steinberg_Vst_ParamValue valueNormalized) {
	(void)thisInterface;

	TRACE("controller normalized param to plain\n");
	int pi = parameterGetIndexById(id);
	return pi >= DATA_PRODUCT_PARAMETERS_N ? valueNormalized : parameterMap(pi, valueNormalized);
}

static Steinberg_Vst_ParamValue controllerPlainParamToNormalized(void* thisInterface, Steinberg_Vst_ParamID id, Steinberg_Vst_ParamValue plainValue) {
	(void)thisInterface;

	TRACE("controller plain param to normalized\n");
	int pi = parameterGetIndexById(id);
	return pi >= DATA_PRODUCT_PARAMETERS_N ? plainValue : parameterUnmap(pi, plainValue);
}

static Steinberg_Vst_ParamValue controllerGetParamNormalized(void* thisInterface, Steinberg_Vst_ParamID id) {
	TRACE("controller get param normalized\n");
#if DATA_PRODUCT_PARAMETERS_N + DATA_PRODUCT_BUSES_MIDI_INPUT_N > 0
	controller *c = (controller *)((char *)thisInterface - offsetof(controller, vtblIEditController));
	int pi = parameterGetIndexById(id);
# if DATA_PRODUCT_BUSES_MIDI_INPUT_N > 0
	return pi >= DATA_PRODUCT_PARAMETERS_N ? c->parameters[pi] : parameterUnmap(pi, c->parameters[pi]);
# else
	return parameterUnmap(pi, c->parameters[pi]);
# endif
#else
	(void)thisInterface;
	(void)id;
	return 0.0;
#endif
}

static Steinberg_tresult controllerSetParamNormalized(void* thisInterface, Steinberg_Vst_ParamID id, Steinberg_Vst_ParamValue value) {
	TRACE("controller set param normalized\n");
#if DATA_PRODUCT_PARAMETERS_N + DATA_PRODUCT_BUSES_MIDI_INPUT_N > 0
	int pi = parameterGetIndexById(id);
	if (pi >= DATA_PRODUCT_PARAMETERS_N + 3 * DATA_PRODUCT_BUSES_MIDI_INPUT_N || pi < 0)
		return Steinberg_kResultFalse;
	controller *c = (controller *)((char *)thisInterface - offsetof(controller, vtblIEditController));
	c->parameters[pi] = pi >= DATA_PRODUCT_PARAMETERS_N ? value : parameterAdjust(pi, parameterMap(pi, value));
	return Steinberg_kResultTrue;
#else
	(void)thisInterface;
	(void)id;
	(void)value;
	return Steinberg_kResultFalse;
#endif
}

static Steinberg_tresult controllerSetComponentHandler(void* thisInterface, struct Steinberg_Vst_IComponentHandler* handler) {
	TRACE("controller set component handler\n");
	controller *c = (controller *)((char *)thisInterface - offsetof(controller, vtblIEditController));
	if (c->componentHandler != handler) {
		if (c->componentHandler != NULL)
			c->componentHandler->lpVtbl->release(c->componentHandler);
		c->componentHandler = handler;
		if (c->componentHandler != NULL)
			c->componentHandler->lpVtbl->addRef(c->componentHandler);
	}
	return Steinberg_kResultTrue;
}

static struct Steinberg_IPlugView* controllerCreateView(void* thisInterface, Steinberg_FIDString name) {
	(void)thisInterface;
	(void)name;

	TRACE("controller create view\n");
	//TBD
	return NULL;
}

static Steinberg_Vst_IEditControllerVtbl controllerVtblIEditController = {
	/* FUnknown */
	/* .queryInterface		= */ controllerIEditControllerQueryInterface,
	/* .addRef			= */ controllerIEditControllerAddRef,
	/* .release			= */ controllerIEditControllerRelease,

	/* IPluginBase */
	/* .initialize			= */ controllerInitialize,
	/* .terminate			= */ controllerTerminate,

	/* IEditController */
	/* .setComponentState		= */ controllerSetComponentState,
	/* .setState			= */ controllerSetState,
	/* .getState			= */ controllerGetState,
	/* .getParameterCount		= */ controllerGetParameterCount,
	/* .getParameterInfo		= */ controllerGetParameterInfo,
	/* .getParamStringByValue	= */ controllerGetParamStringByValue,
	/* .getParamValueByString	= */ controllerGetParamValueByString,
	/* .normalizedParamToPlain	= */ controllerNormalizedParamToPlain,
	/* .plainParamToNormalized	= */ controllerPlainParamToNormalized,
	/* .getParamNormalized		= */ controllerGetParamNormalized,
	/* .setParamNormalized		= */ controllerSetParamNormalized,
	/* .setComponentHandler		= */ controllerSetComponentHandler,
	/* .createView			= */ controllerCreateView
};

static Steinberg_tresult controllerIMidiMappingQueryInterface(void* thisInterface, const Steinberg_TUID iid, void** obj) {
	TRACE("controller IMidiMapping queryInterface %p\n", thisInterface);
	return controllerQueryInterface((controller *)((char *)thisInterface - offsetof(controller, vtblIMidiMapping)), iid, obj);
}

static Steinberg_uint32 controllerIMidiMappingAddRef(void* thisInterface) {
	TRACE("controller IMidiMapping addRef %p\n", thisInterface);
	return controllerAddRef((controller *)((char *)thisInterface - offsetof(controller, vtblIMidiMapping)));
}

static Steinberg_uint32 controllerIMidiMappingRelease (void* thisInterface) {
	TRACE("controller IMidiMapping release %p\n", thisInterface);
	return controllerRelease((controller *)((char *)thisInterface - offsetof(controller, vtblIMidiMapping)));
}

static Steinberg_tresult controllerGetMidiControllerAssignment(void* thisInterface, Steinberg_int32 busIndex, Steinberg_int16 channel, Steinberg_Vst_CtrlNumber midiControllerNumber, Steinberg_Vst_ParamID* id) {
	(void)thisInterface;
	(void)channel;

	TRACE("controller getMidiControllerAssignment\n");
	if (busIndex < 0 || busIndex >= DATA_PRODUCT_BUSES_MIDI_INPUT_N)
		return Steinberg_kInvalidArgument;
	switch (midiControllerNumber) {
	case Steinberg_Vst_ControllerNumbers_kAfterTouch:
		*id = parameterInfo[DATA_PRODUCT_PARAMETERS_N + 3 * busIndex].id;
		return Steinberg_kResultTrue;
		break;
	case Steinberg_Vst_ControllerNumbers_kPitchBend:
		*id = parameterInfo[DATA_PRODUCT_PARAMETERS_N + 3 * busIndex + 1].id;
		return Steinberg_kResultTrue;
		break;
	case Steinberg_Vst_ControllerNumbers_kCtrlModWheel:
		*id = parameterInfo[DATA_PRODUCT_PARAMETERS_N + 3 * busIndex + 2].id;
		return Steinberg_kResultTrue;
		break;
	default:
		return Steinberg_kResultFalse;
		break;
	}
}

static Steinberg_Vst_IMidiMappingVtbl controllerVtblIMidiMapping = {
	/* FUnknown */
	/* .queryInterface		= */ controllerIMidiMappingQueryInterface,
	/* .addRef			= */ controllerIMidiMappingAddRef,
	/* .release			= */ controllerIMidiMappingRelease,

	/* IMidiMapping */
	/* .getMidiControllerAssignment	= */ controllerGetMidiControllerAssignment
};

static Steinberg_tresult factoryQueryInterface(void *thisInterface, const Steinberg_TUID iid, void ** obj) {
	TRACE("factory queryInterface\n");
	if (memcmp(iid, Steinberg_FUnknown_iid, sizeof(Steinberg_TUID))
	    && memcmp(iid, Steinberg_IPluginFactory_iid, sizeof(Steinberg_TUID))
	    && memcmp(iid, Steinberg_IPluginFactory2_iid, sizeof(Steinberg_TUID))
	    && memcmp(iid, Steinberg_IPluginFactory3_iid, sizeof(Steinberg_TUID))) {
		TRACE(" not supported\n");
		*obj = NULL;
		return Steinberg_kNoInterface;
	}
	*obj = thisInterface;
	return Steinberg_kResultOk;
}

static Steinberg_uint32 factoryAddRef(void *thisInterface) {
	(void)thisInterface;

	TRACE("factory add ref\n");
	return 1;
}

static Steinberg_uint32 factoryRelease(void *thisInterface) {
	(void)thisInterface;

	TRACE("factory release\n");
	return 1;
}

static Steinberg_tresult factoryGetFactoryInfo(void *thisInterface, struct Steinberg_PFactoryInfo * info) {
	(void)thisInterface;

	TRACE("getFactoryInfo\n");
	strcpy(info->vendor, DATA_COMPANY_NAME);
	strcpy(info->url, DATA_COMPANY_URL);
	strcpy(info->email, DATA_COMPANY_EMAIL);
	info->flags = Steinberg_PFactoryInfo_FactoryFlags_kUnicode;
	return Steinberg_kResultOk;
}

static Steinberg_int32 factoryCountClasses(void *thisInterface) {
	(void)thisInterface;

	TRACE("countClasses\n");
	return 2;
}

static Steinberg_tresult factoryGetClassInfo(void *thisInterface, Steinberg_int32 index, struct Steinberg_PClassInfo * info) {
	(void)thisInterface;

	TRACE("getClassInfo\n");
	switch (index) {
	case 0:
		TRACE(" class 0\n");
		memcpy(info->cid, dataPluginCID, sizeof(Steinberg_TUID));
		info->cardinality = Steinberg_PClassInfo_ClassCardinality_kManyInstances;
		strcpy(info->category, "Audio Module Class");
		strcpy(info->name, DATA_PRODUCT_NAME);
		break;
	case 1:
		TRACE(" class 1\n");
		memcpy(info->cid, dataControllerCID, sizeof(Steinberg_TUID));
		info->cardinality = Steinberg_PClassInfo_ClassCardinality_kManyInstances;
		strcpy(info->category, "Component Controller Class");
		strcpy(info->name, DATA_PRODUCT_NAME " Controller");
		break;
	default:
		return Steinberg_kInvalidArgument;
		break;
	}
	return Steinberg_kResultOk;
}

static Steinberg_tresult factoryCreateInstance(void *thisInterface, Steinberg_FIDString cid, Steinberg_FIDString iid, void ** obj) {
	(void)thisInterface;

	TRACE("createInstance\n");
	if (memcmp(cid, dataPluginCID, sizeof(Steinberg_TUID)) == 0) {
		TRACE(" plugin\n");
		size_t offset; // does it actually work like this? or is offset always 0? this seems to be correct and works...
		if ((memcmp(iid, Steinberg_FUnknown_iid, sizeof(Steinberg_TUID)) == 0)
		    || (memcmp(iid, Steinberg_IPluginBase_iid, sizeof(Steinberg_TUID)) != 0)
		    || (memcmp(iid, Steinberg_Vst_IComponent_iid, sizeof(Steinberg_TUID)) != 0)) {
			TRACE("  IComponent\n");
			offset = offsetof(pluginInstance, vtblIComponent);
		} else if (memcmp(iid, Steinberg_Vst_IAudioProcessor_iid, sizeof(Steinberg_TUID)) != 0) {
			TRACE("  IAudioProcessor\n");
			offset = offsetof(pluginInstance, vtblIAudioProcessor);
		} else if (memcmp(iid, Steinberg_Vst_IProcessContextRequirements_iid, sizeof(Steinberg_TUID)) != 0) {
			TRACE("  IProcessContextRequirements\n");
			offset = offsetof(pluginInstance, vtblIProcessContextRequirements);
		} else {
			TRACE("  INothing :(\n");
			for (int i = 0; i < 16; i++)
				TRACE(" %x", iid[i]);
			TRACE("\n");
			return Steinberg_kNoInterface;
		}
		pluginInstance *p = malloc(sizeof(pluginInstance));
		if (p == NULL)
			return Steinberg_kOutOfMemory;
		p->vtblIComponent = &pluginVtblIComponent;
		p->vtblIAudioProcessor = &pluginVtblIAudioProcessor;
		p->vtblIProcessContextRequirements = &pluginVtblIProcessContextRequirements;
		p->refs = 1;
		p->context = NULL;
		*obj = (void *)((char *)p + offset);
		TRACE(" instance: %p\n", (void *)p);
	} else if (memcmp(cid, dataControllerCID, sizeof(Steinberg_TUID)) == 0) {
		TRACE(" controller\n");
		if (memcmp(iid, Steinberg_FUnknown_iid, sizeof(Steinberg_TUID))
		    && memcmp(iid, Steinberg_IPluginBase_iid, sizeof(Steinberg_TUID))
		    && memcmp(iid, Steinberg_Vst_IEditController_iid, sizeof(Steinberg_TUID)))
			return Steinberg_kNoInterface;
		controller *c = malloc(sizeof(controller));
		if (c == NULL)
			return Steinberg_kOutOfMemory;
		c->vtblIEditController = &controllerVtblIEditController;
		c->vtblIMidiMapping = &controllerVtblIMidiMapping;
		c->refs = 1;
		c->context = NULL;
		c->componentHandler = NULL;
		*obj = c;
		TRACE(" instance: %p\n", (void *)c);
	} else {
		*obj = NULL;
		return Steinberg_kNoInterface;
	}
	return Steinberg_kResultOk;
}

static Steinberg_tresult factoryGetClassInfo2(void* thisInterface, Steinberg_int32 index, struct Steinberg_PClassInfo2* info) {
	(void)thisInterface;

	TRACE("getClassInfo2\n");
	switch (index) {
	case 0:
		TRACE(" class 0\n");
		memcpy(info->cid, dataPluginCID, sizeof(Steinberg_TUID));
		info->cardinality = Steinberg_PClassInfo_ClassCardinality_kManyInstances;
		strcpy(info->category, "Audio Module Class");
		strcpy(info->name, DATA_PRODUCT_NAME);
		info->classFlags = Steinberg_Vst_ComponentFlags_kDistributable;
		strcpy(info->subCategories, DATA_VST3_SUBCATEGORY);
		*info->vendor = '\0';
		strcpy(info->version, DATA_PRODUCT_VERSION);
		strcpy(info->sdkVersion, "VST " DATA_VST3_SDK_VERSION " | Tibia");
		break;
	case 1:
		TRACE(" class 1\n");
		memcpy(info->cid, dataControllerCID, sizeof(Steinberg_TUID));
		info->cardinality = Steinberg_PClassInfo_ClassCardinality_kManyInstances;
		strcpy(info->category, "Component Controller Class");
		strcpy(info->name, DATA_PRODUCT_NAME " Controller");
		info->classFlags = 0;
		*info->subCategories = '\0';
		*info->vendor = '\0';
		strcpy(info->version, DATA_PRODUCT_VERSION);
		strcpy(info->sdkVersion, "VST " DATA_VST3_SDK_VERSION " | Tibia");
		break;
	default:
		return Steinberg_kInvalidArgument;
		break;
	}
	return Steinberg_kResultOk;
}

static Steinberg_tresult factoryGetClassInfoUnicode(void* thisInterface, Steinberg_int32 index, struct Steinberg_PClassInfoW* info) {
	(void)thisInterface;

	TRACE("getClassInfo unicode\n");
	switch (index) {
	case 0:
		TRACE(" class 0\n");
		memcpy(info->cid, dataPluginCID, sizeof(Steinberg_TUID));
		info->cardinality = Steinberg_PClassInfo_ClassCardinality_kManyInstances;
		strcpy(info->category, "Audio Module Class");
		memcpy(info->name, dataProductNameW, 64 * sizeof(Steinberg_char16));
		info->classFlags = Steinberg_Vst_ComponentFlags_kDistributable;
		strcpy(info->subCategories, DATA_VST3_SUBCATEGORY);
		*info->vendor = '\0';
		memcpy(info->version, dataProductVersionW, 64 * sizeof(Steinberg_char16));
		memcpy(info->sdkVersion, dataVST3SDKVersionW, 64 * sizeof(Steinberg_char16));
		break;
	case 1:
		TRACE(" class 1\n");
		memcpy(info->cid, dataControllerCID, sizeof(Steinberg_TUID));
		info->cardinality = Steinberg_PClassInfo_ClassCardinality_kManyInstances;
		strcpy(info->category, "Component Controller Class");
		memcpy(info->name, dataVST3ControllerNameW, 64 * sizeof(Steinberg_char16));
		info->classFlags = 0;
		*info->subCategories = '\0';
		*info->vendor = '\0';
		memcpy(info->version, dataProductVersionW, 64 * sizeof(Steinberg_char16));
		memcpy(info->sdkVersion, dataVST3SDKVersionW, 64 * sizeof(Steinberg_char16));
		break;
	default:
		return Steinberg_kInvalidArgument;
		break;
	}
	return Steinberg_kResultOk;
}

static Steinberg_tresult factorySetHostContext(void* thisInterface, struct Steinberg_FUnknown* context) {
	(void)thisInterface;
	(void)context;

	TRACE("factory set host context\n");
	//XXX: Linux run loop...
	return Steinberg_kNotImplemented;
}

static Steinberg_IPluginFactory3Vtbl factoryVtbl = {
	/* FUnknown */
	/* .queryInterface	= */ factoryQueryInterface,
	/* .addRef		= */ factoryAddRef,
	/* .release		= */ factoryRelease,

	/* IPluginFactory */
	/* .getFactoryInfo	= */ factoryGetFactoryInfo,
	/* .countClasses	= */ factoryCountClasses,
	/* .getClassInfo	= */ factoryGetClassInfo,
	/* .createInstance	= */ factoryCreateInstance,

	/* IPluginFactory2 */
	/* .getClassInfo2	= */ factoryGetClassInfo2,

	/* IPluginFactory3 */
	/* .getClassInfoUnicode	= */ factoryGetClassInfoUnicode,
	/* .setHostContext	= */ factorySetHostContext
};
static Steinberg_IPluginFactory3 factory = { &factoryVtbl };

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __attribute__((visibility("default")))
#endif

EXPORT
Steinberg_IPluginFactory * GetPluginFactory(void) {
	return (Steinberg_IPluginFactory *)&factory;
}

#ifndef _WIN32
# if defined(__APPLE__)
#  define ENTRY bundleEntry
#  define EXIT bundleExit
# else
#  define ENTRY ModuleEntry
#  define EXIT ModuleExit
# endif

EXPORT
char ENTRY(void *handle) {
	(void)handle;

	return 1;
}

EXPORT
char EXIT(void) {
	return 1;
}

#endif
