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
#include <cstdlib>
#include "stategame.h"
#include "engine/log.h"
#include "engine/data/asset/image.h"
#include "engine/data/asset/palette.h"
#include "definitions.h"
#include "version.h"
#include "engine/data/asset/font.h"
#include "engine/data/asset/level.h"
#include "engine/data/asset/pcx.h"
#include "engine/data/asset/midi.h"
#include "engine/data/asset/leveltileset.h"
#include "engine/data/asset/leveltime.h"
#include "engine/data/asset/levelbackground.h"
#include "engine/data/asset/spriteset.h"
#include "engine/data/asset/iteminfo.h"
#include "engine/provider/provider.h"
#include "engine/data/asset/voc.h"
#include "engine/data/asset/levelmusic.h"
#include "engine/data/asset/levelelevator.h"
#include "engine/musicplayer.h"
#include "settings.h"
#include "optionsmenu.h"
#include "screens/imagepages.h"
#include "screens/slots.h"
#include "screens/message.h"
#include "screens/highscores.h"
#include "screens/tally.h"
#include "screens/textpages.h"
#include "screens/episodecomplete.h"
#include "screens/volume.h"
#include "engine/audiocontrol.h"
#include "exedata.h"

pocus::Game& StateGame::getGame() {
	return this->game;
}

