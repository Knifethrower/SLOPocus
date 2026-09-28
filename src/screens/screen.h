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

#ifndef UI_SCREEN_H
#define UI_SCREEN_H

#include <array>
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include "../engine/renderer.h"
#include "../engine/eventhandler.h"
#include "../engine/texture.h"
#include "../engine/fade.h"
#include "../engine/particles.h"
#include "../engine/animation.h"
#include "../engine/data/datamanager.h"
#include "../engine/data/asset/font.h"
#include "../engine/data/asset/palette.h"

// The original's out-of-game screens (HOCUS.EXE segment 16b8: text pages,
// high scores, save/restore slots, volume, key configuration, message boxes,
// full-screen pictures). Each Screen draws itself over the menu star field
// and reports a result when it finishes; ScreenHost runs one at a time with
// the original's palette fade in/out around it (15d8:046e / 0419, 20 steps
// at 70 Hz).
namespace pocus::ui {

using Rgb = data::asset::PaletteColor;
using Palette256 = std::array<Rgb, 256>;

// Everything the screens share: the menu font/palette, the frame images and
// the sprites cut from them, and the data files for page contents.
class ScreenAssets {
public:
	enum { WIDTH = 320, HEIGHT = 200, FRAME_BOTTOM_Y = 184, FRAME_CAPTION_Y = 188 };

	void load(data::DataManager& dataManager);

	// Text in the original's numbered font: font N (1..6) draws each glyph
	// row in palette colour 192+(N-1)*8+row (menu half: 64+(N-1)*8 here);
	// font 0 is the solid shadow colour. 16b8:071f.
	[[nodiscard]] std::unique_ptr<Texture> text(const std::string& text, int font);
	// 16b8:094b: the text over a copy drawn one pixel down-right in the shadow colour.
	[[nodiscard]] std::unique_ptr<Texture> shadowText(const std::string& text, int font);
	[[nodiscard]] int textWidth(const std::string& text);

	// A planar image file (15d8:0a70 format) rendered with a full 256-colour
	// palette, opaque like the DOS blitter (index 0 is drawn too).
	[[nodiscard]] std::unique_ptr<Texture> planarImage(int fileIndex, const Palette256& palette256);
	[[nodiscard]] static std::unique_ptr<Texture> planarImageFrom(const char* data, uint32_t length, const Palette256& palette256);
	// One of the pictures the text pages place, in the menu palette.
	[[nodiscard]] std::unique_ptr<Texture> pageImage(int fileIndex);
	// The 256-colour palette a PCX file carries (last 768 bytes).
	[[nodiscard]] static Palette256 pcxPalette(const char* data, uint32_t length);

	data::asset::Font font;
	data::asset::Palette palette;              // menu half (DOS colours 128..255)
	Palette256 menuPalette256;                 // game half (file 7) + menu half (file 8), as main() loads them
	std::unique_ptr<Texture> bottom;           // image 9 at (0,184)
	std::unique_ptr<Texture> top;              // image 10 at (0,0)
	std::unique_ptr<Texture> selector;         // image 14: 8 frames of 16x15
	std::unique_ptr<Texture> volumeCell[2];    // image 15: off/on cells, 16x13
	data::DataManager* dataManager { nullptr };
};

class Screen {
public:
	virtual ~Screen() = default;

	virtual void handleEvents(EventHandler& eventHandler) = 0;
	virtual void update(float dt) {}
	virtual void render(Renderer& renderer) = 0;

	// Whether the star field is drawn behind the screen (the original's
	// 16b8:0103 runs in every menu loop; full-screen pictures cover it).
	[[nodiscard]] virtual bool wantsStars() const { return true; }
	// Fade lengths in steps (15d8:046e / 0419 arguments).
	[[nodiscard]] virtual int fadeInSteps() const { return 20; }
	[[nodiscard]] virtual int fadeOutSteps() const { return 20; }
	// Called once the fade-in has finished.
	virtual void onShown() {}

	[[nodiscard]] bool isDone() const { return this->done; }
	[[nodiscard]] int getResult() const { return this->result; }

protected:
	void finish(int result = 0) {
		this->done = true;
		this->result = result;
	}

private:
	bool done { false };
	int result { 0 };
};

// Runs screens one after another. push() shows a screen (fade in), and when
// it finishes the host fades out and calls the completion handler, which may
// push the next one.
class ScreenHost {
public:
	using Completion = std::function<void(int result)>;

	void push(std::unique_ptr<Screen> screen, Completion onDone = nullptr);
	[[nodiscard]] bool isActive() const { return this->screen != nullptr; }

	void handleEvents(EventHandler& eventHandler);
	void update(float dt);
	// Draws the star field (when the screen wants it), the screen, then the fade.
	void render(Renderer& renderer, Particles* stars);

private:
	std::unique_ptr<Screen> screen;
	Completion onDone;
	Fade fade;
	bool leaving { false };
};

// Wall-clock helpers for the original's timer-tick waits (140 Hz).
inline uint32_t ticksToMs(int ticks) { return (uint32_t)(ticks * 1000L / 140); }
// Fade speed (alpha units per engine dt) for an n-step palette fade at 70 Hz.
float fadeSpeedForSteps(int steps);

// Draws a texture horizontally centred, the way (320 - width) / 2 does.
inline void drawCentred(Renderer& renderer, Texture& texture, int y) {
	renderer.drawTexture(texture, Point((float)((ScreenAssets::WIDTH - (int)texture.getWidth() + 1) / 2), (float)y));
}

}

#endif // UI_SCREEN_H
