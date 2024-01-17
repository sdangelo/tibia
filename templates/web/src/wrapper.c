/*
 * Copyright (C) 2022-2024 Orastron Srl unipersonale
 */

#include <stddef.h>
#include <stdint.h>

#include "data.h"
#include "plugin.h"

#include "memset.h"
#include "walloc.h"

typedef struct {
	plugin		p;
	void *		mem;
#if DATA_PRODUCT_AUDIO_INPUT_CHANNELS_N > 0
	float		x_buf[DATA_PRODUCT_AUDIO_INPUT_CHANNELS_N * 128];
	const float *	x[DATA_PRODUCT_AUDIO_INPUT_CHANNELS_N];
#endif
#if DATA_PRODUCT_AUDIO_OUTPUT_CHANNELS_N > 0
	float		y_buf[DATA_PRODUCT_AUDIO_OUTPUT_CHANNELS_N * 128];
	float *		y[DATA_PRODUCT_AUDIO_OUTPUT_CHANNELS_N];
#endif
#if DATA_PRODUCT_PARAMETERS_OUTPUT_N > 0
	float		out_params[DATA_PRODUCT_PARAMETERS_OUTPUT_N];
#endif
} instance;

instance * wrapper_new(float sample_rate) {
	instance * i = malloc(sizeof(instance));
	if (i == NULL)
		return NULL;

	plugin_init(&i->p);

#if DATA_PRODUCT_PARAMETERS_N > 0
	for (size_t j = 0; j < DATA_PRODUCT_PARAMETERS_N; j++)
		if (!param_data[j].out)
			plugin_set_parameter(&i->p, j, param_data[j].def);
#endif

	plugin_set_sample_rate(&i->p, sample_rate);
	size_t req = plugin_mem_req(&i->p);
	if (req != 0) {
		i->mem = malloc(req);
		if (i->mem == NULL) {
			plugin_fini(&i->p);
			return NULL;
		}
		plugin_mem_set(&i->p, i->mem);
	} else
		i->mem = NULL;

	plugin_reset(&i->p);

#if DATA_PRODUCT_AUDIO_INPUT_CHANNELS_N > 0
	for (size_t j = 0; j < DATA_PRODUCT_AUDIO_INPUT_CHANNELS_N; j++)
		i->x[j] = i->x_buf + 128 * j;
#endif
#if DATA_PRODUCT_AUDIO_OUTPUT_CHANNELS_N > 0
	for (size_t j = 0; j < DATA_PRODUCT_AUDIO_OUTPUT_CHANNELS_N; j++)
		i->y[j] = i->y_buf + 128 * j;
#endif

	return i;
}

void wrapper_free(instance * i) {
	plugin_fini(&i->p);
	if (i->mem)
		free(i->mem);
	free(i);
}

float * wrapper_get_x_buf(instance * i) {
#if DATA_PRODUCT_AUDIO_INPUT_CHANNELS_N > 0
	return i->x_buf;
#else
	(void *)i;
	return NULL;
#endif
}

float * wrapper_get_y_buf(instance * i) {
#if DATA_PRODUCT_AUDIO_INPUT_CHANNELS_N > 0
	return i->y_buf;
#else
	(void *)i;
	return NULL;
#endif
}

float * wrapper_get_out_params(instance *i) {
#if DATA_PRODUCT_PARAMETERS_OUTPUT_N > 0
	return i->out_params;
#else
	(void)i;
	return NULL;
#endif
}

void wrapper_set_parameter(instance *i, int32_t index, float value) {
	plugin_set_parameter(&i->p, index, value);
}

void wrapper_process(instance *i, int32_t n_samples) {
#if DATA_PRODUCT_AUDIO_INPUT_CHANNELS_N > 0
	const float **x = i->x;
#else
	const float **x = NULL;
#endif
#if DATA_PRODUCT_AUDIO_OUTPUT_CHANNELS_N > 0
	float **y = i->y;
#else
	float **y = NULL;
#endif

	plugin_process(&i->p, x, y, n_samples);

#if DATA_PRODUCT_PARAMETERS_OUTPUT_N > 0
	for (size_t j = 0; j < DATA_PRODUCT_PARAMETERS_OUTPUT_N; j++)
		i->out_params[j] = plugin_get_parameter(&i->p, param_out_index[j]);
#endif
}
