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

#ifndef STATEGAME_H
#define STATEGAME_H

#include <memory>
#include "engine/game.h"
#include "engine/texture.h"
#include "engine/state.h"
#include "engine/map.h"
#include "engine/sound.h"
#include "engine/data/asset/palette.h"
#include "engine/data/asset/spriteset.h"
#include "engine/menu.h"
#include "engine/particles.h"
#include "screens/screen.h"

class StateGame : public pocus::State {
public:
	void onCreate(pocus::data::DataManager& dataManager) override;
	void onDetach() override;
	void onAttach() override;
	void release() override;
	void handleEvents(pocus::EventHandler &eventHandler) override;
	void render(pocus::Renderer &renderer) override;
	void update(float dt) override;

	pocus::Game& getGame();

	// Jumps straight to a specific episode/stage, bypassing the normal
	// sequential loadNextLevel() progression - used by the test menu's level
	// select. Safe to call any time after onCreate() (sprites/HUD/items/enemy
	// palette are level-independent and already loaded by then).
	void startLevel(uint8_t episode, uint8_t stage);

	// "Begin a new game" (main menu handler 1548:05c2): score 0, fire power
	// 1, the chosen episode's first level at the chosen skill.
	void startNewGame(uint8_t episode, pocus::Difficulty_t difficulty);
	// Pushes GameSettings (game speed etc.) into the running game.
	void applySettings();
	// "Restore an old game" (1548:0736 / 16b8:2b8b): episode and level
	// indices, the original's difficulty index (0..2) and the score the
	// saved level started with.
	void restoreGame(int episodeIndex, int levelIndex, int difficultyIndex, uint32_t score);

private:
	pocus::Game game;
	// The original's in-game menu (menu 5, on ESC): How to play / Abandon
	// level & restart / Save / Restore / Change game options / Back to the
	// action! / Quit to Main Menu.
	pocus::Menu inGameMenu;
	bool inGameMenuOpen { false };
	std::unique_ptr<pocus::Texture> menuTopTexture, menuBottomTexture;
	// Score at the start of the current level: dying or abandoning restarts
	// the level with it (1548:06f1).
	uint32_t scoreAtLevelStart { 0 };
	void buildInGameMenu();
	void openInGameMenu();
	void closeInGameMenu();
	void restartLevel();
	void quitToMainMenu();
	enum InGameMenu_t { IN_GAME_MAIN, IN_GAME_OPTIONS, IN_GAME_SPEED };
	InGameMenu_t inGameMenuKind { IN_GAME_MAIN };
	int8_t inGameSelection[3] { 0, 0, 0 };
	void rememberInGameSelection(InGameMenu_t next);
	// The in-level hotkeys the keyboard handler (2392:026e) maps and
	// level_run acts on: F1 help, F2 save, F3 restore, F10 quit, M music,
	// S sound, V volume, P pause; shared with the ESC menu items.
	void handleHotkeys(pocus::EventHandler& eventHandler);
	void openInstructions(std::function<void()> then);
	void openSaveScreen(std::function<void()> then);
	void openRestoreScreen(std::function<void()> then);
	void confirmQuit(std::function<void()> onNo);
	void openVolumeScreen(std::function<void()> then);
	void toggleMusic();
	void toggleSound();
	// The original's cheat codes: the keyboard handler sums every scancode
	// (make and break) since the last F, Q or B press and acts on totals.
	void handleCheats(pocus::EventHandler& eventHandler);
	int cheatSum { 0 };

	// Text over the frozen level: "Game Paused" (1ba5:3751) and the
	// "... is now on/off" notices (1ba5:37fc); the next key press clears it.
	void showOverlay(const std::string& text, int x, int y);
	void clearOverlay();
	std::unique_ptr<pocus::Texture> overlayText, overlayShadow;
	int overlayX { 0 }, overlayY { 0 };
	bool overlayActive { false };
	bool overlayPausedGame { false };

	void buildInGameOptions();
	void buildInGameSpeed();
	// 1548:06b8 / 0709: the score goes to the episode's high-score table
	// (entry screen when it qualifies), then `then` runs.
	void checkHighScoreThen(std::function<void()> then);
	// 16b8:4889 after a level (completed) or a death, then the next level /
	// the episode ending / the restart.
	void showTally(bool completed);
	// 1548:067c: the episode-complete picture, the ending pages, the
	// high-score check, the main menu.
	void finishEpisode();
	void playMenuMusic();
	// The level tune (level_run starts it with the level).
	void startLevelMusic();
	bool attached { false };
	// Set from the level-end tally until the next level starts: the level
	// underneath must not tick (or re-report its end) while the screens run.
	bool levelFlowActive { false };
	// The original's 706a (0 easy, 1 moderate, 2 hard) for the port's Difficulty_t.
	[[nodiscard]] int difficultyIndex();

	// The out-of-game screens reachable from the ESC menu (instructions,
	// save/restore, options screens, quit confirmation, high-score entry),
	// drawn over the star field like the original's.
	pocus::ui::ScreenAssets screenAssets;
	pocus::ui::ScreenHost screens;
	pocus::Particles menuStars;

	void loadSprites(pocus::data::Data& data);
	void loadLevel(pocus::data::Data& data, pocus::data::Data& executable, uint8_t episode, uint8_t stage);
	void createHud(pocus::data::Data& data);
	void createGame(pocus::data::Data& data);
	void loadItems(pocus::data::Data& executable);
	void loadEnemies(pocus::data::Data& data);
	bool buildEnemy(uint16_t infoIndex, pocus::Enemy& enemy);
	void loadNextLevel();

	std::unique_ptr<pocus::Texture> loadTexture(pocus::data::Data& data, uint32_t paletteFileIndex, uint32_t imageFileIndex);
	std::unique_ptr<pocus::Texture> loadTexture(pocus::data::Data& data, const pocus::data::asset::Palette& palette, uint32_t imageFileIndex);

	std::unique_ptr<pocus::Sound> backgroundMusic;

	// Kept alive for the whole level (not just the loadEnemies() call) - enemy
	// groups spawn on demand via their map trigger tile, not all at once at load.
	pocus::data::asset::Palette enemyPalette;
	pocus::data::asset::SpriteSet enemySpriteSet;

	// Set once in onCreate() and kept for loadNextLevel(), which needs to fetch
	// the same DAT/EXE data again well after onCreate() has returned.
	pocus::data::DataManager* dataManager { nullptr };
	uint8_t currentEpisode { 1 };
	uint8_t currentStage { 1 };
};

#endif //STATEGAME_H
