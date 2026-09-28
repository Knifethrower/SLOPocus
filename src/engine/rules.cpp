/*
 * Copyright (C) 2026, A. Roldán. All rights reserved.
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

#include "rules.h"
#include <tinyxml2.h>

using namespace pocus;

bool Rules::load(const std::string& filename) {
	tinyxml2::XMLDocument document;
	tinyxml2::XMLError error;

	if ((error = document.LoadFile(filename.c_str())) != tinyxml2::XML_SUCCESS) {
		return false;
	}

	tinyxml2::XMLElement* element = document.FirstChildElement("rules");
	if (!element) {
		return false;
	}

	tinyxml2::XMLElement* eHazardDamage = element->FirstChildElement("hazard_damage");
	if (!eHazardDamage) {
		return false;
	}
	this->hazardDamage = std::stoi(eHazardDamage->GetText());

	tinyxml2::XMLElement* eDamageMultiplierPerDifficulty = element->FirstChildElement("damage_multiplier_per_difficulty");
	if (!eDamageMultiplierPerDifficulty) {
		return false;
	}
	this->damageMultiplierPerDifficulty = std::stoi(eDamageMultiplierPerDifficulty->GetText());

	return true;
}

const uint8_t &Rules::getHazardDamage() const {
	return hazardDamage;
}

void Rules::setHazardDamage(uint8_t hazardDamage) {
	Rules::hazardDamage = hazardDamage;
}

const uint8_t &Rules::getDamageMultiplierPerDifficulty() const {
	return damageMultiplierPerDifficulty;
}

void Rules::setDamageMultiplierPerDifficulty(uint8_t damageMultiplierPerDifficulty) {
	Rules::damageMultiplierPerDifficulty = damageMultiplierPerDifficulty;
}
