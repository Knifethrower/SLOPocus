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
#include "textpages.h"
#include "../version.h"
#include "../exedata.h"
#include "../engine/log.h"

using namespace pocus;
using namespace pocus::ui;

namespace {

// DAT text page: int16 lines, int16 lineHeight, int16 blankHeight,
// 20 x { int16 x, int16 font }, 20 x char[80] (DS:7c34.. when loaded).
struct TextPage {
	int lines { 0 };
	int lineHeight { 10 };
	int blankHeight { 10 };
	int x[20] {};
	int font[20] {};
	std::string text[20];

	bool load(const char* data, uint32_t length) {
		if (length < 6 + 80 + 20 * 80) {
			return false;
		}
		const auto word = [&](uint32_t at) { return (int)(int16_t)((uint8_t)data[at] | ((uint8_t)data[at + 1] << 8)); };
		this->lines = word(0);
		this->lineHeight = word(2);
		this->blankHeight = word(4);
		if (this->lines < 0 || this->lines > 20) {
			return false;
		}
		for (int i = 0; i < 20; i++) {
			this->x[i] = word(6 + i * 4);
			this->font[i] = word(8 + i * 4);
			const char* line = data + 0x56 + i * 0x50;
			this->text[i] = std::string(line, strnlen(line, 80));
		}
		return true;
	}
};

}

TextPageScreen::TextPageScreen(ScreenAssets& assets, std::vector<int> files, Style style, std::vector<PageImage> images):
	assets(assets),
	files(std::move(files)),
	style(style),
	images(std::move(images))
{
	buildPage();
}

void TextPageScreen::buildPage() {
	this->lines.clear();
	this->picture = nullptr;
	if (this->files.empty()) {
		finish(-1);
		return;
	}

	data::DataFile& file = this->assets.dataManager->getData().fetchFile((uint32_t)this->files[this->page]);
	TextPage text;
	if (!text.load(file.getContent(), file.getLength())) {
		LOGE << "TextPageScreen: file " << this->files[this->page] << " is not a text page";
		finish(-1);
		return;
	}

	// Caption (3401/3230): "Press any key to continue" for a single page,
	// else "Page N of M - Press PGDN/PGUP - ESC to cancel".
	const int pages = (int)this->files.size();
	std::string captionText;
	// Pad prompts: L1/R1 are PgUp/PgDn and SELECT is Esc.
	const bool pad = padPrompts();
	const ExeData& exe = ExeData::get();
	if (pages == 1) {
		captionText = pad ? "Press any button to continue" : exe.pointerString(PTR_HELP, 2);
	}
	else {
		captionText = exe.string(STR_PAGE) + std::to_string(this->page + 1) + exe.string(STR_OF) + std::to_string(pages);
		if (this->page == 0) {
			captionText += pad ? " - Press R1 - SELECT to cancel" : exe.string(STR_PAGE_NEXT);
		}
		else if (this->page == pages - 1) {
			captionText += pad ? " - Press L1 - SELECT to cancel" : exe.string(STR_PAGE_PREVIOUS);
		}
		else {
			captionText += pad ? " - Press L1/R1 - SELECT to cancel" : exe.string(STR_PAGE_BOTH);
		}
	}
	this->caption = this->assets.shadowText(captionText, 4);

	// Per-page picture (3401/364d: the DS:1912 / 194e tables).
	if (this->style == PICTURE && this->page < (int)this->images.size() && this->images[this->page].imageOffset >= 0) {
		const PageImage& image = this->images[this->page];
		this->picture = this->assets.pageImage(datFiles().pageImageBase + image.imageOffset);
		this->pictureX = image.x;
		this->pictureY = image.y;
	}

	// Vertical centring: the sum of the line heights (blank lines count the
	// blank height) inside the 184 px (PICTURE) or 40..184 (FRAMED) band.
	int total = 0;
	for (int i = 0; i < text.lines; i++) {
		total += text.text[i].empty() ? text.blankHeight : text.lineHeight;
	}
	int y = this->style == PICTURE ? (184 - total) / 2 : (144 - total) / 2 + 40;
	for (int i = 0; i < text.lines; i++) {
		const std::string& line = text.text[i];
		if (!line.empty()) {
			Line label;
			label.texture = this->assets.text(line, text.font[i]);
			// 3230 centres every line; 3401 honours a non-zero x.
			if (this->style == FRAMED || text.x[i] == 0) {
				label.x = (ScreenAssets::WIDTH - this->assets.textWidth(line)) / 2;
			}
			else {
				label.x = text.x[i];
			}
			label.y = y;
			this->lines.push_back(std::move(label));
		}
		y += line.empty() ? text.blankHeight : text.lineHeight;
	}
}

void TextPageScreen::handleEvents(EventHandler& eventHandler) {
	if (this->turning) {
		return;
	}
	const Key_t key = eventHandler.getKeyDown();
	if (key == KEY_NONE) {
		return;
	}
	const int pages = (int)this->files.size();
	int target = this->page;
	// 16b8:3f7d
	if (key == KEY_ESCAPE || pages == 1) {
		finish(-1);
		return;
	}
	switch (key) {
		case KEY_PAGEUP: case KEY_LEFT: case KEY_UP:
			if (this->page != 0) {
				target = this->page - 1;
			}
			break;
		case KEY_PAGEDOWN: case KEY_RIGHT: case KEY_DOWN:
			if (this->page < pages - 1) {
				target = this->page + 1;
			}
			break;
		case KEY_HOME:
			target = 0;
			break;
		case KEY_END:
			target = pages - 1;
			break;
		default:
			return;
	}
	if (target == this->page) {
		return;
	}
	// The page turn fades the screen out (0419(0x14)) and the new page in.
	this->nextPage = target;
	this->turning = true;
	this->pageFade.setSpeed(15.0f);
	this->pageFade.start(Fade::FADE_OUT, [this] {
		this->page = this->nextPage;
		buildPage();
		this->pageFade.start(Fade::FADE_IN, [this] { this->turning = false; });
	});
}

void TextPageScreen::update(float dt) {
	this->pageFade.update(dt);
}

void TextPageScreen::render(Renderer& renderer) {
	renderer.drawTexture(*this->assets.bottom, Point(0, ScreenAssets::FRAME_BOTTOM_Y));
	if (this->style == FRAMED) {
		renderer.drawTexture(*this->assets.top, Point(0, 0));
	}
	else if (this->picture) {
		renderer.drawTexture(*this->picture, Point((float)this->pictureX, (float)this->pictureY));
	}
	for (const Line& line : this->lines) {
		renderer.drawTexture(*line.texture, Point((float)line.x, (float)line.y));
	}
	if (this->caption) {
		renderer.drawTexture(*this->caption, Point((float)((ScreenAssets::WIDTH - (int)this->caption->getWidth() + 1) / 2), ScreenAssets::FRAME_CAPTION_Y));
	}
	this->pageFade.render(renderer);
}
