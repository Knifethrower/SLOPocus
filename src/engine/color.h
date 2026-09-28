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

#ifndef COLOR_H
#define COLOR_H

#include <cstdint>

namespace pocus {

struct Color {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
	uint8_t alpha;
};

namespace color {
	static const Color red = (Color) {255, 0, 0, 255};
	static const Color pink = (Color) {255, 0, 255, 255};
	static const Color green = (Color) {0, 255, 0, 255};
	static const Color blue = (Color) {0, 0, 255, 255};
	static const Color black = (Color) {0, 0, 0, 255};
	static const Color white = (Color) {255, 255, 255, 255};
	static const Color yellow = (Color) {255, 255, 0, 255};
	static const Color redAlpha = (Color){255, 0, 0, 128};
	static const Color blueAlpha = (Color){0, 0, 255, 128};
	static const Color transparent = (Color){0, 0, 0, 0};
}

}

#endif //COLOR_H