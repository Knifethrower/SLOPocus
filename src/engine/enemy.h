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

#ifndef ENEMY_H
#define ENEMY_H

#include <memory>
#include <vector>
#include "entity.h"
#include "data/asset/palette.h"
#include "data/asset/spriteset.h"
#include "data/asset/level.h"

namespace pocus {

// A monster, modelled on the original's per-slot state arrays (HOCUS.EXE's
// monster_update / spawn routines - see the reverse-engineering notes, "Monsters").
// The original runs one monster tick per game frame (20 Hz); Game drives that
// tick and owns all the behaviour logic. This class holds the state and the
// sprite frames.
//
// Units follow the original: x is in 4-pixel columns ("x4"), y in pixels,
// width in 4-pixel columns, height in pixels.
class Enemy : public Entity {
public:
	enum { NO_JUMP = -99 };
	enum { HEALTH_INVULNERABLE = -1, HEALTH_KILLS_ALL_ON_HIT = -2 };

	// Frame indices from the sprite header, -1 when the sprite has none.
	struct Frames {
		int walkBegin { 0 };
		int walkEnd { 0 };
		int jump { -1 };
		int shootBegin { -1 };
		int shootEnd { -1 };
		int projectile { -1 };
		int projectileEnd { -1 };
		int projectileWidth4 { 0 };
		int projectileHeight { 0 };
		int projectileY { 0 };
	};

	// The original's per-slot arrays, one field each.
	struct Monster {
		bool dead { false };          // slot freed (bfd4 = -1); removed at the end of the tick
		int behaviour { 0 };          // bfd4 - lvl monster_info Behavior
		int health { 0 };             // c024 - dies when hit while 0; see HEALTH_* sentinels
		int maxHealth { 0 };          // bf74 - initial health; >20 = boss (contact kills, health bar), >=500 = never despawns
		int infoIndex { 0 };          // bfe4 - index into the level's monster_info
		int spawnTileOffset { 0 };    // c004 - objects-layer cell it came from (marked consumed on death)
		int x4 { 0 };                 // c0b4
		int y { 0 };                  // c0a4
		int width4 { 0 };             // c064
		int height { 0 };             // c054
		int velX4 { 0 };              // c094 - per tick, 4px units (hoppers/walkers: -1..1)
		int velY { 0 };               // c084 - per tick, pixels
		bool facingLeft { false };    // c044
		int animFrame { 0 };          // c034
		int hitFlash { 0 };           // c014 - ticks left of hit flicker
		int spawnTicks { 20 };        // bf84 - materialising; not drawn/collidable while > 0
		int shootWindup { 0 };        // bf64 - type 0 aims for 9 ticks, fires when it reaches 1
		int freezeTicks { 0 };        // bfb4 - skip movement this tick (normal monsters move every other tick)
		int burstTicks { 0 };         // bf94 - type 1 "rush at player" window: moves every tick
		int moveTimer { 0 };          // c074 - hoppers: ticks until a new random hop
		int jumpArc { NO_JUMP };      // bfa4 - NO_JUMP, else the current y step of a -6..6 hop
		bool shooting { false };      // c124 - playing the shoot frames
		int type8Timer { 0 };         // c0c4 - behaviour 8 only
		int targetX4 { 0 };           // behaviour 8 reuses the x-velocity slot as a target column
		int bossState { 0 };          // behaviour 99 reuses c0d4: 0 idle, 1 wind-up, 2 fired, 3 phase change
		int bossTimer { 0 };          // behaviour 99 reuses c0f4 as a frame countdown
		// From monster_info:
		bool targetPlayer { false };  // c0f4
		bool shootProjectiles { false }; // c104
		bool shootOnlyFacing { false };  // c114 (Unknown2): 1 = only fire when facing Hocus
		int projectileXOffset { 0 };  // c0d4
		int projectileHSpeed4 { 0 };  // per tick, 4px units
		int projectileVSpeed { 0 };   // per tick, pixels
		bool projectileHoming { false };  // TargetPlayer also steers projectiles
		bool projectileWobbly { false };
	};

public:
	// The palette supplies the hit-flash colour (0x70, the same silhouette
	// blitter 1ba5:2896 the original uses for every monster).
	void setSprite(const data::asset::Sprite& sprite, Texture& sheet, const data::asset::Palette& palette);
	// flash: draw the current frame as the hit-flash silhouette instead.
	void render(Renderer& renderer, const Point& offset, bool flash);
	void showFrame(int frame, bool facingLeft);
	[[nodiscard]] int getFrameCount() const;

	[[nodiscard]] const Frames& getFrames() const;
	[[nodiscard]] const std::vector<std::shared_ptr<Texture>>& getProjectileFrames(bool facingLeft) const;

	Monster monster;

private:
	Frames frames;
	int frameCount { 0 };
	std::vector<std::shared_ptr<Texture>> projectileFramesLeft, projectileFramesRight;
};

}

#endif // ENEMY_H
