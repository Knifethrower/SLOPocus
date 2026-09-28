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

#include "levelelevator.h"

using namespace pocus::data::asset;

bool LevelElevator::loadFromStream(const char *stream, uint32_t length) {
	const uint32_t entries = 80;

	for (uint32_t i = 0; i < entries; i++) {
		int16_t value = *(int16_t*)(stream);
		this->tiles.push_back(value);
		stream += sizeof(int16_t);
	}

	return true;
}

void LevelElevator::release() {
	this->tiles.erase(this->tiles.begin(), this->tiles.end());
}

int16_t LevelElevator::getLeftTile(uint32_t paddedLevelIndex) const {
	return this->tiles[paddedLevelIndex * 2];
}

int16_t LevelElevator::getRightTile(uint32_t paddedLevelIndex) const {
	return this->tiles[paddedLevelIndex * 2 + 1];
}
