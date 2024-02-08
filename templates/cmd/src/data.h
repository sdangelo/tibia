#define NUM_AUDIO_BUSES_IN		{{=it.product.buses.filter(x => x.type == "audio" && x.direction == "input").length}}
#define NUM_AUDIO_BUSES_OUT		{{=it.product.buses.filter(x => x.type == "audio" && x.direction == "output").length}}

#define AUDIO_BUS_IN			{{=it.product.buses.findIndex(x => x.type == "audio" && x.direction == "input" && !x.cv && !x.sidechain && !x.optional)}}
#define AUDIO_BUS_OUT			{{=it.product.buses.findIndex(x => x.type == "audio" && x.direction == "output" && !x.cv && !x.sidechain && !x.optional)}}

#define NUM_CHANNELS_IN			{{=it.product.buses.filter(x => x.type == "audio" && x.direction == "input" && !x.cv && !x.sidechain && !x.optional) ? (it.product.buses.filter(x => x.type == "audio" && x.direction == "input" && !x.cv && !x.sidechain && !x.optional)[0].channels == "mono" ? 1 : 2) : 0}}
#define NUM_CHANNELS_OUT		{{=it.product.buses.filter(x => x.type == "audio" && x.direction == "output" && !x.cv && !x.sidechain && !x.optional) ? (it.product.buses.filter(x => x.type == "audio" && x.direction == "output" && !x.cv && !x.sidechain && !x.optional)[0].channels == "mono" ? 1 : 2) : 0}}
#define NUM_NON_OPT_CHANNELS_IN		{{=it.product.buses.filter(x => x.type == "audio" && x.direction == "input" && !x.optional).reduce((a, x) => a + (x.channels == "mono" ? 1 : 2), 0)}}
#define NUM_NON_OPT_CHANNELS_OUT	{{=it.product.buses.filter(x => x.type == "audio" && x.direction == "output" && !x.optional).reduce((a, x) => a + (x.channels == "mono" ? 1 : 2), 0)}}
#define NUM_ALL_CHANNELS_IN		{{=it.product.buses.filter(x => x.type == "audio" && x.direction == "input").reduce((a, x) => a + (x.channels == "mono" ? 1 : 2), 0)}}
#define NUM_ALL_CHANNELS_OUT		{{=it.product.buses.filter(x => x.type == "audio" && x.direction == "output").reduce((a, x) => a + (x.channels == "mono" ? 1 : 2), 0)}}

#define NUM_MIDI_INPUTS			{{=it.product.buses.filter(x => x.type == "midi" && x.direction == "input").length}}

#define MIDI_BUS_IN			{{=it.product.buses.findIndex(x => x.type == "midi" && x.direction == "input")}}

#if NUM_AUDIO_BUSES_IN + NUM_AUDIO_BUSES_OUT > 0
static struct {
	char	out;
	char	optional;
	char	channels;
} audio_bus_data[NUM_AUDIO_BUSES_IN + NUM_AUDIO_BUSES_OUT] = {
{{~it.product.buses :b:i}}
{{?b.type == "audio"}}
	{
		/* .out		= */ {{=b.direction == "output" ? 1 : 0}},
		/* .optional	= */ {{=b.optional ? 1 : 0}},
		/* .channels	= */ {{=b.channels == "mono" ? 1 : 2}}
	},
{{?}}
{{~}}
};
#endif

#define PARAMETERS_N			{{=it.product.parameters.length}}

#if PARAMETERS_N > 0

# define PARAM_BYPASS	1
# define PARAM_TOGGLED	(1<<1)
# define PARAM_INTEGER	(1<<2)

static struct {
	const char *	id;
	char		out;
	float		def;
	float		min;
	float		max;
	uint32_t	flags;
} param_data[PARAMETERS_N] = {
{{~it.product.parameters :p:i}}
	{
		/* .id		= */ "{{=it.cmd.parameterIds[i]}}",
		/* .out		= */ {{=p.direction == "output" ? 1 : 0}},
		/* .def		= */ {{=p.defaultValue.toExponential()}}f,
		/* .min		= */ {{=p.minimum.toExponential()}}f,
		/* .max		= */ {{=p.maximum.toExponential()}}f,
		/* .flags	= */ {{?p.isBypass}}PARAM_BYPASS{{??}}0{{?p.toggled}} | PARAM_TOGGLED{{?}}{{?p.integer}} | PARAM_INTEGER{{?}}{{?}}
	},
{{~}}
};
#endif
