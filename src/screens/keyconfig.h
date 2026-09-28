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

#ifndef KEYCONFIG_H
#define KEYCONFIG_H

#include "screen.h"
#include "../savefile.h"

namespace pocus::ui {

// 16b8:17da "Define key controls": the eight actions (A..H) with their keys;
// a letter opens 16b8:10ac, which lists the 18 bindable keys in three
// columns (A..R) and takes one letter. ESC leaves (result 1: the bindings
// may have changed).
class KeyConfigScreen : public Screen {
public:
	KeyConfigScreen(ScreenAssets& assets, SaveFile& save);

	void handleEvents(EventHandler& eventHandler) override;
	void update(float dt) override;
	void render(Renderer& renderer) override;

	static std::string keyName(int index);
	static std::string actionLabel(int index);

private:
	struct Label {
		std::unique_ptr<Texture> texture;
		int x, y;
	};

	void buildList();
	void buildRemap(int action);
	void switchTo(std::function<void()> build);

	ScreenAssets& assets;
	SaveFile& save;
	std::vector<Label> labels;
	std::unique_ptr<Texture> caption;
	int remapAction { -1 };
	Fade pageFade;
	bool turning { false };
};

}

#endif // KEYCONFIG_H
