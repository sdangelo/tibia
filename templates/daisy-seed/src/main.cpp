#include <stdlib.h>
#include <stdint.h>

#include "data.h"
#include "plugin.h"

#include <math.h>

#include "daisy_seed.h"

#define BLOCK_SIZE 32

using namespace daisy;

DaisySeed hardware;
#if MIDI_BUS_IN >= 0
MidiUartHandler midi_uart;
MidiUsbHandler midi_usb;
#else
CpuLoadMeter loadMeter;
#endif

plugin instance;

float buf[2][BLOCK_SIZE];
float *bufs[2] = { buf[0], buf[1] };

#if NUM_ADC >= 0
static float clampf(float x, float m, float M) {
	return x < m ? m : (x > M ? M : x);
}

static float parameterMap(int i, float v) {
	return param_data[i].flags & PARAM_MAP_LOG ? param_data[i].min * expf(param_data[i].mapK * v) : param_data[i].min + (param_data[i].max - param_data[i].min) * v;
}

/*
static float parameterUnmap(int i, float v) {
	return param_data[i].flags & PARAM_MAP_LOG ? logf(v / param_data[i].min) / param_data[i].mapK : (v - param_data[i].min) / (param_data[i].max - param_data[i].min);
}
*/

static float parameterAdjust(int i, float v) {
	v = param_data[i].flags & (PARAM_BYPASS | PARAM_TOGGLED) ? (v >= 0.5f ? 1.f : 0.f)
		: (param_data[i].flags & PARAM_INTEGER ? (int32_t)(v + 0.5f) : v);
	return clampf(v, param_data[i].min, param_data[i].max);
}

static void setParameter(int i, float v) {
	plugin_set_parameter(&instance, i, parameterAdjust(i, v));
}

static void readADCs() {
	for (int i = 0, j = 0; i < NUM_PARAMETERS; i++) {
		if (param_data[i].out || param_data[i].pin < 0)
			continue;
		setParameter(i, parameterMap(i, hardware.adc.GetFloat(j)));
		j++;
	}
}
#endif

static void AudioCallback(
		AudioHandle::InterleavingInputBuffer in,
		AudioHandle::InterleavingOutputBuffer out,
		size_t size) {
#if MIDI_BUS_IN < 0
	loadMeter.OnBlockStart();
#endif

	readADCs();

	const size_t n = size >> 1;
#if NUM_CHANNELS_IN > 0
	for (size_t i = 0; i < n; i++) {
		const size_t j = i << 1;
		buf[0][i] = in[j];
# if NUM_CHANNELS_IN > 1
		buf[1][i] = in[j + 1];
# endif
	}
#endif

	plugin_process(&instance, (const float **)bufs, bufs, n);

	for (size_t i = 0; i < n; i++) {
		const size_t j = i << 1;
#if NUM_CHANNELS_OUT > 0
		out[j] = buf[0][i];
#else
		out[j] = 0.f;
#endif
#if NUM_CHANNELS_OUT > 1
		out[j + 1] = buf[1][i];
#else
		out[j + 1] = 0.f;
#endif
	}

#if MIDI_BUS_IN < 0
	loadMeter.OnBlockEnd();
#endif
}

int main() {
	hardware.Configure();
	hardware.Init();

#if NUM_ADC >= 0
	AdcChannelConfig adcConfig[NUM_ADC];
	for (int i = 0, j = 0; i < NUM_PARAMETERS; i++) {
		if (param_data[i].out || param_data[i].pin < 0)
			continue;
		adcConfig[j].InitSingle(hardware.GetPin(param_data[i].pin));
		j++;
	}

	hardware.adc.Init(adcConfig, NUM_ADC);
	hardware.adc.Start();
#endif

	hardware.SetAudioBlockSize(BLOCK_SIZE);
	float sample_rate = hardware.AudioSampleRate();

	plugin_init(&instance);

	plugin_set_sample_rate(&instance, sample_rate);
	if (plugin_mem_req(&instance) != 0)
		plugin_mem_set(&instance, (void *)0xc0000000);

#if NUM_PARAMETERS > 0
	for (int i = 0; i < NUM_PARAMETERS; i++) {
		if (param_data[i].out)
			continue;
		setParameter(i, param_data[i].def);
	}
#endif

#if MIDI_BUS_IN < 0
	hardware.StartLog();
	loadMeter.Init(sample_rate, BLOCK_SIZE);
#endif

#if NUM_ADC >= 0
	readADCs();
#endif
	plugin_reset(&instance);

	hardware.StartAudio(AudioCallback);

	while (1) {
#if MIDI_BUS_IN >= 0
		midi_usb.Listen();
		midi_uart.Listen();
		while (midi_usb.HasEvents() || midi_uart.HasEvents()) {
			MidiEvent ev = midi_usb.HasEvents() ? midi_usb.PopEvent() : midi_uart.PopEvent();
			uint8_t data[3];
			switch (ev.type) {
			case NoteOff:
				data[0] = 0x80;
				break;
			case NoteOn:
				data[0] = 0x90;
				break;
			case PolyphonicKeyPressure:
				data[0] = 0xa0;
				break;
			case ControlChange:
# if NUM_PARAMETERS > 0 && HAS_MIDI_CC_MAPS
				for (int i = 0; i < NUM_PARAMETERS; i++)
					if (midi_cc_maps[i] == data[1]) {
						setParameter(i, (1.f / 127.f) * data[2]);
						goto loopNext;
					}
# endif
			case ChannelMode:
				data[0] = 0xb0;
				break;
			case ProgramChange:
				data[0] = 0xc0;
				break;
			case ChannelPressure:
				data[0] = 0xd0;
				break;
			case PitchBend:
				data[0] = 0xe0;
				break;
			default:
				continue;
				break;
			}
			data[0] |= ev.channel;
			data[1] = ev.data[1];
			data[2] = ev.data[2];
			plugin_midi_msg_in(&instance, MIDI_BUS_IN, data);
# if NUM_PARAMETERS > 0 && HAS_MIDI_CC_MAPS
loopNext:
			(void)data; // something to make this file build
# endif
		}
#else
		const float avgLoad = loadMeter.GetAvgCpuLoad();
		const float maxLoad = loadMeter.GetMaxCpuLoad();
		const float minLoad = loadMeter.GetMinCpuLoad();
		hardware.PrintLine("---");
# if NUM_PARAMETERS > 0
		for (int i = 0; i < NUM_PARAMETERS; i++)
			if (param_data[i].out)
				hardware.PrintLine("parameter #%i: %g", i, plugin_get_parameter(&instance, i));
# endif
		hardware.PrintLine("---");
		hardware.PrintLine("Processing Load %:");
		hardware.PrintLine("Max: " FLT_FMT3, FLT_VAR3(maxLoad * 100.0f));
		hardware.PrintLine("Avg: " FLT_FMT3, FLT_VAR3(avgLoad * 100.0f));
		hardware.PrintLine("Min: " FLT_FMT3, FLT_VAR3(minLoad * 100.0f));
		System::Delay(500);
#endif
	}

	(void)plugin_fini;
#if MIDI_BUS_IN >= 0 && NUM_PARAMETERS > 0
	(void)plugin_get_parameter;
#endif
}
