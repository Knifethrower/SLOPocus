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

#include "font.h"
#include "../../definitions.h"
#include "../../provider/provider.h"

using namespace pocus::data::asset;


FontGlyph::FontGlyph(const std::vector<bool>& data, char character, uint8_t width):
	character(character),
	width(width)
{
	this->texture = pocus::Provider::provideTexture(Font::GLYPH_SIZE, Font::GLYPH_SIZE);
	this->texture->fill(255, 0, 255);
	for (uint32_t i = 0; i < Font::GLYPH_SIZE * Font::GLYPH_SIZE; i++) {
		if (data[i]) {
			this->texture->setPixel(i, 255, 255, 255, 255);
		}
	}
}

uint8_t FontGlyph::getWidth() const {
	return this->width;
}

const pocus::Texture& FontGlyph::getTexture() const {
	return *this->texture;
}

/*
FontGlyph::FontGlyph(const char* data, uint8_t glyphSize)
{
	for (int i = 0; i < glyphSize * glyphSize; i++ )	{
		const uint32_t x = i % glyphSize;
		const uint32_t y = i / glyphSize;
		
		this->data.push_back(getBit(data[y], x));
	}
}
*/

bool Font::loadFromStream(const char *stream, uint32_t length) {
	// 1bpp (monochrome) 8×720 image; Main font, containing 90 8×8 glyphs
	
	const uint8_t characters = 90;
	const char* chars = "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz";
	const uint8_t offsets[characters] = { 3, 6, 6, 6, 6, 6, 3, 4, 4, 6, 6, 3, 6, 2, 7, 6, 4, 6,
							 6, 7, 6, 6, 7, 6, 6, 2, 2, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 3, 6, 6, 6, 7,
							 6, 6, 6, 7, 6, 6, 7, 6, 6, 7, 6, 7, 6, 5, 7, 5, 5, 7, 3, 6, 6, 6, 6, 6, 6, 6, 6,
							 3, 5, 6, 3, 7, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 6, 6, 6 };
	
	for (int characterIndex = 0; characterIndex < characters; characterIndex++) {
		std::vector<bool> glyphData;
		for (int i = 0; i < GLYPH_SIZE * GLYPH_SIZE; i++ )	{
			const uint32_t x = i % GLYPH_SIZE;
			const uint32_t y = i / GLYPH_SIZE;
			glyphData.push_back(getBit(stream[(characterIndex * GLYPH_SIZE) + y], x));
		}
		
		this->glyphs.insert(std::make_pair(chars[characterIndex], FontGlyph(glyphData, chars[characterIndex], offsets[characterIndex])));
	}
	
	return true;
}

void Font::release() {
	this->glyphs.erase(this->glyphs.begin(), this->glyphs.end());
}

uint32_t Font::calculateWidth(const std::string& text) {
	uint32_t width = 0;
	
	for (auto& character : text) {
		auto glyph = this->glyphs.find(character);
		uint8_t characterWidth = Font::SPACE_WIDTH;
		
		if (glyph != this->glyphs.end()) {
			characterWidth = glyph->second.getWidth() + 1;
		}
		
		width += characterWidth;
	}
	
	return width;
}

std::unique_ptr<pocus::Texture> Font::internalWrite(const std::string& text) {
	const uint32_t labelWidth = calculateWidth(text);
	const uint32_t labelHeight = GLYPH_SIZE;
	uint32_t offset = 0;
	
	auto textureLabel = pocus::Provider::provideTexture(labelWidth, labelHeight);
	
	textureLabel->fill(255, 0, 255, 255);
	
	// Create label
	for (auto& character : text) {
		auto glyph = this->glyphs.find(character);
		uint8_t characterWidth = Font::SPACE_WIDTH;
		
		if (glyph != this->glyphs.end()) {
			textureLabel->paste(glyph->second.getTexture(), 0, 0, offset, 0);
			characterWidth = glyph->second.getWidth() + 1;
		}
		
		offset += characterWidth;
	}
	
	return std::move(textureLabel);
}

std::unique_ptr<pocus::Texture> Font::write(const std::string& text, const Palette& palette, uint8_t color) {
	auto textureLabel = internalWrite(text);
	
	// Apply color
	for (uint32_t i = 0; i < textureLabel->getWidth() * textureLabel->getHeight(); i++) {
		uint8_t red, green, blue;
		const PaletteColor& paletteColor = palette.colors[color];
		textureLabel->getPixel(i, &red, &green, &blue, nullptr);
		if (red == 255 && green == 255 && blue == 255) {
			textureLabel->setPixel(i, paletteColor.r, paletteColor.g, paletteColor.b, 255);
		}
	}
	
	textureLabel->setColorKey(255, 0, 255);
	
	return std::move(textureLabel);
}

