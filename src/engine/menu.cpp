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

#include <algorithm>
#include <iostream>
#include <functional>
#include "menu.h"
#include "../definitions.h"

using namespace pocus;

void Menu::setFont(data::asset::Font font) {
	this->font = std::move(font);
}

void Menu::setPalette(data::asset::Palette& palette) {
	this->palette = palette;
}

void Menu::addOption(const std::string& option) {
	this->options.emplace_back(option, nullptr, this->font.writeGradient(option, this->palette, this->textColor, this->capitalLetterColor));
}

void Menu::addOption(const std::string& option, std::function<void()> handler) {
	this->options.emplace_back(option, std::move(handler), this->font.writeGradient(option, this->palette, this->textColor, this->capitalLetterColor));
}

void Menu::addSpace() {
	this->options.emplace_back("", nullptr, nullptr);
}

void Menu::addTitle(const std::string& title) {
	this->titleText = title;
	this->titleLabel = this->font.writeGradientShadow(title, this->palette, this->bottomTextColor);
}

void Menu::clear() {
	this->options.clear();
	this->titleText.clear();
	this->titleLabel = nullptr;
	this->currentSelection = 0;
}

void Menu::setEscapeHandler(std::function<void()> handler) {
	this->escapeHandler = std::move(handler);
}

bool Menu::isEmpty() const {
	return this->options.empty();
}

void Menu::layoutLikeOriginal() {
	int maxWidth = 0;
	int lines = 0;
	int gaps = 0;
	for (const auto& option : this->options) {
		if (std::get<0>(option).empty()) {
			gaps += SPACE_HEIGHT;
			continue;
		}
		lines++;
		maxWidth = std::max<int>(maxWidth, (int)this->font.calculateWidth(std::get<0>(option)));
	}
	const int x = (SCREEN_WIDTH - maxWidth) / 2;
	int y;
	if (this->titleLabel) {
		// Title 20 px above the block; the original centres title + items.
		y = (144 - (lines + 1) * 10 + gaps) / 2 + 60;
	}
	else {
		y = (144 - (lines * 10 + gaps)) / 2 + 40;
	}
	this->position = Point((float)x, (float)y);
}

void Menu::setIndicator(Animation animation) {
	this->indicatorAnimation = std::move(animation);
}

void Menu::update(float dt) {
	this->indicatorAnimation.update(dt);
}

void Menu::render(Renderer &renderer) {
	if (this->titleLabel) {
		renderer.drawTexture(*this->titleLabel, Point(SCREEN_WIDTH / 2 - this->font.calculateWidth(this->titleText) / 2, this->position.getY() - 20));
	}

	uint32_t label = 0;
	uint32_t yOffset = this->position.getY();
	for (int i = 0; i < this->options.size(); i++) {
		uint32_t offset = this->lineSpacing;
		if (!std::get<0>(this->options[i]).empty()) {
			renderer.drawTexture(*std::get<2>(this->options[i]), pocus::Point(this->position.getX(), yOffset));
			label++;
		}
		else {
			offset = Menu::SPACE_HEIGHT;
		}

		if (this->currentSelection == i) {
			// The original draws the selector 24 px left of the text column,
			// 4 px above the line.
			this->indicatorAnimation.render(renderer, Point(this->position.getX() - 24, yOffset - 4));
		}

		yOffset += offset;
	}
	
	// Bottom text
	if (this->bottomLabel) {
		renderer.drawTexture(*this->bottomLabel, Point(SCREEN_WIDTH / 2 - this->font.calculateWidth(this->bottomText) / 2, Menu::BOTTOM_TEXT_Y));
	}
}

void Menu::handleEvents(EventHandler& eventHandler) {
	if (this->options.empty()) {
		return;
	}
	if (eventHandler.isButtonDown(pocus::BUTTON_DOWN)) {
		moveDown();
	}
	else if (eventHandler.isButtonDown(pocus::BUTTON_UP)) {
		moveUp();
	}
	else if (eventHandler.isButtonDown(pocus::BUTTON_SELECTION)) {
		if (std::get<1>(this->options[this->currentSelection])) {
			// Copy: the handler may rebuild this menu (clear()) while running.
			auto handler = std::get<1>(this->options[this->currentSelection]);
			handler();
		}
	}
	else if (eventHandler.isButtonDown(pocus::BUTTON_BACK)) {
		if (this->escapeHandler) {
			auto handler = this->escapeHandler;
			handler();
		}
	}
}

void Menu::setPosition(const Point& point) {
	this->position = point;
}

void Menu::setLineSpacing(uint32_t spacing) {
	this->lineSpacing = spacing;
}

void Menu::setTextColor(uint8_t color) {
	this->textColor = color;
}

void Menu::setCapitalLetterColor(uint8_t color) {
	this->capitalLetterColor = color;
}

void Menu::setBottomTextColor(uint8_t color) {
	this->bottomTextColor = color;
}

void Menu::setBottomText(const std::string& text) {
	this->bottomText = text;
	this->bottomLabel = this->font.writeGradientShadow(text, this->palette, this->bottomTextColor);
}

void Menu::moveDown() {
	this->currentSelection++;
	
	if (this->currentSelection >= this->options.size()) {
		this->currentSelection = 0;
		return;
	}
	
	const std::string& option = std::get<0>(this->options[this->currentSelection]);
	if (option.empty()) {
		moveDown();
	}
}

void Menu::moveUp() {
	this->currentSelection--;
	
	if (this->currentSelection < 0) {
		this->currentSelection = this->options.size() - 1;
		return;
	}
	
	const std::string& option = std::get<0>(this->options[this->currentSelection]);
	if (option.empty()) {
		moveUp();
	}
}

int8_t Menu::getCurrentSelection() const {
	return currentSelection;
}

void Menu::setCurrentSelection(int8_t selection) {
	if (selection < 0 || selection >= (int8_t)this->options.size() || std::get<0>(this->options[selection]).empty()) {
		this->currentSelection = 0;
		return;
	}
	this->currentSelection = selection;
}
