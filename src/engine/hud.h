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

#ifndef HUD_H
#define HUD_H

#include <memory>
#include "texture.h"
#include "renderer.h"
#include "data/asset/font.h"

namespace pocus {

class Hud {
public:
	enum { SCORE_X = 12, SCORE_Y = 181, SCORE_WIDTH = 56 };
	enum { HEALTH_X = 88, HEALTH_Y = 181, HEALTH_WIDTH = 24 };
	enum { CRYSTALS_X = 156, CRYSTALS_Y = 181, CRYSTALS_WIDTH = 25 };
	enum { LEVEL_X = 296, LEVEL_Y = 182 };
	enum { KEYS_X = 216, KEYS_BOTH_X = 212, KEYS_Y = 180, KEYS_WIDTH = 24 };
	enum { DEFAULT_FONT_COLOR = 112 };
	
public:
	void setBackground(std::unique_ptr<Texture> texture);
	Texture& getBackground();
	void setFont(data::asset::Palette palette, data::asset::Font font, uint8_t fontColor = DEFAULT_FONT_COLOR);
	
	void setSilverKeyTexture(std::unique_ptr<Texture> texture);
	void setGoldenKeyTexture(std::unique_ptr<Texture> texture);
	
	void updateScore(uint32_t score);
	void updateHealth(uint32_t health);
	void updateCrystals(uint32_t current, uint32_t max);
	void updateLevel(uint32_t level);
	void updateKeys(bool silverKey, bool goldenKey);
	// The level number blinks once every treasure is found (348d: glyph 10 of
	// the digit strip, a blank, for 9 of every 20 frames).
	void setLevelVisible(bool visible) { this->levelVisible = visible; }
	
	void render(Renderer& renderer);
	
private:
	data::asset::Palette paletteFont;
	data::asset::Font font;
	std::unique_ptr<Texture> background;
	
	std::unique_ptr<Texture> labelScore;
	std::unique_ptr<Texture> labelHealth;
	std::unique_ptr<Texture> labelCrystals;
	std::unique_ptr<Texture> labelLevel;
	std::unique_ptr<Texture> textureSilverKey;
	std::unique_ptr<Texture> textureGoldenKey;
	bool silverKey;
	bool goldenKey;
	bool levelVisible { true };
	
	uint8_t fontColor;
};

}

#endif // HUD_H
