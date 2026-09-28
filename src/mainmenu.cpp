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

#include <iostream>
#include "mainmenu.h"
#include "engine/log.h"
#include "engine/musicplayer.h"
#include "engine/data/asset/image.h"
#include "engine/data/asset/midi.h"
#include "version.h"
#include "exedata.h"
#include "definitions.h"
#include "settings.h"
#include "optionsmenu.h"
#include "screens/textpages.h"
#include "screens/imagepages.h"
#include "screens/highscores.h"
#include "screens/slots.h"
#include "screens/orderscreens.h"

namespace {
	// Bottom help lines, per menu kind (DS:19dc table in the original).
	// The help lines: the main menu's from the EXE's help table, the sub
	// menus' plain one.
	std::string helpMain() { return pocus::ExeData::get().pointerString(pocus::PTR_HELP, 0); }
	std::string helpSub() { return pocus::ExeData::get().string(pocus::STR_SUB_MENU_HELP); }
	// A menu item's text without the trailing "~" that marks a gap after it.
	std::string itemText(const std::string& item) {
		return !item.empty() && item.back() == '~' ? item.substr(0, item.size() - 1) : item;
	}
	bool itemGap(const std::string& item) {
		return !item.empty() && item.back() == '~';
	}

	// 16b8:2259: 0x1194 timer ticks (140 Hz) of no input show the idle
	// pictures (16b8:42cb), each for 0x9c3 ticks.
	constexpr uint32_t IDLE_MS = 4500 * 1000 / 140;
	constexpr uint32_t IDLE_PAGE_MS = 2499 * 1000 / 140;

	// The pictures the Legends pages place (DS:1912 table: image, x, y),
	// as offsets from DatFiles::pageImageBase.
	const std::vector<pocus::ui::TextPageScreen::PageImage> LEGEND_IMAGES = {
		{ 2, 4, 16 }, { 0, 50, 25 }, { 8, 4, 25 }, { 3, 4, 21 }, { 0, 50, 25 },
		{ -1, 0, 0 }, { -1, 0, 0 }, { -1, 0, 0 }, { -1, 0, 0 }, { 0, 50, 25 }
	};

	std::vector<int> range(int first, int count) {
		std::vector<int> files;
		for (int i = 0; i < count; i++) {
			files.push_back(first + i);
		}
		return files;
	}
}

void MainMenu::setStateGame(StateGame& stateGame) {
	this->stateGame = &stateGame;
}

void MainMenu::onCreate(pocus::data::DataManager& dataManager) {
	LOGI << "MainMenu: create";

	pocus::data::Data& data = dataManager.getData();
	const pocus::DatFiles& files = pocus::datFiles();

	this->assets.load(dataManager);

	pocus::data::asset::Image selectionImage;
	pocus::data::DataFile& selectionImageFile = data.fetchFile(files.imageMenuSelection);
	selectionImage.loadFromStream(selectionImageFile.getContent(), selectionImageFile.getLength());

	pocus::data::asset::Font menuFont;
	pocus::data::DataFile& fontFile = data.fetchFile(files.fontMain);
	menuFont.loadFromStream(fontFile.getContent(), fontFile.getLength());
	this->menu.setFont(std::move(menuFont));
	this->menu.setPalette(this->assets.palette);
	this->menu.setTextColor(72);
	this->menu.setCapitalLetterColor(88);
	this->menu.setBottomTextColor(88);

	auto selectionTexture = selectionImage.createTexture(this->assets.palette, 128);
	this->menu.setIndicator(pocus::Animation::createFromTexture(*selectionTexture, 8, 1));

	pocus::data::asset::Image bottomImage, topImage;
	pocus::data::DataFile& bottomImageFile = data.fetchFile(files.imageBottom);
	pocus::data::DataFile& topImageFile = data.fetchFile(files.imageTop);
	bottomImage.loadFromStream(bottomImageFile.getContent(), bottomImageFile.getLength());
	topImage.loadFromStream(topImageFile.getContent(), topImageFile.getLength());
	this->bottomTexture = bottomImage.createTexture(this->assets.palette, 128);
	this->topTexture = topImage.createTexture(this->assets.palette, 128);

	// The menu tune (DOS 0x25a), for when music is switched back on.
	pocus::data::asset::Midi midi;
	pocus::data::DataFile& musicFile = data.fetchFile(files.musicIntro);
	midi.loadFromStream(musicFile.getContent(), musicFile.getLength());
	this->menuMusic = midi.createAsSound();

	// 75 stars in the original (FUN_16b8_039c): random angle, radius 0..159,
	// speed 1..5.
	this->particles.createStars(this->assets.palette, SCREEN_WIDTH, SCREEN_HEIGHT, 75);

	buildMain();
}

void MainMenu::showScreen(Screen_t newScreen) {
	if (!this->menu.isEmpty()) {
		this->lastSelection[this->screen] = this->menu.getCurrentSelection();
	}
	this->screen = newScreen;
	switch (newScreen) {
		case EPISODE: buildEpisode(); break;
		case SKILL: buildSkill(); break;
		case OPTIONS: buildOptions(); break;
		case SPEED: buildSpeed(); break;
		default: buildMain(); break;
	}
	this->menu.setCurrentSelection(this->lastSelection[newScreen]);
}

