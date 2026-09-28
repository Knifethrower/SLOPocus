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

#ifndef EVENTHANDLER_H
#define EVENTHANDLER_H

namespace pocus {

enum Button_t {
	BUTTON_RIGHT = 0,
	BUTTON_LEFT = 1,
	BUTTON_DOWN = 2,
	BUTTON_UP = 3,
	BUTTON_FIRE = 4,
	BUTTON_JUMP = 5,
    BUTTON_PAUSE = 6,
	BUTTON_SELECTION = 7,
	BUTTON_BACK = 10,
	// The original's "Scroll up/down" keys (key table 1558 entries 6/7): look
	// up/down at once instead of after holding Up/Down for 10 frames.
	BUTTON_SCROLL_UP = 12,
	BUTTON_SCROLL_DOWN = 13
};

// Platform-neutral key identities for the screens that read raw keys (page
// navigation, high-score name entry, key configuration). The first 18 are
// the keys the original lets you bind (its key-name table at DS:1510, in
// the same order), so a saved key index maps straight onto this enum.
enum Key_t {
	KEY_NONE = -1,
	KEY_LSHIFT = 0, KEY_RSHIFT, KEY_LCTRL, KEY_LALT, KEY_CAPSLOCK, KEY_SPACE, KEY_RETURN, KEY_INSERT, KEY_DELETE,
	KEY_HOME, KEY_END, KEY_PAGEUP, KEY_PAGEDOWN, KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_KP5,
	KEY_BINDABLE_COUNT,
	KEY_ESCAPE = KEY_BINDABLE_COUNT, KEY_BACKSPACE, KEY_TAB,
	KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I, KEY_J, KEY_K, KEY_L, KEY_M,
	KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T, KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z,
	KEY_0, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9,
	KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12,
	KEY_OTHER
};

class EventHandler {
public:
	virtual int poll() = 0;

	virtual bool isButtonDown(const Button_t &button) = 0;
	virtual bool isButtonUp(const Button_t &button) = 0;
	virtual bool isQuit() = 0;

	virtual bool isAnyButtonDown() const = 0;

	// The key of the current key-press event, KEY_NONE for any other event.
	[[nodiscard]] virtual Key_t getKeyDown() const = 0;
	// The printable character of the current text-input event, 0 otherwise.
	[[nodiscard]] virtual char getTextInput() const = 0;
	// The PC/XT (set 1) scancode the original's keyboard handler would see
	// for the current event: the make code on a press, make | 0x80 on a
	// release, 0 for anything else. Feeds the original's cheat-code sums.
	[[nodiscard]] virtual int getDosScancode() const = 0;

	// Key bindings for the game buttons (the original's "Define key
	// controls"); only the first KEY_BINDABLE_COUNT keys are accepted.
	virtual void setButtonKey(const Button_t& button, Key_t key) = 0;
	[[nodiscard]] virtual Key_t getButtonKey(const Button_t& button) const = 0;

};

}

#endif //EVENTHANDLER_H
