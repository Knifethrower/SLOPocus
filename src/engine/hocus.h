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

#ifndef _HOCUS_H
#define _HOCUS_H

#include <unordered_map>
#include <string>
#include "entity.h"
#include "animation.h"
#include "data/asset/spriteset.h"
#include "data/asset/palette.h"

namespace pocus {

class Hocus : public Entity {
public:
	// Which of the original's three Hocus blitters draws him this frame
	// (level_run's draw block, 1ba5 offset 0x50b0): the normal one, the
	// hurt flash (every odd frame of the hurt cooldown, palette 0x70) or
	// the Super shot flash (every odd frame of that timer, palette 0x60).
	enum Flash { FLASH_NONE, FLASH_HURT, FLASH_SUPER_SHOT };

	// The sprite header's frame indices (sprite_headers at DS:c1ba): stand
	// 0, walk 1..6 (inclusive), jump 7, fall 8, shoot 9, shoot-up 10 (= the
	// shoot frame + 1, which is what the original draws for an upward shot).
	struct Frames {
		int stand { 0 };
		int walkBegin { 1 };
		int walkEnd { 6 };
		int jump { 7 };
		int fall { 8 };
		int shoot { 9 };
		int shootUp { 10 };
	};

	// Hocus's state as the original keeps it (HOCUS.EXE level_run's movement
	// block - see the reverse-engineering notes, "Hocus"). Game::hocusTick() runs it once
	// per 20 Hz game frame. x is in half-tile columns (8 px), y in pixels; his
	// sprite is 2 half-tiles wide and 2 rows tall, drawn at (x*8, y).
	struct Dos {
		int xHalf { 0 };            // hocus_x_half
		int yPx { 0 };              // hocus_y_px
		bool jumping { false };     // hocus_jumping
		int jumpIndex { 0 };        // iRam0003a426 - index into the arc table
		int jumpLength { 0 };       // iRam0003a422 - 19 (normal) / 25 (high)
		int jumpPeak { 0 };         // iRam0003a424 - 9 / 12: from here on he's descending
		bool highJumpArc { false }; // which table the current jump uses
		bool falling { false };     // iRam0003a418
		bool turnLatch { false };   // iRam0003a3a0 - one-tick delay when starting/turning left
		bool landingSoundPending { false }; // level_run's bVar2: a 2-row fall started; the landing plays sound 15
		bool jumpEdge { false };    // key_jump_edge - set by Game::jump(), consumed by the tick
		bool fireEdge { false };    // key_fire_edge - set by Game::shoot(), consumed by the shot tick
		bool keyFire { false };     // cf49 - fire held: shows the shooting pose
		bool keyLeft { false };
		bool keyRight { false };
		bool keyUp { false };       // Up held (cf48): shots go upward; held 10 frames looks up
		bool keyDown { false };     // held 10 frames looks down
		bool keyScrollUp { false };   // "Scroll up" key (key table entry 6): looks up at once
		bool keyScrollDown { false }; // "Scroll down" key (entry 7)
		bool movedThisTick { false };
		int frame { 0 };            // hocus_anim_frame - the sprite cell drawn this frame
		// Teleport: hocus_scripted_walk_timer / hocus_target_*. 1..24 = pause,
		// 25 = flying toward the target (8 px x / 16 px y per frame), then
		// 25..40 = pause at the destination, 0 = normal control.
		int scriptedWalkTimer { 0 };
		int targetXHalf { 0 };
		int targetYPx { 0 };
		int morphJitter { 0 };      // the +-1 px shake of the morph while flying
	};

public:
	// sprite/sheet: Hocus's own sprite (set 0); morph/morphSheet: the
	// "Morph" sprite (set 2 in the level's slots) drawn instead of him while
	// teleporting. The palette supplies the two flash colours.
	void setSprite(const data::asset::Sprite& sprite, Texture& sheet, const data::asset::Palette& palette);
	void setMorphSprite(const data::asset::Sprite& sprite, Texture& sheet);
	[[nodiscard]] const Frames& getFrames() const { return this->frames; }

	// Draws dos.frame facing the current direction, or one of the flashes.
	void render(Renderer& renderer, const Point& offset, Flash flash);
	// The teleport morph (level_run's scripted-walk branch): the frame for
	// the timer, shaken by dos.morphJitter while flying.
	void renderMorph(Renderer& renderer, const Point& offset);
	// The Laser shot charges: up to 3 laser cells stacked above his head
	// (level_run, `ce94` block).
	void renderLaserCharge(Renderer& renderer, const Point& offset, int charges);

	// Shot art: the sheet holds four projectile cells from projectileFrame on -
	// horizontal, vertical, laser horizontal, laser vertical (the original picks
	// projectileFrame + 0/1/2/3).
	enum ShotVariant { SHOT_HORIZONTAL = 0, SHOT_VERTICAL = 1, SHOT_LASER = 2, SHOT_LASER_VERTICAL = 3 };
	Texture& getProjectileTexture(ShotVariant variant, const Direction_t& direction);
	[[nodiscard]] int getProjectileY() const;

	void startMovement(const Direction_t& direction);
	void stopMovement(const Direction_t& direction);

	// Head cell: column (xHalf+1)/2, row yPx/16 - the cell the original tests
	// for items, triggers and the elevator (feet are two rows below).
	Point getTilePosition();

	// Position is owned by the Dos state; these keep the Entity rect (what
	// renders and what the elevator/teleport code moves) in step with it.
	void syncFromRect();
	void writeRect();

	Dos dos;

private:
	enum { MAX_FRAMES = data::asset::Sprite::COLUMNS, MORPH_FRAMES = 5 };
	Frames frames;
	int frameCount { 0 };
	// [0] = right (the sheet's east row), [1] = left (west row).
	std::unique_ptr<Texture> frameTextures[2][MAX_FRAMES];
	std::unique_ptr<Texture> hurtTextures[2][MAX_FRAMES];
	std::unique_ptr<Texture> superShotTextures[2][MAX_FRAMES];
	std::unique_ptr<Texture> morphTextures[2][MORPH_FRAMES];
	std::unique_ptr<Texture> projectileTexturesLeft[4], projectileTexturesRight[4];
	int projectileY { 0 };
};

}

#endif //_HOCUS_H
