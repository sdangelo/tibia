#include <stdlib.h>
#include <stdint.h>

#include "data.h"
#include "plugin.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <math.h>

#if NUM_CHANNELS_IN + NUM_CHANNELS_OUT > 0
# include <tinywav.h>
#endif
#if NUM_MIDI_INPUTS > 0
# include <midi-parser.h>
#endif

plugin			instance;
void *			mem;
#if (NUM_NON_OPT_CHANNELS_IN > NUM_CHANNELS_IN) || (NUM_NON_OPT_CHANNELS_OUT > NUM_CHANNELS_OUT)
float			zero[BLOCK_SIZE];
#endif
#if NUM_ALL_CHANNELS_IN > 0
float *			x[NUM_ALL_CHANNELS_IN];
#else
const float **		x = NULL;
#endif
#if NUM_ALL_CHANNELS_OUT > 0
float *			y[NUM_ALL_CHANNELS_OUT];
#else
float **		y = NULL;
#endif
float			fs = 44100.f;
size_t			bufsize = 128;
#if NUM_CHANNELS_IN == 0
float			length = 1.f;
#endif
#if PARAMETERS_N > 0
float			param_values[PARAMETERS_N];
#endif
#if NUM_CHANNELS_IN > 0
const char *		infile = NULL;
#endif
#if NUM_CHANNELS_OUT > 0
const char *		outfile = NULL;
#endif
#if NUM_MIDI_INPUTS > 0
const char *		midifile = NULL;
void *			midi_data = NULL;
struct midi_parser	midi_parser;
int16_t			midi_ticks = 0;
uint32_t		midi_tempo = 500000; // microseconds per quarter-note -> 120 bpm
enum midi_parser_status	midi_status;
double			midi_next;
char			midi_next_read;
#endif

void usage(const char * argv0) {
#if NUM_CHANNELS_IN > 0
	fprintf(stderr, "Usage: %s [bufsize=value] infile", argv0);
#else
	fprintf(stderr, "Usage: %s [fs=value] [bufsize=value] [length=value]", argv0);
#endif
#if NUM_CHANNELS_OUT > 0
	fprintf(stderr, " outfile");
#endif
#if NUM_MIDI_INPUTS > 0
	fprintf(stderr, " [midifile]");
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

#if NUM_MIDI_INPUTS > 0
char * read_file(const char * filename, int32_t * size) {
	FILE * fp = fopen(filename, "r");
	if (fp == NULL)
		return NULL;
	if (fseek(fp, 0, SEEK_END) != 0) {
		fclose(fp);
		return NULL;
	}
	*size = ftell(fp);
	if (*size < 0) {
		fclose(fp);
		return NULL;
	}
	void * mem = malloc(*size);
	if (mem == NULL) {
		fclose(fp);
		return NULL;
	}
	if (fseek(fp, 0L, SEEK_SET) != 0) {
		free(mem);
		fclose(fp);
		return NULL;
	}
	size_t n = fread(mem, 1, *size, fp);
	if (n != (uint32_t)(*size)) {
		free(mem);
		fclose(fp);
		return NULL;
	}
	fclose(fp);
	return mem;
}
#endif

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
#if NUM_MIDI_INPUTS > 0
			if (next == NULL && midifile == NULL)
				next = &midifile;
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

	int exit_code = EXIT_FAILURE;

#if NUM_CHANNELS_IN > 0
	TinyWav tw_in;
	if (tinywav_open_read(&tw_in, infile, TW_SPLIT) != 0)
		return EXIT_FAILURE;
	if (tw_in.h.NumChannels != NUM_CHANNELS_IN) {
		fprintf(stderr, "input file has %d channels but %d channels were expected\n", tw_in.h.NumChannels, NUM_CHANNELS_IN);
		goto err_num_channels_in;
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
#if NUM_MIDI_INPUTS > 0
	printf(" midifile: %s\n", midifile ? midifile : "[none]");
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
			fprintf(stderr, "Out of memory\n");
			goto err_mem_alloc;
		}
		plugin_mem_set(&instance, mem);
	} else
	       mem = NULL;	

	plugin_reset(&instance);

