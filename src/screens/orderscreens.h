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

#ifndef ORDERSCREENS_H
#define ORDERSCREENS_H

#include "screen.h"
#include "../engine/definitions.h"

namespace pocus::ui {

// 16b8:4398, the shareware's "Preview future levels": eleven full-screen
// pictures of the registered episodes, each with the 320x40 band (image 13)
// at y=160 and "Check out the entire 4 game series!" / "Order Hocus Pocus
// today!" outlined in font 7 at y=170/183. Any key turns the page, ESC
// leaves. The shareware also shows one page at random when quitting to DOS
// (1548:0aa5): 5 seconds or a key.
class OrderScreen : public Screen {
public:
	enum Mode { ALL_PAGES, RANDOM_PAGE };

	OrderScreen(ScreenAssets& assets, Mode mode);

	void handleEvents(EventHandler& eventHandler) override;
	void update(float dt) override;
	void render(Renderer& renderer) override;
	[[nodiscard]] bool wantsStars() const override { return false; }

private:
	void showPage();
	void nextPage();

	ScreenAssets& assets;
	Mode mode;
	int page { 0 };
	std::unique_ptr<Texture> picture, band, line1, line2;
	Tick shownAt;
	Fade pageFade;
	bool turning { false };
};

}

#endif // ORDERSCREENS_H