void MainMenu::startMenuMusic() {
	if (!GameSettings::get().music() || !this->menuMusic) {
		return;
	}
	pocus::data::asset::Midi midi;
	pocus::data::DataFile& musicFile = this->assets.dataManager->getData().fetchFile(pocus::datFiles().musicIntro);
	midi.loadFromStream(musicFile.getContent(), musicFile.getLength());
	pocus::MusicPlayer::getInstance().play(midi.createAsSound());
}

// "Quit - return to DOS": the registered game leaves at once; the shareware
// first shows one of its order pictures for 5 seconds (1548:0aa5).
void MainMenu::quit() {
	if (this->leaving) {
		return;
	}
	this->leaving = true;
	if (pocus::GameVersion::get().shareware) {
		this->screens.push(std::make_unique<pocus::ui::OrderScreen>(this->assets, pocus::ui::OrderScreen::RANDOM_PAGE), [this](int) {
			setMessage(pocus::State::MESSAGE_QUIT);
		});
		return;
	}
	this->fade.start(pocus::Fade::FADE_OUT, [this] {
		setMessage(pocus::State::MESSAGE_QUIT);
	});
}

// Menu 0: the original's entries (DS:1a06 registered, DS:0dcc shareware);
// "~" items get a 4 px gap after them.
void MainMenu::buildMain() {
	const bool shareware = pocus::GameVersion::get().shareware;
	const pocus::DatFiles& files = pocus::datFiles();
	this->menu.clear();
	this->menu.setBottomText(helpMain());
	this->menu.setEscapeHandler(nullptr);
	// The original's item table: the same actions in the same order, the
	// shareware with its extra "Preview future levels" before the last.
	const std::vector<std::string> items = pocus::ExeData::get().pointerStrings(pocus::PTR_MENU_MAIN);
	int n = 0;
	const auto item = [&](std::function<void()> handler) {
		this->menu.addOption(itemText(items[n]), std::move(handler));
		if (itemGap(items[n])) {
			this->menu.addSpace();
		}
		n++;
	};
	item([this] { showScreen(EPISODE); });
	item([this] {
		this->screens.push(std::make_unique<pocus::ui::SlotScreen>(this->assets, GameSettings::get().save, pocus::ui::SlotScreen::RESTORE), [this](int slot) {
			if (slot < 0 || !this->stateGame) {
				showScreen(MAIN);
				return;
			}
			// 1548:0736: a restored slot starts that level straight away.
			const SaveFile& save = GameSettings::get().save;
			this->leaving = true;
			this->stateGame->restoreGame(save.slotEpisode[slot], save.slotLevel[slot], save.slotDifficulty[slot], save.slotScore[slot]);
			setMessage(pocus::State::MESSAGE_CHANGE, (void*)STATE_GAME);
		});
	});
	item([this, files] {
		this->screens.push(std::make_unique<pocus::ui::TextPageScreen>(this->assets, range(files.orderingTextStart, 9), pocus::ui::TextPageScreen::FRAMED),
			[this](int) { showScreen(MAIN); });
	});
	item([this, files] {
		// 16b8:44cc: page 0 (keyboard) or 1 (joystick), then 2..4.
		std::vector<int> pages = { files.instructionsStart + (GameSettings::get().joystick() ? 1 : 0),
			files.instructionsStart + 2, files.instructionsStart + 3, files.instructionsStart + 4 };
		this->screens.push(std::make_unique<pocus::ui::ImagePagesScreen>(this->assets, pages), [this](int) { showScreen(MAIN); });
	});
	item([this, files] {
		this->screens.push(std::make_unique<pocus::ui::TextPageScreen>(this->assets, range(files.legendsTextStart, 10), pocus::ui::TextPageScreen::PICTURE, LEGEND_IMAGES),
			[this](int) { showScreen(MAIN); });
	});
	item([this] { showScreen(OPTIONS); });
	item([this] {
		this->screens.push(std::make_unique<pocus::ui::HighScoreScreen>(this->assets, GameSettings::get().save, SaveFile::EPISODES),
			[this](int) { showScreen(MAIN); });
	});
	if (shareware) {
		item([this] {
			this->screens.push(std::make_unique<pocus::ui::OrderScreen>(this->assets, pocus::ui::OrderScreen::ALL_PAGES),
				[this](int) { showScreen(MAIN); });
		});
	}
	item([this] { quit(); });
	this->menu.layoutLikeOriginal();
}

