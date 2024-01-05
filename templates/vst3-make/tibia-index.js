module.exports = function (data, api) {
	api.copyFile(`Makefile`, `Makefile`);
	api.generateFileFromTemplateFile(`vars.mk`, `vars.mk`, data);
};
