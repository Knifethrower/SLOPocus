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

#ifndef SDLRENDERER_H
#define SDLRENDERER_H

#include <SDL2/SDL.h>

#include "../renderer.h"
#include "../texture.h"

namespace pocus {

class SdlRenderer : public Renderer {
public:
	SdlRenderer(const RendererParameters &parameters);

	virtual bool initialize() override;
	// Fits the 320x200 frame into the output (see the .cpp).
	void applyScaling();
	virtual void release() override;
	virtual void clear() override;
	virtual void render() override;
	uint32_t getWidth() override;
	uint32_t getHeight() override;
	
	bool createTexture(Texture& texture) override;
	
	void drawTexture(Texture& texture, const Point& point) override;
	void drawRect(const Rect& rect, const Color& color) override;
	void drawPoint(const Point& point, const Color& color) override;



private:
	RendererParameters parameters;
	bool boxed { false };            // SLOPOCUS_ASPECT set
	bool fill { false };
	float aspect { 4.0f / 3.0f };
	SDL_Texture* frame { nullptr };   // 4:3 mode: the 320x200 render target
	SDL_Rect frameBox { 0, 0, 0, 0 };  // where it is presented
	SDL_Window *window;
	SDL_Renderer *renderer;
};

}

#endif //SDLRENDERER_H
