var path = require("path");
var sep = path.sep;

module.exports = function (data, api) {
	api.generateFileFromTemplateFile(`data${sep}AndroidManifest.xml`, `data${sep}AndroidManifest.xml`, data);
	api.generateFileFromTemplateFile(`src${sep}data.h`, `src${sep}data.h`, data);
	api.copyFile(`src${sep}jni.cpp`, `src${sep}jni.cpp`);
	api.generateFileFromTemplateFile(`src${sep}MainActivity.java`, `src${sep}MainActivity.java`, data);
	api.generateFileFromTemplateFile(`src${sep}index.html`, `src${sep}index.html`, data);
};
