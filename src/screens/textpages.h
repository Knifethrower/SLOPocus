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

#ifndef TEXTPAGES_H
#define TEXTPAGES_H

#include "screen.h"

namespace pocus::ui {

// A run of the original's text-page files (Ordering Information, Legends
// and hints, the episode endings): each DAT file is one page of up to 20
// 80-character lines with a per-line x (0 = centred) and font, drawn by
// 16b8:3230 (FRAMED: top and bottom frame images, text centred in the
// 144 px band below the top image) or 16b8:3401/364d (PICTURE: bottom
// image, an optional per-page picture from the DS:1912-style table, text
// centred in the 184 px above the bottom image). Page keys as 16b8:3f7d.
class TextPageScreen : public Screen {
public:
	enum Style { FRAMED, PICTURE };

	struct PageImage {
		int imageOffset;   // offset from DatFiles::pageImageBase, -1 = none
		int x, y;
	};

	TextPageScreen(ScreenAssets& assets, std::vector<int> files, Style style, std::vector<PageImage> images = {});

	void handleEvents(EventHandler& eventHandler) override;
	void update(float dt) override;
	void render(Renderer& renderer) override;

private:
	struct Line {
		std::unique_ptr<Texture> texture;
		int x, y;
	};

	void buildPage();

	ScreenAssets& assets;
	std::vector<int> files;
	Style style;
	std::vector<PageImage> images;
	int page { 0 };
	int nextPage { 0 };
	std::vector<Line> lines;
	std::unique_ptr<Texture> caption;
	std::unique_ptr<Texture> picture;
	int pictureX { 0 }, pictureY { 0 };
	Fade pageFade;
	bool turning { false };
};

}

#endif // TEXTPAGES_H
