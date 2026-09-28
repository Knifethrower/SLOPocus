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

#include <cstdlib>
#include "screen.h"
#include "../version.h"
#include "../engine/data/asset/image.h"
#include "../engine/provider/provider.h"
#include "../engine/log.h"

using namespace pocus;
using namespace pocus::ui;

bool pocus::ui::padPrompts() {
	static const bool enabled = []() {
		const char* value = std::getenv("SLOPOCUS_PAD_PROMPTS");
		return value && value[0] != '\0' && value[0] != '0';
	}();
	return enabled;
}

namespace {

// Menu-half palette index of DOS font N's first gradient colour (DS:1ae6
// table: 192, 200, 208, 216, 224, 232 for fonts 1..6, minus 128).
uint8_t fontBase(int font) {
	if (font < 1) {
		font = 1;
	}
	if (font > 6) {
		font = 6;
	}
	return (uint8_t)(64 + (font - 1) * 8);
}

}

// The engine's dt is in 16 ms units (PocusEngine::loop, fixedFpsDelay); a
// palette fade of n steps takes n vsyncs at 70 Hz, and the overlay runs 0..255.
float pocus::ui::fadeSpeedForSteps(int steps) {
	if (steps <= 0) {
		return 1000.0f;
	}
	return 255.0f * 70.0f / (62.5f * (float)steps);
}

void ScreenAssets::load(data::DataManager& dataManager) {
	this->dataManager = &dataManager;
	data::Data& data = dataManager.getData();
	const DatFiles& files = datFiles();

	data::DataFile& fontFile = data.fetchFile(files.fontMain);
	data::DataFile& paletteFile = data.fetchFile(files.paletteMenu);
	data::DataFile& gamePaletteFile = data.fetchFile(files.paletteGame);
	this->font.loadFromStream(fontFile.getContent(), fontFile.getLength());
	this->palette.loadFromStream(paletteFile.getContent(), paletteFile.getLength());
	data::asset::Palette gamePalette;
	gamePalette.loadFromStream(gamePaletteFile.getContent(), gamePaletteFile.getLength());
	for (int i = 0; i < 128; i++) {
		this->menuPalette256[i] = gamePalette.colors[i];
		this->menuPalette256[128 + i] = this->palette.colors[i];
	}

	const auto image = [&](int index) {
		data::asset::Image img;
		data::DataFile& file = data.fetchFile(index);
		img.loadFromStream(file.getContent(), file.getLength());
		return img.createTexture(this->palette, 128);
	};
	this->bottom = image(files.imageBottom);
	this->top = image(files.imageTop);
	this->selector = image(files.imageMenuSelection);
	// 16b8:1316 cuts the slider cells from the frame buffer where main()
	// blitted image 15 (two 16x13 cells side by side).
	auto cells = image(files.imageVolumeCells);
	this->volumeCell[0] = cells->extract(0, 0, 16, 13);
	this->volumeCell[1] = cells->extract(16, 0, 16, 13);
}

std::unique_ptr<Texture> ScreenAssets::text(const std::string& text, int font) {
	return this->font.writeGradient(text, this->palette, fontBase(font));
}

std::unique_ptr<Texture> ScreenAssets::shadowText(const std::string& text, int font) {
	return this->font.writeGradientShadow(text, this->palette, fontBase(font));
}

int ScreenAssets::textWidth(const std::string& text) {
	return (int)this->font.calculateWidth(text);
}

std::unique_ptr<Texture> ScreenAssets::planarImage(int fileIndex, const Palette256& palette256) {
	data::DataFile& file = this->dataManager->getData().fetchFile((uint32_t)fileIndex);
	auto texture = planarImageFrom(file.getContent(), file.getLength(), palette256);
	if (!texture) {
		LOGE << "ScreenAssets: file " << fileIndex << " is not a planar image";
	}
	return texture;
}

std::unique_ptr<Texture> ScreenAssets::planarImageFrom(const char* data, uint32_t length, const Palette256& palette256) {
	const auto* p = (const uint8_t*)data;
	if (length < 4) {
		return nullptr;
	}
	const int width = (p[0] | (p[1] << 8)) * 4;
	const int height = p[2] | (p[3] << 8);
	if (width <= 0 || height <= 0 || (uint32_t)(width * height + 4) != length) {
		return nullptr;
	}
	auto texture = Provider::provideTexture((uint32_t)width, (uint32_t)height);
	p += 4;
	// Four planes, each holding every fourth pixel (mode X layout).
	for (int plane = 0; plane < 4; plane++) {
		for (int y = 0; y < height; y++) {
			for (int x = plane; x < width; x += 4) {
				// 15d8:0a70 writes every pixel, index 0 included: these images
				// replace what is under them (the title band covers the bottom of
				// the title picture, the order band the bottom of its picture).
				const uint8_t index = *(p++);
				const Rgb& c = palette256[index];
				texture->setPixel((uint32_t)(y * width + x), c.r, c.g, c.b, 255);
			}
		}
	}
	return texture;
}

std::unique_ptr<Texture> ScreenAssets::pageImage(int fileIndex) {
	return planarImage(fileIndex, this->menuPalette256);
}

Palette256 ScreenAssets::pcxPalette(const char* data, uint32_t length) {
	Palette256 palette {};
	if (length < 769) {
		return palette;
	}
	const auto* p = (const uint8_t*)data + length - 768;
	for (int i = 0; i < 256; i++) {
		palette[i] = Rgb { p[i * 3], p[i * 3 + 1], p[i * 3 + 2] };
	}
	return palette;
}

void ScreenHost::push(std::unique_ptr<Screen> newScreen, Completion completion) {
	this->screen = std::move(newScreen);
	this->onDone = std::move(completion);
	this->leaving = false;
	this->fade.setSpeed(fadeSpeedForSteps(this->screen->fadeInSteps()));
	this->fade.start(Fade::FADE_IN, [this] {
		if (this->screen) {
			this->screen->onShown();
		}
	});
}

void ScreenHost::handleEvents(EventHandler& eventHandler) {
	// The original drains the keyboard while fading; input only counts once
	// the screen is fully up.
	if (!this->screen || this->fade.isRunning() || this->leaving) {
		return;
	}
	this->screen->handleEvents(eventHandler);
}

void ScreenHost::update(float dt) {
	this->fade.update(dt);
	if (!this->screen) {
		return;
	}
	if (!this->leaving) {
		this->screen->update(dt);
		if (this->screen->isDone()) {
			this->leaving = true;
			// 15d8:0419: n steps of one vsync each.
			this->fade.setSpeed(fadeSpeedForSteps(this->screen->fadeOutSteps()));
			this->fade.start(Fade::FADE_OUT, [this] {
				const int result = this->screen->getResult();
				Completion completion = std::move(this->onDone);
				this->screen = nullptr;
				this->onDone = nullptr;
				this->leaving = false;
				if (completion) {
					completion(result);
				}
			});
		}
	}
}

void ScreenHost::render(Renderer& renderer, Particles* stars) {
	if (!this->screen) {
		return;
	}
	if (stars && this->screen->wantsStars()) {
		stars->render(renderer);
	}
	this->screen->render(renderer);
	this->fade.render(renderer);
}
