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

#include <cstring>
#include <fstream>
#include <vector>
#include "savefile.h"
#include "engine/log.h"

namespace {

void put16(std::vector<uint8_t>& b, size_t at, int16_t v) {
	b[at] = (uint8_t)(v & 0xff);
	b[at + 1] = (uint8_t)((v >> 8) & 0xff);
}

void put32(std::vector<uint8_t>& b, size_t at, uint32_t v) {
	for (int i = 0; i < 4; i++) {
		b[at + i] = (uint8_t)((v >> (8 * i)) & 0xff);
	}
}

int16_t get16(const std::vector<uint8_t>& b, size_t at) {
	return (int16_t)(b[at] | (b[at + 1] << 8));
}

uint32_t get32(const std::vector<uint8_t>& b, size_t at) {
	return (uint32_t)b[at] | ((uint32_t)b[at + 1] << 8) | ((uint32_t)b[at + 2] << 16) | ((uint32_t)b[at + 3] << 24);
}

// Offsets inside the block (DS address - 0x48ca).
enum {
	OFF_SOUND = 0x000, OFF_MUSIC = 0x002, OFF_JOYSTICK = 0x004, OFF_SPEED = 0x006, OFF_JOY_CAL = 0x008,
	OFF_JOY_FIRE = 0x014, OFF_KEYS = 0x016, OFF_SLOT_EPISODE = 0x01e, OFF_SLOT_LEVEL = 0x030,
	OFF_SLOT_DIFFICULTY = 0x042, OFF_SLOT_NAME = 0x054, OFF_SLOT_UNUSED = 0x13e, OFF_SLOT_SCORE = 0x148,
	OFF_SCORE_NAME = 0x16c, OFF_SCORE = 0x374, OFF_SOUND_SETUP = 0x3c4, OFF_SOUND_VOLUME = 0x3d8,
	OFF_MUSIC_VOLUME = 0x3da, OFF_LAUNCHED = 0x3dc
};

}

SaveFile::SaveFile() {
	setDefaults();
}

void SaveFile::setDefaults() {
	for (int i = 0; i < SLOTS; i++) {
		this->slotEpisode[i] = -1;
		this->slotLevel[i] = 0;
		this->slotDifficulty[i] = 0;
		this->slotScore[i] = 0;
		std::memset(this->slotName[i], 0, NAME_LENGTH);
		std::strncpy(this->slotName[i], "<empty>", NAME_LENGTH - 1);
	}
	std::memset(this->slotUnused, 1, sizeof(this->slotUnused));
	this->slotUnused[9] = 0;
	// The shipped HOCUS.SAV's tables (same in both releases).
	static const char* NAMES[SCORES] = { "Hocus Pocus", "Andre Foucault", "Jamie Cook", "Karim Sultan", "Chris tenDen" };
	static const uint32_t SCORE[SCORES] = { 1000000, 800000, 600000, 400000, 200000 };
	for (int e = 0; e < EPISODES; e++) {
		for (int i = 0; i < SCORES; i++) {
			std::memset(this->scoreName[e][i], 0, NAME_LENGTH);
			std::strncpy(this->scoreName[e][i], NAMES[i], NAME_LENGTH - 1);
			this->score[e][i] = SCORE[i];
		}
	}
	static const int16_t SETUP[10] = { 0, 8, 0x10, 3, 0x330, 0, 7, 1, 0, 6 };
	std::memcpy(this->soundSetup, SETUP, sizeof(this->soundSetup));
}