void StateGame::loadLevel(pocus::data::Data& data, pocus::data::Data& executable, uint8_t episode, uint8_t stage) {
	episode--;
	stage--;

	pocus::Map& map = this->game.getMap();

	const uint32_t filesPerType = pocus::GameVersion::get().episodes * STAGES;
	const uint32_t playerCoordinatesOffset = 0;
	const uint32_t tileAnimationSettingsOffset = 1;
	const uint32_t messagesOffset = 2;
	const uint32_t teleportsOffset = 3;
	const uint32_t switchesOffset = 4;
	const uint32_t toggleInsertsOffset = 5;
	const uint32_t toggleKeyHolesOffset = 6;
	const uint32_t tilePropertiesOffset = 7;
	const uint32_t enemyTriggersOffset = 8;
	const uint32_t backgroundLayerOffset = 9;
	const uint32_t mapLayerOffset = 10;
	const uint32_t additionalLayerOffset = 11;
	const uint32_t eventLayerOffset = 12;
	const uint32_t fileIndex = (STAGES * episode) + stage;

	const uint32_t offsetPlayerCoordinates = pocus::datFiles().levelsStart + (playerCoordinatesOffset * filesPerType) + (fileIndex);
	const uint32_t offsetTileAnimationSettings = pocus::datFiles().levelsStart + (tileAnimationSettingsOffset * filesPerType) + (fileIndex);
	const uint32_t offsetMessages = pocus::datFiles().levelsStart + (messagesOffset * filesPerType) + (fileIndex);
	const uint32_t offsetTeleports = pocus::datFiles().levelsStart + (teleportsOffset * filesPerType) + (fileIndex);
	const uint32_t offsetSwitches = pocus::datFiles().levelsStart + (switchesOffset * filesPerType) + (fileIndex);
	const uint32_t offsetInserts = pocus::datFiles().levelsStart + (toggleInsertsOffset * filesPerType) + (fileIndex);
	const uint32_t offsetKeyHoles = pocus::datFiles().levelsStart + (toggleKeyHolesOffset * filesPerType) + (fileIndex);
	const uint32_t offsetTileProperties = pocus::datFiles().levelsStart + (tilePropertiesOffset * filesPerType) + (fileIndex);
	const uint32_t offsetEnemyTriggers = pocus::datFiles().levelsStart + (enemyTriggersOffset * filesPerType) + (fileIndex);
	const uint32_t offsetBackgroundLayer = pocus::datFiles().levelsStart + (backgroundLayerOffset * filesPerType) + (fileIndex);
	const uint32_t offsetMapLayer = pocus::datFiles().levelsStart + (mapLayerOffset * filesPerType) + (fileIndex);
	const uint32_t offsetAdditionalLayer = pocus::datFiles().levelsStart + (additionalLayerOffset * filesPerType) + (fileIndex);
	const uint32_t offsetEventLayer = pocus::datFiles().levelsStart + (eventLayerOffset * filesPerType) + (fileIndex);

	pocus::data::DataFile& playerCoordinatesFile = data.fetchFile(offsetPlayerCoordinates);
	pocus::data::DataFile& tileAnimationSettingsFile = data.fetchFile(offsetTileAnimationSettings);
	pocus::data::DataFile& messagesFile = data.fetchFile(offsetMessages);
	pocus::data::DataFile& teleportsFile = data.fetchFile(offsetTeleports);
	pocus::data::DataFile& switchesFile = data.fetchFile(offsetSwitches);
	pocus::data::DataFile& insertsFile = data.fetchFile(offsetInserts);
	pocus::data::DataFile& keyHolesFile = data.fetchFile(offsetKeyHoles);
	pocus::data::DataFile& tilePropertiesFile = data.fetchFile(offsetTileProperties);
	pocus::data::DataFile& enemyTriggersFile = data.fetchFile(offsetEnemyTriggers);
	pocus::data::DataFile& backgroundLayerFile = data.fetchFile(offsetBackgroundLayer);
	pocus::data::DataFile& mapLayerFile = data.fetchFile(offsetMapLayer);
	pocus::data::DataFile& additionalLayerFile = data.fetchFile(offsetAdditionalLayer);
	pocus::data::DataFile& eventLayerFile = data.fetchFile(offsetEventLayer);

	map.getPlayerCoordinates().loadFromStream(playerCoordinatesFile.getContent(), playerCoordinatesFile.getLength());
	map.getTileAnimationSettings().loadFromStream(tileAnimationSettingsFile.getContent(), tileAnimationSettingsFile.getLength());
	map.getMessages().loadFromStream(messagesFile.getContent(), messagesFile.getLength());
	map.getTeleports().loadFromStream(teleportsFile.getContent(), teleportsFile.getLength());
	map.getSwitchCoordinates().loadFromStream(switchesFile.getContent(), switchesFile.getLength());
	map.getInsertToggles().loadFromStream(insertsFile.getContent(), insertsFile.getLength());
	map.getKeyHoleToggles().loadFromStream(keyHolesFile.getContent(), keyHolesFile.getLength());
	map.getTileProperties().loadFromStream(tilePropertiesFile.getContent(), tilePropertiesFile.getLength());
	map.getEnemyTrigger().loadFromStream(enemyTriggersFile.getContent(), enemyTriggersFile.getLength());
	map.getBackgroundLayer().loadFromStream(backgroundLayerFile.getContent(), backgroundLayerFile.getLength());
	map.getMapLayer().loadFromStream(mapLayerFile.getContent(), mapLayerFile.getLength());
	map.getAdditionalLayer().loadFromStream(additionalLayerFile.getContent(), additionalLayerFile.getLength());
	map.getEventLayer().loadFromStream(eventLayerFile.getContent(), eventLayerFile.getLength());

	// Load executable stuff

	const uint32_t absoluteLevel = (STAGES * episode) + stage;

	// LevelTileSet/LevelBackground's raw EXE tables reserve one padding slot
	// per episode (confirmed by direct dump: 40 entries = 4 episodes * 10,
	// with a trailing 0 at index 9/19/29/39 of each episode's block) even
	// though only 9 stages are ever used - LevelTime/LevelMusic don't do this
	// (36 entries each, no padding, verified separately). Using the plain
	// 9-wide absoluteLevel here silently drifted onto the previous episode's
	// tail data for episode 2+ (1-1..1-9 happen to land correctly since
	// episode 1 is first, which is why this went unnoticed until testing
	// past episode 1 became possible).
	const uint32_t paddedAbsoluteLevel = ((STAGES + 1) * episode) + stage;

	// Time limit
	pocus::data::DataFile& levelLimitFile = executable.fetchFile(pocus::exeFiles().limitTime);
	pocus::data::asset::LevelTime levelTime;
	levelTime.loadFromStream(levelLimitFile.getContent(), levelLimitFile.getLength());
	map.setLimitTime(levelTime.getTime()[absoluteLevel]);

	// Tile set
	pocus::data::DataFile& levelTileSetFile = executable.fetchFile(pocus::exeFiles().tilesets);
	pocus::data::asset::LevelTileSet levelTileSet;
	levelTileSet.loadFromStream(levelTileSetFile.getContent(), levelTileSetFile.getLength());
	pocus::data::asset::Pcx tileSet;
	pocus::data::DataFile& tileSetFile = data.fetchFile(pocus::datFiles().tileset01 + levelTileSet.getTileSetIds()[paddedAbsoluteLevel]);
	tileSet.loadFromStream(tileSetFile.getContent(), tileSetFile.getLength());
	auto tileSetTexture = tileSet.createTexture();
	if (tileSetTexture) {
		map.setTileSet(tileSetTexture);
	}
	else {
		LOGE << "StateGame: level " << (int)(episode + 1) << "-" << (int)(stage + 1) << " has an invalid tileset - keeping the previous one";
	}

	// Background - same padded per-episode indexing as the tileset above, now
	// that pocus::exeFiles().backgrounds actually points at the real backdrop_numbers
	// table (see the comment on that constant in version.h). Confirmed
	// against moddingwiki's documented formula and two independent ground-
	// truth reference maps (1-3, 4-9) - always resolves to a real id in
	// [0, 15], matching the 16 real background PCX/palette files that exist.
	pocus::data::DataFile& backgroundInfoFile = executable.fetchFile(pocus::exeFiles().backgrounds);
	pocus::data::asset::LevelBackground levelBackgroundInfo;
	levelBackgroundInfo.loadFromStream(backgroundInfoFile.getContent(), backgroundInfoFile.getLength());
	pocus::data::asset::Pcx background;
	pocus::data::DataFile& backgroundFile = data.fetchFile(pocus::datFiles().imageBackground01 + levelBackgroundInfo.getBackgroundIds()[paddedAbsoluteLevel]);
	background.loadFromStream(backgroundFile.getContent(), backgroundFile.getLength());
	auto backgroundTexture = background.createTexture();
	if (backgroundTexture) {
		map.setBackground(std::move(backgroundTexture));
	}
	else {
		LOGE << "StateGame: level " << (int)(episode + 1) << "-" << (int)(stage + 1) << " has an invalid background - keeping the previous one";
	}

	// Elevators - same padded per-episode indexing as tileset/background.
	// Game::start() scans the collision layer for these tile ids once it's
	// built (right after map.create() below) to find where the actual cars
	// are; a left id of -1 means this level has none.
	pocus::data::DataFile& elevatorFile = executable.fetchFile(pocus::exeFiles().elevators);
	pocus::data::asset::LevelElevator levelElevator;
	levelElevator.loadFromStream(elevatorFile.getContent(), elevatorFile.getLength());
	map.setElevatorTiles(levelElevator.getLeftTile(paddedAbsoluteLevel), levelElevator.getRightTile(paddedAbsoluteLevel));

	map.create(MAP_WIDTH, MAP_HEIGHT);

	// Music
	pocus::data::DataFile& levelMusicInfoFile = executable.fetchFile(pocus::exeFiles().music);
	pocus::data::asset::LevelMusic levelMusicInfo;
	levelMusicInfo.loadFromStream(levelMusicInfoFile.getContent(), levelMusicInfoFile.getLength());
	pocus::data::DataFile& backgroundMusicFile = data.fetchFile(pocus::datFiles().musicLevelStart + levelMusicInfo.getMusicId(episode, stage));
	pocus::data::asset::Midi musicMidi;
	musicMidi.loadFromStream(backgroundMusicFile.getContent(), backgroundMusicFile.getLength());
	this->backgroundMusic = musicMidi.createAsSound();
}

void StateGame::createHud(pocus::data::Data& data) {
	pocus::data::asset::Palette paletteGame;
	pocus::data::DataFile& paletteGameFile = data.fetchFile(pocus::datFiles().paletteGame);
	paletteGame.loadFromStream(paletteGameFile.getContent(), paletteGameFile.getLength());

	pocus::data::asset::Font font;
	pocus::data::DataFile& fontFile = data.fetchFile(pocus::datFiles().fontMain);
	font.loadFromStream(fontFile.getContent(), fontFile.getLength());

	pocus::data::asset::Image imageStuff;
	pocus::data::DataFile& imageStuffFile = data.fetchFile(pocus::datFiles().imageStuff);
	imageStuff.loadFromStream(imageStuffFile.getContent(), imageStuffFile.getLength());
	auto stuffTexture = imageStuff.createTexture(paletteGame);

	this->game.getHud().setBackground(loadTexture(data, paletteGame, pocus::datFiles().imageHud));
	this->game.getHud().setSilverKeyTexture(stuffTexture->extract(88, 0, 7, 11));
	this->game.getHud().setGoldenKeyTexture(stuffTexture->extract(96, 0, 7, 11));
	this->game.getHud().setFont(std::move(paletteGame), std::move(font), pocus::Hud::DEFAULT_FONT_COLOR);
}

