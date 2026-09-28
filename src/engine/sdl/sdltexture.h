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

#ifndef _SDLTEXTURE_H
#define _SDLTEXTURE_H

#include <SDL2/SDL.h>
#include <string>
#include "../texture.h"

namespace pocus {

class SdlTexture : public Texture {
public:
	friend class SdlRenderer;
	
public:
	SdlTexture();
	virtual ~SdlTexture();
	
	bool createBlank(uint32_t width, uint32_t height) override;
	void setPixel(uint32_t index, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha) override;
	void getPixel(uint32_t index, uint8_t* red, uint8_t* green, uint8_t* blue, uint8_t* alpha) override;
	uint32_t getWidth() const override;
	uint32_t getHeight() const override;
	bool isReady() const override;
	bool loadFromStream(const char* stream, uint32_t length) override;
	void fill(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha) override;
	void paste(const Texture& texture, uint32_t fx, uint32_t fy, uint32_t tx, uint32_t ty) override;
	void setColorKey(uint8_t red, uint8_t green, uint8_t blue) override;
	std::unique_ptr<Texture> extract(uint32_t x, uint32_t y, uint32_t w, uint32_t h, const Color* colorKey) override;
	bool saveToFile(const std::string& filename) override;
	void setOverlayColor(const Color& color) override;
	void restoreColor() override;
	
private:
	SDL_Surface* surface { nullptr };
	SDL_Texture* texture { nullptr };
};

}

#endif //_SDLTEXTURE_H
