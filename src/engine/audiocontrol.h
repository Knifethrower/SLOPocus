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

#ifndef AUDIOCONTROL_H
#define AUDIOCONTROL_H

namespace pocus {

// Global mixer levels, the way the original's volume screen (16b8:137d)
// drives its sound driver: 0..255 for effects and music, plus the on/off
// switches of the options menu. Implemented by the SDL audio backend.
class AudioControl {
public:
	static void setSoundVolume(int volume);   // 256d:02a3
	static void setMusicVolume(int volume);   // 25d4:015e
	static void setSoundEnabled(bool enabled);
	static void setMusicEnabled(bool enabled);
};

}

#endif // AUDIOCONTROL_H
