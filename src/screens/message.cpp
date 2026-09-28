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

#include "message.h"
#include "../exedata.h"

using namespace pocus;
using namespace pocus::ui;

std::string pocus::ui::helpLine(int index) {
	// The pad wording of the EXE's help lines (gptokeyb2: START = Enter,
	// SELECT = Esc, A = fire = yes, B = jump = no).
	static const char* PAD_LINES[6] = {
		"Use UP/DOWN to move - START to select",
		"Use UP/DOWN to move - START to select",
		"Press any button to continue",
		"Press A for yes - B for no - SELECT to exit",
		"Use UP/DOWN to move - START to select",
		"SELECT to exit"
	};
	const int i = index < 0 || index > 5 ? 2 : index;
	return padPrompts() ? PAD_LINES[i] : ExeData::get().pointerString(PTR_HELP, i);
}

namespace {

void drawCentred(Renderer& renderer, Texture& texture, int y) {
	renderer.drawTexture(texture, Point((float)((ScreenAssets::WIDTH - (int)texture.getWidth() + 1) / 2), (float)y));
}

}

MessageScreen::MessageScreen(ScreenAssets& assets, const std::string& text, int help):
	assets(assets)
{
	this->label = assets.text(text, 5);
	this->caption = assets.shadowText(helpLine(help), 4);
}

void MessageScreen::handleEvents(EventHandler& eventHandler) {
	if (eventHandler.getKeyDown() != KEY_NONE) {
		finish(0);
	}
}

void MessageScreen::render(Renderer& renderer) {
	drawCentred(renderer, *this->label, 107);
	renderer.drawTexture(*this->assets.bottom, Point(0, ScreenAssets::FRAME_BOTTOM_Y));
	drawCentred(renderer, *this->caption, ScreenAssets::FRAME_CAPTION_Y);
	renderer.drawTexture(*this->assets.top, Point(0, 0));
}

YesNoScreen::YesNoScreen(ScreenAssets& assets, const std::string& line1, const std::string& line2, int help):
	assets(assets)
{
	this->label1 = assets.text(line1, 5);
	this->label2 = assets.text(line2, 3);
	this->caption = assets.shadowText(helpLine(help), 4);
}

void YesNoScreen::handleEvents(EventHandler& eventHandler) {
	switch (eventHandler.getKeyDown()) {
		case KEY_Y: finish(1); return;
		case KEY_N: finish(0); return;
		case KEY_ESCAPE: finish(-1); return;
		default: break;
	}
	// On the pad the fire button (A) answers yes and the jump button (B) no.
	if (padPrompts()) {
		if (eventHandler.isButtonDown(BUTTON_FIRE)) {
			finish(1);
		}
		else if (eventHandler.isButtonDown(BUTTON_JUMP)) {
			finish(0);
		}
	}
}

void YesNoScreen::render(Renderer& renderer) {
	drawCentred(renderer, *this->label1, 97);
	drawCentred(renderer, *this->label2, 117);
	renderer.drawTexture(*this->assets.bottom, Point(0, ScreenAssets::FRAME_BOTTOM_Y));
	drawCentred(renderer, *this->caption, ScreenAssets::FRAME_CAPTION_Y);
	renderer.drawTexture(*this->assets.top, Point(0, 0));
}