void StateGame::createGame(pocus::data::Data &data) {
	pocus::data::DataFile& paletteGameFile = data.fetchFile(pocus::datFiles().paletteGame);
	pocus::data::DataFile& fontFile = data.fetchFile(pocus::datFiles().fontMain);

	this->game.getPalette().loadFromStream(paletteGameFile.getContent(), paletteGameFile.getLength());
	this->game.getFont().loadFromStream(fontFile.getContent(), fontFile.getLength());
	this->game.setTextColor(pocus::Game::DEFAULT_TEXT_COLOR);

	// The effects, by their index in the EXE's sound table.
	const pocus::ExeData& exe = pocus::ExeData::get();
	const auto loadVoc = [&data, &exe](int sound) -> std::unique_ptr<pocus::Sound> {
		pocus::data::DataFile& file = data.fetchFile((uint32_t)exe.soundFile(sound));
		pocus::data::asset::Voc voc;
		voc.loadFromStream(file.getContent(), file.getLength());
		return voc.createAsSound();
	};
	this->game.getSoundShot() = loadVoc(0);
	this->game.getSoundPotion() = loadVoc(1);
	this->game.getSoundsItem().push_back(loadVoc(2));
	this->game.getSoundsItem().push_back(loadVoc(3));
	this->game.getSoundCrystal() = loadVoc(4);
	this->game.getSoundHint() = loadVoc(5);
	this->game.getSoundSwitchRefused() = loadVoc(6);
	this->game.getSoundReveal() = loadVoc(7);
	this->game.getSoundHit() = loadVoc(8);
	this->game.getSoundSpecialItem() = loadVoc(9);
	this->game.getSoundKill() = loadVoc(10);
	this->game.getSoundMonsterShot() = loadVoc(12);
	this->game.getSoundEnemyShot() = loadVoc(13);
	this->game.getSoundJump() = loadVoc(15);

	this->game.getHocus().setPosition(
		pocus::Point(
			this->game.getMap().getPlayerCoordinates().getX() * TILE_SIZE,
			this->game.getMap().getPlayerCoordinates().getY() * TILE_SIZE
		)
	);

	for (int i = 0; i < pocus::data::asset::Messages::MESSAGES; i++) {
		const auto& message = this->game.getMap().getMessages().getMessages()[i];

		if (message.x == 0xffff) {
			continue;
		}

		uint32_t lineIndex = 0;
		uint32_t maxWidth = 0;
		std::vector<std::unique_ptr<pocus::Texture>> textures;

		do {
			textures.push_back(this->game.getFont().writeShadow(std::string(message.lines[lineIndex]), this->game.getPalette(), pocus::Game::DEFAULT_TEXT_COLOR));
			if (textures[textures.size() - 1]->getWidth() > maxWidth) {
				maxWidth = textures[textures.size() - 1]->getWidth();
			}
		} while(message.lines[++lineIndex][0] != '\0');

		auto hintTexture = pocus::Provider::provideTexture(maxWidth, lineIndex * 8 + lineIndex * 3);
		hintTexture->fill(255, 0, 255);

		lineIndex = 0;

		for (auto& texture : textures) {
			hintTexture->paste(*texture, 0, 0, maxWidth / 2 - texture->getWidth() / 2, lineIndex * 12);
			lineIndex++;
		}

		hintTexture->setColorKey(255, 0, 255);

		this->game.getHintTextures().push_back(std::move(hintTexture));
	}

	this->game.getViewportSize().set(SCREEN_WIDTH, SCREEN_HEIGHT);
}

void StateGame::loadSprites(pocus::data::Data& data) {
	pocus::data::asset::Palette paletteGame;
	pocus::data::DataFile& paletteGameFile = data.fetchFile(pocus::datFiles().paletteGame);
	paletteGame.loadFromStream(paletteGameFile.getContent(), paletteGameFile.getLength());

	pocus::data::asset::SpriteSet spriteSet;
	pocus::data::DataFile& spriteSetFile = data.fetchFile(pocus::datFiles().spriteSet);
	spriteSet.loadFromStream(spriteSetFile.getContent(), spriteSetFile.getLength());

	this->game.getHocus().setSprite(spriteSet.getSprite(pocus::SPRITE_HOCUS),
									*spriteSet.getSprite(pocus::SPRITE_HOCUS).createAsTexture(paletteGame), paletteGame);
	this->game.getHocus().setMorphSprite(spriteSet.getSprite(pocus::SPRITE_MORPH),
									*spriteSet.getSprite(pocus::SPRITE_MORPH).createAsTexture(paletteGame));

	// The score tags / pickup icons and the twinks: 5 cells per row, the
	// sheet's east row first.
	const pocus::Color colorKey = pocus::color::pink;
	const auto cells = [&](uint32_t index, const std::function<void(int, int, std::unique_ptr<pocus::Texture>)>& put) {
		pocus::data::asset::Sprite& sprite = spriteSet.getSprite(index);
		auto sheet = sprite.createAsTexture(paletteGame);
		const uint32_t w = sprite.header.width4;
		const uint32_t h = sprite.header.height;
		for (int row = 0; row < 2; row++) {
			for (int frame = 0; frame < 5; frame++) {
				put(row, frame, sheet->extract((uint32_t)frame * w, (uint32_t)row * h, w, h, &colorKey));
			}
		}
	};
	cells(pocus::SPRITE_SCORE, [this](int row, int frame, std::unique_ptr<pocus::Texture> t) { this->game.setTagTexture(row, frame, std::move(t)); });
	cells(pocus::SPRITE_TWINKS, [this](int row, int frame, std::unique_ptr<pocus::Texture> t) { this->game.setTwinkTexture(row, frame, std::move(t)); });
}

void StateGame::loadItems(pocus::data::Data& executable) {
	pocus::data::DataFile& itemsFile = executable.fetchFile(pocus::exeFiles().items);

	this->game.getItemInfo().loadFromStream(itemsFile.getContent(), itemsFile.getLength());
}

