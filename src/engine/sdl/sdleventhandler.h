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

#ifndef SDLEVENTHANDLER_H
#define SDLEVENTHANDLER_H

#include "../eventhandler.h"

#include <SDL2/SDL.h>
#include <unordered_map>

namespace pocus {

class SdlEventHandler : public EventHandler {
public:
    SdlEventHandler();

	int poll() override;

	bool isButtonDown(const Button_t &button) override;
	bool isButtonUp(const Button_t &button) override;
	bool isQuit() override;
    bool isAnyButtonDown() const override;
	[[nodiscard]] Key_t getKeyDown() const override;
	[[nodiscard]] char getTextInput() const override;
	[[nodiscard]] int getDosScancode() const override;
	void setButtonKey(const Button_t& button, Key_t key) override;
	[[nodiscard]] Key_t getButtonKey(const Button_t& button) const override;

private:
	static Key_t toKey(SDL_Keycode keycode);
	static SDL_Keycode toKeycode(Key_t key);

	SDL_Event event;

	std::unordered_map<Button_t, SDL_Keycode> buttonMapping;
};

}

#endif //SDLEVENTHANDLER_H
