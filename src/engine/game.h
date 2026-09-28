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

#ifndef GAME_H
#define GAME_H

#include <map>
#include <functional>
#include "map.h"
#include "player.h"
#include "hud.h"
#include "hocus.h"
#include "enemy.h"
#include "fade.h"
#include "rules.h"
#include "data/asset/iteminfo.h"

namespace pocus {

class Game {
public:
	enum { PLAYER_MAX_HEALTH = 100 };
	enum { DEFAULT_TEXT_COLOR = 104 };
	// Monsters are simulated exactly like the original: one tick per DOS game
	// frame. The original waits 7 ticks of its 140 Hz timer per frame at the
	// default speed setting (HOCUS.EXE level_run's pacing loop and the delay
	// table at DS:14f8 = {8,7,6}), i.e. 20 frames/s = 50 ms - the same rate
	// the elevator was independently calibrated to. dt here is in 1/62.5 s
	// units (see ELEVATOR_SPEED), so one monster tick is 3.125 of them.
	// "Game playing speed": 8/7/6 timer ticks per frame (slow/medium/fast).
	void setFrameTicks(int ticks);
	enum { MAX_MONSTERS = 8, MAX_PENDING_MONSTERS = 32, MAX_ENEMY_PROJECTILES = 8 };
	// Hocus's own fire rate wasn't limited at all before - WHITE_POTION's
	// documented "rapid fire" effect needs a normal rate to be faster than.
	enum { PLAYER_SHOT_INTERVAL = 350, PLAYER_RAPID_SHOT_INTERVAL = 120 };
	static constexpr float JUMP_VELOCITY = -1.5f;
	// GOLDEN_POTION's "jumping power" - stronger takeoff, same gravity per frame
	// (Hocus::move()), so the arc is simply higher/longer rather than modeled
	// on the original's separate 25-entry Y-increment table (see moddingwiki).
	static constexpr float SUPER_JUMP_VELOCITY = -2.2f;
	// Triple shot (GREY_POTION) spawns two extra projectiles offset from the
	// main one by this many pixels, above and below.
	static constexpr float TRIPLE_SHOT_OFFSET = 12.0f;
	// Elevators move Hocus continuously at this speed, expressed as px per
	// dt-unit (not px/s) - dt is elapsed_ms/16 (PocusEngine::processFrameRate,
	// pocusengine.cpp), so it accumulates ~62.5 units per real second
	// regardless of actual frame rate. Every other speed/velocity constant in
	// this file (JUMP_VELOCITY, PROJECTILE_SPEED, gravity in Hocus::move())
	// is already expressed the same way - "per dt-unit", not "per second".
	//
	// Calibrated against the real DOS original under DOSBox-X with cycles
	// pinned to a period-correct Pentium-75 (cycles=fixed 43500, not the
	// default "auto", to rule out emulated-CPU-speed as a confound) rather
	// than guessed: riding the level 4-1 elevator (a ~10-tile/160px shaft)
	// took well under half a second in both an auto-cycles run and the
	// pinned-Pentium-75 run - consistent timing across two very different
	// emulated CPU speeds is exactly what you'd expect if the game paces
	// this off a hardware timer interrupt rather than raw instruction count,
	// meaning that measurement should hold regardless of the host CPU. That
	// works out to roughly 3x the speed this constant was previously
	// (guess-)calibrated to (one tile every ~150ms, TILE_SIZE/(0.15*62.5)).
	// The car's own tile graphic still only moves in whole-tile jumps
	// (Layer/Tile rendering has no sub-tile position, though the car renders
	// as a floating sprite while actively ridden - see Game::render() - so
	// this no longer causes the visible snapping it otherwise would), but
	// Hocus riding it is tracked in smooth pixels, same as his normal
	// walk/fall/jump movement.
	static constexpr float ELEVATOR_SPEED = TILE_SIZE / (0.05f * 62.5f);

public:
	Game();