void StateGame::loadEnemies(pocus::data::Data& data) {
	pocus::data::DataFile& paletteGameFile = data.fetchFile(pocus::datFiles().paletteGame);
	this->enemyPalette.loadFromStream(paletteGameFile.getContent(), paletteGameFile.getLength());

	pocus::data::DataFile& spriteSetFile = data.fetchFile(pocus::datFiles().spriteSet);
	this->enemySpriteSet.loadFromStream(spriteSetFile.getContent(), spriteSetFile.getLength());

	// Monsters spawn on demand (Game::checkMonsterTriggers / spawnMonster); Game
	// only needs us for the part that requires these assets: the sprite frames.
	this->game.setMonsterSpawner([this](uint16_t infoIndex, pocus::Enemy& enemy) {
		return buildEnemy(infoIndex, enemy);
	});
}

bool StateGame::buildEnemy(uint16_t infoIndex, pocus::Enemy& enemy) {
	const pocus::data::asset::TileProperties& tileProperties = this->game.getMap().getTileProperties();
	if (infoIndex >= pocus::data::asset::TileProperties::PROPERTIES) {
		return false;
	}

	const pocus::data::asset::TileProperties::Entry& properties = tileProperties.getProperties()[infoIndex];
	if (properties.spriteSet >= this->enemySpriteSet.getSpriteCount()) {
		LOGW << "Monster info " << infoIndex << " names sprite set " << properties.spriteSet << " which isn't loaded";
		return false;
	}

	pocus::data::asset::Sprite& sprite = this->enemySpriteSet.getSprite(properties.spriteSet);
	enemy.setSprite(sprite, *sprite.createAsTexture(this->enemyPalette), this->enemyPalette);
	return true;
}

void StateGame::onCreate(pocus::data::DataManager& dataManager) {
	this->dataManager = &dataManager;

	pocus::data::Data& data = dataManager.getData();
	pocus::data::Data& executable = dataManager.getExecutable();

	loadLevel(data, executable, this->currentEpisode, this->currentStage);
	loadSprites(data);
	createHud(data);
	createGame(data);
	loadItems(executable);
	loadEnemies(data);

	// In-game menu assets: same font/palette/selector/frame images as the main menu.
	{
		pocus::data::asset::Font font;
		pocus::data::asset::Palette palette;
		pocus::data::asset::Image bottomImage, topImage, selectionImage;
		pocus::data::DataFile& fontFile = data.fetchFile(pocus::datFiles().fontMain);
		pocus::data::DataFile& paletteFile = data.fetchFile(pocus::datFiles().paletteMenu);
		pocus::data::DataFile& bottomImageFile = data.fetchFile(pocus::datFiles().imageBottom);
		pocus::data::DataFile& topImageFile = data.fetchFile(pocus::datFiles().imageTop);
		pocus::data::DataFile& selectionImageFile = data.fetchFile(pocus::datFiles().imageMenuSelection);
		font.loadFromStream(fontFile.getContent(), fontFile.getLength());
		palette.loadFromStream(paletteFile.getContent(), paletteFile.getLength());
		bottomImage.loadFromStream(bottomImageFile.getContent(), bottomImageFile.getLength());
		topImage.loadFromStream(topImageFile.getContent(), topImageFile.getLength());
		selectionImage.loadFromStream(selectionImageFile.getContent(), selectionImageFile.getLength());
		this->inGameMenu.setFont(std::move(font));
		this->inGameMenu.setPalette(palette);
		this->inGameMenu.setTextColor(72);
		this->inGameMenu.setCapitalLetterColor(88);
		this->inGameMenu.setBottomTextColor(88);
		auto selectionTexture = selectionImage.createTexture(palette, 128);
		this->inGameMenu.setIndicator(pocus::Animation::createFromTexture(*selectionTexture, 8, 1));
		this->menuBottomTexture = bottomImage.createTexture(palette, 128);
		this->menuTopTexture = topImage.createTexture(palette, 128);
		this->screenAssets.load(dataManager);
		this->menuStars.createStars(palette, SCREEN_WIDTH, SCREEN_HEIGHT, 75);
	}
	applySettings();
}

void StateGame::loadNextLevel() {
	uint8_t nextEpisode = this->currentEpisode;
	uint8_t nextStage = this->currentStage + 1;
	if (nextStage > STAGES) {
		nextStage = 1;
		nextEpisode++;
		if (nextEpisode > pocus::GameVersion::get().episodes) {
			nextEpisode = 1;
		}
	}

	startLevel(nextEpisode, nextStage);
}

void StateGame::startLevel(uint8_t episode, uint8_t stage) {
	this->levelFlowActive = false;
	this->currentEpisode = episode;
	this->currentStage = stage;
	this->inGameMenuOpen = false;
	this->scoreAtLevelStart = this->game.getPlayer().getScore();

	pocus::data::Data& data = this->dataManager->getData();
	pocus::data::Data& executable = this->dataManager->getExecutable();

	this->game.resetForNewLevel();
	this->game.getPlayer().setEpisode(this->currentEpisode);
	this->game.getPlayer().setLevel(this->currentStage);

	loadLevel(data, executable, this->currentEpisode, this->currentStage);
	// Re-runs sound/palette/font loading too - level-independent and technically
	// redundant here, but cheap (already-in-memory DAT bytes, no disk I/O) and
	// this is also where Hocus's spawn position and this level's hint text
	// textures get (re)built, both of which are needed.
	createGame(data);

	this->game.start();

	LOGI << "StateGame: loaded level " << (int)this->currentEpisode << "-" << (int)this->currentStage;
}

std::unique_ptr<pocus::Texture> StateGame::loadTexture(pocus::data::Data& data, uint32_t paletteFileIndex, uint32_t imageFileIndex) {
	pocus::data::asset::Image image;
	pocus::data::asset::Palette palette {};

	pocus::data::DataFile& paletteFile = data.fetchFile(paletteFileIndex);
	palette.loadFromStream(paletteFile.getContent(), paletteFile.getLength());

	pocus::data::DataFile& imageFile = data.fetchFile(imageFileIndex);
	image.loadFromStream(imageFile.getContent(), imageFile.getLength());

	return image.createTexture(palette);
}

