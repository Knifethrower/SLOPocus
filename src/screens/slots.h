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

#ifndef SLOTS_H
#define SLOTS_H

#include "screen.h"
#include "../savefile.h"

namespace pocus::ui {

// The nine save slots (16b8:25cc "Select SAVE slot", 16b8:2b8b "Select
// RESTORE slot"): title in font 5 at y=59, rows from y=79 every 10 px with
// the slot number (font 4, x=35) and name (font 3, x=45), the animated
// selector at x=16, and the caption "Use UP/DOWN/NUMBER to move - ENTER to
// select". Up/Down wrap, a digit jumps to that slot, ESC cancels.
//
// RESTORE: ENTER on a used slot finishes with that slot index (-1 = cancel).
// SAVE: ENTER opens the name for typing (an empty slot starts blank); ENTER
// again stores the game in the slot and writes HOCUS.SAV, ESC while typing
// puts the old name back. Result 1 = saved, 2 = saved but the file could not
// be written (the caller shows "I/O Error - HOCUS.SAV"), 0 = cancelled.
class SlotScreen : public Screen {
public:
	enum Mode { SAVE, RESTORE };

	struct GameState {
		int episode;
		int level;
		int difficulty;   // the original's 706a: 0 easy, 1 moderate, 2 hard
		uint32_t score;
		GameState(): episode(0), level(0), difficulty(0), score(0) {}
		GameState(int episode, int level, int difficulty, uint32_t score): episode(episode), level(level), difficulty(difficulty), score(score) {}
	};

	SlotScreen(ScreenAssets& assets, SaveFile& save, Mode mode, GameState current = GameState(), std::function<bool()> store = nullptr);

	void handleEvents(EventHandler& eventHandler) override;
	void update(float dt) override;
	void render(Renderer& renderer) override;

private:
	void rebuildRow(int slot);
	void moveTo(int slot);
	void handleSelection(EventHandler& eventHandler);
	void handleTyping(EventHandler& eventHandler);

	ScreenAssets& assets;
	SaveFile& save;
	Mode mode;
	GameState current;
	std::function<bool()> store;

	std::unique_ptr<Texture> title, caption, cursor;
	std::unique_ptr<Texture> numbers[SaveFile::SLOTS];
	std::unique_ptr<Texture> names[SaveFile::SLOTS];
	Animation selector;
	int selected { 0 };
	bool typing { false };
	std::string name, oldName;
};

}

#endif // SLOTS_H
