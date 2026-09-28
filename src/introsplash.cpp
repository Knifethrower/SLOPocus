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

#include "introsplash.h"
#include "engine/data/asset/pcx.h"
#include "engine/data/asset/midi.h"
#include "version.h"
#include "engine/log.h"
#include "engine/data/asset/voc.h"
#include "engine/musicplayer.h"
#include "definitions.h"
#include "settings.h"
#include "screens/screen.h"

void IntroSplash::onCreate(pocus::data::DataManager& dataManager) {
	const pocus::DatFiles& files = pocus::datFiles();
	pocus::data::asset::Pcx introSplashImage;
	pocus::data::asset::Midi titleMusicMidi;
	pocus::data::asset::Voc laughSoundVoc;

	pocus::data::DataFile& introSplashFile = dataManager.getData().fetchFile(files.splashIntro);
	pocus::data::DataFile& titleMusicFile = dataManager.getData().fetchFile(files.musicIntro);
	pocus::data::DataFile& laughSoundFile = dataManager.getData().fetchFile(files.vocLaugh);

	introSplashImage.loadFromStream(introSplashFile.getContent(), introSplashFile.getLength());
	titleMusicMidi.loadFromStream(titleMusicFile.getContent(), titleMusicFile.getLength());
	laughSoundVoc.loadFromStream(laughSoundFile.getContent(), laughSoundFile.getLength());

	this->backgroundImage = introSplashImage.createTexture();
	this->laughSound = laughSoundVoc.createAsSound();
	this->backgroundMusic = titleMusicMidi.createAsSound();

	// The band is a planar image in the title picture's palette (DS:14f6 = 1).
	if (files.titleBand >= 0) {
		pocus::data::DataFile& bandFile = dataManager.getData().fetchFile(files.titleBand);
		const pocus::ui::Palette256 palette = pocus::ui::ScreenAssets::pcxPalette(introSplashFile.getContent(), introSplashFile.getLength());
		this->bandImage = pocus::ui::ScreenAssets::planarImageFrom(bandFile.getContent(), bandFile.getLength(), palette);
	}
}

void IntroSplash::onDetach() {
	LOGI << "IntroSplash: onDetach";
}

void IntroSplash::onAttach() {
	LOGI << "IntroSplash: onAttach";
	this->leaving = false;
	this->fade.setSpeed(pocus::ui::fadeSpeedForSteps(40));
	this->fade.start(pocus::Fade::FADE_IN);
	this->startTick = pocus::getNow();
	if (GameSettings::get().sound() && this->laughSound) {
		this->laughSound->play();
	}
	if (GameSettings::get().music() && this->backgroundMusic) {
		pocus::MusicPlayer::getInstance().play(std::move(this->backgroundMusic));
	}
}

void IntroSplash::release() {
	LOGI << "IntroSplash: release";
}

void IntroSplash::leave() {
	if (this->leaving || this->fade.isRunning()) {
		return;
	}
	this->leaving = true;
	this->fade.setSpeed(pocus::ui::fadeSpeedForSteps(30));
	this->fade.start(pocus::Fade::FADE_OUT, [this] {
		setMessage(pocus::State::MESSAGE_CHANGE, (void*)STATE_MENU_MAIN);
	});
}

void IntroSplash::handleEvents(pocus::EventHandler &eventHandler) {
	if (eventHandler.isAnyButtonDown()) {
		if (this->fade.isRunning() && !this->leaving) {
			this->skipPending = true;
		}
		else {
			leave();
		}
	}
}

void IntroSplash::render(pocus::Renderer &renderer) {
	renderer.drawTexture(*this->backgroundImage, pocus::Point(0, 0));
	if (this->bandImage) {
		renderer.drawTexture(*this->bandImage, pocus::Point(0, 188));
	}
	this->fade.render(renderer);
}

void IntroSplash::update(float dt) {
	this->fade.update(dt);
	if (this->skipPending && !this->fade.isRunning()) {
		this->skipPending = false;
		leave();
	}
	if (pocus::getElapsedTime(this->startTick) >= IntroSplash::TIME) {
		leave();
	}
}
