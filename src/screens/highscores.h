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

#ifndef HIGHSCORES_H
#define HIGHSCORES_H

#include "screen.h"
#include "../savefile.h"

namespace pocus::ui {

// The high-score table of one episode as 16b8:2018 / 1c9b lay it out:
// "High Scores For Game N" in font 5 at y=72, the five entries from y=92
// (rank 1 in font 4, 20 px tall; the rest font 3, 10 px) with the name at
// x=27 and the score right-aligned to x=290, the caption at y=188 between
// the frame images.
class HighScoreTable {
public:
	void build(ScreenAssets& assets, const SaveFile& save, int episode, const std::string& caption);
	void render(Renderer& renderer, ScreenAssets& assets);
	// Screen row and font of a rank, for the entry cursor.
	[[nodiscard]] int rowY(int rank) const { return rank == 0 ? 92 : 92 + 20 + (rank - 1) * 10; }
	[[nodiscard]] static int rowFont(int rank) { return rank == 0 ? 4 : 3; }

private:
	std::unique_ptr<Texture> title, caption;
	std::unique_ptr<Texture> names[SaveFile::SCORES];
	std::unique_ptr<Texture> scores[SaveFile::SCORES];
};

// 16b8:2018: the tables of every episode in turn; any key turns to the
// next episode, ESC leaves.
class HighScoreScreen : public Screen {
public:
	HighScoreScreen(ScreenAssets& assets, const SaveFile& save, int episodes = 4);

	void handleEvents(EventHandler& eventHandler) override;
	void update(float dt) override;
	void render(Renderer& renderer) override;

private:
	ScreenAssets& assets;
	const SaveFile& save;
	int episodes;
	int episode { 0 };
	HighScoreTable table;
	Fade pageFade;
	bool turning { false };
};

// 16b8:1c9b: the player types a name into the new entry (already inserted
// in the table at `rank`), ENTER stores it. Result 1 = name entered.
class HighScoreEntryScreen : public Screen {
public:
	HighScoreEntryScreen(ScreenAssets& assets, SaveFile& save, int episode, int rank);

	void handleEvents(EventHandler& eventHandler) override;
	void render(Renderer& renderer) override;

private:
	void rebuild();

	ScreenAssets& assets;
	SaveFile& save;
	int episode, rank;
	HighScoreTable table;
	std::unique_ptr<Texture> cursor;
	std::string name;
};

}

#endif // HIGHSCORES_H
