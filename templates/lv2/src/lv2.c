#include <stdlib.h>
#include <stdint.h>

#include "data.h"
#include "plugin.h"

#include "lv2/core/lv2.h"
#if DATA_PRODUCT_MIDI_INPUTS_N + DATA_PRODUCT_MIDI_OUTPUTS_N > 0
#include "lv2/core/lv2_util.h"
#include "lv2/atom/util.h"
#include "lv2/atom/atom.h"
#include "lv2/log/log.h"
#include "lv2/log/logger.h"
#include "lv2/midi/midi.h"
#include "lv2/urid/urid.h"
#endif

#if defined(__i386__) || defined(__x86_64__)
#include <xmmintrin.h>
#include <pmmintrin.h>
#endif

typedef struct {
	plugin				p;
#if DATA_PRODUCT_AUDIO_INPUT_CHANNELS_N > 0
	const float *			x[DATA_PRODUCT_AUDIO_INPUT_CHANNELS_N];
#endif
#if DATA_PRODUCT_AUDIO_OUTPUT_CHANNELS_N > 0
	float *				y[DATA_PRODUCT_AUDIO_OUTPUT_CHANNELS_N];
#endif
#if DATA_PRODUCT_MIDI_INPUTS_N > 0
	const LV2_Atom_Sequence *	x_midi[DATA_PRODUCT_MIDI_INPUTS_N];
#endif
#if DATA_PRODUCT_MIDI_OUTPUTS_N > 0
	LV2_Atom_Sequence *		y_midi[DATA_PRODUCT_MIDI_OUTPUTS_N];
#endif
#if (DATA_PRODUCT_CONTROL_INPUTS_N + DATA_PRODUCT_CONTROL_OUTPUTS_N) > 0
	float *				c[DATA_PRODUCT_CONTROL_INPUTS_N + DATA_PRODUCT_CONTROL_OUTPUTS_N];
#endif
#if DATA_PRODUCT_CONTROL_INPUTS_N > 0
	float				params[DATA_PRODUCT_CONTROL_INPUTS_N];
#endif
	void *				mem;
#if DATA_PRODUCT_MIDI_INPUTS_N + DATA_PRODUCT_MIDI_OUTPUT_N > 0
	LV2_URID_Map *			map;
	LV2_Log_Logger			logger;
	LV2_URID			uri_midi_MidiEvent;
#endif
} plugin_instance;

static LV2_Handle instantiate(const struct LV2_Descriptor * descriptor, double sample_rate, const char * bundle_path, const LV2_Feature * const * features) {
	(void)descriptor;
	(void)bundle_path;

	plugin_instance *instance = malloc(sizeof(plugin_instance));
	if (instance == NULL)
		return NULL;

#if DATA_PRODUCT_MIDI_INPUTS_N + DATA_PRODUCT_MIDI_OUTPUT_N > 0
	// from https://lv2plug.in/book
	const char* missing = lv2_features_query(features,
		LV2_LOG__log,	&instance->logger.log,	false,
		LV2_URID__map,	&instance->map,		true,
		NULL);

	lv2_log_logger_set_map(&instance->logger, instance->map);
	if (missing) {
		lv2_log_error(&instance->logger, "Missing feature <%s>\n", missing);
		free(instance);
		return NULL;
	}

	instance->uri_midi_MidiEvent = instance->map->map(instance->map->handle, LV2_MIDI__MidiEvent);
#endif

	plugin_init(&instance->p);

	plugin_set_sample_rate(&instance->p, sample_rate);
	size_t req = plugin_mem_req(&instance->p);
	if (req != 0) {
		instance->mem = malloc(req);
		if (instance->mem == NULL) {
			plugin_fini(&instance->p);
			return NULL;
		}
		plugin_mem_set(&instance->p, instance->mem);
	} else
		instance->mem = NULL;

#if DATA_PRODUCT_AUDIO_INPUT_CHANNELS_N > 0
	for (uint32_t i = 0; i < DATA_PRODUCT_AUDIO_INPUT_CHANNELS_N; i++)
		instance->x[i] = NULL;
#endif
#if DATA_PRODUCT_AUDIO_OUTPUT_CHANNELS_N > 0
	for (uint32_t i = 0; i < DATA_PRODUCT_AUDIO_OUTPUT_CHANNELS_N; i++)
		instance->y[i] = NULL;
#endif
#if DATA_PRODUCT_MIDI_INPUTS_N > 0
	for (uint32_t i = 0; i < DATA_PRODUCT_MIDI_INPUTS_N; i++)
		instance->x_midi[i] = NULL;
#endif
#if DATA_PRODUCT_MIDI_OUTPUTS_N > 0
	for (uint32_t i = 0; i < DATA_PRODUCT_MIDI_OUTPUTS_N; i++)
		instance->y_midi[i] = NULL;
#endif
#if (DATA_PRODUCT_CONTROL_INPUTS_N + DATA_PRODUCT_CONTROL_OUTPUTS_N) > 0
	for (uint32_t i = 0; i < DATA_PRODUCT_CONTROL_INPUTS_N + DATA_PRODUCT_CONTROL_OUTPUTS_N; i++)
		instance->c[i] = NULL;
#endif

	return instance;
}

