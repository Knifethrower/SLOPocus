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

#include "levelmusic.h"

using namespace pocus::data::asset;

bool LevelMusic::loadFromStream(const char *stream, uint32_t length) {
	const uint32_t entries = 9 * 4;

	for (int i = 0; i < entries; i++) {
		uint16_t musicId = *(uint16_t*)(stream);
		this->musicIds.push_back(musicId);
		stream += sizeof(uint16_t);
	}

	return true;
}

void LevelMusic::release() {
	this->musicIds.erase(this->musicIds.begin(), this->musicIds.end());
}

const std::vector<uint16_t>& LevelMusic::getMusicIds() const {
	return this->musicIds;
}

const uint16_t LevelMusic::getMusicId(uint8_t episode, uint8_t level) const {
	return this->musicIds[episode * 9 + level];
}
