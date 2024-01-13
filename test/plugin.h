#include <string.h>
#include <math.h>

typedef struct plugin {
	float	sample_rate;
	size_t	delay_line_length;

	float	gain;
	float	delay;
	char	bypass;

	float *	delay_line;
	size_t	delay_line_cur;
} plugin;

static void plugin_init(plugin *instance) {
}

static void plugin_fini(plugin *instance) {
}

static void plugin_set_sample_rate(plugin *instance, float sample_rate) {
	instance->sample_rate = sample_rate;
	instance->delay_line_length = ceilf(sample_rate) + 1;
}

static size_t plugin_mem_req(plugin *instance) {
	return instance->delay_line_length * sizeof(float);
}

static void plugin_mem_set(plugin *instance, void *mem) {
	instance->delay_line = mem;
}

static void plugin_reset(plugin *instance) {
	memset(instance->delay_line, 0, instance->delay_line_length * sizeof(float));
	instance->delay_line_cur = 0;
}

static void plugin_set_parameter(plugin *instance, size_t index, float value) {
	switch (index) {
	case 0:
		instance->gain = powf(10.f, 0.05f * value);
		break;
	case 1:
		instance->delay = 0.001f * value;
		break;
	case 2:
		instance->bypass = value >= 0.5f;
		break;
	}
}

static float plugin_get_parameter(plugin *instance, size_t index) {
	// no output parameters
	return 0.f;
}

static size_t calc_index(size_t cur, size_t delay, size_t len) {
	return (cur < delay ? cur + len : cur) - delay;
}

static void plugin_process(plugin *instance, const float **inputs, float **outputs, size_t n_samples) {
	size_t delay = roundf(instance->sample_rate * instance->delay);
	for (size_t i = 0; i < n_samples; i++) {
		instance->delay_line[instance->delay_line_cur] = inputs[0][i];
		const float y = instance->delay_line[calc_index(instance->delay_line_cur, delay, instance->delay_line_length)];
		instance->delay_line_cur++;
		if (instance->delay_line_cur == instance->delay_line_length)
			instance->delay_line_cur = 0;
		outputs[0][i] = instance->bypass ? inputs[0][i] : instance->gain * y;
	}
}
