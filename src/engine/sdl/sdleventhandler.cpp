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

#include "sdleventhandler.h"

using namespace pocus;

namespace {

// The 18 bindable keys, in the original's key-name table order (DS:1510:
// Left shift, Right shift, Ctrl, Alt, Caps, Space, Enter, Insert, Delete,
// Home, End, Page up, Page down, Up arrow, Down arrow, Left arrow, Right
// arrow, Cursor 5).
const SDL_Keycode BINDABLE[KEY_BINDABLE_COUNT] = {
	SDLK_LSHIFT, SDLK_RSHIFT, SDLK_LCTRL, SDLK_LALT, SDLK_CAPSLOCK, SDLK_SPACE, SDLK_RETURN, SDLK_INSERT, SDLK_DELETE,
	SDLK_HOME, SDLK_END, SDLK_PAGEUP, SDLK_PAGEDOWN, SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT, SDLK_KP_5
};

}

SdlEventHandler::SdlEventHandler() {
	this->event.type = 0;
    // Mapping - the original's defaults (HOCUS.SAV key indices 0f 10 02 03
    // 0d 0e 0b 0c = Left/Right arrow, Ctrl, Alt, Up/Down arrow, PgUp/PgDn).
    this->buttonMapping[BUTTON_FIRE] = SDLK_LALT;
    this->buttonMapping[BUTTON_JUMP] = SDLK_LCTRL;
    this->buttonMapping[BUTTON_LEFT] = SDLK_LEFT;
    this->buttonMapping[BUTTON_RIGHT] = SDLK_RIGHT;
    this->buttonMapping[BUTTON_UP] = SDLK_UP;
    this->buttonMapping[BUTTON_DOWN] = SDLK_DOWN;
    this->buttonMapping[BUTTON_SCROLL_UP] = SDLK_PAGEUP;
    this->buttonMapping[BUTTON_SCROLL_DOWN] = SDLK_PAGEDOWN;
    this->buttonMapping[BUTTON_PAUSE] = SDLK_p;
    this->buttonMapping[BUTTON_SELECTION] = SDLK_RETURN;
    this->buttonMapping[BUTTON_BACK] = SDLK_ESCAPE;
}

bool SdlEventHandler::isButtonDown(const Button_t &button) {
    if (this->event.type == SDL_KEYDOWN && !this->event.key.repeat) {
        auto iterator = buttonMapping.find(button);
        if (iterator != this->buttonMapping.end()) {
            if (this->event.key.keysym.sym == iterator->second) {
                return true;
            }
        }
    }

    return false;
}

bool SdlEventHandler::isButtonUp(const Button_t &button)   {
    if (this->event.type == SDL_KEYUP) {
        auto iterator = buttonMapping.find(button);
        if (iterator != this->buttonMapping.end()) {
            if (this->event.key.keysym.sym == iterator->second) {
                return true;
            }
        }
    }

    return false;
}


int SdlEventHandler::poll() {
    return SDL_PollEvent(&this->event);
}

bool SdlEventHandler::isQuit() {
    return this->event.type == SDL_QUIT;
}

bool SdlEventHandler::isAnyButtonDown() const {
    return (this->event.type == SDL_KEYDOWN && !this->event.key.repeat);
}

Key_t SdlEventHandler::toKey(SDL_Keycode keycode) {
	for (int i = 0; i < KEY_BINDABLE_COUNT; i++) {
		if (BINDABLE[i] == keycode) {
			return (Key_t)i;
		}
	}
	if (keycode >= SDLK_a && keycode <= SDLK_z) {
		return (Key_t)(KEY_A + (keycode - SDLK_a));
	}
	if (keycode >= SDLK_0 && keycode <= SDLK_9) {
		return (Key_t)(KEY_0 + (keycode - SDLK_0));
	}
	if (keycode >= SDLK_F1 && keycode <= SDLK_F12) {
		return (Key_t)(KEY_F1 + (keycode - SDLK_F1));
	}
	switch (keycode) {
		case SDLK_ESCAPE: return KEY_ESCAPE;
		case SDLK_BACKSPACE: return KEY_BACKSPACE;
		case SDLK_TAB: return KEY_TAB;
		case SDLK_KP_ENTER: return KEY_RETURN;
		default: return KEY_OTHER;
	}
}

SDL_Keycode SdlEventHandler::toKeycode(Key_t key) {
	if (key >= 0 && key < KEY_BINDABLE_COUNT) {
		return BINDABLE[key];
	}
	return SDLK_UNKNOWN;
}

Key_t SdlEventHandler::getKeyDown() const {
	if (this->event.type != SDL_KEYDOWN) {
		return KEY_NONE;
	}
	return toKey(this->event.key.keysym.sym);
}

