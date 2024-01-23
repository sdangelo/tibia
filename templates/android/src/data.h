#define AUDIO_BUS_IN		{{=it.product.buses.findIndex(x => x.type == "audio" && x.direction == "input" && !x.cv && !x.sidechain)}}
#define AUDIO_BUS_OUT		{{=it.product.buses.findIndex(x => x.type == "audio" && x.direction == "output" && !x.cv && !x.sidechain)}}

#define NUM_CHANNELS_IN		{{=it.product.buses.filter(x => x.type == "audio" && x.direction == "input" && !x.cv && !x.sidechain) ? (it.product.buses.filter(x => x.type == "audio" && x.direction == "input" && !x.cv && !x.sidechain)[0].channels == "mono" ? 1 : 2) : 0}}
#define NUM_CHANNELS_OUT	{{=it.product.buses.filter(x => x.type == "audio" && x.direction == "output" && !x.cv && !x.sidechain) ? (it.product.buses.filter(x => x.type == "audio" && x.direction == "output" && !x.cv && !x.sidechain)[0].channels == "mono" ? 1 : 2) : 0}}

#define PARAMETERS_N		{{=it.product.parameters.length}}

#if PARAMETERS_N > 0
static struct {
	char	out;
	float	def;
} param_data[PARAMETERS_N] = {
{{~it.product.parameters :p}}
	{
		/* .out	= */ {{=p.direction == "output" ? 1 : 0}},
		/* .def	= */ {{=p.defaultValue.toExponential()}}
	},
{{~}}
};
#endif

#define JNI_FUNC(x) Java_{{=it.android.javaPackageName.replaceAll("_", "_1").replaceAll(".", "_")}}_MainActivity_##x
