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

#include "keyconfig.h"
#include "../exedata.h"

using namespace pocus;
using namespace pocus::ui;

// The key names and action labels are the EXE's own tables.
std::string KeyConfigScreen::keyName(int index) {
	return ExeData::get().pointerString(PTR_KEY_NAMES, index);
}

std::string KeyConfigScreen::actionLabel(int index) {
	return ExeData::get().pointerString(PTR_ACTIONS, index);
}

KeyConfigScreen::KeyConfigScreen(ScreenAssets& assets, SaveFile& save):
	assets(assets),
	save(save)
{
	this->caption = assets.shadowText(ExeData::get().string(STR_KEY_SELECT_HELP), 4);
	buildList();
}

// 16b8:17da: title at y=62; rows from y=82: letter (font 4, x=70), "-"
// (font 2, x=80), action (font 2, x=90), key name right-aligned to x=250 (font 3).
void KeyConfigScreen::buildList() {
	this->labels.clear();
	this->remapAction = -1;
	const auto add = [&](const std::string& text, int font, int x, int y) {
		Label label;
		label.texture = this->assets.text(text, font);
		label.x = x;
		label.y = y;
		this->labels.push_back(std::move(label));
	};
	const std::string title = ExeData::get().string(STR_KEY_SELECT_TITLE);
	add(title, 5, (ScreenAssets::WIDTH - this->assets.textWidth(title)) / 2, 62);
	for (int i = 0; i < 8; i++) {
		const int y = 82 + i * 10;
		add(std::string(1, (char)('A' + i)), 4, 70, y);
		add("-", 2, 80, y);
		add(actionLabel(i), 2, 90, y);
		const std::string name = keyName(this->save.keys[i]);
		add(name, 3, 250 - this->assets.textWidth(name), y);
	}
}

// 16b8:10ac: "Select new key for: <action>" at y=72; six rows from y=92 in
// three columns (A-F at x=27/37/47, G-L at 125/135/145, M-R at 200/210/220).
void KeyConfigScreen::buildRemap(int action) {
	this->labels.clear();
	this->remapAction = action;
	const auto add = [&](const std::string& text, int font, int x, int y) {
		Label label;
		label.texture = this->assets.text(text, font);
		label.x = x;
		label.y = y;
		this->labels.push_back(std::move(label));
	};
	const std::string title = ExeData::get().string(STR_KEY_NEW_TITLE) + actionLabel(action);
	add(title, 5, (ScreenAssets::WIDTH - this->assets.textWidth(title)) / 2, 72);
	static const int COLUMN_X[3][3] = { { 27, 37, 47 }, { 125, 135, 145 }, { 200, 210, 220 } };
	for (int j = 0; j < 6; j++) {
		const int y = 92 + j * 10;
		for (int column = 0; column < 3; column++) {
			const int key = column * 6 + j;
			add(std::string(1, (char)('A' + key)), 4, COLUMN_X[column][0], y);
			add("-", 2, COLUMN_X[column][1], y);
			add(keyName(key), 2, COLUMN_X[column][2], y);
		}
	}
}

void KeyConfigScreen::switchTo(std::function<void()> build) {
	this->turning = true;
	this->pageFade.setSpeed(fadeSpeedForSteps(20));
	this->pageFade.start(Fade::FADE_OUT, [this, build] {
		build();
		this->pageFade.start(Fade::FADE_IN, [this] { this->turning = false; });
	});
}

void KeyConfigScreen::handleEvents(EventHandler& eventHandler) {
	if (this->turning) {
		return;
	}
	const Key_t key = eventHandler.getKeyDown();
	if (key == KEY_NONE) {
		return;
	}
	if (this->remapAction < 0) {
		if (key == KEY_ESCAPE) {
			finish(1);
		}
		else if (key >= KEY_A && key <= KEY_H) {
			const int action = key - KEY_A;
			switchTo([this, action] { buildRemap(action); });
		}
		return;
	}
	if (key == KEY_ESCAPE) {
		switchTo([this] { buildList(); });
	}
	else if (key >= KEY_A && key <= KEY_R) {
		this->save.keys[this->remapAction] = (uint8_t)(key - KEY_A);
		switchTo([this] { buildList(); });
	}
}

void KeyConfigScreen::update(float dt) {
	this->pageFade.update(dt);
}

void KeyConfigScreen::render(Renderer& renderer) {
	for (const Label& label : this->labels) {
		renderer.drawTexture(*label.texture, Point((float)label.x, (float)label.y));
	}
	renderer.drawTexture(*this->assets.bottom, Point(0, ScreenAssets::FRAME_BOTTOM_Y));
	drawCentred(renderer, *this->caption, ScreenAssets::FRAME_CAPTION_Y);
	renderer.drawTexture(*this->assets.top, Point(0, 0));
	this->pageFade.render(renderer);
}
