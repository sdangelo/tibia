#include "lv2/core/lv2.h"
#include <stdlib.h>

#include "data.h"
#include "plugin.h"

typedef struct {
	plugin		p;
#if DATA_PRODUCT_AUDIO_INPUT_CHANNELS_N > 0
	const float *	x[DATA_PRODUCT_AUDIO_INPUT_CHANNELS_N];
#endif
#if DATA_PRODUCT_AUDIO_OUTPUT_CHANNELS_N > 0
	float *		y[DATA_PRODUCT_AUDIO_OUTPUT_CHANNELS_N];
#endif
#if (DATA_PRODUCT_CONTROL_INPUTS_N + DATA_PRODUCT_CONTROL_OUTPUTS_N) > 0
	float *		c[DATA_PRODUCT_CONTROL_INPUTS_N + DATA_PRODUCT_CONTROL_OUTPUTS_N];
#endif
#if DATA_PRODUCT_CONTROL_INPUTS_N > 0
	float		params[DATA_PRODUCT_CONTROL_INPUTS_N];
#endif
} plugin_instance;

static LV2_Handle instantiate(const struct LV2_Descriptor * descriptor, double sample_rate, const char * bundle_path, const LV2_Feature * const * features) {
	plugin_instance *instance = malloc(sizeof(plugin_instance));
	if (instance == NULL)
		return NULL;
	plugin_init(&instance->p);
	plugin_set_sample_rate(&instance->p, sample_rate);
#if DATA_PRODUCT_AUDIO_INPUT_CHANNELS_N > 0
	for (size_t i = 0; i < DATA_PRODUCT_AUDIO_INPUT_CHANNELS_N; i++)
		instance->x[i] = NULL;
#endif
#if DATA_PRODUCT_AUDIO_OUTPUT_CHANNELS_N > 0
	for (size_t i = 0; i < DATA_PRODUCT_AUDIO_OUTPUT_CHANNELS_N; i++)
		instance->y[i] = NULL;
#endif
#if DATA_PRODUCT_CONTROL_INPUTS_N > 0
	for (size_t i = 0; i < DATA_PRODUCT_CONTROL_INPUTS_N; i++)
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
#if (DATA_PRODUCT_CONTROL_INPUTS_N + DATA_PRODUCT_CONTROL_OUTPUTS_N) > 0
	i->c[port] = data_location;
#endif
}

static void activate(LV2_Handle instance) {
	plugin_instance * i = (plugin_instance *)instance;
#if DATA_PRODUCT_CONTROL_INPUTS_N > 0
	for (size_t j = 0; j < DATA_PRODUCT_CONTROL_INPUTS_N; j++) {
		i->params[j] = i->c[j] != NULL ? *i->c[j] : param_data[j].def;
		plugin_set_parameter(&i->p, j, i->params[j]);
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
	for (size_t j = 0; j < DATA_PRODUCT_CONTROL_INPUTS_N; j++) {
		if (i->c[j] == NULL)
			continue;
		float v;
		if (param_data[j].flags & DATA_PARAM_BYPASS)
			v = *i->c[j] > 0.f ? 0.f : 1.f;
		else if (param_data[j].flags & DATA_PARAM_TOGGLED)
			v = *i->c[j] > 0.f ? 1.f : 0.f;
		else
			v = clampf(*i->c[j], param_data[j].min, param_data[j].max);

		if (v != i->params[j]) {
			i->params[j] = v;
			plugin_set_parameter(&i->p, j, v);
		}
	}
#endif

	plugin_process(&i->p, i->x, i->y, sample_count);
}

static void cleanup(LV2_Handle instance) {
	plugin_instance * i = (plugin_instance *)instance;
	plugin_fini(&i->p);
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
