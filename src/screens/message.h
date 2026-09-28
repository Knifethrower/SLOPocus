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

#ifndef MESSAGE_H
#define MESSAGE_H

#include "screen.h"

namespace pocus::ui {

// The original's help-line table (DS:19dc), indexed by the menu/box kind.
enum HelpLine {
	HELP_MAIN_MENU = 0,      // "Use UP/DOWN/LETTER to move - ENTER to select"
	HELP_MENU = 1,           // same text
	HELP_ANY_KEY = 2,        // "Press any key to continue"
	HELP_YES_NO = 3,         // "Press Y for yes - N for no - ESC to exit"
	HELP_NUMBER = 4,         // "Use UP/DOWN/NUMBER to move - ENTER to select"
	HELP_ESC = 5             // "ESC to exit"
};
std::string helpLine(int index);

// 16b8:2578 / 0d99: one line in font 5 at y=107 between the frame images,
// any key closes it.
class MessageScreen : public Screen {
public:
	MessageScreen(ScreenAssets& assets, const std::string& text, int help = HELP_ANY_KEY);

	void handleEvents(EventHandler& eventHandler) override;
	void render(Renderer& renderer) override;

private:
	ScreenAssets& assets;
	std::unique_ptr<Texture> label;
	std::unique_ptr<Texture> caption;
};

// 16b8:0e52 / 0fbc: two lines (font 5 at y=97, font 3 at y=117); Y gives 1,
// N gives 0, ESC gives -1.
class YesNoScreen : public Screen {
public:
	YesNoScreen(ScreenAssets& assets, const std::string& line1, const std::string& line2, int help = HELP_YES_NO);

	void handleEvents(EventHandler& eventHandler) override;
	void render(Renderer& renderer) override;

private:
	ScreenAssets& assets;
	std::unique_ptr<Texture> label1, label2;
	std::unique_ptr<Texture> caption;
};

}

#endif // MESSAGE_H
