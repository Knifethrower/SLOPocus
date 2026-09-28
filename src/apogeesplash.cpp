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

#include "apogeesplash.h"
#include "engine/log.h"
#include "engine/data/asset/pcx.h"
#include "version.h"
#include "engine/data/asset/midi.h"
#include "definitions.h"
#include "settings.h"
#include "screens/screen.h"

void ApogeeSplash::onCreate(pocus::data::DataManager& dataManager) {
	const pocus::DatFiles& files = pocus::datFiles();
	pocus::data::asset::Pcx logoPcx;
	pocus::data::asset::Midi apogeeMusicMidi;

	pocus::data::DataFile& logoFile = dataManager.getData().fetchFile(files.splashApogee);
	pocus::data::DataFile& apogeeMusicFile = dataManager.getData().fetchFile(files.musicApogee);

	logoPcx.loadFromStream(logoFile.getContent(), logoFile.getLength());
	apogeeMusicMidi.loadFromStream(apogeeMusicFile.getContent(), apogeeMusicFile.getLength());

	this->logoImage = logoPcx.createTexture();
	this->backgroundMusic = apogeeMusicMidi.createAsSound();

	if (files.antiPiracyPcx >= 0) {
		pocus::data::asset::Pcx noticePcx;
		pocus::data::DataFile& noticeFile = dataManager.getData().fetchFile(files.antiPiracyPcx);
		noticePcx.loadFromStream(noticeFile.getContent(), noticeFile.getLength());
		this->noticeImage = noticePcx.createTexture();
	}
}

void ApogeeSplash::onDetach() {
	LOGI << "ApogeeSplash: onDetach";
}

void ApogeeSplash::onAttach() {
	LOGI << "ApogeeSplash: onAttach";
	this->leaving = false;
	if (this->noticeImage) {
		this->phase = NOTICE;
		this->startTick = pocus::getNow();
		this->fade.setSpeed(pocus::ui::fadeSpeedForSteps(20));
		this->fade.start(pocus::Fade::FADE_IN);
	}
	else {
		showLogo();
	}
}

void ApogeeSplash::showLogo() {
	this->phase = LOGO;
	this->startTick = pocus::getNow();
	if (GameSettings::get().music() && this->backgroundMusic) {
		this->backgroundMusic->play();
	}
	this->fade.setSpeed(pocus::ui::fadeSpeedForSteps(20));
	this->fade.start(pocus::Fade::FADE_IN);
}

void ApogeeSplash::leave() {
	if (this->leaving || this->fade.isRunning()) {
		return;
	}
	this->leaving = true;
	if (this->phase == NOTICE) {
		this->fade.setSpeed(pocus::ui::fadeSpeedForSteps(20));
		this->fade.start(pocus::Fade::FADE_OUT, [this] {
			this->leaving = false;
			showLogo();
		});
		return;
	}
	this->fade.setSpeed(pocus::ui::fadeSpeedForSteps(30));
	this->fade.start(pocus::Fade::FADE_OUT, [this] {
		if (this->backgroundMusic) {
			this->backgroundMusic->stop();
		}
		setMessage(pocus::State::MESSAGE_CHANGE, (void*)STATE_SPLASH_INTRO);
	});
}

void ApogeeSplash::release() {
	LOGI << "ApogeeSplash: release";
}

void ApogeeSplash::handleEvents(pocus::EventHandler &eventHandler) {
	if (eventHandler.isAnyButtonDown()) {
		if (this->fade.isRunning() && !this->leaving) {
			this->skipPending = true;
		}
		else {
			leave();
		}
	}
}

void ApogeeSplash::render(pocus::Renderer &renderer) {
	pocus::Texture* image = this->phase == NOTICE ? this->noticeImage.get() : this->logoImage.get();
	if (image) {
		renderer.drawTexture(*image, pocus::Point(0, 0));
	}
	this->fade.render(renderer);
}

void ApogeeSplash::update(float dt) {
	this->fade.update(dt);
	if (this->skipPending && !this->fade.isRunning()) {
		this->skipPending = false;
		leave();
	}
	const uint32_t limit = this->phase == NOTICE ? NOTICE_TIME : LOGO_TIME;
	if (pocus::getElapsedTime(this->startTick) >= limit) {
		leave();
	}
}
