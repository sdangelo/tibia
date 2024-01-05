#include <math.h>

typedef struct plugin {
	float	gain;
	char	bypass;
} plugin;

void plugin_init(plugin *instance) {
	instance->gain = 1.f;
	instance->bypass = 0;
}

void plugin_fini(plugin *instance) {
}

void plugin_set_sample_rate(plugin *instance, float sample_rate) {
}

void plugin_reset(plugin *instance) {
}

void plugin_set_parameter(plugin *instance, size_t index, float value) {
	switch (index) {
	case 0:
		instance->gain = value;
		break;
	case 1:
		instance->bypass = value >= 0.5f;
		break;
	}
}

void plugin_process(plugin *instance, const float **inputs, float **outputs, size_t n_samples) {
	const float g = instance->bypass ? 1.f : powf(10.f, 0.05f * instance->gain);
	for (size_t i = 0; i < n_samples; i++)
		outputs[0][i] = g * inputs[0][i];
}
