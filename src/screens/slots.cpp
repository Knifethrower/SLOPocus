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

#include <cstring>
#include "slots.h"
#include "message.h"
#include "../exedata.h"

using namespace pocus;
using namespace pocus::ui;

namespace {
constexpr int TITLE_Y = 59;     // 0x3b
constexpr int FIRST_ROW_Y = 79; // 0x4f (SAVE: 0x3b + 0x14)
constexpr int NUMBER_X = 35;    // 0x23
constexpr int NAME_X = 45;      // 0x2d
constexpr int SELECTOR_X = 16;  // 82f8 = 4 -> 4 * 4
constexpr int MAX_NAME = 25;

// The original remembers the slot last pointed at (DS:eee8 / eeea).
int lastSaveSlot = 0;
int lastRestoreSlot = 0;
}

SlotScreen::SlotScreen(ScreenAssets& assets, SaveFile& save, Mode mode, GameState current, std::function<bool()> store):
	assets(assets),
	save(save),
	mode(mode),
	current(current),
	store(std::move(store))
{
	this->title = assets.text(ExeData::get().string(mode == SAVE ? STR_SELECT_SAVE_SLOT : STR_SELECT_RESTORE_SLOT), 5);
	this->caption = assets.shadowText(helpLine(HELP_NUMBER), 4);
	this->cursor = assets.text("_", 2);
	for (int i = 0; i < SaveFile::SLOTS; i++) {
		this->numbers[i] = assets.text(std::to_string(i + 1), 4);
		rebuildRow(i);
	}
	// 16b8:0080: one selector frame per 5 vsyncs.
	this->selector = Animation::createFromTexture(*assets.selector, 8, 1);
	this->selector.setFps(14);
	this->selected = mode == SAVE ? lastSaveSlot : lastRestoreSlot;
	if (this->selected < 0 || this->selected >= SaveFile::SLOTS) {
		this->selected = 0;
	}
}

void SlotScreen::rebuildRow(int slot) {
	const std::string text(this->save.slotName[slot], strnlen(this->save.slotName[slot], SaveFile::NAME_LENGTH));
	this->names[slot] = text.empty() ? nullptr : this->assets.text(text, 3);
}

void SlotScreen::moveTo(int slot) {
	this->selected = slot;
	if (this->mode == SAVE) {
		lastSaveSlot = slot;
	}
	else {
		lastRestoreSlot = slot;
	}
}

void SlotScreen::handleEvents(EventHandler& eventHandler) {
	if (this->typing) {
		handleTyping(eventHandler);
	}
	else {
		handleSelection(eventHandler);
	}
}

void SlotScreen::handleSelection(EventHandler& eventHandler) {
	const Key_t key = eventHandler.getKeyDown();
	if (key == KEY_NONE) {
		return;
	}
	if (key >= KEY_1 && key <= KEY_9) {
		moveTo(key - KEY_1);
		return;
	}
	switch (key) {
		case KEY_UP:
			moveTo(this->selected == 0 ? SaveFile::SLOTS - 1 : this->selected - 1);
			break;
		case KEY_DOWN:
			moveTo(this->selected + 1 >= SaveFile::SLOTS ? 0 : this->selected + 1);
			break;
		case KEY_ESCAPE:
			finish(this->mode == RESTORE ? -1 : 0);
			break;
		case KEY_RETURN:
			if (this->mode == RESTORE) {
				// 16b8:2b8b ignores ENTER on an empty slot.
				if (this->save.slotEpisode[this->selected] != -1) {
					finish(this->selected);
				}
				break;
			}
			// 16b8:25cc: an empty slot's "<empty>" is wiped, the old name is
			// kept for ESC.
			this->oldName = std::string(this->save.slotName[this->selected], strnlen(this->save.slotName[this->selected], SaveFile::NAME_LENGTH));
			this->name = this->save.slotEpisode[this->selected] == -1 ? "" : this->oldName;
			std::memset(this->save.slotName[this->selected], 0, SaveFile::NAME_LENGTH);
			std::strncpy(this->save.slotName[this->selected], this->name.c_str(), SaveFile::NAME_LENGTH - 1);
			rebuildRow(this->selected);
			this->typing = true;
			break;
		default:
			break;
	}
}

void SlotScreen::handleTyping(EventHandler& eventHandler) {
	const int slot = this->selected;
	const char c = eventHandler.getTextInput();
	if (c >= 0x20 && c <= 0x7a) {
		if ((int)this->name.size() < MAX_NAME) {
			this->name.push_back(c);
			std::strncpy(this->save.slotName[slot], this->name.c_str(), SaveFile::NAME_LENGTH - 1);
			this->save.slotName[slot][this->name.size()] = '\0';
			rebuildRow(slot);
		}
		return;
	}
	switch (eventHandler.getKeyDown()) {
		case KEY_BACKSPACE:
			if (!this->name.empty()) {
				this->name.pop_back();
				this->save.slotName[slot][this->name.size()] = '\0';
				rebuildRow(slot);
			}
			break;
		case KEY_ESCAPE:
			// Old name back, keep choosing.
			std::memset(this->save.slotName[slot], 0, SaveFile::NAME_LENGTH);
			std::strncpy(this->save.slotName[slot], this->oldName.c_str(), SaveFile::NAME_LENGTH - 1);
			rebuildRow(slot);
			this->typing = false;
			break;
		case KEY_RETURN: {
			this->save.slotEpisode[slot] = (int16_t)this->current.episode;
			this->save.slotLevel[slot] = (int16_t)this->current.level;
			this->save.slotDifficulty[slot] = (int16_t)this->current.difficulty;
			this->save.slotScore[slot] = this->current.score;
			const bool written = this->store ? this->store() : false;
			finish(written ? 1 : 2);
			break;
		}
		default:
			break;
	}
}

void SlotScreen::update(float dt) {
	this->selector.update(dt);
}

void SlotScreen::render(Renderer& renderer) {
	drawCentred(renderer, *this->title, TITLE_Y);
	for (int i = 0; i < SaveFile::SLOTS; i++) {
		const int y = FIRST_ROW_Y + i * 10;
		renderer.drawTexture(*this->numbers[i], Point(NUMBER_X, (float)y));
		if (this->names[i]) {
			renderer.drawTexture(*this->names[i], Point(NAME_X, (float)y));
		}
	}
	const int selectedY = FIRST_ROW_Y + this->selected * 10;
	this->selector.render(renderer, Point(SELECTOR_X, (float)(selectedY - 4)));
	if (this->typing) {
		renderer.drawTexture(*this->cursor, Point((float)(NAME_X + this->assets.textWidth(this->name)), (float)selectedY));
	}
	renderer.drawTexture(*this->assets.bottom, Point(0, ScreenAssets::FRAME_BOTTOM_Y));
	drawCentred(renderer, *this->caption, ScreenAssets::FRAME_CAPTION_Y);
	renderer.drawTexture(*this->assets.top, Point(0, 0));
}
