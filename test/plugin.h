typedef struct plugin {
	float	sample_rate;
	size_t	delay_line_length;

	float	gain;
	float	delay;
	float	cutoff;
	char	bypass;

	float *	delay_line;
	size_t	delay_line_cur;
	float	z1;
	float	cutoff_k;
	float	yz1;
} plugin;

static void plugin_init(plugin *instance) {
}

static void plugin_fini(plugin *instance) {
}

static void plugin_set_sample_rate(plugin *instance, float sample_rate) {
	instance->sample_rate = sample_rate;
	//safe approx instance->delay_line_length = ceilf(sample_rate) + 1;
	instance->delay_line_length = (size_t)(sample_rate + 1.f) + 1;
}

static size_t plugin_mem_req(plugin *instance) {
	return instance->delay_line_length * sizeof(float);
}

static void plugin_mem_set(plugin *instance, void *mem) {
	instance->delay_line = mem;
}

static void plugin_reset(plugin *instance) {
	for (size_t i = 0; i < instance->delay_line_length; i++)
		instance->delay_line[i] = 0.f;
	instance->delay_line_cur = 0;
	instance->z1 = 0.f;
	instance->cutoff_k = 1.f;
	instance->yz1 = 0.f;
}

static void plugin_set_parameter(plugin *instance, size_t index, float value) {
	switch (index) {
	case 0:
		//approx instance->gain = powf(10.f, 0.05f * value);
		instance->gain = ((2.6039890429412597e-4f * value + 0.032131027163547855f) * value + 1.f) / ((0.0012705124328080768f * value - 0.0666763481312185f) * value + 1.f);
		break;
	case 1:
		instance->delay = 0.001f * value;
		break;
	case 2:
		instance->cutoff = value;
		break;
	case 3:
		instance->bypass = value >= 0.5f;
		break;
	}
}

static float plugin_get_parameter(plugin *instance, size_t index) {
	return instance->yz1;
}

static size_t calc_index(size_t cur, size_t delay, size_t len) {
	return (cur < delay ? cur + len : cur) - delay;
}

static void plugin_process(plugin *instance, const float **inputs, float **outputs, size_t n_samples) {
	//approx size_t delay = roundf(instance->sample_rate * instance->delay);
	size_t delay = (size_t)(instance->sample_rate * instance->delay + 0.5f);
	const float mA1 = instance->sample_rate / (instance->sample_rate + 6.283185307179586f * instance->cutoff * instance->cutoff_k);
	for (size_t i = 0; i < n_samples; i++) {
		instance->delay_line[instance->delay_line_cur] = inputs[0][i];
		const float x = instance->delay_line[calc_index(instance->delay_line_cur, delay, instance->delay_line_length)];
		instance->delay_line_cur++;
		if (instance->delay_line_cur == instance->delay_line_length)
			instance->delay_line_cur = 0;
		const float y = x + mA1 * (instance->z1 - x);
		instance->z1 = y;
		outputs[0][i] = instance->bypass ? inputs[0][i] : instance->gain * y;
		instance->yz1 = outputs[0][i];
	}
}

static void plugin_note_on(plugin *instance, size_t index, uint8_t note, float velocity) {
	//approx instance->cutoff_k = powf(2.f, (1.f / 12.f) * (note - 60));
	instance->cutoff_k = note < 64 ? (-0.19558034980097166f * note - 2.361735109225749f) / (note - 75.57552349522389f) : (393.95397927344214f - 7.660826245588588f * note) / (note - 139.0755234952239f);
}

static void plugin_note_off(plugin *instance, size_t index, uint8_t note, float velocity) {
}

static void plugin_all_sounds_off(plugin *instance, size_t index) {
}

static void plugin_all_notes_off(plugin *instance, size_t index) {
}

static void plugin_channel_pressure(plugin *instance, size_t index, float value) {
}

static void plugin_pitch_bend_change(plugin *instance, size_t index, float value) {
}
