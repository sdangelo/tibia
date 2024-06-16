/*
 * Tibia
 *
 * Copyright (C) 2024 Orastron Srl unipersonale
 *
 * Tibia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * Tibia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Tibia.  If not, see <http://www.gnu.org/licenses/>.
 *
 * File author: Stefano D'Angelo
 */

var path = require("path");
var sep = path.sep;

module.exports = function (data, api) {
	api.generateFileFromTemplateFile(`src${sep}index.html`, `src${sep}index.html`, data);
	api.copyFile(`vars-extra.mk`, `vars-extra.mk`);
	api.copyFile(`rules-extra.mk`, `rules-extra.mk`);
};
