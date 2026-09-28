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

#include "leveltime.h"

using namespace pocus::data::asset;

bool LevelTime::loadFromStream(const char *stream, uint32_t length) {
	const uint32_t entries = 9 * 4;
	
	for (int i = 0; i < entries; i++) {
		uint16_t levelTimeLimit = *(uint16_t*)(stream);
		this->time.push_back(levelTimeLimit);
		stream += sizeof(uint16_t);
	}

	return true;
}

void LevelTime::release() {
	this->time.erase(this->time.begin(), this->time.end());
}

const std::vector<uint16_t>& LevelTime::getTime() const {
	return this->time;
}