	Map& getMap();
	Player& getPlayer();
	Hud& getHud();
	data::asset::Palette& getPalette();
	data::asset::Font& getFont();
	Point& getOffset();
	Hocus& getHocus();
	Size& getViewportSize();
	data::asset::ItemInfo& getItemInfo();
	std::vector<Enemy>& getEnemies();
	// The "Score Tags" sprite (set 1): row 0 = the 100/250/500/1000/5000 tags,
	// row 1 = the key/teleport/super-jump/invisibility/super-shot icons; and
	// the "Twinks" sprite (set 3): the 5-frame sparkle, two rows.
	void setTagTexture(int row, int frame, std::unique_ptr<Texture> texture);
	void setTwinkTexture(int row, int frame, std::unique_ptr<Texture> texture);
	std::vector<std::unique_ptr<Texture>>& getHintTextures();
	std::unique_ptr<Sound>& getSoundHint();
	std::vector<std::unique_ptr<Sound>>& getSoundsItem();
	std::unique_ptr<Sound>& getSoundCrystal();
	std::unique_ptr<Sound>& getSoundSpecialItem();
	std::unique_ptr<Sound>& getSoundPotion();
	std::unique_ptr<Sound>& getSoundHit();
	std::unique_ptr<Sound>& getSoundKill();
	std::unique_ptr<Sound>& getSoundShot();
	std::unique_ptr<Sound>& getSoundEnemyShot();
	std::unique_ptr<Sound>& getSoundSwitchRefused();
	std::unique_ptr<Sound>& getSoundReveal();
	std::unique_ptr<Sound>& getSoundMonsterShot();
	std::unique_ptr<Sound>& getSoundJump();

	void setTextColor(uint8_t color);

	void setRules(const Rules& rules);

	// The level's monsters aren't all present from the start - a trigger tile
	// (EventLayer values in the "monster trigger" range, see
	// checkMonsterTriggers()) queues the cells listed in its EnemyTrigger entry,
	// and each is spawned when one of the original's 8 monster slots is free.
	// Game itself doesn't own the sprite/palette assets needed to build an
	// Enemy's frames, so that part is delegated back out to whoever does
	// (StateGame): given the monster_info index, fill in the Enemy's sprite and
	// return false if it can't.
	void setMonsterSpawner(std::function<bool(uint16_t infoIndex, Enemy& enemy)> spawner);

	// Set once addCrystal() sees every crystal in the level has been collected;
	// stays set until resetForNewLevel() clears it. Checked by StateGame after
	// each Game::update() rather than acted on with a callback fired from inside
	// addCrystal() itself - that call is mid-way through checkItems(), itself
	// mid-way through update(), and a level transition tears down/rebuilds
	// enemies/projectiles/the map out from under whatever in this same frame's
	// update() hasn't run yet, which is exactly the kind of thing that segfaults.
	[[nodiscard]] bool isLevelComplete() const;
	// Seconds since the level started, as the bonus screen reports them.
	[[nodiscard]] uint32_t getLevelSeconds() const { return this->levelTicks / 140; }

	// True once health hits 0, unless debug invincibility is on. Checked by
	// StateGame after each Game::update(), same reasoning/timing as
	// isLevelComplete() - health can reach 0 mid-checkHazards()/updateEnemies(),
	// still mid-update(), so the actual respawn is handled a level up.
	[[nodiscard]] bool isPlayerDead() const;


	// Clears the transient per-level state (enemies, projectiles, particle
	// bursts, pending spawns, which enemy groups have already triggered, the
	// crystal/key pickups that belong to this level, the level-complete flag)
	// so the same long-lived Game/Player can carry on into a freshly loaded
	// level instead of being reconstructed from scratch. Score, health, fire
	// power and difficulty are deliberately left alone - those persist across
	// levels. Player level/episode aren't touched either - StateGame computes
	// those and sets them explicitly, since only it knows the episode/stage
	// numbering and wraparound.
	void resetForNewLevel();

	void start();
	void render(Renderer& renderer);
	void update(float dt);

	//uint32_t getElapsedTime();

