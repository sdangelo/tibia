#include <stdlib.h>
#include <stdint.h>

#include "data.h"
#include "plugin.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <math.h>

#include <tinywav.h>

plugin		instance;
void *		mem;
#if NUM_CHANNELS_IN > 0
float *		x[NUM_CHANNELS_IN];
#endif
#if NUM_CHANNELS_OUT > 0
float *		y[NUM_CHANNELS_OUT];
#endif
float		fs = 44100.f;
size_t		bufsize = 128;
#if NUM_CHANNELS_IN == 0
float		length = 1.f;
#endif
#if PARAMETERS_N > 0
float		param_values[PARAMETERS_N];
#endif
const char *	infile = NULL;
const char *	outfile = NULL;

void usage(const char * argv0) {
#if NUM_CHANNELS_IN > 0
	fprintf(stderr, "Usage: %s [bufsize=value] infile", argv0);
#else
	fprintf(stderr, "Usage: %s [fs=value] [bufsize=value] [length=value]", argv0);
#endif
#if NUM_CHANNELS_OUT > 0
	fprintf(stderr, " outfile");
#endif
#if PARAMETERS_N > 0
	fprintf(stderr, " [param=value] ...");
#endif
	fprintf(stderr, "\n");

#if NUM_CHANNELS_IN > 0
	fprintf(stderr, " defaults: bufsize=128");
#else
	fprintf(stderr, " defaults: fs=44100, bufsize=128");
#endif
#if PARAMETERS_N > 0
	for (size_t i = 0; i < PARAMETERS_N; i++)
		if (!param_data[i].out)
			fprintf(stderr, ", %s=%g", param_data[i].id, param_data[i].def);
#endif
	fprintf(stderr, "\n");
}

float clampf(float x, float m, float M) {
	return x < m ? m : (x > M ? M : x);
}

