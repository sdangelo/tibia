#define DATA_LV2_URI				"{{=it.tibia.CGetUTF8StringLiteral(it.tibia.lv2.expandURI(it.lv2.uri))}}"

#define DATA_PRODUCT_AUDIO_INPUT_CHANNELS_N	{{=it.product.buses.filter(x => x.type == "audio" && x.direction == "input").reduce((s, x) => s += x.channels == "mono" ? 1 : 2, 0)}}
#define DATA_PRODUCT_AUDIO_OUTPUT_CHANNELS_N	{{=it.product.buses.filter(x => x.type == "audio" && x.direction == "output").reduce((s, x) => s += x.channels == "mono" ? 1 : 2, 0)}}
#define DATA_PRODUCT_MIDI_INPUTS_N		{{=it.product.buses.filter(x => x.type == "midi" && x.direction == "input").length}}
#define DATA_PRODUCT_MIDI_OUTPUTS_N		{{=it.product.buses.filter(x => x.type == "midi" && x.direction == "output").length}}
#define DATA_PRODUCT_CONTROL_INPUTS_N		{{=it.product.parameters.filter(x => x.direction == "input").length}}
#define DATA_PRODUCT_CONTROL_OUTPUTS_N		{{=it.product.parameters.filter(x => x.direction == "output").length}}

#if DATA_PRODUCT_MIDI_INPUTS_N > 0
static uint32_t midi_in_index[DATA_PRODUCT_MIDI_INPUTS_N] = {
	{{~it.tibia.lv2.ports.filter(x => x.type == "midi" && x.direction == "input") :p}}{{=p.busIndex}}, {{~}}
};
#endif

#if DATA_PRODUCT_CONTROL_INPUTS_N > 0

# define DATA_PARAM_BYPASS	1
# define DATA_PARAM_TOGGLED	(1<<1)
# define DATA_PARAM_INTEGER	(1<<2)
# define DATA_PARAM_MAP_LOG	(1<<3)

static struct {
	uint32_t	index;
	float		min;
	float		max;
	float		def;
	uint32_t	flags;
	float		mapK;
} param_data[DATA_PRODUCT_CONTROL_INPUTS_N] = {
	{{~it.tibia.lv2.ports.filter(x => x.type == "control" && x.direction == "input") :p}}
	{
		/* .index	= */ {{=p.paramIndex}},
		/* .min		= */ {{=p.minimum.toExponential()}}f,
		/* .max		= */ {{=p.maximum.toExponential()}}f,
		/* .def		= */ {{=p.defaultValue.toExponential()}}f,
		/* .flags	= */ {{?p.isBypass}}DATA_PARAM_BYPASS{{??}}0{{?p.toggled}} | DATA_PARAM_TOGGLED{{?}}{{?p.integer}} | DATA_PARAM_INTEGER{{?}}{{?p.map == "logarithmic"}} | DATA_PARAM_MAP_LOG{{?}}{{?}},
		/* .mapK	= */ {{?p.map == "logarithmic"}}{{=Number(2.0 * Math.log(Math.sqrt(p.maximum * p.minimum) / Math.abs(p.minimum))).toExponential()}}f{{??}}0.f{{?}}
	},
	{{~}}
};

static uint32_t param_in_data_index[DATA_PRODUCT_CONTROL_INPUTS_N + DATA_PRODUCT_CONTROL_OUTPUTS_N] = {
	{{~it.tibia.lv2.ports.filter(x => x.type == "control") :p}}{{=p.inDataIndex >= 0 ? p.inDataIndex : "(uint32_t)-1"}}, {{~}}
};

#endif

#if DATA_PRODUCT_CONTROL_OUTPUTS_N > 0
static uint32_t param_out_index[DATA_PRODUCT_CONTROL_OUTPUTS_N] = {
	{{~it.tibia.lv2.ports.filter(x => x.type == "control" && x.direction == "output") :p}}{{=p.paramIndex}}, {{~}}
};
#endif