#if NUM_CHANNELS_IN > 0
	float * x_buf = malloc(NUM_CHANNELS_IN * bufsize * sizeof(float));
	if (x_buf == NULL) {
		fprintf(stderr, "Out of memory\n");
		goto err_x_buf;
	}
#endif
#if NUM_ALL_CHANNELS_IN > 0
# if AUDIO_BUS_IN >= 0
	size_t ix = 0;
	size_t ixb = 0;
	for (size_t j = 0; j < NUM_AUDIO_BUSES_IN + NUM_AUDIO_BUSES_OUT; j++) {
		if (audio_bus_data[j].out)
			continue;
		if (audio_bus_data[j].index == AUDIO_BUS_IN)
			for (char k = 0; k < audio_bus_data[j].channels; k++, ix++, ixb++)
				x[ix] = x_buf + bufsize * ixb;
#  if NUM_NON_OPT_CHANNELS_IN > NUM_CHANNELS_IN
		else if (!audio_bus_data[j].optional)
			for (char k = 0; k < audio_bus_data[j].channels; k++, ix++)
				x[ix] = zero;
#  endif
		else
			for (char k = 0; k < audio_bus_data[j].channels; k++, ix++)
				x[ix] = NULL;
	}
# else
	for (size_t i = 0; i < NUM_ALL_CHANNELS_IN; i++)
		x[i] = NULL;
# endif
#endif

#if NUM_CHANNELS_OUT > 0
	float * y_buf = malloc(NUM_CHANNELS_OUT * bufsize * sizeof(float));
	if (y_buf == NULL) {
		fprintf(stderr, "Out of memory\n");
		goto err_y_buf;
	}
#endif
#if NUM_ALL_CHANNELS_OUT > 0
# if AUDIO_BUS_OUT >= 0
	size_t iy = 0;
	size_t iyb = 0;
	for (size_t j = 0; j < NUM_AUDIO_BUSES_IN + NUM_AUDIO_BUSES_OUT; j++) {
		if (!audio_bus_data[j].out)
			continue;
		if (audio_bus_data[j].index == AUDIO_BUS_OUT)
			for (char k = 0; k < audio_bus_data[j].channels; k++, iy++, iyb++)
				y[iy] = y_buf + bufsize * iyb;
#  if NUM_NON_OPT_CHANNELS_OUT > NUM_CHANNELS_OUT
		else if (!audio_bus_data[j].optional)
			for (char k = 0; k < audio_bus_data[j].channels; k++, iy++)
				y[iy] = zero;
#  endif
		else
			for (char k = 0; k < audio_bus_data[j].channels; k++, iy++)
				y[iy] = NULL;
	}
# else
	for (size_t i = 0; i < NUM_ALL_CHANNELS_OUT; i++)
		y[i] = NULL;
# endif
#endif

#if NUM_MIDI_INPUTS > 0
	if (midifile != NULL) {
		int32_t midi_data_size;
		midi_data = read_file(midifile, &midi_data_size);
		if (midi_data == NULL)
			goto err_midi_read;

		midi_parser.state = MIDI_PARSER_INIT;
		midi_parser.size = midi_data_size;
		midi_parser.in = midi_data;

		midi_status = midi_parse(&midi_parser);
		if (midi_status != MIDI_PARSER_HEADER) {
			fprintf(stderr, "Header not found in MIDI file\n");
			goto err_midi_parse;
		}
		if (midi_parser.header.format != 0) {
			fprintf(stderr, "Only MIDI file format 0 is supported\n");
			goto err_midi_parse;
		}
		if ((midi_parser.header.time_division & 0x80) != 0x80) {
			fprintf(stderr, "Only ticks per quarter-note time division is supported when reading MIDI files\n");
			goto err_midi_parse;
		}
		if (midi_parser.header.time_division == 0) {
			fprintf(stderr, "Invalid 0 tick per quarter-note in MIDI file\n");
			goto err_midi_parse;
		}

		midi_ticks = midi_parser.header.time_division;
		midi_next = 0.0;
		midi_next_read = 1;
	} else {
		midi_status = MIDI_PARSER_EOB;
		midi_next = 0.0;
		midi_next_read = 0;
	}
