var path = require("path");
var sep = path.sep;

module.exports = function (data, api) {
	api.generateFileFromTemplateFile(`src${sep}index.html`, `src${sep}index.html`, data);
	api.copyFile(`demo.mk`, `demo.mk`);
};