bool SaveFile::load(const std::string& path) {
	std::ifstream file(path, std::ios::binary);
	if (!file) {
		LOGW << "SaveFile: " << path << " not found - using the defaults";
		return false;
	}
	std::vector<uint8_t> b((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	if (b.size() != SIZE) {
		LOGW << "SaveFile: " << path << " is " << b.size() << " bytes, expected " << (int)SIZE << " - using the defaults";
		return false;
	}
	this->soundOn = get16(b, OFF_SOUND);
	this->musicOn = get16(b, OFF_MUSIC);
	this->joystickOn = get16(b, OFF_JOYSTICK);
	this->gameSpeed = get16(b, OFF_SPEED);
	for (int i = 0; i < 6; i++) {
		this->joystickCalibration[i] = get16(b, OFF_JOY_CAL + 2 * i);
	}
	this->joystickFireButton = get16(b, OFF_JOY_FIRE);
	for (int i = 0; i < KEYS; i++) {
		this->keys[i] = b[OFF_KEYS + i];
	}
	for (int i = 0; i < SLOTS; i++) {
		this->slotEpisode[i] = get16(b, OFF_SLOT_EPISODE + 2 * i);
		this->slotLevel[i] = get16(b, OFF_SLOT_LEVEL + 2 * i);
		this->slotDifficulty[i] = get16(b, OFF_SLOT_DIFFICULTY + 2 * i);
		std::memcpy(this->slotName[i], &b[OFF_SLOT_NAME + NAME_LENGTH * i], NAME_LENGTH);
		this->slotName[i][NAME_LENGTH - 1] = '\0';
		this->slotScore[i] = get32(b, OFF_SLOT_SCORE + 4 * i);
	}
	std::memcpy(this->slotUnused, &b[OFF_SLOT_UNUSED], sizeof(this->slotUnused));
	for (int e = 0; e < EPISODES; e++) {
		for (int i = 0; i < SCORES; i++) {
			std::memcpy(this->scoreName[e][i], &b[OFF_SCORE_NAME + (e * SCORES + i) * NAME_LENGTH], NAME_LENGTH);
			this->scoreName[e][i][NAME_LENGTH - 1] = '\0';
			this->score[e][i] = get32(b, OFF_SCORE + (e * SCORES + i) * 4);
		}
	}
	for (int i = 0; i < 10; i++) {
		this->soundSetup[i] = get16(b, OFF_SOUND_SETUP + 2 * i);
	}
	this->soundVolume = get16(b, OFF_SOUND_VOLUME);
	this->musicVolume = get16(b, OFF_MUSIC_VOLUME);
	this->launchedAsHocus = get16(b, OFF_LAUNCHED);
	LOGI << "SaveFile: loaded " << path;
	return true;
}

bool SaveFile::save(const std::string& path) const {
	std::vector<uint8_t> b(SIZE, 0);
	put16(b, OFF_SOUND, this->soundOn);
	put16(b, OFF_MUSIC, this->musicOn);
	put16(b, OFF_JOYSTICK, this->joystickOn);
	put16(b, OFF_SPEED, this->gameSpeed);
	for (int i = 0; i < 6; i++) {
		put16(b, OFF_JOY_CAL + 2 * i, this->joystickCalibration[i]);
	}
	put16(b, OFF_JOY_FIRE, this->joystickFireButton);
	for (int i = 0; i < KEYS; i++) {
		b[OFF_KEYS + i] = this->keys[i];
	}
	for (int i = 0; i < SLOTS; i++) {
		put16(b, OFF_SLOT_EPISODE + 2 * i, this->slotEpisode[i]);
		put16(b, OFF_SLOT_LEVEL + 2 * i, this->slotLevel[i]);
		put16(b, OFF_SLOT_DIFFICULTY + 2 * i, this->slotDifficulty[i]);
		std::memcpy(&b[OFF_SLOT_NAME + NAME_LENGTH * i], this->slotName[i], NAME_LENGTH);
		put32(b, OFF_SLOT_SCORE + 4 * i, this->slotScore[i]);
	}
	std::memcpy(&b[OFF_SLOT_UNUSED], this->slotUnused, sizeof(this->slotUnused));
	for (int e = 0; e < EPISODES; e++) {
		for (int i = 0; i < SCORES; i++) {
			std::memcpy(&b[OFF_SCORE_NAME + (e * SCORES + i) * NAME_LENGTH], this->scoreName[e][i], NAME_LENGTH);
			put32(b, OFF_SCORE + (e * SCORES + i) * 4, this->score[e][i]);
		}
	}
	for (int i = 0; i < 10; i++) {
		put16(b, OFF_SOUND_SETUP + 2 * i, this->soundSetup[i]);
	}
	put16(b, OFF_SOUND_VOLUME, this->soundVolume);
	put16(b, OFF_MUSIC_VOLUME, this->musicVolume);
	put16(b, OFF_LAUNCHED, this->launchedAsHocus);

	std::ofstream file(path, std::ios::binary | std::ios::trunc);
	if (!file) {
		LOGE << "SaveFile: cannot write " << path;
		return false;
	}
	file.write((const char*)b.data(), (std::streamsize)b.size());
	if (!file) {
		LOGE << "SaveFile: error writing " << path;
		return false;
	}
	LOGI << "SaveFile: wrote " << path;
	return true;
}

// 16b8:1bae: first entry whose score is below the new one (32-bit compare,
// strictly lower), entries below it shift down one place.
int SaveFile::highScoreRank(int episode, uint32_t newScore) const {
	if (episode < 0 || episode >= EPISODES) {
		return -1;
	}
	for (int i = 0; i < SCORES; i++) {
		if (this->score[episode][i] < newScore) {
			return i;
		}
	}
	return -1;
}

void SaveFile::insertHighScore(int episode, int rank, uint32_t newScore) {
	if (episode < 0 || episode >= EPISODES || rank < 0 || rank >= SCORES) {
		return;
	}
	for (int i = SCORES - 1; i > rank; i--) {
		std::memcpy(this->scoreName[episode][i], this->scoreName[episode][i - 1], NAME_LENGTH);
		this->score[episode][i] = this->score[episode][i - 1];
	}
	std::memset(this->scoreName[episode][rank], 0, NAME_LENGTH);
	this->score[episode][rank] = newScore;
}
