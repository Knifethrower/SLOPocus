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

#ifndef RENDERER_H
#define RENDERER_H

#include "texture.h"
#include "color.h"
#include "rect.h"
#include <cstdint>
#include <string>

namespace pocus {

struct RendererParameters {
	uint32_t width;
	uint32_t height;
	std::string title;
	float scaleFactor;
};

class Renderer {
public:
	virtual bool initialize() = 0;
	virtual void release() = 0;
	virtual void clear() = 0;
	virtual void render() = 0;
	virtual uint32_t getWidth() = 0;
	virtual uint32_t getHeight() = 0;
	
	virtual bool createTexture(Texture& texture) = 0;
	
	virtual void drawTexture(Texture& texture, const Point& point) = 0;
	virtual void drawRect(const Rect& rect, const Color& color) = 0;
	virtual void drawPoint(const Point& point, const Color& color) = 0;

};

}

#endif //RENDERER_H
