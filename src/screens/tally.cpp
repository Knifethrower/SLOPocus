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

#include "tally.h"
#include "../exedata.h"

using namespace pocus;
using namespace pocus::ui;

namespace {
// The bonus amounts are immediates in the tally routine; the texts are the EXE's.
const uint32_t BONUS_POINTS[3] = { 25000, 50000, 75000 };
std::string bonusText(int level) { return ExeData::get().string((ExeString)(STR_BONUS_EASY + level)); }
}

TallyScreen::TallyScreen(ScreenAssets& assets, bool completed, int levelNumber, int found, int total,
						 int difficulty, int parSeconds, int elapsedSeconds):
	assets(assets)
{
	const int level = difficulty < 0 ? 0 : (difficulty > 2 ? 2 : difficulty);
	// (0x90 - (completed * 0x28 + 0x40)) / 2 + 0x28
	int y = (144 - ((completed ? 40 : 0) + 64)) / 2 + 40;
	const auto add = [&](const std::string& text, int font, int advance) {
		Line line;
		line.texture = this->assets.text(text, font);
		line.y = y;
		this->lines.push_back(std::move(line));
		y += advance;
	};

	const ExeData& exe = ExeData::get();
	add(exe.string(completed ? STR_CASTLE_COMPLETE : STR_TOUGH_BREAK), 4, 16);
	add(exe.string(STR_RESULTS_FOR_LEVEL) + std::to_string(levelNumber), 2, 24);
	add(exe.string(STR_TREASURES_FOUND) + std::to_string(found) + "  " + exe.string(STR_TREASURES_AVAILABLE) + std::to_string(total), 3, 16);

	// Accuracy: found / total * 100 as a float, truncated; the 100 % bonus
	// only when the level was completed.
	int accuracy = 0;
	if (found != 0 && total != 0) {
		accuracy = (int)((float)found / (float)total * 100.0f);
	}
	std::string accuracyLine = exe.string(STR_ACCURACY) + std::to_string(accuracy) + "%  ";
	if (completed) {
		if (found != 0 && found >= total) {
			accuracyLine += bonusText(level);
			this->bonus += BONUS_POINTS[level];
		}
		else {
			accuracyLine += exe.string(STR_NO_BONUS_ACCURACY);
		}
	}
	else {
		accuracyLine += exe.string(STR_CASTLE_NOT_COMPLETE);
	}
	add(accuracyLine, 2, 24);

	if (completed) {
		add(exe.string(STR_TIME_TO_BEAT) + std::to_string(parSeconds) + "  " + exe.string(STR_YOUR_TIME) + std::to_string(elapsedSeconds), 3, 16);
		// 16b8:4bfa: the bonus needs the par time to be strictly above the elapsed time.
		if (parSeconds > elapsedSeconds) {
			add(bonusText(level), 2, 24);
			this->bonus += BONUS_POINTS[level];
		}
		else {
			add(exe.string(STR_NO_BONUS_TIME), 2, 24);
		}
	}

	this->caption = assets.shadowText(padPrompts() ? "Press any button" : exe.string(STR_PRESS_ANY_KEY), 4);
}

void TallyScreen::handleEvents(EventHandler& eventHandler) {
	if (eventHandler.getKeyDown() != KEY_NONE) {
		finish(0);
	}
}

void TallyScreen::render(Renderer& renderer) {
	for (const Line& line : this->lines) {
		drawCentred(renderer, *line.texture, line.y);
	}
	renderer.drawTexture(*this->assets.bottom, Point(0, ScreenAssets::FRAME_BOTTOM_Y));
	drawCentred(renderer, *this->caption, ScreenAssets::FRAME_CAPTION_Y);
	renderer.drawTexture(*this->assets.top, Point(0, 0));
}
