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

#ifndef TALLY_H
#define TALLY_H

#include "screen.h"

namespace pocus::ui {

// 16b8:4889, shown by level_run after a level ends (completed = 1) or after
// a death (completed = 0), over the menu tune: "Castle complete -
// congratulations!" / "Tough break - you'll get it next time!", "Results
// for Level N", the treasures found/available, the accuracy with the
// 100 % bonus (25,000 / 50,000 / 75,000 by difficulty) and, when completed,
// the time to beat (the EXE's par table) against the elapsed time with the
// same bonus. Lines are centred in the 40..184 band; any key closes it.
class TallyScreen : public Screen {
public:
	// levelNumber 1-based, difficulty = the original's 706a (0..2), times in seconds.
	TallyScreen(ScreenAssets& assets, bool completed, int levelNumber, int found, int total,
				int difficulty, int parSeconds, int elapsedSeconds);

	void handleEvents(EventHandler& eventHandler) override;
	void render(Renderer& renderer) override;

	// The points the screen added to the score (7052/7054).
	[[nodiscard]] uint32_t getBonus() const { return this->bonus; }

private:
	struct Line {
		std::unique_ptr<Texture> texture;
		int y;
	};

	ScreenAssets& assets;
	std::vector<Line> lines;
	std::unique_ptr<Texture> caption;
	uint32_t bonus { 0 };
};

}

#endif // TALLY_H