	void addScore(uint32_t score);
	void removeHealth(uint8_t health);
	void addHealth(uint8_t health);
	void addSilverKey();
	void removeSilverKey();
	void addGoldenKey();
	void removeGoldenKey();
	void togglePause();
	[[nodiscard]] bool isPaused() const { return this->paused; }
	void startMovement(const Entity::Direction_t& direction);
	void stopMovement(const Entity::Direction_t& direction);
	void addCrystal(uint32_t amount);
	void jump();
	void showHint(uint32_t id);
	void hideHint();
	void activate();
	// Damage with the original's gate: refused while the hurt cooldown or the
	// Invisible potion is running; sets the cooldown (20 frames, 10 for lava).
	void hurt(uint8_t health, int cooldownTicks = 20);
	void shoot();
	// Fire released: drops the original's fire-held flag (cf49), which is what
	// shows the shooting pose.
	void releaseFire();
	// ce6a / ce68: treasures (score items) found and in the level, for the tally.
	[[nodiscard]] int getTreasuresFound() const { return this->treasuresFound; }
	[[nodiscard]] int getTreasuresTotal() const { return this->treasuresTotal; }

	// Manual: "Up or Down Arrow Keys - Moves elevators..." - a held state
	// (like startMovement/stopMovement), separate from activate()'s single
	// press-triggered wizard/switch handling that Up is also bound to.
	// direction: 1 = up, -1 = down, 0 = released.
	void setElevatorInput(int direction);
	// The original's Scroll up/down keys (PgUp/PgDn by default): +1 up, -1 down, 0 released.
	void setScrollInput(int direction);

private:
	Size viewportSize { 320.0f, 200.0f };
	data::asset::Font font;
	data::asset::Palette palette;
	uint8_t textColor { DEFAULT_TEXT_COLOR };
	Point offset;
	Map map;
	Player player;
	Hud hud;
	Tick tickStart;
	bool paused { false };
	Hocus hocus;
	data::asset::ItemInfo itemInfo;
	// --- The original's screen effects. HOCUS.EXE keeps them screen-relative
	// and the camera routine (1ba5:29c5) shifts them as it scrolls; they are
	// kept in world coordinates here, which draws the same picture. x4 = 4-px
	// columns, y = px. All of them advance once per game frame in
	// effectsTick(), which - like the original's draw pass - records what to
	// draw and then steps them.
	// Score tags / pickup icons (spawn_effect_q 1ba5:3684, drawn by 36f0): 10
	// slots, 17 frames, rising 2 px per frame.
	struct Tag { int x4 { 0 }; int y { 0 }; int row { 0 }; int frame { 0 }; int timer { 0 }; };
	Tag tags[10];
	std::unique_ptr<Texture> tagTextures[2][5];
	// Twinks (1ba5:2174 spawns, the draw loop at 1ba5:1fae): 8 slots used
	// round-robin, 9 frames long, cells 0 1 2 3 4 3 2 1 0 of a random row.
	struct Twink { int x4 { 0 }; int y { 0 }; int row { 0 }; int timer { 0 }; };
	Twink twinks[8];
	int nextTwink { 0 };
	std::unique_ptr<Texture> twinkTextures[2][5];
	// Puffs (1ba5:2f1c spawns, 2ff3 moves and draws): 8 groups of 16 pixels
	// thrown up and falling back (vy += 1 per frame) for 16 frames, each
	// palette 0x70 (red) or 0x60 (white).
	struct PuffPixel { int x { 0 }; int y { 0 }; int vx { 0 }; int vy { 0 }; uint8_t color { 0 }; };
	struct Puff { int life { 0 }; PuffPixel pixels[16]; };
	Puff puffs[8];
	int nextPuff { 0 };
	// Shot trails (1ba5:21c6 spawns, 20b1 moves and draws): 20 slots, a pixel
	// sinking 1 px per frame for 16 frames, drawn 1 frame in 2 on average at a
	// random one of its column's 4 pixels, palette 0x78 + age/4.
	struct Trail { bool live { false }; int x4 { 0 }; int y { 0 }; int age { 0 }; };
	Trail trails[20];
	int nextTrail { 0 };
	// What the last effectsTick() decided to draw.
	struct EffectDraw { Texture* texture; Color color; int x; int y; };
	std::vector<EffectDraw> effectDraws;
	void spawnTag(int x4, int y, int type);
	void spawnTwink(int x4, int y);
	void spawnPuff(int x, int y);
	void spawnTrail(int x4, int y);
	void effectsTick();
	void renderEffects(Renderer& renderer);
	void clearEffects();
	void play(std::unique_ptr<Sound>& sound);
	void updateHocusFrame();
	// Hocus's shots, the original's 10 slots (hproj_x4/y at ceee/ceda). World
	// units: x4 in 4-px columns, y in px; 16 px per game frame.
	struct HocusShot {
		int x4 { 0 };
		int y { 0 };
		bool facingLeft { false };
		bool vertical { false };
		bool piercing { false };
	};
	std::vector<HocusShot> hocusShots;
	int hurtCooldownTicks { 0 };    // ce74 - frames until Hocus can be hurt again
	std::vector<Enemy> enemies;
	std::vector<std::unique_ptr<Texture>> hintTextures;
	int currentHint { -1 };
	std::unique_ptr<Sound> soundHint, soundCrystal, soundPotion, soundHit, soundKill, soundSpecialItem, soundShot, soundEnemyShot;
	std::unique_ptr<Sound> soundSwitchRefused, soundReveal, soundMonsterShot, soundJump;
	std::vector<std::unique_ptr<Sound>> soundsItem;
	Rules rules;
	std::function<bool(uint16_t infoIndex, Enemy& enemy)> monsterSpawner;
	std::vector<bool> triggeredEnemyGroups;