std::unique_ptr<pocus::Texture> StateGame::loadTexture(pocus::data::Data& data, const pocus::data::asset::Palette& palette, uint32_t imageFileIndex) {
	pocus::data::asset::Image image;
	pocus::data::DataFile& imageFile = data.fetchFile(imageFileIndex);
	image.loadFromStream(imageFile.getContent(), imageFile.getLength());
	return image.createTexture(palette);
}

void StateGame::onDetach() {
	LOGI << "StateGame: onDetach";
	this->attached = false;
}

void StateGame::onAttach() {
	LOGI << "StateGame: onAttach";

	this->game.start();

	this->attached = true;
	startLevelMusic();
}

void StateGame::release() {
	LOGI << "StateGame: release";
}

void StateGame::handleEvents(pocus::EventHandler &eventHandler) {
	GameSettings::get().applyKeys(eventHandler);
	if (this->screens.isActive()) {
		this->screens.handleEvents(eventHandler);
		return;
	}
	// A notice or the pause text waits for the next key press (1ba5:3751/37fc).
	if (this->overlayActive) {
		if (eventHandler.getKeyDown() != pocus::KEY_NONE) {
			clearOverlay();
		}
		return;
	}

	// The original's ESC menu (menu 5) - the game is paused underneath.
	if (this->inGameMenuOpen) {
		this->inGameMenu.handleEvents(eventHandler);
		return;
	}
	if (eventHandler.isButtonDown(pocus::BUTTON_BACK)) {
		openInGameMenu();
		return;
	}
	if (this->levelFlowActive) {
		return;
	}
	handleCheats(eventHandler);
	handleHotkeys(eventHandler);
	if (this->screens.isActive() || this->overlayActive) {
		return;
	}

	if (eventHandler.isButtonDown(pocus::BUTTON_LEFT)) {
		this->game.startMovement(pocus::Entity::LEFT);
	}
	else if (eventHandler.isButtonUp(pocus::BUTTON_LEFT)) {
		this->game.stopMovement(pocus::Entity::LEFT);
	}

	if (eventHandler.isButtonDown(pocus::BUTTON_RIGHT)) {
		this->game.startMovement(pocus::Entity::RIGHT);
	}
	else if (eventHandler.isButtonUp(pocus::BUTTON_RIGHT)) {
		this->game.stopMovement(pocus::Entity::RIGHT);
	}

	if (eventHandler.isButtonDown(pocus::BUTTON_JUMP)) {
		this->game.jump();
	}

	if (eventHandler.isButtonDown(pocus::BUTTON_FIRE)) {
		this->game.shoot();
	}
	else if (eventHandler.isButtonUp(pocus::BUTTON_FIRE)) {
		this->game.releaseFire();
	}

	if (eventHandler.isButtonDown(pocus::BUTTON_UP)) {
		this->game.activate();
		this->game.setElevatorInput(1);
	}
	else if (eventHandler.isButtonUp(pocus::BUTTON_UP)) {
		this->game.setElevatorInput(0);
	}

	if (eventHandler.isButtonDown(pocus::BUTTON_DOWN)) {
		this->game.setElevatorInput(-1);
	}
	else if (eventHandler.isButtonUp(pocus::BUTTON_DOWN)) {
		this->game.setElevatorInput(0);
	}

	if (eventHandler.isButtonDown(pocus::BUTTON_SCROLL_UP)) {
		this->game.setScrollInput(1);
	}
	else if (eventHandler.isButtonUp(pocus::BUTTON_SCROLL_UP)) {
		this->game.setScrollInput(0);
	}

	if (eventHandler.isButtonDown(pocus::BUTTON_SCROLL_DOWN)) {
		this->game.setScrollInput(-1);
	}
	else if (eventHandler.isButtonUp(pocus::BUTTON_SCROLL_DOWN)) {
		this->game.setScrollInput(0);
	}


	/*
	else if (eventHandler.isButtonDown(pocus::BUTTON_LEFT)) {
		this->game.getOffset().setX(this->game.getOffset().getX() + 16.0f);
		std::cout << "OFFSET: " << this->game.getOffset().getX() << ", " << this->game.getOffset().getY() << std::endl;
	}
	else if (eventHandler.isButtonDown(pocus::BUTTON_RIGHT)) {
		this->game.getOffset().setX(this->game.getOffset().getX() - 16.0f);
		std::cout << "OFFSET: " << this->game.getOffset().getX() << ", " << this->game.getOffset().getY() << std::endl;
	}
	if (eventHandler.isButtonDown(pocus::BUTTON_UP)) {
		this->game.getOffset().setY(this->game.getOffset().getY() + 16.0f);
		std::cout << "OFFSET: " << this->game.getOffset().getX() << ", " << this->game.getOffset().getY() << std::endl;
	}
	else if (eventHandler.isButtonDown(pocus::BUTTON_DOWN)) {
		this->game.getOffset().setY(this->game.getOffset().getY() - 16.0f);
		std::cout << "OFFSET: " << this->game.getOffset().getX() << ", " << this->game.getOffset().getY() << std::endl;
	}
	*/
}

void StateGame::render(pocus::Renderer &renderer) {
	if (this->screens.isActive()) {
		this->screens.render(renderer, &this->menuStars);
		return;
	}
	if (this->inGameMenuOpen) {
		this->menuStars.render(renderer);
		// The original's menu drawer clears the screen and draws the menu
		// between the top and bottom images (FUN_16b8_19a4).
		if (this->menuBottomTexture) {
			renderer.drawTexture(*this->menuBottomTexture, pocus::Point(0, SCREEN_HEIGHT - this->menuBottomTexture->getHeight()));
		}
		if (this->menuTopTexture) {
			renderer.drawTexture(*this->menuTopTexture, pocus::Point(0, 0));
		}
		this->inGameMenu.render(renderer);
		return;
	}
	this->game.render(renderer);
	if (this->overlayActive) {
		if (this->overlayShadow) {
			renderer.drawTexture(*this->overlayShadow, pocus::Point((float)(this->overlayX + 1), (float)(this->overlayY + 1)));
		}
		if (this->overlayText) {
			renderer.drawTexture(*this->overlayText, pocus::Point((float)this->overlayX, (float)this->overlayY));
		}
	}
}