// USB (SDL) scancode -> PC/XT set 1 make code, for the keys the original's
// handler can see (extended keys report their second byte, e.g. Up = 0x48).
static int dosScancode(SDL_Scancode scancode) {
	if (scancode >= SDL_SCANCODE_A && scancode <= SDL_SCANCODE_Z) {
		static const int LETTERS[26] = {
			0x1e, 0x30, 0x2e, 0x20, 0x12, 0x21, 0x22, 0x23, 0x17, 0x24, 0x25, 0x26, 0x32,
			0x31, 0x18, 0x19, 0x10, 0x13, 0x1f, 0x14, 0x16, 0x2f, 0x11, 0x2d, 0x15, 0x2c
		};
		return LETTERS[scancode - SDL_SCANCODE_A];
	}
	if (scancode >= SDL_SCANCODE_1 && scancode <= SDL_SCANCODE_9) {
		return 0x02 + (scancode - SDL_SCANCODE_1);
	}
	if (scancode >= SDL_SCANCODE_F1 && scancode <= SDL_SCANCODE_F10) {
		return 0x3b + (scancode - SDL_SCANCODE_F1);
	}
	switch (scancode) {
		case SDL_SCANCODE_0: return 0x0b;
		case SDL_SCANCODE_ESCAPE: return 0x01;
		case SDL_SCANCODE_MINUS: return 0x0c;
		case SDL_SCANCODE_EQUALS: return 0x0d;
		case SDL_SCANCODE_BACKSPACE: return 0x0e;
		case SDL_SCANCODE_TAB: return 0x0f;
		case SDL_SCANCODE_LEFTBRACKET: return 0x1a;
		case SDL_SCANCODE_RIGHTBRACKET: return 0x1b;
		case SDL_SCANCODE_RETURN: case SDL_SCANCODE_KP_ENTER: return 0x1c;
		case SDL_SCANCODE_LCTRL: case SDL_SCANCODE_RCTRL: return 0x1d;
		case SDL_SCANCODE_SEMICOLON: return 0x27;
		case SDL_SCANCODE_APOSTROPHE: return 0x28;
		case SDL_SCANCODE_GRAVE: return 0x29;
		case SDL_SCANCODE_LSHIFT: return 0x2a;
		case SDL_SCANCODE_BACKSLASH: return 0x2b;
		case SDL_SCANCODE_COMMA: return 0x33;
		case SDL_SCANCODE_PERIOD: return 0x34;
		case SDL_SCANCODE_SLASH: return 0x35;
		case SDL_SCANCODE_RSHIFT: return 0x36;
		case SDL_SCANCODE_KP_MULTIPLY: return 0x37;
		case SDL_SCANCODE_LALT: case SDL_SCANCODE_RALT: return 0x38;
		case SDL_SCANCODE_SPACE: return 0x39;
		case SDL_SCANCODE_CAPSLOCK: return 0x3a;
		case SDL_SCANCODE_NUMLOCKCLEAR: return 0x45;
		case SDL_SCANCODE_SCROLLLOCK: return 0x46;
		case SDL_SCANCODE_HOME: case SDL_SCANCODE_KP_7: return 0x47;
		case SDL_SCANCODE_UP: case SDL_SCANCODE_KP_8: return 0x48;
		case SDL_SCANCODE_PAGEUP: case SDL_SCANCODE_KP_9: return 0x49;
		case SDL_SCANCODE_KP_MINUS: return 0x4a;
		case SDL_SCANCODE_LEFT: case SDL_SCANCODE_KP_4: return 0x4b;
		case SDL_SCANCODE_KP_5: return 0x4c;
		case SDL_SCANCODE_RIGHT: case SDL_SCANCODE_KP_6: return 0x4d;
		case SDL_SCANCODE_KP_PLUS: return 0x4e;
		case SDL_SCANCODE_END: case SDL_SCANCODE_KP_1: return 0x4f;
		case SDL_SCANCODE_DOWN: case SDL_SCANCODE_KP_2: return 0x50;
		case SDL_SCANCODE_PAGEDOWN: case SDL_SCANCODE_KP_3: return 0x51;
		case SDL_SCANCODE_INSERT: case SDL_SCANCODE_KP_0: return 0x52;
		case SDL_SCANCODE_DELETE: case SDL_SCANCODE_KP_PERIOD: return 0x53;
		case SDL_SCANCODE_F11: return 0x57;
		case SDL_SCANCODE_F12: return 0x58;
		default: return 0;
	}
}

int SdlEventHandler::getDosScancode() const {
	if (this->event.type != SDL_KEYDOWN && this->event.type != SDL_KEYUP) {
		return 0;
	}
	if (this->event.type == SDL_KEYDOWN && this->event.key.repeat) {
		return 0;
	}
	const int code = dosScancode(this->event.key.keysym.scancode);
	if (code == 0) {
		return 0;
	}
	return this->event.type == SDL_KEYUP ? (code | 0x80) : code;
}

char SdlEventHandler::getTextInput() const {
	if (this->event.type != SDL_TEXTINPUT) {
		return 0;
	}
	const char c = this->event.text.text[0];
	// Single-byte printable ASCII only - the original's font has no more.
	if (c < 0x20 || c > 0x7e || this->event.text.text[1] != '\0') {
		return 0;
	}
	return c;
}

void SdlEventHandler::setButtonKey(const Button_t& button, Key_t key) {
	const SDL_Keycode keycode = toKeycode(key);
	if (keycode != SDLK_UNKNOWN) {
		this->buttonMapping[button] = keycode;
	}
}

Key_t SdlEventHandler::getButtonKey(const Button_t& button) const {
	auto iterator = this->buttonMapping.find(button);
	if (iterator == this->buttonMapping.end()) {
		return KEY_NONE;
	}
	return toKey(iterator->second);
}
