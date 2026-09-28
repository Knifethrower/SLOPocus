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

#include "imagepages.h"
#include "../engine/data/asset/pcx.h"
#include "../engine/log.h"

using namespace pocus;
using namespace pocus::ui;

ImagePagesScreen::ImagePagesScreen(ScreenAssets& assets, std::vector<int> files, uint32_t timeoutMs, int fadeOut):
	assets(assets),
	files(std::move(files)),
	timeoutMs(timeoutMs),
	fadeOut(fadeOut)
{
	showPage();
}

void ImagePagesScreen::showPage() {
	this->picture = nullptr;
	if (this->page >= (int)this->files.size()) {
		finish(0);
		return;
	}
	data::asset::Pcx pcx;
	data::DataFile& file = this->assets.dataManager->getData().fetchFile((uint32_t)this->files[this->page]);
	pcx.loadFromStream(file.getContent(), file.getLength());
	this->picture = pcx.createTexture();
	if (!this->picture) {
		LOGE << "ImagePagesScreen: file " << this->files[this->page] << " is not a PCX";
		finish(-1);
		return;
	}
	this->shownAt = getNow();
}

void ImagePagesScreen::nextPage() {
	if (this->turning) {
		return;
	}
	if (this->page + 1 >= (int)this->files.size()) {
		finish(0);
		return;
	}
	this->turning = true;
	this->pageFade.setSpeed(this->fadeOut > 0 ? 300.0f / (float)this->fadeOut : 1000.0f);
	this->pageFade.start(Fade::FADE_OUT, [this] {
		this->page++;
		showPage();
		this->pageFade.setSpeed(15.0f);
		this->pageFade.start(Fade::FADE_IN, [this] { this->turning = false; });
	});
}

void ImagePagesScreen::handleEvents(EventHandler& eventHandler) {
	// 16b8:0f3d / 42cb: any key (including ESC) moves on.
	if (eventHandler.getKeyDown() != KEY_NONE) {
		nextPage();
	}
}

void ImagePagesScreen::update(float dt) {
	this->pageFade.update(dt);
	if (!this->turning && this->timeoutMs != 0 && getElapsedTime(this->shownAt) >= this->timeoutMs) {
		nextPage();
	}
}

void ImagePagesScreen::render(Renderer& renderer) {
	if (this->picture) {
		renderer.drawTexture(*this->picture, Point(0, 0));
	}
	this->pageFade.render(renderer);
}
