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

#include "episodecomplete.h"
#include "../version.h"
#include "../engine/data/asset/pcx.h"
#include "../engine/data/asset/voc.h"
#include "../engine/rect.h"

using namespace pocus;
using namespace pocus::ui;

namespace {
constexpr uint32_t WHITEOUT_MS = 70 * 1000 / 70;   // 70 vsyncs
}

EpisodeCompleteScreen::EpisodeCompleteScreen(ScreenAssets& assets) {
	data::Data& data = assets.dataManager->getData();
	const DatFiles& files = datFiles();

	data::asset::Pcx pcx;
	data::DataFile& pcxFile = data.fetchFile(files.episodeCompletePcx);
	pcx.loadFromStream(pcxFile.getContent(), pcxFile.getLength());
	this->picture = pcx.createTexture();

	// Sound 0x267 = 615 in the registered numbering (vocLaugh + 4).
	data::asset::Voc voc;
	data::DataFile& vocFile = data.fetchFile((uint32_t)(files.vocLaugh + 4));
	voc.loadFromStream(vocFile.getContent(), vocFile.getLength());
	this->sound = voc.createAsSound();
	if (this->sound) {
		this->sound->play();
	}
}

void EpisodeCompleteScreen::onShown() {
	this->shown = true;
	this->shownAt = getNow();
}

void EpisodeCompleteScreen::update(float dt) {
	if (this->shown && getElapsedTime(this->shownAt) >= WHITEOUT_MS) {
		finish(0);
	}
}

void EpisodeCompleteScreen::render(Renderer& renderer) {
	if (this->picture) {
		renderer.drawTexture(*this->picture, Point(0, 0));
	}
	if (this->shown) {
		// 15d8:04b0: every component climbs to 63 one step per vsync.
		const uint32_t elapsed = getElapsedTime(this->shownAt);
		const uint32_t alpha = elapsed >= WHITEOUT_MS ? 255 : elapsed * 255 / WHITEOUT_MS;
		renderer.drawRect(Rect(Point(0, 0), Size(-1, -1)), Color { 255, 255, 255, (uint8_t)alpha });
	}
}
