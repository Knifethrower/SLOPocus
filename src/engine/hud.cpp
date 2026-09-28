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

#include "hud.h"
#include <sstream>
#include <utility>

using namespace pocus;

void Hud::setBackground(std::unique_ptr<Texture> texture) {
	this->background = std::move(texture);
}

Texture& Hud::getBackground() {
	return *this->background;
}

void Hud::setSilverKeyTexture(std::unique_ptr<Texture> texture) {
	this->textureSilverKey = std::move(texture);
}

void Hud::setGoldenKeyTexture(std::unique_ptr<Texture> texture) {
	this->textureGoldenKey = std::move(texture);
}

void Hud::render(Renderer &renderer) {
	renderer.drawTexture(*this->background, Point(0, renderer.getHeight() - this->background->getHeight()));
	renderer.drawTexture(*this->labelScore, Point(SCORE_X + ((SCORE_WIDTH / 2) - (this->labelScore->getWidth() / 2)), SCORE_Y));
	renderer.drawTexture(*this->labelHealth, Point(HEALTH_X + ((HEALTH_WIDTH / 2) - (this->labelHealth->getWidth() / 2)), HEALTH_Y));
	renderer.drawTexture(*this->labelCrystals, Point(CRYSTALS_X + ((CRYSTALS_WIDTH / 2) - (this->labelCrystals->getWidth() / 2)), CRYSTALS_Y));
	if (this->levelVisible) {
		renderer.drawTexture(*this->labelLevel, Point(LEVEL_X, LEVEL_Y));
	}
	
	if (this->goldenKey) {
		if (this->silverKey) {
			renderer.drawTexture(*this->textureSilverKey, Point(KEYS_BOTH_X, KEYS_Y));
			renderer.drawTexture(*this->textureGoldenKey, Point(KEYS_BOTH_X + 8, KEYS_Y));
		}
		else {
			renderer.drawTexture(*this->textureGoldenKey, Point(KEYS_X, KEYS_Y));
		}
	}
	else {
		if (this->silverKey) {
			renderer.drawTexture(*this->textureSilverKey, Point(KEYS_X, KEYS_Y));
		}
	}
}

void Hud::updateKeys(bool silverKey, bool goldenKey) {
	this->silverKey = silverKey;
	this->goldenKey = goldenKey;
}

void Hud::setFont(data::asset::Palette palette, data::asset::Font font, uint8_t color) {
	this->font = std::move(font);
	this->paletteFont = std::move(palette);
	this->fontColor = color;
}

void Hud::updateScore(uint32_t score) {
	char text[32];
	sprintf(text, "%d", score);
	this->labelScore = this->font.writeGradientShadow(std::string(text), this->paletteFont, this->fontColor);
}

void Hud::updateHealth(uint32_t health) {
	char text[32];
	sprintf(text, "%d", health);
	this->labelHealth = this->font.writeGradientShadow(std::string(text), this->paletteFont, this->fontColor);
}

void Hud::updateCrystals(uint32_t current, uint32_t max) {
	char text[32];
	sprintf(text, "%d/%d", current, max);
	this->labelCrystals = this->font.writeGradientShadow(std::string(text), this->paletteFont, this->fontColor);
}

void Hud::updateLevel(uint32_t level) {
	char text[32];
	sprintf(text, "%d", level);
	this->labelLevel = this->font.writeGradientShadow(std::string(text), this->paletteFont, this->fontColor);
}