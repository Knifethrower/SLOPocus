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

#include <SDL_image.h>
#include "sdltexture.h"

using namespace pocus;

SdlTexture::SdlTexture() :
	surface(nullptr),
	texture(nullptr)
{
}

SdlTexture::~SdlTexture() {
	if (this->surface) {
		SDL_FreeSurface(this->surface);
		this->surface = nullptr;
	}
	
	if (this->texture) {
		SDL_DestroyTexture(this->texture);
		this->texture = nullptr;
	}
}

bool SdlTexture::createBlank(uint32_t width, uint32_t height) {
	this->surface = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
	if (!this->surface )	{
		return false;
	}
	
	return true;
}

void SdlTexture::setPixel(uint32_t index, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha) {
	if (!this->surface || index >= (uint32_t)(this->surface->w * this->surface->h)) {
		return;
	}
	auto* pixels = (Uint32*)this->surface->pixels;
	pixels[index] = SDL_MapRGB(this->surface->format,red,green,blue);
}

uint32_t SdlTexture::getWidth() const {
	return this->surface ? (uint32_t)this->surface->w : 0;
}

uint32_t SdlTexture::getHeight() const {
	return this->surface ? (uint32_t)this->surface->h : 0;
}

bool SdlTexture::isReady() const {
	return this->texture != nullptr;
}

bool SdlTexture::loadFromStream(const char *stream, uint32_t length) {
	SDL_RWops* rWops;
	
	rWops = SDL_RWFromConstMem((const void*)stream, (int)length);
	if (!rWops)	{
		return false;
	}
	
	this->surface = IMG_Load_RW(rWops, 0);
	if (!this->surface)	{
		SDL_FreeRW(rWops);
		return false;
	}
	
	SDL_FreeRW(rWops);
	
	return true;
}

void SdlTexture::fill(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha) {
	uint32_t color = SDL_MapRGBA(this->surface->format, red, green, blue, alpha);
	SDL_FillRect(this->surface, nullptr, color);
}

void SdlTexture::paste(const Texture& texture, uint32_t fx, uint32_t fy, uint32_t tx, uint32_t ty) {
	SDL_Rect sdlRectSrc = (SDL_Rect){ (int)fx, (int)fy, (int)texture.getWidth(), (int)texture.getHeight() };
	SDL_Rect sdlRectDst = (SDL_Rect){ (int)tx, (int)ty, (int)texture.getWidth(), (int)texture.getHeight() };
	
	const auto& sdlTexture = reinterpret_cast<const SdlTexture&>(texture);
	
	SDL_BlitSurface(sdlTexture.surface, &sdlRectSrc, this->surface, &sdlRectDst);
}

void SdlTexture::getPixel(uint32_t index, uint8_t* red, uint8_t* green, uint8_t* blue, uint8_t* alpha) {
	if (!this->surface || index >= (uint32_t)(this->surface->w * this->surface->h)) {
		if (red) *red = 0;
		if (green) *green = 0;
		if (blue) *blue = 0;
		if (alpha) *alpha = 0;
		return;
	}
	Uint32 *pixels = (Uint32*)this->surface->pixels;
	Uint32 tempPixel = pixels[index];
	
	if (alpha == nullptr) {
		SDL_GetRGB(tempPixel, this->surface->format, red, green, blue);
	}
	else {
		SDL_GetRGBA(tempPixel, this->surface->format, red, green, blue, alpha);
	}
}

void SdlTexture::setColorKey(uint8_t red, uint8_t green, uint8_t blue) {
	SDL_SetColorKey(this->surface, SDL_TRUE, SDL_MapRGB(this->surface->format, red, green, blue));
}

std::unique_ptr<Texture> SdlTexture::extract(uint32_t x, uint32_t y, uint32_t w, uint32_t h, const Color* colorKey) {
	auto sdlTexture = std::make_unique<SdlTexture>();

	SDL_Rect rect = (SDL_Rect){(int)x, (int)y, (int)w, (int)h};

	sdlTexture->createBlank(w, h);

	// If the source is already color-keyed (e.g. re-extracting an already-extracted
	// sprite frame), SDL_BlitSurface skips copying keyed-out pixels, leaving the
	// destination's uninitialized (black) memory there instead of the key color.
	// Pre-fill with the key color so skipped pixels land on the right transparent
	// color instead of black.
	if (colorKey) {
		sdlTexture->fill(colorKey->red, colorKey->green, colorKey->blue, 255);
	}

	SDL_BlitSurface(this->surface, &rect, sdlTexture->surface, nullptr);

	if (colorKey) {
		sdlTexture->setColorKey(colorKey->red, colorKey->green, colorKey->blue);
	}

	return std::move(sdlTexture);
}

bool SdlTexture::saveToFile(const std::string& filename) {
	return SDL_SaveBMP(this->surface, filename.c_str()) == 0;
}

void SdlTexture::setOverlayColor(const Color& color) {
	if (this->texture) {
		SDL_SetTextureColorMod(this->texture, color.red, color.green, color.blue);
	}
}

void SdlTexture::restoreColor() {
	if (this->texture) {
		SDL_SetTextureColorMod(this->texture, 255, 255, 255);
	}
}