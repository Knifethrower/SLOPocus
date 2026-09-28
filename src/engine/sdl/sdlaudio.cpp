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

#include <SDL_mixer.h>
#include "sdlaudio.h"
#include "../audiocontrol.h"
#include "../log.h"

using namespace pocus;

bool SdlAudio::initialize() {
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) == -1) {
		return false;
	}

	// Default is 8 - with several enemies able to fire independently plus player
	// shots and item/hazard sounds, 8 was proving tight enough to starve legitimate
	// sound effects of a channel during busy moments.
	Mix_AllocateChannels(16);

	return true;
}

void SdlAudio::release() {
	Mix_CloseAudio();
}
// AudioControl: the mixer-wide levels (0..255 like the DOS driver, halved
// for SDL_mixer's 0..128) and the options menu's on/off switches.
namespace {
int soundVolume = 255;
int musicVolume = 192;
bool soundEnabled = true;
bool musicEnabled = true;

void applySound() {
	Mix_Volume(-1, soundEnabled ? soundVolume / 2 : 0);
	LOGD << "AudioControl: effects volume " << (soundEnabled ? soundVolume / 2 : 0);
}

void applyMusic() {
	// Only the level: switching music off halts the tune (MusicPlayer::stop),
	// so the mixer never has to hold a zero volume, which the Windows MIDI
	// device does not reliably come back from.
	Mix_VolumeMusic(musicVolume / 2);
	LOGD << "AudioControl: music volume " << musicVolume / 2;
}
}

void AudioControl::setSoundVolume(int volume) {
	soundVolume = volume < 0 ? 0 : (volume > 255 ? 255 : volume);
	applySound();
}

void AudioControl::setMusicVolume(int volume) {
	musicVolume = volume < 0 ? 0 : (volume > 255 ? 255 : volume);
	applyMusic();
}

void AudioControl::setSoundEnabled(bool enabled) {
	soundEnabled = enabled;
	applySound();
}

void AudioControl::setMusicEnabled(bool enabled) {
	musicEnabled = enabled;
	if (!enabled) {
		Mix_HaltMusic();
	}
}
