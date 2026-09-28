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

#ifndef LEVELELEVATOR_H
#define LEVELELEVATOR_H

#include <vector>
#include <cstdint>
#include "asset.h"

namespace pocus::data::asset {

// EXE elevator table: 80 SINT16LE values (moddingwiki's Hocus Pocus Map Format
// page, "Elevator Tiles") - left/right tile id pairs, 10 slots per episode
// (only 9 used, same padding as tileset/backdrop numbers) x 4 episodes. A
// left tile of -1 means the level has no elevators; the game only checks the
// left value.
class LevelElevator : public Asset {
public:
	bool loadFromStream(const char* stream, uint32_t length) override;
	void release() override;

	[[nodiscard]] int16_t getLeftTile(uint32_t paddedLevelIndex) const;
	[[nodiscard]] int16_t getRightTile(uint32_t paddedLevelIndex) const;

private:
	std::vector<int16_t> tiles;
};

}

#endif // LEVELELEVATOR_H
