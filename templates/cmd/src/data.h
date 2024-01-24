#define AUDIO_BUS_IN		{{=it.product.buses.findIndex(x => x.type == "audio" && x.direction == "input" && !x.cv && !x.sidechain)}}
#define AUDIO_BUS_OUT		{{=it.product.buses.findIndex(x => x.type == "audio" && x.direction == "output" && !x.cv && !x.sidechain)}}

#define NUM_CHANNELS_IN		{{=it.product.buses.filter(x => x.type == "audio" && x.direction == "input" && !x.cv && !x.sidechain) ? (it.product.buses.filter(x => x.type == "audio" && x.direction == "input" && !x.cv && !x.sidechain)[0].channels == "mono" ? 1 : 2) : 0}}
#define NUM_CHANNELS_OUT	{{=it.product.buses.filter(x => x.type == "audio" && x.direction == "output" && !x.cv && !x.sidechain) ? (it.product.buses.filter(x => x.type == "audio" && x.direction == "output" && !x.cv && !x.sidechain)[0].channels == "mono" ? 1 : 2) : 0}}

#define PARAMETERS_N		{{=it.product.parameters.length}}

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
		/* .flags	= */ {{?p.isBypass}}PARAM_BYPASS{{??p.isLatency}}PARAM_INTEGER{{??}}0{{?p.toggled}} | PARAM_TOGGLED{{?}}{{?p.integer}} | PARAM_INTEGER{{?}}{{?}}
	},
{{~}}
};
#endif
