var path = require("path");
var sep = path.sep;

module.exports = function (data, api) {
	api.generateFileFromTemplateFile(`data${sep}Info.plist`, `data${sep}Info.plist`, data);
	api.copyFile(`src${sep}vst3.c`, `src${sep}vst3.c`);
	api.generateFileFromTemplateFile(`src${sep}data.h`, `src${sep}data.h`, data);
	api.copyFileIfNotExists(`src${sep}plugin.h`, `src${sep}plugin.h`);
};
