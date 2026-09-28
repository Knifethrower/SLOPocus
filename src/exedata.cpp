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

#include <fstream>
#include <iterator>
#include "exedata.h"
#include "engine/log.h"

using namespace pocus;

namespace {
	ExeData instance;
}

bool ExeData::load(const std::string& exePath, bool shareware) {
	std::ifstream file(exePath, std::ios::binary);
	if (!file) {
		LOGE << "ExeData: cannot open " << exePath;
		return false;
	}
	std::vector<uint8_t> image((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	const ExeLayout* layout = shareware ? &EXE_LAYOUT_SHAREWARE : &EXE_LAYOUT_REGISTERED;
	if (image.size() <= layout->dsBase) {
		LOGE << "ExeData: " << exePath << " is too short for its data segment";
		return false;
	}
	instance.ds.assign(image.begin() + layout->dsBase, image.end());
	instance.layout = layout;
	LOGI << "ExeData: " << instance.ds.size() << " bytes of game data from " << exePath;
	return true;
}

const ExeData& ExeData::get() {
	return instance;
}

int16_t ExeData::word(ExeTable table, int index) const {
	if (!this->layout || index < 0 || index >= words(table)) {
		return 0;
	}
	const uint32_t at = this->layout->tables[table].offset + (uint32_t)index * 2;
	return (int16_t)(this->ds[at] | (this->ds[at + 1] << 8));
}

int ExeData::words(ExeTable table) const {
	return this->layout ? this->layout->tables[table].length / 2 : 0;
}

uint8_t ExeData::byte(ExeTable table, int index) const {
	if (!this->layout || index < 0 || index >= bytes(table)) {
		return 0;
	}
	return this->ds[this->layout->tables[table].offset + (uint32_t)index];
}

int ExeData::bytes(ExeTable table) const {
	return this->layout ? this->layout->tables[table].length : 0;
}

std::string ExeData::pointerString(ExePointers table, int index) const {
	if (!this->layout || index < 0 || index >= pointerCount(table)) {
		return "";
	}
	const uint32_t at = this->layout->pointers[table].offset + (uint32_t)index * 4;
	const uint16_t offset = (uint16_t)(this->ds[at] | (this->ds[at + 1] << 8));
	return cstr(offset);
}

int ExeData::pointerCount(ExePointers table) const {
	return this->layout ? this->layout->pointers[table].count : 0;
}

std::vector<std::string> ExeData::pointerStrings(ExePointers table) const {
	std::vector<std::string> out;
	for (int i = 0; i < pointerCount(table); i++) {
		out.push_back(pointerString(table, i));
	}
	return out;
}

std::string ExeData::string(ExeString id) const {
	if (!this->layout || this->layout->strings[id] == 0) {
		return "";
	}
	return cstr(this->layout->strings[id]);
}

int ExeData::soundFile(int sound) const {
	return word(TBL_SOUND, sound * 3);
}

std::string ExeData::cstr(uint32_t offset) const {
	std::string out;
	while (offset < this->ds.size() && this->ds[offset] != 0) {
		out.push_back((char)this->ds[offset++]);
	}
	return out;
}