void StateGame::update(float dt) {
	this->screens.update(dt);
	if (this->screens.isActive() || this->levelFlowActive) {
		this->menuStars.update(dt);
		return;
	}
	if (this->inGameMenuOpen) {
		this->menuStars.update(dt);
		this->inGameMenu.update(dt);
		return;
	}


	this->game.update(dt);

	if (this->game.isLevelComplete()) {
		showTally(true);
		return;
	}

	if (this->game.isPlayerDead()) {
		LOGI << "StateGame: player died on level " << (int)this->currentEpisode << "-" << (int)this->currentStage;
		showTally(false);
	}
}

// level_run's exit: the menu palette and tune come back, then 16b8:4889
// reports the level; its bonus goes on the score (7052/7054).
void StateGame::showTally(bool completed) {
	this->levelFlowActive = true;
	playMenuMusic();
	auto tally = std::make_unique<pocus::ui::TallyScreen>(this->screenAssets, completed, (int)this->currentStage,
		this->game.getTreasuresFound(), this->game.getTreasuresTotal(), difficultyIndex(),
		(int)this->game.getMap().getLimitTime(), (int)this->game.getLevelSeconds());
	const uint32_t bonus = tally->getBonus();
	this->screens.push(std::move(tally), [this, completed, bonus](int) {
		this->game.getPlayer().setScore(this->game.getPlayer().getScore() + bonus);
		if (!completed) {
			restartLevel();
			startLevelMusic();
			return;
		}
		if (this->currentStage >= STAGES) {
			finishEpisode();
			return;
		}
		loadNextLevel();
		startLevelMusic();
	});
}

void StateGame::startLevelMusic() {
	if (GameSettings::get().music() && this->backgroundMusic) {
		pocus::MusicPlayer::getInstance().play(std::move(this->backgroundMusic));
	}
	else {
		pocus::MusicPlayer::getInstance().stop();
	}
}

void StateGame::playMenuMusic() {
	if (!GameSettings::get().music()) {
		pocus::MusicPlayer::getInstance().stop();
		return;
	}
	pocus::data::asset::Midi midi;
	pocus::data::DataFile& musicFile = this->dataManager->getData().fetchFile(pocus::datFiles().musicIntro);
	midi.loadFromStream(musicFile.getContent(), musicFile.getLength());
	pocus::MusicPlayer::getInstance().play(midi.createAsSound());
}

// 1548:067c: level 8 done -> 16b8:4d76, then the episode's ending pages
// (40fd/413d/417d/41bd; the fourth adds PCX 72 with tune 610), then the
// high-score check and the main menu.
void StateGame::finishEpisode() {
	LOGI << "StateGame: episode " << (int)this->currentEpisode << " complete";
	const int episode = this->currentEpisode - 1;
	const pocus::DatFiles& files = pocus::datFiles();

	// Ending text pages and the pictures they place (DS:194e / 195a / 1966 /
	// 197e tables, as offsets from pageImageBase).
	using PageImage = pocus::ui::TextPageScreen::PageImage;
	static const int PAGE_START[4] = { 0, 2, 4, 8 };
	static const int PAGE_COUNT[4] = { 2, 2, 4, 2 };
	static const std::vector<PageImage> IMAGES[4] = {
		{ { 7, 4, 21 }, { 0, 50, 25 } },
		{ { 0, 50, 25 }, { 0, 50, 25 } },
		{ { 6, 4, 42 }, { 0, 50, 25 }, { -1, 0, 0 }, { 0, 50, 25 } },
		{ { 5, 4, 19 }, { -1, 0, 0 } }
	};
	std::vector<int> pages;
	for (int i = 0; i < PAGE_COUNT[episode]; i++) {
		pages.push_back(files.endingTextStart + PAGE_START[episode] + i);
	}

	auto toMainMenu = [this] {
		checkHighScoreThen([this] { setMessage(pocus::State::MESSAGE_CHANGE, (void*)STATE_MENU_MAIN); });
	};
	auto afterPages = [this, episode, files, toMainMenu](int) {
		if (episode != 3 || files.endingFinalPcx < 0) {
			toMainMenu();
			return;
		}
		// 16b8:41bd: PCX 72 with tune 0x262 (610) until a key, then the menu tune.
		if (GameSettings::get().music()) {
			pocus::data::asset::Midi midi;
			pocus::data::DataFile& musicFile = this->dataManager->getData().fetchFile((uint32_t)(files.musicLevelStart + 10));
			midi.loadFromStream(musicFile.getContent(), musicFile.getLength());
			pocus::MusicPlayer::getInstance().play(midi.createAsSound());
		}
		this->screens.push(std::make_unique<pocus::ui::ImagePagesScreen>(this->screenAssets, std::vector<int> { files.endingFinalPcx }), [this, toMainMenu](int) {
			playMenuMusic();
			toMainMenu();
		});
	};
	this->screens.push(std::make_unique<pocus::ui::EpisodeCompleteScreen>(this->screenAssets), [this, pages, episode, afterPages](int) {
		this->screens.push(std::make_unique<pocus::ui::TextPageScreen>(this->screenAssets, pages, pocus::ui::TextPageScreen::PICTURE, IMAGES[episode]), afterPages);
	});
}

// Death or "Abandon level & restart" (1548:06f1 / 0x655): the level starts
// over with the score it began with.
void StateGame::restartLevel() {
	this->game.getPlayer().setScore(this->scoreAtLevelStart);
	this->game.addHealth(pocus::Game::PLAYER_MAX_HEALTH);
	startLevel(this->currentEpisode, this->currentStage);
}

void StateGame::startNewGame(uint8_t episode, pocus::Difficulty_t difficulty) {
	this->game.getPlayer().setDifficulty(difficulty);
	this->game.getPlayer().setScore(0);
	this->game.getPlayer().setFirePower(1);
	this->game.addHealth(pocus::Game::PLAYER_MAX_HEALTH);
	applySettings();
	startLevel(episode, 1);
}

void StateGame::applySettings() {
	this->game.setFrameTicks(GameSettings::get().frameTicks());
}

void StateGame::openInGameMenu() {
	this->inGameMenuOpen = true;
	buildInGameMenu();
}

void StateGame::closeInGameMenu() {
	this->inGameMenuOpen = false;
}

// 1548:0704: leaving a game (level_run result -1) still puts the score up
// against the episode's high scores.
void StateGame::quitToMainMenu() {
	this->inGameMenuOpen = false;
	checkHighScoreThen([this] { setMessage(pocus::State::MESSAGE_CHANGE, (void*)STATE_MENU_MAIN); });
}