int main(int argc, char * argv[]) {
#if PARAMETERS_N > 0
	for (size_t i = 0; i < PARAMETERS_N; i++)
		param_values[i] = param_data[i].def;
#endif

	char parsingState = 0; // 0 = fs/bufsize/length, 1 = filenames, 2 = params
	for (int i = 1; i < argc; i++) {
		switch (parsingState) {
		case 0:
		{
			char * c = strchr(argv[i], '=');
			if (c == NULL) {
				parsingState = 1;
				i--;
				continue;
			}
			if (strncmp(argv[i], "bufsize", 7) == 0) {
				char * e;
				ssize_t v = strtol(c + 1, &e, 10);
				if (errno || v <= 0 || *e != '\0') {
					fprintf(stderr, "invalid format of argument '%s'\n", argv[i]);
					usage(argv[0]);
					return EXIT_FAILURE;
				}
				bufsize = v;
#if NUM_CHANNELS_IN == 0
			} else if (strncmp(argv[i], "fs", 2) == 0) {
				char * e;
				float v = strtof(c + 1, &e);
				if (errno || !isfinite(v) || v <= 0.f || *e != '\0') {
					fprintf(stderr, "invalid format of argument '%s'\n", argv[i]);
					usage(argv[0]);
					return EXIT_FAILURE;
				}
				fs = v;
			} else if (strncmp(argv[i], "length", 6) == 0) {
				char * e;
				float v = strtof(c + 1, &e);
				if (errno || !isfinite(v) || v <= 0.f || *e != '\0') {
					fprintf(stderr, "invalid format of argument '%s'\n", argv[i]);
					usage(argv[0]);
					return EXIT_FAILURE;
				}
				length = v;
#endif
			} else {
				fprintf(stderr, "invalid format of argument '%s'\n", argv[i]);
				usage(argv[0]);
				return EXIT_FAILURE;
			}
		}
			break;
		case 1:
		{
			char * c = strchr(argv[i], '=');
			if (c != NULL) {
#if PARAMETERS_N == 0
				fprintf(stderr, "invalid format of argument '%s'\n", argv[i]);
				usage(argv[0]);
				return EXIT_FAILURE;
#endif
				parsingState = 2;
				i--;
				continue;
			}
			const char ** next = NULL;
#if NUM_CHANNELS_IN > 0
			if (infile == NULL)
				next = &infile;
#endif
#if NUM_CHANNELS_OUT > 0
			if (next == NULL && outfile == NULL)
				next = &outfile;
#endif
			if (next == NULL) {
				fprintf(stderr, "invalid argument '%s' (in/out files already specified)\n", argv[i]);
				usage(argv[0]);
				return EXIT_FAILURE;
			}
			*next = argv[i];
		}
			break;
#if PARAMETERS_N > 0
		case 2:
		{
			char * c = strchr(argv[i], '=');
			if (c == NULL) {
				fprintf(stderr, "invalid format of argument '%s'\n", argv[i]);
				usage(argv[0]);
				return EXIT_FAILURE;
			}
			char * e;
			float v = strtof(c + 1, &e);
			if (errno || !isfinite(v) || *e != '\0') {
				fprintf(stderr, "invalid format of argument '%s'\n", argv[i]);
				usage(argv[0]);
				return EXIT_FAILURE;
			}
			int len = c - argv[i];
			int j = 0;
			for (; j < PARAMETERS_N; j++) {
				if (strncmp(argv[i], param_data[j].id, len) == 0 && param_data[j].id[len] == '\0')
					break;
			}
			if (j == PARAMETERS_N) {
				fprintf(stderr, "parameter for '%s' not found\n", argv[i]);
				usage(argv[0]);
				return EXIT_FAILURE;
			}
			param_values[j] = v;

		}
			break;
#endif
		}
	}
#if NUM_CHANNELS_IN > 0
	if (infile == NULL) {
		fprintf(stderr, "infile not specified\n");
		usage(argv[0]);
		return EXIT_FAILURE;
	}
#endif
#if NUM_CHANNELS_OUT > 0
	if (outfile == NULL) {
		fprintf(stderr, "outfile not specified\n");
		usage(argv[0]);
		return EXIT_FAILURE;
	}
#endif

#if PARAMETERS_N > 0
	for (size_t i = 0; i < PARAMETERS_N; i++) {
		if (param_data[i].out)
			continue;
		float v = param_values[i];
		if (param_data[i].flags & (PARAM_BYPASS | PARAM_TOGGLED))
			v = v > 0.5f ? 1.f : 0.f;
		else if (param_data[i].flags & PARAM_INTEGER)
			v = (int32_t)(v + 0.5f);

		param_values[i] = clampf(v, param_data[i].min, param_data[i].max);
	}
#endif

#if NUM_CHANNELS_IN > 0
	TinyWav tw_in;
	if (tinywav_open_read(&tw_in, infile, TW_SPLIT) != 0)
		return EXIT_FAILURE;
	if (tw_in.h.NumChannels != NUM_CHANNELS_IN) {
		fprintf(stderr, "input file has %d channels but %d channels were expected\n", tw_in.h.NumChannels, NUM_CHANNELS_IN);
		return EXIT_FAILURE;
	}
	fs = tw_in.h.SampleRate;
#endif

	printf(" fs: %g\n", fs);
	printf(" bufsize: %zu\n", bufsize);
#if NUM_CHANNELS_IN > 0
	printf(" length: %g\n", (double)tw_in.numFramesInHeader / (double)tw_in.h.SampleRate);
	printf(" infile: %s\n", infile);
#else
	printf(" length: %g\n", length);
#endif
#if NUM_CHANNELS_OUT > 0
	printf(" outfile: %s\n", outfile);
#endif
#if PARAMETERS_N > 0
	for (size_t i = 0; i < PARAMETERS_N; i++)
		if (!param_data[i].out)
			printf(" %s: %g\n", param_data[i].id, param_values[i]);
#endif

	plugin_init(&instance);

#if PARAMETERS_N > 0
	for (size_t i = 0; i < PARAMETERS_N; i++)
		if (!param_data[i].out)
			plugin_set_parameter(&instance, i, param_values[i]);
#endif

	plugin_set_sample_rate(&instance, fs);
	size_t req = plugin_mem_req(&instance);
	if (req != 0) {
		mem = malloc(req);
		if (mem == NULL) {
			plugin_fini(&instance);
#if NUM_CHANNELS_IN > 0
			tinywav_close_read(&tw_in);
#endif
			fprintf(stderr, "Out of memory\n");
			return EXIT_FAILURE;
		}
		plugin_mem_set(&instance, mem);
	} else
	       mem = NULL;	

	plugin_reset(&instance);

#if NUM_CHANNELS_IN > 0
	float * x_buf = malloc(NUM_CHANNELS_IN * bufsize * sizeof(float));
	if (x_buf == NULL) {
		if (mem != NULL)
			free(mem);
		plugin_fini(&instance);
		tinywav_close_read(&tw_in);
		fprintf(stderr, "Out of memory\n");
		return EXIT_FAILURE;
	}
	for (size_t i = 0; i < NUM_CHANNELS_IN; i++)
		x[i] = x_buf + i * bufsize;
#else
	float ** x = NULL;
#endif

#if NUM_CHANNELS_OUT > 0
	float * y_buf = malloc(NUM_CHANNELS_OUT * bufsize * sizeof(float));
	if (y_buf == NULL) {
# if NUM_CHANNELS_IN > 0
		free(x_buf);
# endif
		if (mem != NULL)
			free(mem);
		plugin_fini(&instance);
# if NUM_CHANNELS_IN > 0
		tinywav_close_read(&tw_in);
# endif
		fprintf(stderr, "Out of memory\n");
		return EXIT_FAILURE;
	}
	for (size_t i = 0; i < NUM_CHANNELS_IN; i++)
		y[i] = y_buf + i * bufsize;
#else
	float ** y = NULL;
#endif

#if NUM_CHANNELS_OUT > 0
	TinyWav tw_out;
	if (tinywav_open_write(&tw_out, 1, fs, TW_FLOAT32, TW_SPLIT, outfile) != 0) {
		free(y_buf);
# if NUM_CHANNELS_IN > 0
		free(x_buf);
# endif
		if (mem != NULL)
			free(mem);
		plugin_fini(&instance);
# if NUM_CHANNELS_IN > 0
		tinywav_close_read(&tw_in);
# endif
		return EXIT_FAILURE;
	}
#endif

#if NUM_CHANNELS_IN > 0
	while (1) {
		int32_t n = tinywav_read_f(&tw_in, x, bufsize);
		if (n == 0)
			break;
		plugin_process(&instance, (const float **)x, y, n);
# if PARAMETERS_N > 0
		for (size_t j = 0; j < PARAMETERS_N; j++) {
			if (!param_data[j].out)
				continue;
			param_values[j] = plugin_get_parameter(&instance, j);
			printf("  %s: %g\n", param_data[j].id, param_values[j]);
		}
# endif
# if NUM_CHANNELS_OUT > 0
		tinywav_write_f(&tw_out, y, n);
# endif
	}
#else
	size_t i = 0;
	size_t len = (size_t)(tw_in.h.SampleRate * length + 0.5f);
	while (i < len) {
		size_t left = len - i;
		size_t n = left > bufsize ? bufsize : left;
		plugin_process(&instance, (const float **)x, y, n);
# if PARAMETERS_N > 0
		for (size_t j = 0; j < PARAMETERS_N; j++) {
			if (!param_data[j].out)
				continue;
			param_values[j] = plugin_get_parameter(&instance, j);
			printf("  %s: %g\n", param_data[j].id, param_values[j]);
		}
# endif
# if NUM_CHANNELS_OUT > 0
		tinywav_write_f(&tw_out, y, n);
# endif
		i += n;
	}
#endif

#if NUM_CHANNELS_OUT > 0
	tinywav_close_write(&tw_out);
	free(y_buf);
#endif
#if NUM_CHANNELS_IN > 0
	free(x_buf);
#endif
	if (mem != NULL)
		free(mem);
	plugin_fini(&instance);
#if NUM_CHANNELS_IN > 0
	tinywav_close_read(&tw_in);
#endif

	return EXIT_SUCCESS;
}
