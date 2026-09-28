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

#include "slopocus.h"
#include "engine/provider/provider.h"
#include "stategame.h"
#include "engine/data/fatloader.h"
#include "version.h"
#include "apogeesplash.h"
#include "definitions.h"
#include "introsplash.h"
#include "mainmenu.h"
#include "settings.h"
#include "optionsmenu.h"
#include "version.h"


SLOPocus::SLOPocus(const pocus::RendererParameters &rendererParameters):
	PocusEngine(
			pocus::Provider::provideRenderer(rendererParameters),
			pocus::Provider::provideEventHandler(),
			pocus::Provider::provideAudio()
			)
{
}

void SLOPocus::createStates(pocus::StateManager& stateManager) {
	// HOCUS.SAV lives next to the detected HOCUS.DAT (1548:035a builds the
	// path the same way); missing file = the shipped defaults.
	GameSettings::get().load(pocus::GameVersion::get().savePath);
	OptionsMenu::applyAudioSettings();

	stateManager.addState(STATE_SPLASH_APOGEE, std::make_unique<ApogeeSplash>());
	stateManager.addState(STATE_SPLASH_INTRO, std::make_unique<IntroSplash>());
	auto& stateGame = static_cast<StateGame&>(stateManager.addState(STATE_GAME, std::make_unique<StateGame>()));
	stateGame.getGame().setRules(getRules());
	auto& mainMenu = static_cast<MainMenu&>(stateManager.addState(STATE_MENU_MAIN, std::make_unique<MainMenu>()));
	mainMenu.setStateGame(stateGame);

	// The original's flow: Apogee logo -> title -> main menu.
	stateManager.setStartupState(STATE_SPLASH_APOGEE);
}