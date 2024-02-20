#define DATA_PRODUCT_AUDIO_INPUT_CHANNELS_N	{{=it.product.buses.filter(x => x.type == "audio" && x.direction == "input").reduce((s, x) => s += x.channels == "mono" ? 1 : 2, 0)}}
#define DATA_PRODUCT_AUDIO_OUTPUT_CHANNELS_N	{{=it.product.buses.filter(x => x.type == "audio" && x.direction == "output").reduce((s, x) => s += x.channels == "mono" ? 1 : 2, 0)}}
#define DATA_PRODUCT_MIDI_INPUTS_N		{{=it.product.buses.filter(x => x.type == "midi" && x.direction == "input").length}}
#define DATA_PRODUCT_MIDI_OUTPUTS_N		{{=it.product.buses.filter(x => x.type == "midi" && x.direction == "output").length}}
#define DATA_PRODUCT_PARAMETERS_N		{{=it.product.parameters.length}}
#define DATA_PRODUCT_PARAMETERS_OUTPUT_N	{{=it.product.parameters.filter(x => x.direction == "output").length}}

#if DATA_PRODUCT_PARAMETERS_N > 0
static struct {
	char	out;
	float	def;
} param_data[DATA_PRODUCT_PARAMETERS_N] = {
{{~it.product.parameters :p}}
	{
		/* .out	= */ {{=p.direction == "output" ? 1 : 0}},
		/* .def	= */ {{=p.defaultValue.toExponential()}}
	},
{{~}}
};
#endif

#if DATA_PRODUCT_PARAMETERS_OUTPUT_N > 0
static size_t param_out_index[DATA_PRODUCT_PARAMETERS_OUTPUT_N] = {
	{{~it.product.parameters :p:i}}{{?p.direction == "output"}}{{=i}}, {{?}}{{~}}
};
#endif