	// A monster's shot, one of the original's 8 projectile slots. Units as in
	// Enemy::Monster (x4 = 4px columns, y = px, speeds per tick).
	struct EnemyProjectile {
		int x4 { 0 };
		int y { 0 };
		bool facingLeft { false };
		int delay { 0 };            // ticks before it exists on screen (the shooter's shoot-animation length)
		int frame { 0 };            // current sprite frame; advances one per tick up to frameEnd
		int frameEnd { 0 };
		int hspeed4 { 0 };
		int vspeed { 0 };
		bool homing { false };
		bool wobbly { false };
		int width4 { 0 };
		int height { 0 };
		int frameBase { 0 };        // sprite frame index of frames[0]
		std::vector<std::shared_ptr<Texture>> frames;
	};
	std::vector<EnemyProjectile> enemyProjectiles;
	// Cells (row*240+col) waiting for a free monster slot - the original's
	// 32-entry pending list.
	std::vector<uint16_t> pendingMonsterOffsets;
	float monsterTickAccumulator { 0.0f };
	// Length of one original game frame in dt units (dt = 1/62.5 s): the
	// original waits 7 of its 140 Hz timer ticks per frame at medium speed.
	float frameDt { 62.5f * 7.0f / 140.0f };
	int frameTicks { 7 };            // timer ticks per game frame (DS:14f8 table entry)
	uint32_t levelTicks { 0 };       // 140 Hz ticks since the level started (7908 - 7904)
	std::vector<bool> activatedSwitches;    // switch group done (the original blanks its 4 lever offsets)
	// The original's pending tile-change buffer (tile_change_buf_ptr_q): per
	// cell 0 = nothing, 1 = clear the foreground tile, 2 = copy the hidden
	// layer's tile in. Applied on screen over tileChangeTicks frames.
	std::vector<uint8_t> tileChangeBuffer;
	int tileChangeTicks { 0 };              // ce76
	int lookUpFrames { 0 };                 // iRam0003a466 - frames Up has been held (capped at 10)
	int lookDownFrames { 0 };               // iRam0003a464
	Tick lastShotTick {};
	bool levelComplete { false };
	int levelEndTicks { 0 };        // iRam0003a39a - celebration frames after the last crystal (90)
	int deathTicks { 0 };           // ce8c - the death sequence: 90 frames of puffs and twinks, then the tally
	int crystalFlashTicks { 0 };    // ce78 - 8 frames of a red fill instead of the map after a crystal
	int superJumpTwinkTicks { 0 };  // cf0e - twinks around Hocus while he holds a Super jump, and 25 frames after
	bool laserBlinkOn { false };    // ce98 - the laser charge icons show on alternate 5-frame halves
	int laserBlinkTimer { 0 };      // ce96
	int treasuresTotal { 0 };       // ce68 - score items in the level
	int treasuresFound { 0 };       // ce6a
	int treasureBlink { 0 };        // cfd8 - once every treasure is found the HUD level number blinks (1..20 cycle)

