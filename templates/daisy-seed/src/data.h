#define AUDIO_BUS_IN			{{=it.product.buses.findIndex(x => x.type == "audio" && x.direction == "input" && !x.cv && !x.sidechain)}}
#define AUDIO_BUS_OUT			{{=it.product.buses.findIndex(x => x.type == "audio" && x.direction == "output" && !x.cv && !x.sidechain)}}

#define NUM_CHANNELS_IN			{{=it.product.buses.filter(x => x.type == "audio" && x.direction == "input" && !x.cv && !x.sidechain) ? (it.product.buses.filter(x => x.type == "audio" && x.direction == "input" && !x.cv && !x.sidechain)[0].channels == "mono" ? 1 : 2) : 0}}
#define NUM_CHANNELS_OUT		{{=it.product.buses.filter(x => x.type == "audio" && x.direction == "output" && !x.cv && !x.sidechain) ? (it.product.buses.filter(x => x.type == "audio" && x.direction == "output" && !x.cv && !x.sidechain)[0].channels == "mono" ? 1 : 2) : 0}}

#define MIDI_BUS_IN			{{=it.product.buses.findIndex(x => x.type == "midi" && x.direction == "input")}}

#define NUM_PARAMETERS			{{=it.product.parameters.length}}

#define NUM_ADC				{{=it.product.parameters.filter((x, i) => x.direction == "input" && it.daisy_seed.parameterPins[i] >= 0).length}}
#define NUM_DAC				{{=it.product.parameters.filter((x, i) => x.direction == "output" && it.daisy_seed.parameterPins[i] >= 0).length}}

#if NUM_PARAMETERS > 0

# define PARAM_BYPASS	1
# define PARAM_TOGGLED	(1<<1)
# define PARAM_INTEGER	(1<<2)
# define PARAM_MAP_LOG	(1<<3)

static struct {
	char		out;
	int8_t		pin;
	float		min;
	float		max;
	float		def;
	uint32_t	flags;
	float		mapK;
} param_data[NUM_PARAMETERS] = {
{{~it.product.parameters :p:i}}
	{
		/* .out		= */ {{=p.direction == "output" ? 1 : 0}},
		/* .pin		= */ {{=it.daisy_seed.parameterPins[i]}},
		/* .min		= */ {{=p.minimum.toExponential()}}f,
		/* .max		= */ {{=p.maximum.toExponential()}}f,
		/* .def		= */ {{=p.defaultValue.toExponential()}}f,
		/* .flags	= */ {{?p.isBypass}}PARAM_BYPASS{{??}}0{{?p.toggled}} | PARAM_TOGGLED{{?}}{{?p.integer}} | PARAM_INTEGER{{?}}{{?p.map == "logarithmic"}} | PARAM_MAP_LOG{{?}}{{?}},
		/* .mapK	= */ {{?p.map == "logarithmic"}}{{=Number(2.0 * Math.log(Math.sqrt(p.maximum * p.minimum) / Math.abs(p.minimum))).toExponential()}}{{??}}0.0{{?}}f
	},
{{~}}
};

# if MIDI_BUS_IN >= 0

#  define HAS_MIDI_CC_MAPS	{{=it.daisy_seed.midiCCMaps ? 1 : 0}}

#  if HAS_MIDI_CC_MAPS
static int midi_cc_maps[NUM_PARAMETERS] = { {{~it.daisy_seed.midiCCMaps :p}}{{=p}} ,{{~}} };
#  endif

# endif

#endif
