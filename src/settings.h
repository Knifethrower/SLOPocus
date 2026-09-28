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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>
#include "savefile.h"
#include "engine/eventhandler.h"
#include "exedata.h"

// The original's persistent state: HOCUS.SAV (options at DS:48ca.., key
// bindings, save slots, high scores, volumes), loaded once at start-up
// (1548:035a) and written back whole whenever the DOS game does (leaving the
// options menu, volume/key screens, saving a game, entering a high score).
struct GameSettings {
	SaveFile save;
	std::string path;
	// Set whenever the key bindings may have changed; the states push them
	// into the event handler with applyKeys().
	bool keysChanged { true };

	static GameSettings& get() {
		static GameSettings instance;
		return instance;
	}

	bool load(const std::string& savePath) {
		this->path = savePath;
		this->keysChanged = true;
		return this->save.load(savePath);
	}

	// 168c:01ee - the whole block goes back to disk.
	bool store() const {
		return this->path.empty() ? false : this->save.save(this->path);
	}

	[[nodiscard]] bool sound() const { return this->save.soundOn != 0; }
	[[nodiscard]] bool music() const { return this->save.musicOn != 0; }
	[[nodiscard]] bool joystick() const { return this->save.joystickOn != 0; }
	[[nodiscard]] int speed() const {
		const int speed = this->save.gameSpeed;
		return speed < 0 ? 0 : (speed > 2 ? 2 : speed);
	}
	void setSound(bool on) { this->save.soundOn = on ? 1 : 0; }
	void setMusic(bool on) { this->save.musicOn = on ? 1 : 0; }
	void setJoystick(bool on) { this->save.joystickOn = on ? 1 : 0; }
	void setSpeed(int speed) { this->save.gameSpeed = (int16_t)speed; }

	// Frame delay in 140 Hz timer ticks: the EXE's speed table, indexed by the
	// saved game speed.
	[[nodiscard]] int frameTicks() const {
		return pocus::ExeData::get().word(pocus::TBL_SPEED, speed());
	}

	// The 8 configurable actions in HOCUS.SAV order (DS:1ab0 labels): Left,
	// Right, Jump, Fire, Up, Down, Scroll up, Scroll down.
	static constexpr pocus::Button_t KEY_BUTTONS[SaveFile::KEYS] = {
		pocus::BUTTON_LEFT, pocus::BUTTON_RIGHT, pocus::BUTTON_JUMP, pocus::BUTTON_FIRE,
		pocus::BUTTON_UP, pocus::BUTTON_DOWN, pocus::BUTTON_SCROLL_UP, pocus::BUTTON_SCROLL_DOWN
	};

	// Pushes the saved key indices (which are indices into the original's
	// 18-key table, the same order as Key_t) into the input device.
	void applyKeys(pocus::EventHandler& eventHandler) {
		if (!this->keysChanged) {
			return;
		}
		for (int i = 0; i < SaveFile::KEYS; i++) {
			if (this->save.keys[i] < pocus::KEY_BINDABLE_COUNT) {
				eventHandler.setButtonKey(KEY_BUTTONS[i], (pocus::Key_t)this->save.keys[i]);
			}
		}
		this->keysChanged = false;
	}
};

#endif // SETTINGS_H