std::unique_ptr<pocus::Texture> Font::writeShadow(const std::string& text, const Palette& palette, uint8_t color) {
	auto textureLabel = pocus::Provider::provideTexture(calculateWidth(text) + 1, GLYPH_SIZE + 1);
	textureLabel->fill(255, 0, 255, 255);
	
	auto foreground = internalWrite(text);
	auto shadow = internalWrite(text);
	
	// Apply color
	// The foreground/shadow copies are one pixel smaller than the canvas.
	for (uint32_t i = 0; i < foreground->getWidth() * foreground->getHeight(); i++) {
		uint8_t red, green, blue;
		const PaletteColor& paletteColor = palette.colors[color];

		foreground->getPixel(i, &red, &green, &blue, nullptr);
		if (red == 255 && green == 255 && blue == 255) {
			foreground->setPixel(i, paletteColor.r, paletteColor.g, paletteColor.b, 255);
		}
		
		shadow->getPixel(i, &red, &green, &blue, nullptr);
		if (red == 255 && green == 255 && blue == 255) {
			shadow->setPixel(i, 0, 0, 0, 255);
		}
	}
	
	// Lay out both
	shadow->setColorKey(255, 0, 255);
	textureLabel->paste(*shadow, 0, 0, 1, 1);
	
	foreground->setColorKey(255, 0, 255);
	textureLabel->paste(*foreground, 0, 0, 0, 0);
	
	// Color key
	textureLabel->setColorKey(255, 0, 255);
	
	return std::move(textureLabel);
}

std::unique_ptr<pocus::Texture> Font::writeGradient(const std::string& text, const Palette& palette, uint8_t startColor, uint8_t capitalLetter) {
	auto capitalTexture = writeGradient(std::string(text.substr(0, 1)), palette, capitalLetter);
	auto textTexture = writeGradient(std::string(text.substr(1, text.length())), palette, startColor);
	
	auto layout = pocus::Provider::provideTexture(calculateWidth(text), Font::GLYPH_SIZE);
	layout->fill(255, 0, 255);
	layout->paste(*capitalTexture, 0, 0, 0, 0);
	layout->paste(*textTexture, 0, 0, calculateWidth(std::string(text.substr(0, 1))), 0);
	layout->setColorKey(255, 0, 255);
	
	return std::move(layout);
}

std::unique_ptr<pocus::Texture> Font::writeGradient(const std::string& text, const Palette& palette, uint8_t startColor) {
	auto textureLabel = internalWrite(text);
	
	// Apply color
	for (uint32_t i = 0; i < textureLabel->getWidth() * textureLabel->getHeight(); i++) {
		const uint32_t y = i / textureLabel->getWidth();
		uint8_t red, green, blue;
		
		const PaletteColor& paletteColor = palette.colors[startColor + y];
		
		textureLabel->getPixel(i, &red, &green, &blue, nullptr);
		
		if (red == 255 && green == 255 && blue == 255) {
			textureLabel->setPixel(i, paletteColor.r, paletteColor.g, paletteColor.b, 255);
		}
	}
	
	textureLabel->setColorKey(255, 0, 255);
	
	return std::move(textureLabel);
}


std::unique_ptr<pocus::Texture> Font::writeGradientShadow(const std::string& text, const Palette& palette, uint8_t startColor) {
	auto textureLabel = pocus::Provider::provideTexture(calculateWidth(text) + 1, GLYPH_SIZE + 1);
	textureLabel->fill(255, 0, 255, 255);
	
	auto foreground = internalWrite(text);
	auto shadow = internalWrite(text);
	
	// Apply color
	// The foreground/shadow copies are one pixel smaller than the canvas.
	for (uint32_t i = 0; i < foreground->getWidth() * foreground->getHeight(); i++) {
		const uint32_t y = i / foreground->getWidth();
		uint8_t red, green, blue;

		const PaletteColor& paletteColor = palette.colors[startColor + y];

		foreground->getPixel(i, &red, &green, &blue, nullptr);
		if (red == 255 && green == 255 && blue == 255) {
			foreground->setPixel(i, paletteColor.r, paletteColor.g, paletteColor.b, 255);
		}
		
		shadow->getPixel(i, &red, &green, &blue, nullptr);
		if (red == 255 && green == 255 && blue == 255) {
			shadow->setPixel(i, 0, 0, 0, 255);
		}
	}
	
	// Lay out both
	shadow->setColorKey(255, 0, 255);
	textureLabel->paste(*shadow, 0, 0, 1, 1);
	
	foreground->setColorKey(255, 0, 255);
	textureLabel->paste(*foreground, 0, 0, 0, 0);
	
	textureLabel->setColorKey(255, 0, 255);
	
	return std::move(textureLabel);
}