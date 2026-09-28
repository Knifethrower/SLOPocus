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

#include "palette.h"
#include <cstring>

using namespace pocus::data::asset;

bool Palette::loadFromStream(const char *stream, uint32_t length) {
	memcpy((void*)this->colors, stream, length);
	
	for (int i = 0; i < 128; i++)	{
		this->colors[i].r = this->colors[i].r << 2;
		this->colors[i].g = this->colors[i].g << 2;
		this->colors[i].b = this->colors[i].b << 2;
	}

	return true;
}

void Palette::release() {

}
