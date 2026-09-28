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

#include "sdlrenderer.h"
#include "../log.h"
#include "sdltexture.h"

using namespace pocus;

SdlRenderer::SdlRenderer(const RendererParameters &parameters) :
	parameters(parameters),
	window(nullptr),
	renderer(nullptr)
{
}

bool SdlRenderer::initialize() {
	SDL_Init(SDL_INIT_EVERYTHING);

	this->window = SDL_CreateWindow(
			this->parameters.title.c_str(),
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			this->parameters.width * this->parameters.scaleFactor, this->parameters.height * this->parameters.scaleFactor,
			SDL_WINDOW_RESIZABLE);
	if (!this->window) {
		return false;
	}

	this->renderer = SDL_CreateRenderer(this->window, -1,
			SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!this->renderer) {
		// No accelerated renderer (headless runs with SDL's dummy video
		// driver, or a bare VM): the software one draws the same picture.
		this->renderer = SDL_CreateRenderer(this->window, -1, SDL_RENDERER_SOFTWARE);
	}
	if (!this->renderer) {
		return false;
	}

	SDL_SetRenderDrawBlendMode(this->renderer, SDL_BLENDMODE_BLEND);

	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
	SDL_RenderSetLogicalSize(this->renderer, this->parameters.width, this->parameters.height);

	return true;
}

void SdlRenderer::release() {
	if (this->window) {
		SDL_DestroyWindow(this->window);
		this->window = nullptr;
	}

	if (this->renderer) {
		SDL_DestroyRenderer(this->renderer);
		this->renderer = nullptr;
	}

	SDL_Quit();
}

void SdlRenderer::clear() {
	SDL_SetRenderDrawColor(this->renderer, 0, 0, 0, 0);
	SDL_RenderClear(this->renderer);
}

void SdlRenderer::render() {
	SDL_RenderPresent(this->renderer);
}

bool SdlRenderer::createTexture(Texture &texture) {
	auto sdlTexture = reinterpret_cast<SdlTexture*>(&texture);
	
	sdlTexture->texture = SDL_CreateTextureFromSurface(this->renderer, sdlTexture->surface);
	if (!sdlTexture->texture) {
		return false;
	}
	
	return true;
}

void SdlRenderer::drawTexture(Texture& texture, const Point& point) {
	if (!texture.isReady()) {
		createTexture(texture);
	}
	
	auto sdlTexture = reinterpret_cast<SdlTexture*>(&texture);
	
	SDL_Rect rect = (SDL_Rect){ (int)point.getX(), (int)point.getY(), (int)texture.getWidth(), (int)texture.getHeight() };
	SDL_RenderCopy(this->renderer, sdlTexture->texture, nullptr, &rect);
}

void SdlRenderer::drawRect(const Rect& rect, const Color &color) {
	int w = (int)rect.getSize().getWidth();
	int h = (int)rect.getSize().getHeight();
	if (w == -1 || h == -1) {
		w = (int)this->parameters.width;
		h = (int)this->parameters.height;
	}
	
	SDL_Rect sdlRect = (SDL_Rect){ (int)rect.getPosition().getX(), (int)rect.getPosition().getY(), w, h };
	SDL_SetRenderDrawColor(this->renderer, color.red, color.green, color.blue, color.alpha);
	SDL_RenderFillRect(this->renderer, &sdlRect);
}

void SdlRenderer::drawPoint(const Point& point, const Color& color) {
	SDL_SetRenderDrawColor(this->renderer, color.red, color.green, color.blue, color.alpha);
	SDL_RenderDrawPoint(this->renderer, (int)point.getX(), (int)point.getY());
}

uint32_t SdlRenderer::getWidth() {
	return this->parameters.width;
}

uint32_t SdlRenderer::getHeight() {
	return this->parameters.height;
}