static void connect_port(LV2_Handle instance, uint32_t port, void * data_location) {
	plugin_instance * i = (plugin_instance *)instance;
#if DATA_PRODUCT_AUDIO_INPUT_CHANNELS_N > 0
	if (port < DATA_PRODUCT_AUDIO_INPUT_CHANNELS_N) {
		i->x[port] = data_location;
		return;
	}
	port -= DATA_PRODUCT_AUDIO_INPUT_CHANNELS_N;
#endif
#if DATA_PRODUCT_AUDIO_OUTPUT_CHANNELS_N > 0
	if (port < DATA_PRODUCT_AUDIO_OUTPUT_CHANNELS_N) {
		i->y[port] = data_location;
		return;
	}
	port -= DATA_PRODUCT_AUDIO_OUTPUT_CHANNELS_N;
#endif
#if DATA_PRODUCT_MIDI_INPUTS_N > 0
	if (port < DATA_PRODUCT_MIDI_INPUTS_N) {
		i->x_midi[port] = data_location;
		return;
	}
	port -= DATA_PRODUCT_MIDI_INPUTS_N;
#endif
#if DATA_PRODUCT_MIDI_OUTPUTS_N > 0
	if (port < DATA_PRODUCT_MIDI_OUTPUTS_N) {
		i->y_midi[port] = data_location;
		return;
	}
	port -= DATA_PRODUCT_MIDI_OUTPUTS_N;
#endif
#if (DATA_PRODUCT_CONTROL_INPUTS_N + DATA_PRODUCT_CONTROL_OUTPUTS_N) > 0
	i->c[port] = data_location;
#endif
}

static void activate(LV2_Handle instance) {
	plugin_instance * i = (plugin_instance *)instance;
#if DATA_PRODUCT_CONTROL_INPUTS_N > 0
	for (uint32_t j = 0; j < DATA_PRODUCT_CONTROL_INPUTS_N; j++) {
		uint32_t k = param_data[j].index;
		i->params[j] = i->c[k] != NULL ? *i->c[k] : param_data[j].def;
		plugin_set_parameter(&i->p, k, i->params[j]);
	}
#endif
	plugin_reset(&i->p);
}

static inline float clampf(float x, float m, float M) {
	return x < m ? m : (x > M ? M : x);
}

static void run(LV2_Handle instance, uint32_t sample_count) {
	plugin_instance * i = (plugin_instance *)instance;

#if DATA_PRODUCT_CONTROL_INPUTS_N > 0
	for (uint32_t j = 0; j < DATA_PRODUCT_CONTROL_INPUTS_N; j++) {
		uint32_t k = param_data[j].index;
		if (i->c[k] == NULL)
			continue;
		float v;
		if (param_data[j].flags & DATA_PARAM_BYPASS)
			v = *i->c[k] > 0.f ? 0.f : 1.f;
		else if (param_data[j].flags & DATA_PARAM_TOGGLED)
			v = *i->c[k] > 0.f ? 1.f : 0.f;
		else if (param_data[j].flags & DATA_PARAM_INTEGER)
			v = (int32_t)(*i->c[k] + 0.5f);
		else
			v = *i->c[k];

		v = clampf(v, param_data[j].min, param_data[j].max);
		if (v != i->params[j]) {
			i->params[j] = v;
			plugin_set_parameter(&i->p, k, v);
		}
	}
#endif

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

	// from https://lv2plug.in/book
#if DATA_PRODUCT_MIDI_INPUTS_N > 0
	for (size_t j = 0; j < DATA_PRODUCT_MIDI_INPUTS_N; j++) {
		if (i->x_midi[j] == NULL)
			continue;
		LV2_ATOM_SEQUENCE_FOREACH(i->x_midi[j], ev) {
			if (ev->body.type == i->uri_midi_MidiEvent) {
				const uint8_t * data = (const uint8_t *)(ev + 1);
				if ((data[0] & 0xf0) != 0xf0)
					plugin_midi_msg_in(&i->p, midi_in_index[j], data);
			}
		}
	}
#endif

	plugin_process(&i->p, i->x, i->y, sample_count);

#if defined(__aarch64__)
	__asm__ __volatile__ ("msr fpcr, %0" : : "r"(fpcr));
#elif defined(__i386__) || defined(__x86_64__)
	_MM_SET_FLUSH_ZERO_MODE(flush_zero_mode);
	_MM_SET_DENORMALS_ZERO_MODE(denormals_zero_mode);
#endif

#if DATA_PRODUCT_CONTROL_OUTPUTS_N > 0
	for (uint32_t j = 0; j < DATA_PRODUCT_CONTROL_OUTPUTS_N; j++) {
		uint32_t k = param_out_index[j];
		if (i->c[k] != NULL)
			*i->c[k] = plugin_get_parameter(&i->p, k);
	}
#endif
}

static void cleanup(LV2_Handle instance) {
	plugin_instance * i = (plugin_instance *)instance;
	plugin_fini(&i->p);
	if (i->mem)
		free(i->mem);
	free(instance);
}

static const LV2_Descriptor descriptor = {
	/* .URI			= */ DATA_LV2_URI,
	/* .instantiate		= */ instantiate,
	/* .connect_port	= */ connect_port,
	/* .activate		= */ activate,
	/* .run			= */ run,
	/* .deactivate		= */ NULL,
	/* .cleanup		= */ cleanup,
	/* .extension_data	= */ NULL
};

LV2_SYMBOL_EXPORT const LV2_Descriptor * lv2_descriptor(uint32_t index) {
	return index == 0 ? &descriptor : NULL;
}