int StateGame::difficultyIndex() {
	switch (this->game.getPlayer().getDifficulty()) {
		case pocus::EASY: return 0;
		case pocus::HARD: return 2;
		default: return 1;
	}
}

void StateGame::checkHighScoreThen(std::function<void()> then) {
	SaveFile& save = GameSettings::get().save;
	const int episode = this->currentEpisode - 1;
	const uint32_t score = this->game.getPlayer().getScore();
	const int rank = save.highScoreRank(episode, score);
	if (rank < 0) {
		then();
		return;
	}
	// 16b8:1bae shifts the table and 1c9b takes the name, then HOCUS.SAV is
	// written (an I/O error shows the original's box).
	save.insertHighScore(episode, rank, score);
	this->screens.push(std::make_unique<pocus::ui::HighScoreEntryScreen>(this->screenAssets, save, episode, rank), [this, then](int) {
		OptionsMenu::storeSettings(this->screens, this->screenAssets, then);
	});
}

void StateGame::restoreGame(int episodeIndex, int levelIndex, int difficultyIdx, uint32_t score) {
	static const pocus::Difficulty_t LEVELS[3] = { pocus::EASY, pocus::NORMAL, pocus::HARD };
	this->game.getPlayer().setDifficulty(LEVELS[difficultyIdx < 0 ? 0 : (difficultyIdx > 2 ? 2 : difficultyIdx)]);
	this->game.getPlayer().setScore(score);
	this->game.getPlayer().setFirePower(1);
	this->game.addHealth(pocus::Game::PLAYER_MAX_HEALTH);
	applySettings();
	this->inGameMenuOpen = false;
	startLevel((uint8_t)(episodeIndex + 1), (uint8_t)(levelIndex + 1));
	if (this->attached) {
		startLevelMusic();
	}
}

// Menu 2 from inside the game (25d4:01d2 handles the same flags).
void StateGame::buildInGameOptions() {
	rememberInGameSelection(IN_GAME_OPTIONS);
	OptionsMenu::Callbacks callbacks;
	callbacks.onBack = [this] { buildInGameMenu(); };
	callbacks.onSpeed = [this] { buildInGameSpeed(); };
	callbacks.onRebuild = [this] { buildInGameOptions(); };
	callbacks.onMusicToggled = [this] {
		if (GameSettings::get().music()) {
			pocus::MusicPlayer::getInstance().play();
		}
		else {
			pocus::MusicPlayer::getInstance().stop();
		}
	};
	OptionsMenu::buildOptions(this->inGameMenu, this->screens, this->screenAssets, callbacks);
	this->inGameMenu.setCurrentSelection(this->inGameSelection[IN_GAME_OPTIONS]);
}

void StateGame::buildInGameSpeed() {
	rememberInGameSelection(IN_GAME_SPEED);
	OptionsMenu::buildSpeed(this->inGameMenu, [this] { buildInGameOptions(); }, [this] { applySettings(); });
	this->inGameMenu.setCurrentSelection(this->inGameSelection[IN_GAME_SPEED]);
}

void StateGame::rememberInGameSelection(InGameMenu_t next) {
	if (!this->inGameMenu.isEmpty()) {
		this->inGameSelection[this->inGameMenuKind] = this->inGameMenu.getCurrentSelection();
	}
	this->inGameMenuKind = next;
}

void StateGame::buildInGameMenu() {
	rememberInGameSelection(IN_GAME_MAIN);
	this->inGameMenu.clear();
	const pocus::ExeData& exe = pocus::ExeData::get();
	const std::vector<std::string> items = exe.pointerStrings(pocus::PTR_MENU_INGAME);
	this->inGameMenu.setBottomText(exe.string(pocus::STR_SUB_MENU_HELP));
	this->inGameMenu.setEscapeHandler([this] { closeInGameMenu(); });
	this->inGameMenu.addOption(items[0], [this] { openInstructions([this] { buildInGameMenu(); }); });
	this->inGameMenu.addOption(items[1], [this] {
		closeInGameMenu();
		restartLevel();
	});
	this->inGameMenu.addOption(items[2], [this] { openSaveScreen([this] { buildInGameMenu(); }); });
	this->inGameMenu.addOption(items[3], [this] { openRestoreScreen([this] { buildInGameMenu(); }); });
	this->inGameMenu.addOption(items[4], [this] { buildInGameOptions(); });
	this->inGameMenu.addOption(items[5], [this] { closeInGameMenu(); });
	this->inGameMenu.addOption(items[6], [this] { confirmQuit([this] { closeInGameMenu(); }); });
	this->inGameMenu.layoutLikeOriginal();
	this->inGameMenu.setCurrentSelection(this->inGameSelection[IN_GAME_MAIN]);
}

// 16b8:44cc(1): the instruction pictures, then back where we were.
void StateGame::openInstructions(std::function<void()> then) {
	const pocus::DatFiles& files = pocus::datFiles();
	std::vector<int> pages = { files.instructionsStart + (GameSettings::get().joystick() ? 1 : 0),
		files.instructionsStart + 2, files.instructionsStart + 3, files.instructionsStart + 4 };
	this->screens.push(std::make_unique<pocus::ui::ImagePagesScreen>(this->screenAssets, pages), [then](int) {
		if (then) {
			then();
		}
	});
}

// 16b8:25cc via 3048: the slot takes the level's starting score (704e/7050).
void StateGame::openSaveScreen(std::function<void()> then) {
	pocus::ui::SlotScreen::GameState state((int)this->currentEpisode - 1, (int)this->currentStage - 1, difficultyIndex(), this->scoreAtLevelStart);
	this->screens.push(std::make_unique<pocus::ui::SlotScreen>(this->screenAssets, GameSettings::get().save, pocus::ui::SlotScreen::SAVE, state,
		[] { return GameSettings::get().store(); }), [this, then](int result) {
			if (result == 2) {
				this->screens.push(std::make_unique<pocus::ui::MessageScreen>(this->screenAssets, pocus::ExeData::get().string(pocus::STR_IO_ERROR)), [then](int) {
					if (then) {
						then();
					}
				});
				return;
			}
			if (then) {
				then();
			}
		});
}

