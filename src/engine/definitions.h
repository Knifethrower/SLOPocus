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

#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <chrono>
#include <thread>

namespace pocus {

#define TILE_SIZE		16
#define MAP_WIDTH		240
#define MAP_HEIGHT		60

enum Difficulty_t {
	EASY = 1,
	NORMAL = 3,
	HARD = 4
};

#define getBit(n,b) ((n >> b) & 1)

typedef std::chrono::time_point<std::chrono::system_clock> Tick;

static uint32_t getElapsedTime(const Tick& tick) {
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - tick).count();
}

static Tick getNow() {
	return std::chrono::system_clock::now();
}

static void wait(uint32_t ms) {
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

}

#endif //DEFINITIONS_H
