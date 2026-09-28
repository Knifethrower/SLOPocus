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
#include "orderscreens.h"
#include "../version.h"
#include "../exedata.h"
#include "../engine/data/asset/pcx.h"
#include "../engine/provider/provider.h"
#include "../engine/log.h"

using namespace pocus;
using namespace pocus::ui;

namespace {
constexpr int PAGES = 11;
constexpr int BAND_Y = 160;      // 0xa0
constexpr int LINE1_Y = 170;     // 0xaa
constexpr int LINE2_Y = 183;     // 0xb7
constexpr uint32_t RANDOM_PAGE_MS = 5000;
constexpr uint8_t FONT7_BASE = 112;   // DS:1ae6[6]
constexpr uint8_t OUTLINE_COLOUR = 1; // 16b8:04bb's colour argument

// 16b8:09f8: the text in font 7 over four copies one pixel out in the
// outline colour, all in the picture's own palette.
std::unique_ptr<Texture> outlinedText(data::asset::Font& font, const data::asset::Palette& palette, const std::string& text) {
	auto fill = font.writeGradient(text, palette, FONT7_BASE);
	auto outline = font.write(text, palette, OUTLINE_COLOUR);
	const uint32_t w = fill->getWidth() + 2;
	const uint32_t h = fill->getHeight() + 2;
	auto layout = Provider::provideTexture(w, h);
	layout->fill(255, 0, 255, 255);
	layout->paste(*outline, 0, 0, 2, 1);
	layout->paste(*outline, 0, 0, 0, 1);
	layout->paste(*outline, 0, 0, 1, 2);
	layout->paste(*outline, 0, 0, 1, 0);
	layout->paste(*fill, 0, 0, 1, 1);
	layout->setColorKey(255, 0, 255);
	return layout;
}
}

OrderScreen::OrderScreen(ScreenAssets& assets, Mode mode):
	assets(assets),
	mode(mode)
{
	if (mode == RANDOM_PAGE) {
		this->page = rand() % PAGES;
	}
	showPage();
}

void OrderScreen::showPage() {
	this->picture = nullptr;
	data::Data& data = this->assets.dataManager->getData();
	data::DataFile& file = data.fetchFile((uint32_t)(datFiles().orderScreensStart + this->page));
	data::asset::Pcx pcx;
	pcx.loadFromStream(file.getContent(), file.getLength());
	this->picture = pcx.createTexture();
	if (!this->picture) {
		LOGE << "OrderScreen: page " << this->page << " is not a PCX";
		finish(0);
		return;
	}
	// The band and the text use the picture's palette (15d8:0673 loads it).
	const Palette256 palette256 = ScreenAssets::pcxPalette(file.getContent(), file.getLength());
	data::asset::Palette palette;
	for (int i = 0; i < 128; i++) {
		palette.colors[i] = palette256[i];
	}
	this->band = this->assets.planarImage(datFiles().imageOrderBand, palette256);
	this->line1 = outlinedText(this->assets.font, palette, ExeData::get().string(STR_ORDER_LINE_1));
	this->line2 = outlinedText(this->assets.font, palette, ExeData::get().string(STR_ORDER_LINE_2));
	this->shownAt = getNow();
}

void OrderScreen::nextPage() {
	if (this->turning) {
		return;
	}
	if (this->mode == RANDOM_PAGE || this->page + 1 >= PAGES) {
		finish(0);
		return;
	}
	this->turning = true;
	this->pageFade.setSpeed(fadeSpeedForSteps(20));
	this->pageFade.start(Fade::FADE_OUT, [this] {
		this->page++;
		showPage();
		this->pageFade.start(Fade::FADE_IN, [this] { this->turning = false; });
	});
}

void OrderScreen::handleEvents(EventHandler& eventHandler) {
	const Key_t key = eventHandler.getKeyDown();
	if (key == KEY_NONE || this->turning) {
		return;
	}
	if (key == KEY_ESCAPE && this->mode == ALL_PAGES) {
		finish(0);
		return;
	}
	nextPage();
}

void OrderScreen::update(float dt) {
	this->pageFade.update(dt);
	if (this->mode == RANDOM_PAGE && !this->turning && getElapsedTime(this->shownAt) >= RANDOM_PAGE_MS) {
		finish(0);
	}
}

void OrderScreen::render(Renderer& renderer) {
	if (this->picture) {
		renderer.drawTexture(*this->picture, Point(0, 0));
	}
	if (this->band) {
		renderer.drawTexture(*this->band, Point(0, BAND_Y));
	}
	if (this->line1) {
		// The outline texture is one pixel larger on each side.
		drawCentred(renderer, *this->line1, LINE1_Y - 1);
	}
	if (this->line2) {
		drawCentred(renderer, *this->line2, LINE2_Y - 1);
	}
	this->pageFade.render(renderer);
}
