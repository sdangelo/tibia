var path = require("path");
var sep = path.sep;

module.exports = function (data, api) {
	api.copyFile(`src${sep}memset.h`, `src${sep}memset.h`);
	api.copyFile(`src${sep}memset.c`, `src${sep}memset.c`);
	api.copyFile(`src${sep}walloc.h`, `src${sep}walloc.h`);
	api.copyFile(`src${sep}walloc.c`, `src${sep}walloc.c`);
	api.copyFile(`src${sep}new.cpp`, `src${sep}new.cpp`);
	api.copyFile(`src${sep}processor.c`, `src${sep}processor.c`);
	api.generateFileFromTemplateFile(`src${sep}data.h`, `src${sep}data.h`, data);
	api.generateFileFromTemplateFile(`src${sep}processor.js`, `src${sep}processor.js`, data);
	api.generateFileFromTemplateFile(`src${sep}module.js`, `src${sep}module.js`, data);
};
