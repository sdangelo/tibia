typedef struct plugin {
	// FILL ME
	char abc;
} plugin;

void plugin_init(plugin *instance) {
	// WRITE ME
}

void plugin_fini(plugin *instance) {
	// WRITE ME
}

void plugin_set_sample_rate(plugin *instance, float sample_rate) {
	// WRITE ME
}

void plugin_reset(plugin *instance) {
	// WRITE ME
}

void plugin_set_parameter(plugin *instance, size_t index, float value) {
	// WRITE ME
}

void plugin_process(plugin *instance, const float **inputs, float **outputs, size_t n_samples) {
	// WRITE ME
}
