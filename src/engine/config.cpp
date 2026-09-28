/*
 * Copyright (C) 2023, A. Roldán. All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include <tinyxml2.h>

using namespace pocus;

bool Config::load(const std::string& filename) {
	tinyxml2::XMLDocument document;
	tinyxml2::XMLError error;

	if ((error = document.LoadFile(filename.c_str())) != tinyxml2::XML_SUCCESS) {
		return false;
	}

	tinyxml2::XMLElement* element = document.FirstChildElement("config");
	if (!element) {
		return false;
	}

	for (tinyxml2::XMLElement* eInstallationPath = element->FirstChildElement("installation_path");
		 eInstallationPath;
		 eInstallationPath = eInstallationPath->NextSiblingElement("installation_path")) {
		if (eInstallationPath->GetText()) {
			this->installationPaths.emplace_back(eInstallationPath->GetText());
		}
	}

	return !this->installationPaths.empty();
}

const std::vector<std::string>& Config::getInstallationPaths() const {
	return this->installationPaths;
}

void Config::addInstallationPath(const std::string& installationPath) {
	this->installationPaths.push_back(installationPath);
}
