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

#ifndef _APOGEESPLASH_H
#define _APOGEESPLASH_H

#include <memory>
#include "engine/texture.h"
#include "engine/state.h"
#include "engine/definitions.h"
#include "engine/fade.h"
#include "engine/sound.h"

// The start of the original's intro (1b97:0001 registered, 1b93:000c
// shareware): the registered game first shows its "This game IS NOT
// shareware" notice (file 17) for up to 15 s, then both show the Apogee
// logo with its fanfare for up to 9 s. Any key cuts a picture short
// (2392:0108); the logo fades out in 30 steps and the title follows.
class ApogeeSplash : public pocus::State {
public:
	enum { NOTICE_TIME = 15000, LOGO_TIME = 9000 };

public:
	void onCreate(pocus::data::DataManager& dataManager) override;
	void onDetach() override;
	void onAttach() override;
	void release() override;
	void handleEvents(pocus::EventHandler &eventHandler) override;
	void render(pocus::Renderer &renderer) override;
	void update(float dt) override;

private:
	enum Phase { NOTICE, LOGO };

	void showLogo();
	void leave();

	std::unique_ptr<pocus::Texture> noticeImage;
	std::unique_ptr<pocus::Texture> logoImage;
	std::unique_ptr<pocus::Sound> backgroundMusic;
	pocus::Tick startTick { pocus::getNow() };
	pocus::Fade fade;
	Phase phase { LOGO };
	bool leaving { false };
	bool skipPending { false };
};

#endif //_APOGEESPLASH_H
