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

#ifndef RULES_H
#define RULES_H

#include <string>
#include <cstdint>

namespace pocus {

class Rules {
public:
	bool load(const std::string& filename);

	const uint8_t &getHazardDamage() const;
	void setHazardDamage(uint8_t hazardDamage);

	const uint8_t &getDamageMultiplierPerDifficulty() const;
	void setDamageMultiplierPerDifficulty(uint8_t damageMultiplierPerDifficulty);

private:
	uint8_t hazardDamage { 4 };
	uint8_t damageMultiplierPerDifficulty { 1 };
};

}

#endif // RULES_H
