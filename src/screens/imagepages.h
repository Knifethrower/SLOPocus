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

#ifndef IMAGEPAGES_H
#define IMAGEPAGES_H

#include "screen.h"
#include "../engine/definitions.h"

namespace pocus::ui {

// Full-screen PCX pictures shown one after another: the Instructions
// (16b8:44cc, any key turns the page) and the idle screens (16b8:42cb, each
// shown for 2499 timer ticks or until a key). Every picture brings its own
// palette, so the star field is not drawn.
class ImagePagesScreen : public Screen {
public:
	// timeoutMs 0 = wait for a key only. fadeOut = 15d8:0419 steps between pages.
	ImagePagesScreen(ScreenAssets& assets, std::vector<int> files, uint32_t timeoutMs = 0, int fadeOut = 20);

	void handleEvents(EventHandler& eventHandler) override;
	void update(float dt) override;
	void render(Renderer& renderer) override;
	[[nodiscard]] bool wantsStars() const override { return false; }
	[[nodiscard]] int fadeOutSteps() const override { return this->fadeOut; }

private:
	void showPage();
	void nextPage();

	ScreenAssets& assets;
	std::vector<int> files;
	uint32_t timeoutMs;
	int fadeOut;
	int page { 0 };
	std::unique_ptr<Texture> picture;
	Tick shownAt;
	Fade pageFade;
	bool turning { false };
};

}

#endif // IMAGEPAGES_H
