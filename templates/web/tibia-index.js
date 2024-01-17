var path = require("path");
var sep = path.sep;

module.exports = function (data, api) {
	api.copyFile(`src${sep}memset.h`, `src${sep}memset.h`);
	api.copyFile(`src${sep}walloc.h`, `src${sep}walloc.h`);
	api.copyFile(`src${sep}wrapper.c`, `src${sep}wrapper.c`);
	api.generateFileFromTemplateFile(`src${sep}data.h`, `src${sep}data.h`, data);
};