// Menu 1. Both releases list the four games; the shareware answers episodes
// 2-4 with its "Sorry, this game is not yet registered" page (1548:05d3 /
// 16b8:429a, gated on the registered flag DS:14f6).
void MainMenu::buildEpisode() {
	this->menu.clear();
	this->menu.setBottomText(helpSub());
	this->menu.setEscapeHandler([this] { showScreen(MAIN); });
	const std::vector<std::string> names = pocus::ExeData::get().pointerStrings(pocus::PTR_MENU_EPISODE);
	this->menu.addTitle(names[0]);
	for (uint8_t i = 0; i < 4; i++) {
		this->menu.addOption(names[i + 1], [this, i] {
			if (pocus::GameVersion::get().shareware && i > 0) {
				this->screens.push(std::make_unique<pocus::ui::TextPageScreen>(this->assets, std::vector<int> { pocus::datFiles().notRegisteredText }, pocus::ui::TextPageScreen::FRAMED),
					[this](int) { showScreen(MAIN); });
				return;
			}
			this->chosenEpisode = (uint8_t)(i + 1);
			showScreen(SKILL);
		});
	}
	this->menu.layoutLikeOriginal();
}

// Menu 4.
void MainMenu::buildSkill() {
	this->menu.clear();
	this->menu.setBottomText(helpSub());
	this->menu.setEscapeHandler([this] { showScreen(EPISODE); });
	const std::vector<std::string> names = pocus::ExeData::get().pointerStrings(pocus::PTR_MENU_SKILL);
	this->menu.addTitle(names[0]);
	const pocus::Difficulty_t levels[3] = { pocus::EASY, pocus::NORMAL, pocus::HARD };
	for (int i = 0; i < 3; i++) {
		this->menu.addOption(names[i + 1], [this, i, levels] {
			if (this->leaving || !this->stateGame) {
				return;
			}
			this->leaving = true;
			const uint8_t episode = this->chosenEpisode;
			const pocus::Difficulty_t difficulty = levels[i];
			this->fade.start(pocus::Fade::FADE_OUT, [this, episode, difficulty] {
				this->stateGame->startNewGame(episode, difficulty);
				setMessage(pocus::State::MESSAGE_CHANGE, (void*)STATE_GAME);
			});
		});
	}
	this->menu.layoutLikeOriginal();
}

// Menu 2.
void MainMenu::buildOptions() {
	OptionsMenu::Callbacks callbacks;
	callbacks.onBack = [this] { showScreen(MAIN); };
	callbacks.onSpeed = [this] { showScreen(SPEED); };
	callbacks.onRebuild = [this] { showScreen(OPTIONS); };
	// 1548:07d5: music off stops the tune, on restarts the menu tune.
	callbacks.onMusicToggled = [this] {
		if (GameSettings::get().music()) {
			startMenuMusic();
		}
		else {
			pocus::MusicPlayer::getInstance().stop();
		}
	};
	OptionsMenu::buildOptions(this->menu, this->screens, this->assets, callbacks);
}

// Menu 7.
void MainMenu::buildSpeed() {
	OptionsMenu::buildSpeed(this->menu, [this] { showScreen(OPTIONS); }, [this] {
		if (this->stateGame) {
			this->stateGame->applySettings();
		}
	});
}

void MainMenu::onDetach() {
	LOGI << "MainMenu: onDetach";
}

void MainMenu::onAttach() {
	LOGI << "MainMenu: onAttach";
	this->leaving = false;
	this->lastInput = pocus::getNow();
	showScreen(MAIN);
	this->fade.setSpeed(3.0f);
	this->fade.start(pocus::Fade::FADE_IN);
}

void MainMenu::release() {
	LOGI << "MainMenu: release";
}

void MainMenu::handleEvents(pocus::EventHandler &eventHandler) {
	GameSettings::get().applyKeys(eventHandler);
	if (eventHandler.isAnyButtonDown()) {
		this->lastInput = pocus::getNow();
	}
	if (this->screens.isActive()) {
		this->screens.handleEvents(eventHandler);
		return;
	}
	if (this->leaving) {
		return;
	}
	this->menu.handleEvents(eventHandler);
}

void MainMenu::render(pocus::Renderer &renderer) {
	if (this->screens.isActive()) {
		this->screens.render(renderer, &this->particles);
		return;
	}
	this->particles.render(renderer);
	renderer.drawTexture(*this->bottomTexture, pocus::Point(0, SCREEN_HEIGHT - this->bottomTexture->getHeight()));
	renderer.drawTexture(*this->topTexture, pocus::Point(0, 0));
	this->menu.render(renderer);
	this->fade.render(renderer);
}

void MainMenu::update(float dt) {
	this->fade.update(dt);
	this->particles.update(dt);
	this->screens.update(dt);
	if (this->screens.isActive() || this->leaving) {
		return;
	}
	this->menu.update(dt);

	// Idle: the two pictures, then back to the menu (the original goes on to
	// a demo level after another idle period - not ported yet).
	if (this->screen == MAIN && pocus::getElapsedTime(this->lastInput) >= IDLE_MS) {
		this->lastInput = pocus::getNow();
		this->screens.push(std::make_unique<pocus::ui::ImagePagesScreen>(this->assets, range(pocus::datFiles().idleScreensStart, 2), IDLE_PAGE_MS, 15),
			[this](int) {
				this->lastInput = pocus::getNow();
				showScreen(MAIN);
			});
	}
}
