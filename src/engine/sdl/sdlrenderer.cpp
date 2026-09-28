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

#include <cstdio>
#include <cstdlib>
#include <string>
#include <cstring>
#include "sdlrenderer.h"
#include "../log.h"
#include "sdltexture.h"

using namespace pocus;

namespace {

bool envFlag(const char* name) {
	const char* value = std::getenv(name);
	return value && value[0] != '\0' && std::strcmp(value, "0") != 0;
}

// SLOPOCUS_ASPECT: "W:H" (4:3, 16:10, 1:1, 3:2, 16:9, 5:3 ...) presents the
// frame in the largest centred box of that shape, "fill" stretches it over
// the whole output. Returns false when unset (SDL's own 16:10 logical size).
bool envAspect(float& aspect, bool& fill) {
	const char* value = std::getenv("SLOPOCUS_ASPECT");
	if (!value || value[0] == 0) {
		return false;
	}
	if (std::strcmp(value, "fill") == 0 || std::strcmp(value, "stretch") == 0) {
		fill = true;
		return true;
	}
	int w = 0, h = 0;
	if (std::sscanf(value, "%d:%d", &w, &h) == 2 && w > 0 && h > 0) {
		aspect = (float)w / (float)h;
		fill = false;
		return true;
	}
	LOGW << "Renderer: SLOPOCUS_ASPECT '" << value << "' not understood (use W:H or fill)";
	return false;
}

}

SdlRenderer::SdlRenderer(const RendererParameters &parameters) :
	parameters(parameters),
	window(nullptr),
	renderer(nullptr)
{
}

bool SdlRenderer::initialize() {
	SDL_Init(SDL_INIT_EVERYTHING);

	// Handheld/PortMaster switches (set by the launch script):
	//   SLOPOCUS_FULLSCREEN=1   borderless fullscreen at the desktop resolution
	//   SLOPOCUS_SOFTWARE=1     SDL's software renderer (GLES2/Mali targets reject
	//                            the texture formats the accelerated one wants)
	//   SLOPOCUS_ASPECT=4:3     present the 320x200 frame in a centred box of
	//                            that shape (a VGA monitor showed mode 13h at
	//                            4:3), any W:H, or "fill" for the whole panel;
	//                            unset = SDL's 16:10 logical size (desktop)
	const bool fullscreen = envFlag("SLOPOCUS_FULLSCREEN");
	const bool software = envFlag("SLOPOCUS_SOFTWARE");
	this->boxed = envAspect(this->aspect, this->fill);

	Uint32 windowFlags = SDL_WINDOW_RESIZABLE;
	if (fullscreen) {
		windowFlags = SDL_WINDOW_FULLSCREEN_DESKTOP;
	}
	this->window = SDL_CreateWindow(
			this->parameters.title.c_str(),
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			this->parameters.width * this->parameters.scaleFactor, this->parameters.height * this->parameters.scaleFactor,
			windowFlags);
	if (!this->window) {
		LOGE << "Renderer: SDL_CreateWindow failed: " << SDL_GetError();
		return false;
	}

	if (!software) {
		SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
	}
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
	this->renderer = SDL_CreateRenderer(this->window, -1,
			software ? SDL_RENDERER_SOFTWARE : (SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC));
	if (!this->renderer) {
		// No accelerated renderer (headless runs with SDL's dummy video
		// driver, or a bare VM): the software one draws the same picture.
		this->renderer = SDL_CreateRenderer(this->window, -1, SDL_RENDERER_SOFTWARE);
	}
	if (!this->renderer) {
		LOGE << "Renderer: SDL_CreateRenderer failed: " << SDL_GetError();
		return false;
	}
	{
		SDL_RendererInfo info;
		if (SDL_GetRendererInfo(this->renderer, &info) == 0) {
			LOGI << "Renderer: " << info.name << (fullscreen ? ", fullscreen" : "")
				 << (this->boxed ? (this->fill ? ", fill" : ", boxed ") : "") << (this->boxed && !this->fill ? std::to_string(this->aspect) : "");
		}
	}

	SDL_SetRenderDrawBlendMode(this->renderer, SDL_BLENDMODE_BLEND);
	applyScaling();

	return true;
}

// Fits the 320x200 frame into the output. Without SLOPOCUS_ASPECT SDL's
// logical size keeps the frame's own 16:10 shape. With it, everything is
// drawn into a 320x200 target texture and presented in one blit into the
// requested box (a per-draw non-uniform scale leaves hairline seams between
// tiles), centred with black bars on whatever panel shape the device has:
// 1:1 (RGB30), 4:3, 3:2 (480x320), 16:9, 5:3 ...
void SdlRenderer::applyScaling() {
	int outputWidth = 0, outputHeight = 0;
	SDL_GetRendererOutputSize(this->renderer, &outputWidth, &outputHeight);
	if (!this->boxed || outputWidth <= 0 || outputHeight <= 0) {
		SDL_RenderSetLogicalSize(this->renderer, this->parameters.width, this->parameters.height);
		return;
	}
	if (!this->frame) {
		this->frame = SDL_CreateTexture(this->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET,
			(int)this->parameters.width, (int)this->parameters.height);
		if (!this->frame) {
			LOGE << "Renderer: no render target (" << SDL_GetError() << "), using the logical size";
			this->boxed = false;
			SDL_RenderSetLogicalSize(this->renderer, this->parameters.width, this->parameters.height);
			return;
		}
	}
	int boxWidth = outputWidth;
	int boxHeight = outputHeight;
	if (!this->fill) {
		boxHeight = (int)((float)outputWidth / this->aspect + 0.5f);
		if (boxHeight > outputHeight) {
			boxHeight = outputHeight;
			boxWidth = (int)((float)outputHeight * this->aspect + 0.5f);
		}
	}
	this->frameBox = SDL_Rect { (outputWidth - boxWidth) / 2, (outputHeight - boxHeight) / 2, boxWidth, boxHeight };
	LOGI << "Renderer: output " << outputWidth << "x" << outputHeight << ", frame box " << boxWidth << "x" << boxHeight
		 << " at " << this->frameBox.x << "," << this->frameBox.y;
}

void SdlRenderer::release() {
	if (this->frame) {
		SDL_DestroyTexture(this->frame);
		this->frame = nullptr;
	}

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
	if (this->frame) {
		SDL_SetRenderTarget(this->renderer, this->frame);
	}
	SDL_SetRenderDrawColor(this->renderer, 0, 0, 0, 0);
	SDL_RenderClear(this->renderer);
}

void SdlRenderer::render() {
	if (this->frame) {
		SDL_SetRenderTarget(this->renderer, nullptr);
		SDL_SetRenderDrawColor(this->renderer, 0, 0, 0, 255);
		SDL_RenderClear(this->renderer);
		SDL_RenderCopy(this->renderer, this->frame, nullptr, &this->frameBox);
	}
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