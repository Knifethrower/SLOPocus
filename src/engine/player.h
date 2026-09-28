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

#ifndef PLAYER_H
#define PLAYER_H

#include <cstdint>
#include "definitions.h"

namespace pocus {

class Player {
public:
	[[nodiscard]] uint8_t getCrystals() const;
	void setCrystals(uint8_t crystals);
	[[nodiscard]] uint32_t getScore() const;
	void setScore(uint32_t score);
	[[nodiscard]] uint8_t getHealth() const;
	void setHealth(uint8_t health);
	[[nodiscard]] uint8_t getLevel() const;
	void setLevel(uint8_t level);
	[[nodiscard]] uint8_t getEpisode() const;
	void setEpisode(uint8_t episode);
	[[nodiscard]] bool hasSilverKey() const;
	void setSilverKey(bool silverKey);
	[[nodiscard]] bool hasGoldenKey() const;
	void setGoldKey(bool goldKey);
	[[nodiscard]] uint8_t getFirePower() const;
	void setFirePower(uint8_t power);
	[[nodiscard]] bool hasSuperJump() const;
	void setSuperJump(bool superJump);
	[[nodiscard]] Difficulty_t getDifficulty() const;
	void setDifficulty(Difficulty_t difficulty);

	// The original's timed pickups, in 20 Hz game frames (HOCUS.EXE's item
	// handler in process_event_tiles - see the reverse-engineering notes, "Items"):
	struct Effects {
		int invisibilityTicks { 0 };  // ce8e - "Invisible" potion: 400 frames immune to monsters and hazards
		int superShotTicks { 0 };     // ce88 - "Super shot": fire power 10 and autofire for 600 frames
		int savedFirePower { 1 };     // ce86 - fire power to restore after the super shot
		int laserShots { 0 };         // ce94 - "Laser Shot": 3 piercing shots per pickup
	};
	Effects effects;

private:
	uint8_t crystals { 0 };
	uint32_t score { 0 };
	uint8_t health { 100 }; // matches Game::PLAYER_MAX_HEALTH
	uint8_t level { 0 };
	uint8_t episode { 0 };
	bool silverKey { false }, goldKey { false };
	uint8_t firePower { 1 };   // DAT_2d51_7046: max simultaneous shots, 1..10 ("Add shot" +1)
	// "Super jump" potion: one high-arc jump, consumed when it starts.
	bool superJump { false };
	Difficulty_t difficulty { Difficulty_t::NORMAL };
};

}

#endif //PLAYER_H
