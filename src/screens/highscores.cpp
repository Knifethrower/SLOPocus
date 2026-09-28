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
#include "highscores.h"
#include "../exedata.h"

using namespace pocus;
using namespace pocus::ui;

namespace {
constexpr int NAME_X = 27;        // 0x1b
constexpr int SCORE_RIGHT = 290;  // 0x122
constexpr int TITLE_Y = 72;       // 0x48
constexpr int MAX_NAME = 25;      // 16b8:1c9b accepts characters while strlen < 0x19
}

void HighScoreTable::build(ScreenAssets& assets, const SaveFile& save, int episode, const std::string& captionText) {
	this->title = assets.text(ExeData::get().string(STR_HIGH_SCORES_FOR) + std::to_string(episode + 1), 5);
	this->caption = assets.shadowText(captionText, 4);
	for (int i = 0; i < SaveFile::SCORES; i++) {
		const int font = rowFont(i);
		const std::string name(save.scoreName[episode][i], strnlen(save.scoreName[episode][i], SaveFile::NAME_LENGTH));
		this->names[i] = name.empty() ? nullptr : assets.text(name, font);
		this->scores[i] = assets.text(std::to_string(save.score[episode][i]), font);
	}
}

void HighScoreTable::render(Renderer& renderer, ScreenAssets& assets) {
	drawCentred(renderer, *this->title, TITLE_Y);
	for (int i = 0; i < SaveFile::SCORES; i++) {
		const int y = rowY(i);
		if (this->names[i]) {
			renderer.drawTexture(*this->names[i], Point(NAME_X, (float)y));
		}
		renderer.drawTexture(*this->scores[i], Point((float)(SCORE_RIGHT - (int)this->scores[i]->getWidth()), (float)y));
	}
	renderer.drawTexture(*assets.bottom, Point(0, ScreenAssets::FRAME_BOTTOM_Y));
	drawCentred(renderer, *this->caption, ScreenAssets::FRAME_CAPTION_Y);
	renderer.drawTexture(*assets.top, Point(0, 0));
}

HighScoreScreen::HighScoreScreen(ScreenAssets& assets, const SaveFile& save, int episodes):
	assets(assets),
	save(save),
	episodes(episodes)
{
	this->table.build(assets, save, 0, padPrompts() ? "Press any button to continue" : ExeData::get().pointerString(PTR_HELP, 2));
}

void HighScoreScreen::handleEvents(EventHandler& eventHandler) {
	if (this->turning) {
		return;
	}
	const Key_t key = eventHandler.getKeyDown();
	if (key == KEY_NONE) {
		return;
	}
	// 16b8:0f3d: ESC ends the show, any other key turns to the next episode.
	if (key == KEY_ESCAPE || this->episode + 1 >= this->episodes) {
		finish(0);
		return;
	}
	this->turning = true;
	this->pageFade.setSpeed(fadeSpeedForSteps(20));
	this->pageFade.start(Fade::FADE_OUT, [this] {
		this->episode++;
		this->table.build(this->assets, this->save, this->episode, padPrompts() ? "Press any button to continue" : ExeData::get().pointerString(PTR_HELP, 2));
		this->pageFade.start(Fade::FADE_IN, [this] { this->turning = false; });
	});
}

void HighScoreScreen::update(float dt) {
	this->pageFade.update(dt);
}

void HighScoreScreen::render(Renderer& renderer) {
	this->table.render(renderer, this->assets);
	this->pageFade.render(renderer);
}

HighScoreEntryScreen::HighScoreEntryScreen(ScreenAssets& assets, SaveFile& save, int episode, int rank):
	assets(assets),
	save(save),
	episode(episode),
	rank(rank)
{
	this->cursor = assets.text("_", 2);
	rebuild();
}

void HighScoreEntryScreen::rebuild() {
	std::memset(this->save.scoreName[this->episode][this->rank], 0, SaveFile::NAME_LENGTH);
	std::strncpy(this->save.scoreName[this->episode][this->rank], this->name.c_str(), SaveFile::NAME_LENGTH - 1);
	this->table.build(this->assets, this->save, this->episode, padPrompts() ? "Press START to keep this name" : ExeData::get().string(STR_ENTER_NAME));
}

void HighScoreEntryScreen::handleEvents(EventHandler& eventHandler) {
	const char c = eventHandler.getTextInput();
	if (c >= 0x20 && c <= 0x7a) {
		if ((int)this->name.size() < MAX_NAME) {
			this->name.push_back(c);
			rebuild();
		}
		return;
	}
	switch (eventHandler.getKeyDown()) {
		case KEY_BACKSPACE:
			if (!this->name.empty()) {
				this->name.pop_back();
				rebuild();
			}
			break;
		case KEY_RETURN:
			finish(1);
			break;
		default:
			break;
	}
}

void HighScoreEntryScreen::render(Renderer& renderer) {
	this->table.render(renderer, this->assets);
	// The '_' cursor in font 2 right after the name.
	const int x = NAME_X + this->assets.textWidth(this->name);
	renderer.drawTexture(*this->cursor, Point((float)x, (float)this->table.rowY(this->rank)));
}
