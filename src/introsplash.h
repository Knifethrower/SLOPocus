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

#ifndef _INTROSPLASH_H
#define _INTROSPLASH_H

#include "engine/state.h"
#include "engine/sound.h"
#include "engine/definitions.h"
#include "engine/fade.h"

// The title screen (1b97:0001 / 1b93:000c): the menu tune starts, the
// title picture fades in over 40 steps with the laugh, and after 4 s or a
// key it fades out in 30 steps into the main menu. The registered game
// draws its "Registered Version 1.1" band (file 3) at (0,188) over it.
class IntroSplash : public pocus::State {
public:
	enum { TIME = 4000 };

public:
	void onCreate(pocus::data::DataManager& dataManager) override;
	void onDetach() override;
	void onAttach() override;
	void release() override;
	void handleEvents(pocus::EventHandler &eventHandler) override;
	void render(pocus::Renderer &renderer) override;
	void update(float dt) override;

private:
	void leave();

	std::unique_ptr<pocus::Texture> backgroundImage;
	std::unique_ptr<pocus::Texture> bandImage;
	std::unique_ptr<pocus::Sound> backgroundMusic;
	std::unique_ptr<pocus::Sound> laughSound;
	pocus::Tick startTick { pocus::getNow() };
	pocus::Fade fade;
	bool leaving { false };
	bool skipPending { false };
};


#endif //_INTROSPLASH_H