	struct Elevator {
		int x; // left column; right column is always x+1
		int gridY; // which row currently holds the rendered/collidable tile
		float pixelY; // smooth position (top-of-car, px) - what Hocus actually tracks
		// Tracks whether Hocus was standing on this car as of the previous
		// updateElevators() call, so a false->true transition (a fresh mount)
		// can be detected and his position snapped to the exact correct
		// offset - see the comment at that snap for why it's needed.
		bool hocusWasOnCar { false };
	};
	std::vector<Elevator> elevators;
	int elevatorInput { 0 };
	// Set each frame by updateElevators() when Hocus is actively being carried
	// by a car, so move() (called right after) knows to skip its own
	// collision/step-up logic that frame rather than fight over his position -
	// see the comment above move()'s early-out in game.cpp for why.
	bool ridingElevator { false };
	// Index into elevators of the car currently being ridden, or -1. render()
	// uses this to draw that one car as a smoothly-floating sprite instead of
	// the grid's own whole-tile-snapped rendering - see the comment in
	// render() for why the grid rendering alone looks disconnected from Hocus.
	int ridingElevatorIndex { -1 };
	// Consecutive frames elevatorInput has read 0. Switching from holding Up
	// to holding Down (or vice versa) reliably produces one frame where it
	// reads 0 in between - normal human key timing, not a real release - so
	// the fall-safety check below only acts once this has been 0 for several
	// frames running, not on every single zero reading.
	int elevatorInputZeroFrames { 0 };


	// Hocus and camera, one call per 20 Hz game frame - see the block comment
	// above hocusTick() in game.cpp.
	void hocusTick();
	void cameraTick(bool snap);
	int cameraXHalf { 0 };          // ce80 - camera left edge in half-tiles
	int cameraRow { 0 };            // camera_row
	int cameraRowOffset { 5 };      // cf36 - Hocus's row on screen (PgUp/PgDn shift it in the original)
	bool cameraScrolledVertically { false }; // cf0a - jumping is blocked while set
	// Monster simulation - see the block comment above monsterTick() in game.cpp.
	void monsterTick();
	void spawnPendingMonsters();
	bool spawnMonster(uint16_t tileOffset);
	bool spawnMonsterAt(int infoIndex, int x4, int y, uint16_t tileOffset);
	int bossPhase { 0 };            // cf46 - end boss phase (behaviour 99), 0..3
	int bossRespawnPhase { -1 };    // set by the boss tick; the respawn happens after the monster loop
	void monsterBehaviourTick(Enemy& enemy);
	void monsterShoot(Enemy& enemy);
	bool fireMonsterProjectile(const Enemy& enemy, int x4, int y, bool horizontal, int delay, int hocusX4);
	void enemyProjectileTick();
	[[nodiscard]] bool monsterBoxTouchesSolid(const Enemy::Monster& m);
	[[nodiscard]] bool isSolidCell(int col, int row);
	[[nodiscard]] int dosDifficultyIndex() const;
	void killMonster(Enemy& enemy, int puffs = 3);
	void checkItems();
	void hocusShotTick();
	void checkMonsterTriggers();
	void checkSwitches();
	void checkTeleports();
	void checkWallTriggers();
	void queueTileChange(uint16_t upperLeftX, uint16_t upperLeftY, uint16_t lowerRightX, uint16_t lowerRightY, bool insert);
	void tileChangeTick();
	void initElevators();
	void updateElevators(float dt);
	void updateEnemies(float dt);
	bool isShowingHint() const;
};

}

#endif // GAME_H
