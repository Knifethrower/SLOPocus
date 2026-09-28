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

#ifndef MENU_H
#define MENU_H

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include "texture.h"
#include "renderer.h"
#include "data/asset/font.h"
#include "eventhandler.h"
#include "animation.h"

namespace pocus {

class Menu {
public:
	enum { SPACE_HEIGHT = 4, BOTTOM_TEXT_Y = 188 };
	
public:
	void setIndicator(Animation animation);
	void addOption(const std::string& option);
	void addOption(const std::string& option, std::function<void()> handler);
	// A non-selectable heading line (the original's title menus: "Which game
	// do you want to play?" etc.), drawn 20 px above the first item.
	void addTitle(const std::string& title);
	void addSpace();
	void clear();
	// Called on the back/escape button; not set = escape ignored (the
	// original's main menu can't be escaped).
	void setEscapeHandler(std::function<void()> handler);
	// Positions the block the way the original's menu drawer (HOCUS.EXE
	// FUN_16b8_19a4) does: centred horizontally on the widest item, and
	// vertically within the 144 px band below the top image (y 40..184),
	// 10 px per line plus 4 px after each "spacer" item.
	void layoutLikeOriginal();
	[[nodiscard]] bool isEmpty() const;
	void setFont(data::asset::Font font);
	void setPalette(data::asset::Palette& palette);
	void render(Renderer& renderer);
	void handleEvents(EventHandler& eventHandler);
	void update(float dt);
	void setPosition(const Point& point);
	void setLineSpacing(uint32_t spacing);
	void setTextColor(uint8_t color);
	void setCapitalLetterColor(uint8_t color);
	void setBottomText(const std::string& text);
	void setBottomTextColor(uint8_t color);
	void moveDown();
	void moveUp();
	
	int8_t getCurrentSelection() const;
	// The original keeps each menu's cursor between visits (16b8:0b21 loads it).
	void setCurrentSelection(int8_t selection);

private:
	data::asset::Font font;
	data::asset::Palette palette;
	Point position;
	uint32_t lineSpacing { 10 };
	std::vector<std::tuple<std::string, std::function<void()>, std::unique_ptr<Texture>>> options;
	uint8_t textColor, capitalLetterColor;
	uint8_t bottomTextColor;
	std::string bottomText;
	std::unique_ptr<Texture> bottomLabel { nullptr };
	int8_t currentSelection { 0 };
	Animation indicatorAnimation;
	std::string titleText;
	std::unique_ptr<Texture> titleLabel { nullptr };
	std::function<void()> escapeHandler;
	
};

}

#endif // MENU_H
