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
#include "volume.h"
#include "../version.h"
#include "../exedata.h"
#include "../engine/audiocontrol.h"
#include "../engine/data/asset/voc.h"

using namespace pocus;
using namespace pocus::ui;

namespace {
constexpr int TITLE_Y[2] = { 62, 122 };   // 0x3e, 0x7a
constexpr int CELLS_Y[2] = { 82, 142 };   // 0x52, 0x8e
constexpr int CELL_X0 = 32;               // byte offset 8 -> pixel 32
constexpr int CELL_STEP = 16;
constexpr uint32_t BLINK_ON_MS = 143;     // 10 of 21 vsyncs lit
constexpr uint32_t BLINK_PERIOD_MS = 300; // 21 vsyncs
constexpr uint32_t SOUND_PERIOD_MS = 729; // every 51 loop iterations

}

VolumeScreen::VolumeScreen(ScreenAssets& assets, SaveFile& save, std::function<bool()> store):
	assets(assets),
	save(save),
	store(std::move(store))
{
	const ExeData& exe = ExeData::get();
	this->titles[0] = assets.text(exe.string(STR_SOUND_FX_VOLUME), 5);
	this->titles[1] = assets.text(exe.string(STR_MUSIC_VOLUME), 5);
	this->caption = assets.shadowText(exe.string(STR_VOLUME_HELP), 4);
	this->level[0] = std::min(15, std::max(0, save.soundVolume / 16));
	this->level[1] = std::min(15, std::max(0, save.musicVolume / 16));
	this->blinkStart = getNow();
	this->lastSound = getNow();

	// The 16 effects the DOS game keeps loaded, from the EXE's sound table.
	data::Data& data = assets.dataManager->getData();
	for (int sound = 0; sound < 16; sound++) {
		data::asset::Voc voc;
		data::DataFile& vocFile = data.fetchFile((uint32_t)exe.soundFile(sound));
		voc.loadFromStream(vocFile.getContent(), vocFile.getLength());
		this->sounds.push_back(voc.createAsSound());
	}
}

void VolumeScreen::applyVolumes() {
	this->save.soundVolume = (int16_t)(this->level[0] * 16 + 15);
	this->save.musicVolume = (int16_t)(this->level[1] * 16 + 15);
	AudioControl::setSoundVolume(this->save.soundVolume);
	AudioControl::setMusicVolume(this->save.musicVolume);
}

void VolumeScreen::handleEvents(EventHandler& eventHandler) {
	switch (eventHandler.getKeyDown()) {
		case KEY_RIGHT:
			if (this->level[this->row] < 15) {
				this->level[this->row]++;
			}
			applyVolumes();
			break;
		case KEY_LEFT:
			if (this->level[this->row] != 0) {
				this->level[this->row]--;
			}
			applyVolumes();
			break;
		case KEY_UP:
			this->row = 0;
			applyVolumes();
			break;
		case KEY_DOWN:
			this->row = 1;
			applyVolumes();
			break;
		case KEY_RETURN:
		case KEY_ESCAPE:
			applyVolumes();
			finish(this->store && this->store() ? 1 : 2);
			break;
		default:
			break;
	}
}

void VolumeScreen::update(float dt) {
	if (getElapsedTime(this->lastSound) >= SOUND_PERIOD_MS) {
		this->lastSound = getNow();
		if (!this->sounds.empty()) {
			auto& sound = this->sounds[(size_t)(rand() % 16) % this->sounds.size()];
			if (sound) {
				sound->play();
			}
		}
	}
}

void VolumeScreen::render(Renderer& renderer) {
	const bool lit = (getElapsedTime(this->blinkStart) % BLINK_PERIOD_MS) < BLINK_ON_MS;
	for (int r = 0; r < 2; r++) {
		drawCentred(renderer, *this->titles[r], TITLE_Y[r]);
		for (int i = 0; i < 16; i++) {
			const bool on = i == this->level[r] && (r != this->row || lit);
			renderer.drawTexture(*this->assets.volumeCell[on ? 1 : 0], Point((float)(CELL_X0 + i * CELL_STEP), (float)CELLS_Y[r]));
		}
	}
	renderer.drawTexture(*this->assets.bottom, Point(0, ScreenAssets::FRAME_BOTTOM_Y));
	drawCentred(renderer, *this->caption, ScreenAssets::FRAME_CAPTION_Y);
	renderer.drawTexture(*this->assets.top, Point(0, 0));
}
