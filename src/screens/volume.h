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

#ifndef VOLUME_H
#define VOLUME_H

#include "screen.h"
#include "../savefile.h"
#include "../engine/sound.h"
#include "../engine/definitions.h"

namespace pocus::ui {

// 16b8:137d "Volume control": two 16-cell sliders ("Sound FX Volume" at
// y=62/82, "Music Volume" at y=122/142, cells 16 px apart from x=32), the
// lit cell of the active row blinking; Left/Right move it, Up/Down pick the
// row, the level is applied at once (cell*16+15) and a random effect plays
// every 51 frames so it can be heard. ENTER or ESC stores the volumes in
// HOCUS.SAV. Result 1 = stored, 2 = the file could not be written.
class VolumeScreen : public Screen {
public:
	VolumeScreen(ScreenAssets& assets, SaveFile& save, std::function<bool()> store);

	void handleEvents(EventHandler& eventHandler) override;
	void update(float dt) override;
	void render(Renderer& renderer) override;

private:
	void applyVolumes();

	ScreenAssets& assets;
	SaveFile& save;
	std::function<bool()> store;
	std::unique_ptr<Texture> titles[2];
	std::unique_ptr<Texture> caption;
	int level[2] { 0, 0 };
	int row { 0 };
	Tick blinkStart;
	Tick lastSound;
	std::vector<std::unique_ptr<Sound>> sounds;
};

}

#endif // VOLUME_H
