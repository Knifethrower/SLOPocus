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

#ifndef EPISODECOMPLETE_H
#define EPISODECOMPLETE_H

#include "screen.h"
#include "../engine/sound.h"
#include "../engine/definitions.h"

namespace pocus::ui {

// 16b8:4d76, after the boss level: the episode-complete picture (PCX 4)
// fades in over 60 steps with sound 615, then the palette brightens one
// step per vsync for 70 vsyncs (15d8:04b0, a white-out), then fades out.
class EpisodeCompleteScreen : public Screen {
public:
	explicit EpisodeCompleteScreen(ScreenAssets& assets);

	void handleEvents(EventHandler& eventHandler) override {}
	void update(float dt) override;
	void render(Renderer& renderer) override;
	void onShown() override;
	[[nodiscard]] bool wantsStars() const override { return false; }
	[[nodiscard]] int fadeInSteps() const override { return 60; }

private:
	std::unique_ptr<Texture> picture;
	std::unique_ptr<Sound> sound;
	bool shown { false };
	Tick shownAt;
};

}

#endif // EPISODECOMPLETE_H
