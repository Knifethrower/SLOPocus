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

#include "levelbackground.h"

using namespace pocus::data::asset;

bool LevelBackground::loadFromStream(const char *stream, uint32_t length) {
	const uint32_t entries = 10 * 4;
	
	for (int i = 0; i < entries; i++) {
		uint16_t id = *(uint16_t*)(stream);
		this->backgroundIds.push_back(id);
		stream += sizeof(uint16_t);
	}

	return true;
}

void LevelBackground::release() {
	this->backgroundIds.erase(this->backgroundIds.begin(), this->backgroundIds.end());
}

const std::vector<uint16_t>& LevelBackground::getBackgroundIds() const {
	return this->backgroundIds;
}