#endif

#if NUM_CHANNELS_OUT > 0
	TinyWav tw_out;
	if (tinywav_open_write(&tw_out, 1, fs, TW_FLOAT32, TW_SPLIT, outfile) != 0)
		goto err_outfile;
#endif

#if NUM_CHANNELS_IN > 0
	while (1) {
		int32_t n = tinywav_read_f(&tw_in, x, bufsize);
		if (n == 0)
			break;

# if NUM_NON_OPT_CHANNELS_IN > NUM_CHANNELS_IN
		memset(zero, 0, bufsize * sizeof(float));
# endif

#else
	size_t i = 0;
	size_t len = (size_t)(tw_in.h.SampleRate * length + 0.5f);
	while (i < len) {
		size_t left = len - i;
		size_t n = left > bufsize ? bufsize : left;
#endif

#if NUM_MIDI_INPUTS > 0
		while (1) {
			if (midi_next > 0.0)
				break;
			else if (midi_next_read == 0) {
				if (midi_status == MIDI_PARSER_TRACK_META && midi_parser.meta.type == MIDI_META_SET_TEMPO)
					midi_tempo = (midi_parser.meta.bytes[0] << 16) | (midi_parser.meta.bytes[1] << 8) | midi_parser.meta.bytes[2];
				else if (midi_status == MIDI_PARSER_TRACK_MIDI) {
					uint8_t data[3] = { (midi_parser.midi.status << 4) | midi_parser.midi.channel, midi_parser.midi.param1, midi_parser.midi.param2 };
					plugin_midi_msg_in(&instance, MIDI_BUS_IN, data);
				}
				midi_next_read = 1;
			}

			if (midi_status == MIDI_PARSER_EOB)
				break;

			midi_status = midi_parse(&midi_parser);
			switch (midi_status) {
			case MIDI_PARSER_ERROR:
			case MIDI_PARSER_HEADER:
				fprintf(stderr, "Error while parsing MIDI file\n");
				goto err_midi_parse;
				break;
			case MIDI_PARSER_TRACK_META:
			case MIDI_PARSER_TRACK_MIDI:
			case MIDI_PARSER_TRACK_SYSEX:
				midi_next += ((double)midi_tempo / (double)midi_ticks) * midi_parser.vtime;
				midi_next_read = 0;
				break;
			default:
				break;
			}
		}
		midi_next -= 1e6 * ((double)n / (double)tw_in.h.SampleRate);
#endif

		plugin_process(&instance, (const float **)x, y, n);

#if PARAMETERS_N > 0
		for (size_t j = 0; j < PARAMETERS_N; j++) {
			if (!param_data[j].out)
				continue;
			param_values[j] = plugin_get_parameter(&instance, j);
			printf("  %s: %g\n", param_data[j].id, param_values[j]);
		}
#endif

#if NUM_CHANNELS_OUT > 0
		tinywav_write_f(&tw_out, y, n);
#endif

#if NUM_CHANNELS_IN == 0
		i += n;
#endif
	}

	exit_code = EXIT_SUCCESS;

#if NUM_CHANNELS_OUT > 0
	tinywav_close_write(&tw_out);
#endif
err_outfile:
err_midi_parse:
#if NUM_MIDI_INPUTS > 0
	if (midi_data != NULL)
		free(midi_data);
#endif
err_midi_read:
#if NUM_CHANNELS_OUT > 0
	free(y_buf);
#endif
err_y_buf:
#if NUM_CHANNELS_IN > 0
	free(x_buf);
#endif
err_x_buf:
	if (mem != NULL)
		free(mem);
err_mem_alloc:
	plugin_fini(&instance);
err_num_channels_in:
#if NUM_CHANNELS_IN > 0
	tinywav_close_read(&tw_in);
#endif

	return exit_code;
}
