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

#ifndef MAINMENU_H
#define MAINMENU_H

#include "engine/state.h"
#include "engine/menu.h"
#include "engine/particles.h"
#include "engine/animation.h"
#include "engine/fade.h"
#include "engine/sound.h"
#include "engine/data/asset/font.h"
#include "engine/data/asset/palette.h"
#include "screens/screen.h"
#include "stategame.h"

// The original's out-of-game menus (HOCUS.EXE main loop 1548:035a and the
// menu tables at DS:1a06.. - see the reverse-engineering notes, "Menus"): the main menu
// and its sub-menus (episode, skill, options, game speed), drawn over the
// star field between the top and bottom images, and the screens the items
// open (Restore, Ordering, Instructions, Legends, High scores, Volume, Keys,
// the shareware's Preview future levels). The shareware main menu (its
// table at DS:0dcc) has nine items and shows the preview/order screens.
class MainMenu : public pocus::State {
public:
	enum Screen_t { MAIN, EPISODE, SKILL, OPTIONS, SPEED };

public:
	void onCreate(pocus::data::DataManager& dataManager) override;
	void onDetach() override;
	void onAttach() override;
	void release() override;
	void handleEvents(pocus::EventHandler &eventHandler) override;
	void render(pocus::Renderer &renderer) override;
	void update(float dt) override;

	void setStateGame(StateGame& stateGame);

private:
	void showScreen(Screen_t screen);
	void buildMain();
	void buildEpisode();
	void buildSkill();
	void buildOptions();
	void buildSpeed();
	void startMenuMusic();
	void quit();

	StateGame* stateGame { nullptr };
	pocus::Menu menu;
	pocus::Particles particles;
	std::unique_ptr<pocus::Texture> bottomTexture;
	std::unique_ptr<pocus::Texture> topTexture;
	std::unique_ptr<pocus::Sound> menuMusic;
	pocus::ui::ScreenAssets assets;
	pocus::ui::ScreenHost screens;
	pocus::Fade fade;
	Screen_t screen { MAIN };
	// Each menu's cursor position, kept between visits like the original.
	int8_t lastSelection[5] { 0, 0, 0, 0, 0 };
	uint8_t chosenEpisode { 1 };
	bool leaving { false };
	// 16b8:2259: 4500 timer ticks without a key show the idle pictures.
	pocus::Tick lastInput { pocus::getNow() };
};

#endif // MAINMENU_H
