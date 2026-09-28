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

#ifndef SAVEFILE_H
#define SAVEFILE_H

#include <cstdint>
#include <string>

// HOCUS.SAV: the 990-byte block the original keeps at DS:48ca..4ca8 and
// writes back whole (168c:01ee) after every options change, save, high score
// or volume change. Layout from the DOS code that reads each field (see
// the reverse-engineering notes, "HOCUS.SAV"); identical in both releases. The port
// reads and writes the real file in the game directory so saves stay
// compatible with the DOS game.
struct SaveFile {
	enum { SIZE = 990, SLOTS = 9, EPISODES = 4, SCORES = 5, NAME_LENGTH = 26, KEYS = 8 };

	// 48ca..48e7: options
	int16_t soundOn { 1 };          // 48ca
	int16_t musicOn { 1 };          // 48cc
	int16_t joystickOn { 0 };       // 48ce
	int16_t gameSpeed { 1 };        // 48d0 -> 790e: 0 slow, 1 medium, 2 fast
	int16_t joystickCalibration[6] { 0, 0, 0, 0, 0, 0 };   // 48d2..48dc (2392:0944)
	int16_t joystickFireButton { 0 };                      // 48de
	uint8_t keys[KEYS] { 0x0f, 0x10, 0x02, 0x03, 0x0d, 0x0e, 0x0b, 0x0c };   // 48e0: Left Right Jump Fire Up Down Scroll-up Scroll-down, indices into the 18-key table

	// Save slots (16b8:25cc / 2b8b)
	int16_t slotEpisode[SLOTS];     // 48e8, -1 = empty
	int16_t slotLevel[SLOTS];       // 48fa
	int16_t slotDifficulty[SLOTS];  // 490c (706a)
	char slotName[SLOTS][NAME_LENGTH];   // 491e
	uint8_t slotUnused[10];         // 4a08 (never read by the game)
	uint32_t slotScore[SLOTS];      // 4a12 (704e/7050: score at the start of the saved level)

	// High scores (16b8:1bae / 2018)
	char scoreName[EPISODES][SCORES][NAME_LENGTH];   // 4a36
	uint32_t score[EPISODES][SCORES];                // 4c3e

	// 4c8e..4ca7: SETUP.EXE sound configuration; only the volumes matter here.
	int16_t soundSetup[10];         // 4c8e..4ca0 (card type, port, IRQ, DMA, MIDI port ... left as found)
	int16_t soundVolume { 255 };    // 4ca2 (16b8:137d: cell*16+15)
	int16_t musicVolume { 192 };    // 4ca4
	int16_t launchedAsHocus { 0 };  // 4ca6

	SaveFile();

	// Fills the slots and high-score tables with what the shipped HOCUS.SAV
	// holds (the original refuses to start without the file).
	void setDefaults();

	bool load(const std::string& path);
	bool save(const std::string& path) const;

	// High-score check (16b8:1bae): the rank (0..4) the score would take in
	// the episode's table, or -1 when it does not qualify.
	int highScoreRank(int episode, uint32_t score) const;
	// Inserts a new entry at that rank with an empty name, dropping the last.
	void insertHighScore(int episode, int rank, uint32_t score);
};

#endif // SAVEFILE_H