// 16b8:2b8b via 30de: a restored slot restarts at its level.
void StateGame::openRestoreScreen(std::function<void()> then) {
	this->screens.push(std::make_unique<pocus::ui::SlotScreen>(this->screenAssets, GameSettings::get().save, pocus::ui::SlotScreen::RESTORE), [this, then](int slot) {
		if (slot < 0) {
			if (then) {
				then();
			}
			return;
		}
		const SaveFile& save = GameSettings::get().save;
		restoreGame(save.slotEpisode[slot], save.slotLevel[slot], save.slotDifficulty[slot], save.slotScore[slot]);
	});
}

// 16b8:2ee1 via 3187: "Reminder: Save your game before quitting / Quit game?"
void StateGame::confirmQuit(std::function<void()> onNo) {
	const pocus::ExeData& exe = pocus::ExeData::get();
	this->screens.push(std::make_unique<pocus::ui::YesNoScreen>(this->screenAssets, exe.string(pocus::STR_QUIT_REMINDER), exe.string(pocus::STR_QUIT_QUESTION)), [this, onNo](int answer) {
		if (answer == 1) {
			quitToMainMenu();
		}
		else if (onNo) {
			onNo();
		}
	});
}

// 16b8:137d via 1744.
void StateGame::openVolumeScreen(std::function<void()> then) {
	this->screens.push(std::make_unique<pocus::ui::VolumeScreen>(this->screenAssets, GameSettings::get().save, [] { return GameSettings::get().store(); }),
		[this, then](int result) {
			if (result == 2) {
				this->screens.push(std::make_unique<pocus::ui::MessageScreen>(this->screenAssets, pocus::ExeData::get().string(pocus::STR_IO_ERROR)), [then](int) {
					if (then) {
						then();
					}
				});
				return;
			}
			if (then) {
				then();
			}
		});
}

// M key (level_run 709d): flip the flag, write HOCUS.SAV, stop or restart the
// level tune, and say so over the level.
void StateGame::toggleMusic() {
	GameSettings& settings = GameSettings::get();
	settings.setMusic(!settings.music());
	settings.store();
	pocus::AudioControl::setMusicEnabled(settings.music());
	if (settings.music()) {
		pocus::MusicPlayer::getInstance().play();
	}
	else {
		pocus::MusicPlayer::getInstance().stop();
	}
	showOverlay(pocus::ExeData::get().string(settings.music() ? pocus::STR_MUSIC_ON : pocus::STR_MUSIC_OFF), -1, 80);
}

// S key (level_run 709c).
void StateGame::toggleSound() {
	GameSettings& settings = GameSettings::get();
	settings.setSound(!settings.sound());
	settings.store();
	pocus::AudioControl::setSoundEnabled(settings.sound());
	showOverlay(pocus::ExeData::get().string(settings.sound() ? pocus::STR_SOUND_ON : pocus::STR_SOUND_OFF), -1, 80);
}

void StateGame::handleHotkeys(pocus::EventHandler& eventHandler) {
	const pocus::Key_t key = eventHandler.getKeyDown();
	if (eventHandler.isButtonDown(pocus::BUTTON_PAUSE) || key == pocus::KEY_P) {
		// 1ba5:3751: "Game Paused" at (160 - w/2, 76), shadow one pixel down-right.
		const std::string paused = pocus::ExeData::get().string(pocus::STR_GAME_PAUSED);
		const int width = (int)this->game.getFont().calculateWidth(paused);
		showOverlay(paused, 160 - width / 2, 76);
		return;
	}
	switch (key) {
		case pocus::KEY_F1: openInstructions(nullptr); break;
		case pocus::KEY_F2: openSaveScreen(nullptr); break;
		case pocus::KEY_F3: openRestoreScreen(nullptr); break;
		case pocus::KEY_F10: confirmQuit(nullptr); break;
		case pocus::KEY_M: toggleMusic(); break;
		case pocus::KEY_S: toggleSound(); break;
		case pocus::KEY_V: openVolumeScreen(nullptr); break;
		default: break;
	}
}

// 2392:044e: F, Q and B restart the sum; every make and break code is added;
// 0x5ba = full health, 0x3d6 = both keys, 0x378 = super shot (fire power 5
// for 600 frames), 0x4d8 = three laser shots. Only the health total resets
// the sum.
void StateGame::handleCheats(pocus::EventHandler& eventHandler) {
	const int code = eventHandler.getDosScancode();
	if (code == 0) {
		return;
	}
	if (code == 0x21 || code == 0x10 || code == 0x30) {
		this->cheatSum = 0;
	}
	this->cheatSum += code;
	pocus::Player& player = this->game.getPlayer();
	if (this->cheatSum == 0x5ba) {
		this->cheatSum = 0;
		this->game.addHealth(pocus::Game::PLAYER_MAX_HEALTH);
		LOGI << "StateGame: cheat - full health";
	}
	if (this->cheatSum == 0x3d6) {
		player.setSilverKey(true);
		player.setGoldKey(true);
		LOGI << "StateGame: cheat - keys";
	}
	if (this->cheatSum == 0x378) {
		if (player.effects.superShotTicks == 0) {
			player.effects.savedFirePower = player.getFirePower();
			player.setFirePower(5);
		}
		player.effects.superShotTicks = 600;
		LOGI << "StateGame: cheat - super shot";
	}
	if (this->cheatSum == 0x4d8) {
		if (player.effects.laserShots == 0) {
			player.effects.laserShots = 3;
		}
		LOGI << "StateGame: cheat - laser shots";
	}
}

// Text in the game palette's colour 104 with a colour-1 shadow (16b8:05e9
// draws), the level frozen underneath until a key is pressed.
void StateGame::showOverlay(const std::string& text, int x, int y) {
	this->overlayText = this->game.getFont().write(text, this->game.getPalette(), pocus::Game::DEFAULT_TEXT_COLOR);
	this->overlayShadow = this->game.getFont().write(text, this->game.getPalette(), 1);
	this->overlayX = x < 0 ? (SCREEN_WIDTH - (int)this->game.getFont().calculateWidth(text)) / 2 : x;
	this->overlayY = y;
	this->overlayActive = true;
	if (!this->game.isPaused()) {
		this->game.togglePause();
		this->overlayPausedGame = true;
	}
}

void StateGame::clearOverlay() {
	this->overlayActive = false;
	this->overlayText = nullptr;
	this->overlayShadow = nullptr;
	if (this->overlayPausedGame) {
		this->overlayPausedGame = false;
		if (this->game.isPaused()) {
			this->game.togglePause();
		}
	}
}
