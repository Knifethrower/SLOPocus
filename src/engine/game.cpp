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

#include <iostream>
#include <cmath>
#include <algorithm>
#include <cstdlib>
#include "game.h"
#include "log.h"
#include "texturetools.h"
#include "../exedata.h"

using namespace pocus;

namespace {
	int dosRandom(int n);
	int damageForDifficulty(int difficulty);
	int bossPhaseX(int phase);
	int bossPhaseY(int phase);
	int bossPhaseFacingLeft(int phase);
	int bossPhaseThreshold(int phase);
	int bossPhaseHealth(int phase);
}

Game::Game() {
	this->triggeredEnemyGroups.assign(data::asset::EnemyTrigger::ENEMIES, false);
	this->activatedSwitches.assign(data::asset::SwitchCoordinates::SWITCHES, false);
	this->tileChangeBuffer.assign(MAP_WIDTH * MAP_HEIGHT, 0);
}

void Game::setRules(const Rules& rules) {
	this->rules = rules;
}

void Game::setFrameTicks(int ticks) {
	if (ticks < 1) {
		ticks = 1;
	}
	this->frameDt = 62.5f * (float)ticks / 140.0f;
	this->frameTicks = ticks;
}

void Game::setMonsterSpawner(std::function<bool(uint16_t, Enemy&)> spawner) {
	this->monsterSpawner = std::move(spawner);
}

Map& Game::getMap() {
	return this->map;
}

Player& Game::getPlayer() {
	return this->player;
}

Hud& Game::getHud() {
	return this->hud;
}

Point& Game::getOffset() {
	return this->offset;
}

Hocus& Game::getHocus() {
	return this->hocus;
}

Size& Game::getViewportSize() {
	return this->viewportSize;
}

data::asset::ItemInfo& Game::getItemInfo() {
	return this->itemInfo;
}

std::vector<Enemy>& Game::getEnemies() {
	return this->enemies;
}

static bool rectsOverlap(const Rect& a, const Rect& b) {
	return a.getPosition().getX() < b.getPosition().getX() + b.getSize().getWidth() &&
		a.getPosition().getX() + a.getSize().getWidth() > b.getPosition().getX() &&
		a.getPosition().getY() < b.getPosition().getY() + b.getSize().getHeight() &&
		a.getPosition().getY() + a.getSize().getHeight() > b.getPosition().getY();
}

void Game::setTagTexture(int row, int frame, std::unique_ptr<Texture> texture) {
	if (row >= 0 && row < 2 && frame >= 0 && frame < 5) {
		this->tagTextures[row][frame] = std::move(texture);
	}
}

void Game::setTwinkTexture(int row, int frame, std::unique_ptr<Texture> texture) {
	if (row >= 0 && row < 2 && frame >= 0 && frame < 5) {
		this->twinkTextures[row][frame] = std::move(texture);
	}
}

void Game::play(std::unique_ptr<Sound>& sound) {
	if (sound) {
		sound->play();
	}
}

std::vector<std::unique_ptr<Texture>>& Game::getHintTextures() {
	return this->hintTextures;
}

std::unique_ptr<Sound>& Game::getSoundHint() {
	return this->soundHint;
}

std::vector<std::unique_ptr<Sound>>& Game::getSoundsItem() {
	return this->soundsItem;
}

std::unique_ptr<Sound>& Game::getSoundCrystal() {
	return this->soundCrystal;
}

std::unique_ptr<Sound>& Game::getSoundSpecialItem() {
	return this->soundSpecialItem;
}

std::unique_ptr<Sound>& Game::getSoundPotion() {
	return this->soundPotion;
}

std::unique_ptr<Sound>& Game::getSoundHit() {
	return this->soundHit;
}

std::unique_ptr<Sound>& Game::getSoundKill() {
	return this->soundKill;
}

std::unique_ptr<Sound>& Game::getSoundShot() {
	return this->soundShot;
}

std::unique_ptr<Sound>& Game::getSoundEnemyShot() {
	return this->soundEnemyShot;
}

std::unique_ptr<Sound>& Game::getSoundSwitchRefused() {
	return this->soundSwitchRefused;
}

std::unique_ptr<Sound>& Game::getSoundReveal() {
	return this->soundReveal;
}

std::unique_ptr<Sound>& Game::getSoundMonsterShot() {
	return this->soundMonsterShot;
}

std::unique_ptr<Sound>& Game::getSoundJump() {
	return this->soundJump;
}

void Game::start() {
	this->hud.updateScore(this->player.getScore());
	this->hud.updateCrystals(this->player.getCrystals(), this->map.getCrystals());
	this->hud.updateHealth(this->player.getHealth());
	this->hud.updateLevel(this->player.getLevel());


	this->tickStart = getNow();
	this->map.start();
	initElevators();

	// ce68: the treasures (items with a score) in the level, for the tally.
	this->treasuresTotal = 0;
	for (int row = 0; row < MAP_HEIGHT; row++) {
		for (int col = 0; col < MAP_WIDTH; col++) {
			const uint16_t event = (uint16_t)this->map.getEvent(Point((float)col, (float)row));
			if (event != data::asset::EventLayer::EMPTY && event < 0x17 && this->itemInfo.getItems()[event].score != 0) {
				this->treasuresTotal++;
			}
		}
	}

	this->hocus.syncFromRect();
	this->hocus.dos = Hocus::Dos {};
	this->hocus.syncFromRect();
	this->monsterTickAccumulator = 0.0f;
	cameraTick(true);
}

void Game::render(Renderer &renderer) {
	// The elevator car currently being ridden needs to render at a smooth,
	// continuous Y position matching Hocus, not the grid's own whole-tile-
	// snapped one - otherwise the car visibly jumps in 16px increments while
	// Hocus glides continuously each frame, and the two read as separate
	// objects moving near each other rather than one carrying the other.
	// Temporarily hide the grid's own copy for this one frame's map.render()
	// pass below and draw a floating copy afterward instead, reusing the same
	// tile Animation the grid would have drawn (kept in sync by
	// updateElevators()'s existing setTile() calls whenever the car crosses a
	// row boundary). Toggling visibility here and restoring it before this
	// function returns is safe because render() runs once per frame after all
	// of this frame's update()/collision logic has already completed - the
	// flip never leaks into anything that reads isVisible() for gameplay.
	Elevator* ridingCar = nullptr;
	int ridingCarGridY = -1;
	if (this->ridingElevator && this->ridingElevatorIndex >= 0 &&
			this->ridingElevatorIndex < (int)this->elevators.size()) {
		ridingCar = &this->elevators[this->ridingElevatorIndex];
		ridingCarGridY = ridingCar->gridY;
		this->map.getLayer(1).getTile(ridingCar->x, ridingCarGridY).setVisible(false);
		this->map.getLayer(1).getTile(ridingCar->x + 1, ridingCarGridY).setVisible(false);
	}

	if (this->crystalFlashTicks > 0) {
		// A crystal's flash (ce78): for 8 frames the play area is filled with
		// palette 119 - ce78 (a dark red, then bright red fading) instead of
		// the map; everything else is drawn on top as usual.
		renderer.drawRect(Rect(Point(0, 0), Size(320, 160)), paletteColor(this->palette, (uint8_t)(119 - this->crystalFlashTicks)));
	}
	else {
		this->map.render(renderer, this->offset);
	}

	if (ridingCar) {
		Tile& carLeft = this->map.getLayer(1).getTile(ridingCar->x, ridingCarGridY);
		Tile& carRight = this->map.getLayer(1).getTile(ridingCar->x + 1, ridingCarGridY);
		carLeft.setVisible(true);
		carRight.setVisible(true);

		carLeft.getAnimation().render(renderer, Point(
			ridingCar->x * TILE_SIZE - this->offset.getX(),
			ridingCar->pixelY - this->offset.getY()
		));
		carRight.getAnimation().render(renderer, Point(
			(ridingCar->x + 1) * TILE_SIZE - this->offset.getX(),
			ridingCar->pixelY - this->offset.getY()
		));
	}

	for (auto& enemy : this->enemies) {
		// Not drawn while materialising (the original only shows twinks for
		// those 20 ticks); the hit flash (c014) draws the red silhouette on
		// its odd frames (1ba5:1c12).
		if (enemy.monster.spawnTicks > 0) {
			continue;
		}
		enemy.render(renderer, this->offset, (enemy.monster.hitFlash % 2) == 1);
	}

	for (const auto& shot : this->hocusShots) {
		const Hocus::ShotVariant variant = shot.vertical
			? (shot.piercing ? Hocus::SHOT_LASER_VERTICAL : Hocus::SHOT_VERTICAL)
			: (shot.piercing ? Hocus::SHOT_LASER : Hocus::SHOT_HORIZONTAL);
		renderer.drawTexture(this->hocus.getProjectileTexture(variant, shot.facingLeft ? Entity::LEFT : Entity::RIGHT),
			Point((float)(shot.x4 * 4) - this->offset.getX(), (float)shot.y - this->offset.getY()));
	}

	for (const auto& shot : this->enemyProjectiles) {
		if (shot.delay > 0 || shot.frames.empty()) {
			continue;
		}
		const int index = std::min<int>(std::max(shot.frame - shot.frameBase, 0), (int)shot.frames.size() - 1);
		renderer.drawTexture(*shot.frames[index], Point(
			(float)(shot.x4 * 4) - this->offset.getX(),
			(float)shot.y - this->offset.getY()));
	}

	// Hocus (level_run's draw block): not drawn at all while invisible -
	// which the Invisible potion, the level-end celebration and the death
	// sequence all set (ce8e) - the morph instead while teleporting, and one
	// of the flash silhouettes on the odd frames of the hurt cooldown / the
	// Super shot. The laser charges stack above him.
	if (this->player.effects.invisibilityTicks == 0) {
		const Hocus::Dos& d = this->hocus.dos;
		if (d.scriptedWalkTimer != 0) {
			this->hocus.renderMorph(renderer, this->offset);
		}
		else {
			Hocus::Flash flash = Hocus::FLASH_NONE;
			if (this->hurtCooldownTicks % 2 == 1) {
				flash = Hocus::FLASH_HURT;
			}
			else if (this->player.effects.superShotTicks % 2 == 1) {
				flash = Hocus::FLASH_SUPER_SHOT;
			}
			this->hocus.render(renderer, this->offset, flash);
		}
		if (this->player.effects.laserShots > 0 && this->laserBlinkOn) {
			this->hocus.renderLaserCharge(renderer, this->offset, this->player.effects.laserShots);
		}
	}

	renderEffects(renderer);

	this->hud.render(renderer);

	if (this->paused) {
		if (isShowingHint()) {
			Texture& hintTexture = *this->hintTextures[this->currentHint];
			renderer.drawTexture(hintTexture,
								 Point(
									 this->viewportSize.getWidth() / 2 - hintTexture.getWidth() / 2,
									 (this->viewportSize.getHeight() - this->hud.getBackground().getHeight()) / 2 -
										 hintTexture.getHeight() / 2
								 )
			);
		}
		// The pause text itself is StateGame's overlay (1ba5:3751).
	}
}

void Game::update(float dt) {
	if (this->paused) {
		return;
	}

	this->map.update(dt);

	updateElevators(dt);

	// Hocus, his shots, monsters and the camera run on the original's fixed
	// 20 Hz game frame, independent of the render rate. Capped so a stall
	// doesn't turn into a burst of catch-up ticks.
	this->monsterTickAccumulator += dt;
	int ticks = 0;
	while (this->monsterTickAccumulator >= this->frameDt && ticks < 4) {
		this->monsterTickAccumulator -= this->frameDt;
		this->levelTicks += (uint32_t)this->frameTicks;
		hocusTick();
		hocusShotTick();
		checkItems();
		checkWallTriggers();
		checkMonsterTriggers();
		checkTeleports();
		monsterTick();
		tileChangeTick();
		this->map.animateTick(this->cameraXHalf / 2, this->cameraRow);
		cameraTick(false);
		effectsTick();
		ticks++;
	}
	if (this->monsterTickAccumulator >= this->frameDt) {
		this->monsterTickAccumulator = 0.0f;
	}

	updateEnemies(dt);
}

// ---------------------------------------------------------------------------
// Screen effects, reverse engineered from HOCUS.EXE (the notes, "Effects").
// effectsTick() runs once per game frame at the end of the tick, doing what
// the original's draw pass does: it records each effect's drawing and then
// steps it, so a fresh effect draws its first frame in the frame it was
// spawned in and each one lasts exactly its original number of frames.

void Game::spawnTag(int x4, int y, int type) {
	// spawn_effect_q (1ba5:3684): the first free of 10 slots; types 0..4 are
	// the score tags (row 0), 5..9 the pickup icons (row 1).
	for (Tag& tag : this->tags) {
		if (tag.timer != 0) {
			continue;
		}
		tag.x4 = x4;
		tag.y = y;
		tag.row = type < 5 ? 0 : 1;
		tag.frame = type < 5 ? type : type - 5;
		tag.timer = 17;
		return;
	}
}

void Game::spawnTwink(int x4, int y) {
	// 1ba5:2174: the next of 8 slots round-robin (an older twink is replaced).
	Twink& twink = this->twinks[this->nextTwink];
	twink.x4 = x4;
	twink.y = y;
	twink.timer = 9;
	twink.row = dosRandom(2);
	this->nextTwink = (this->nextTwink + 1) % 8;
}

void Game::spawnPuff(int x, int y) {
	// 1ba5:2f1c: the next of 8 groups round-robin; 16 pixels thrown up.
	Puff& puff = this->puffs[this->nextPuff];
	puff.life = 16;
	for (PuffPixel& pixel : puff.pixels) {
		pixel.x = x;
		pixel.y = y;
		pixel.vx = dosRandom(10) - dosRandom(10);
		pixel.vy = -(dosRandom(7) + 5);
		pixel.color = dosRandom(2) == 0 ? 0x70 : 0x60;
	}
	this->nextPuff = (this->nextPuff + 1) % 8;
}

void Game::spawnTrail(int x4, int y) {
	// 1ba5:21c6: the slot index advances on every call; only a free slot
	// takes the trail.
	Trail& trail = this->trails[this->nextTrail];
	if (!trail.live) {
		trail.live = true;
		trail.x4 = x4;
		trail.y = y;
		trail.age = 0;
	}
	this->nextTrail = (this->nextTrail + 1) % 20;
}

void Game::effectsTick() {
	this->effectDraws.clear();

	// Puffs (1ba5:2ff3): move, gravity, then draw. (The colour step in that
	// routine tests `!life % 4`, which never triggers - the colour stays.)
	for (Puff& puff : this->puffs) {
		if (puff.life <= 0) {
			continue;
		}
		for (PuffPixel& pixel : puff.pixels) {
			pixel.y += pixel.vy;
			pixel.vy += 1;
			pixel.x += pixel.vx;
			this->effectDraws.push_back({ nullptr, paletteColor(this->palette, pixel.color), pixel.x, pixel.y });
		}
		puff.life--;
	}

	// Tags (1ba5:36f0): draw, then rise 2 px, for 17 frames.
	for (Tag& tag : this->tags) {
		if (tag.timer == 0) {
			continue;
		}
		this->effectDraws.push_back({ this->tagTextures[tag.row][tag.frame].get(), Color {}, tag.x4 * 4, tag.y });
		tag.y -= 2;
		tag.timer--;
	}

	// Trails (1ba5:20b1): a random pixel of the column, drawn 1 frame in 2 on
	// average, then sinking 1 px; gone after 16 frames.
	for (Trail& trail : this->trails) {
		if (!trail.live) {
			continue;
		}
		const int x = trail.x4 * 4 + dosRandom(4);
		if (dosRandom(2) == 0) {
			this->effectDraws.push_back({ nullptr, paletteColor(this->palette, (uint8_t)(0x78 + trail.age / 4)), x, trail.y });
		}
		trail.y += 1;
		trail.age += 1;
		if (trail.age > 15) {
			trail.live = false;
		}
	}

	// Twinks (the draw loop at 1ba5:1fae): timer 9..1 shows cells 0 1 2 3 4 3 2 1 0.
	for (Twink& twink : this->twinks) {
		if (twink.timer == 0) {
			continue;
		}
		const int t = twink.timer;
		const int frame = (t == 9 || t == 1) ? 0 : (t == 8 || t == 2) ? 1 : (t == 7 || t == 3) ? 2 : (t == 6 || t == 4) ? 3 : 4;
		this->effectDraws.push_back({ this->twinkTextures[twink.row][frame].get(), Color {}, twink.x4 * 4, twink.y });
		twink.timer--;
	}

	// The crystal flash counts down in the draw pass too (1ba5:220f).
	if (this->crystalFlashTicks > 0) {
		this->crystalFlashTicks--;
	}

	// The HUD level number (348d): once every treasure is found it blinks -
	// blank for cfd8 1..9, shown for 10..20, wrapping to 1.
	if (this->treasureBlink != 0) {
		this->hud.setLevelVisible(this->treasureBlink >= 10);
		this->treasureBlink++;
		if (this->treasureBlink > 20) {
			this->treasureBlink = 1;
		}
	}
}

void Game::renderEffects(Renderer& renderer) {
	for (const EffectDraw& draw : this->effectDraws) {
		const Point position((float)draw.x - this->offset.getX(), (float)draw.y - this->offset.getY());
		if (draw.texture) {
			renderer.drawTexture(*draw.texture, position);
		}
		else if (position.getX() >= 0 && position.getX() < 320 && position.getY() >= 0 && position.getY() < 160) {
			renderer.drawPoint(position, draw.color);
		}
	}
}

void Game::clearEffects() {
	for (Tag& tag : this->tags) {
		tag.timer = 0;
	}
	for (Twink& twink : this->twinks) {
		twink.timer = 0;
	}
	for (Puff& puff : this->puffs) {
		puff.life = 0;
	}
	for (Trail& trail : this->trails) {
		trail.live = false;
	}
	this->nextTwink = 0;
	this->nextPuff = 0;
	this->nextTrail = 0;
	this->effectDraws.clear();
}

/*
uint32_t Game::getElapsedTime() {
	return ::getElapsedTime(this->tickStart) / 1000;
}
*/

void Game::addScore(uint32_t score) {
	this->player.setScore(this->player.getScore() + score);
	this->hud.updateScore(this->player.getScore());
}

void Game::addCrystal(uint32_t amount) {
	this->player.setCrystals(this->player.getCrystals() + amount);
	this->hud.updateCrystals(this->player.getCrystals(), this->map.getCrystals());
	this->crystalFlashTicks = 8;

	// The original (level_run): the last crystal starts a 90-frame
	// celebration - Hocus is made invulnerable, sparkles burst around him,
	// jumping/falling stop (he can still walk) - and then the level ends.
	if (this->player.getCrystals() >= this->map.getCrystals() && this->levelEndTicks == 0 && !this->levelComplete) {
		this->levelEndTicks = 90;
		this->player.effects.invisibilityTicks = 30000;
	}
}

bool Game::isLevelComplete() const {
	return this->levelComplete;
}

bool Game::isPlayerDead() const {
	// The original checks `ce8c == 1` at the top of its loop: the death
	// sequence (hocusTick) has run its course.
	return this->deathTicks == 1;
}

void Game::resetForNewLevel() {
	this->enemies.clear();
	this->hocusShots.clear();
	this->hurtCooldownTicks = 0;
	// The original resets fire power to 1 and drops laser shots / timed
	// effects at every level start (level_run's init).
	this->player.setFirePower(1);
	this->player.effects = Player::Effects {};
	this->enemyProjectiles.clear();
	clearEffects();
	this->deathTicks = 0;
	this->crystalFlashTicks = 0;
	this->superJumpTwinkTicks = 0;
	this->laserBlinkOn = false;
	this->laserBlinkTimer = 0;
	this->treasuresFound = 0;
	this->treasureBlink = 0;
	this->hud.setLevelVisible(true);
	this->pendingMonsterOffsets.clear();
	this->monsterTickAccumulator = 0.0f;
	this->triggeredEnemyGroups.assign(data::asset::EnemyTrigger::ENEMIES, false);
	this->activatedSwitches.assign(data::asset::SwitchCoordinates::SWITCHES, false);
	this->tileChangeBuffer.assign(MAP_WIDTH * MAP_HEIGHT, 0);
	this->tileChangeTicks = 0;
	this->lookUpFrames = 0;
	this->lookDownFrames = 0;
	this->cameraRowOffset = 5;

	this->player.setCrystals(0);
	this->player.setSilverKey(false);
	this->player.setGoldKey(false);

	this->levelComplete = false;
	this->levelEndTicks = 0;
	this->levelTicks = 0;
	this->bossPhase = 0;
	this->bossRespawnPhase = -1;
}

void Game::removeHealth(uint8_t health) {
	if (this->player.getHealth() - health < 0) {
		this->player.setHealth(0);
	}
	else {
		this->player.setHealth(this->player.getHealth() - health);
	}

	this->hud.updateHealth(this->player.getHealth());
}

void Game::addHealth(uint8_t health) {
	if (this->player.getHealth() + health > PLAYER_MAX_HEALTH) {
		this->player.setHealth(PLAYER_MAX_HEALTH);
	}
	else {
		this->player.setHealth(this->player.getHealth() + health);
	}

	this->hud.updateHealth(this->player.getHealth());
}

void Game::addSilverKey() {
	this->player.setSilverKey(true);
	this->hud.updateKeys(this->player.hasSilverKey(), this->player.hasGoldenKey());
}

void Game::removeSilverKey(){
	this->player.setSilverKey(false);
	this->hud.updateKeys(this->player.hasSilverKey(), this->player.hasGoldenKey());
}

void Game::addGoldenKey(){
	this->player.setGoldKey(true);
	this->hud.updateKeys(this->player.hasSilverKey(), this->player.hasGoldenKey());
}

void Game::removeGoldenKey(){
	this->player.setGoldKey(false);
	this->hud.updateKeys(this->player.hasSilverKey(), this->player.hasGoldenKey());
}

void Game::togglePause() {
	this->paused = !this->paused;
}

data::asset::Palette& Game::getPalette() {
	return this->palette;
}

data::asset::Font& Game::getFont() {
	return this->font;
}

void Game::setTextColor(uint8_t color) {
	this->textColor = color;
}

void Game::startMovement(const Entity::Direction_t& direction) {
	if (this->paused) {
		if (isShowingHint()) {
			hideHint();
		}

		return;
	}

	this->hocus.startMovement(direction);
}

void Game::stopMovement(const Entity::Direction_t& direction) {
	if (this->paused) {
		if (isShowingHint()) {
			hideHint();
		}

		return;
	}

	this->hocus.stopMovement(direction);
}

void Game::jump() {
	if (this->paused) {
		if (isShowingHint()) {
			hideHint();
		}

		return;
	}

	// Edge-triggered like the original's key_jump_edge; hocusTick() decides
	// whether a jump can actually start this frame.
	this->hocus.dos.jumpEdge = true;
}

void Game::setScrollInput(int direction) {
	this->hocus.dos.keyScrollUp = direction > 0;
	this->hocus.dos.keyScrollDown = direction < 0;
}

void Game::setElevatorInput(int direction) {
	this->elevatorInput = direction;
	this->hocus.dos.keyUp = direction > 0;
	this->hocus.dos.keyDown = direction < 0;

	if (direction != 0 && !this->elevators.empty()) {
		const Point headTile = this->hocus.getTilePosition();
		const int feetX = (int)headTile.getX();
		const int feetY1 = (int)headTile.getY() + 1;
		const int feetY2 = (int)headTile.getY() + 2;

		const Elevator* nearest = nullptr;
		int nearestDist = 999999;
		for (const auto& elevator : this->elevators) {
			const int currentGridY = elevator.gridY;
			const int dist = std::abs(feetX - elevator.x) + std::abs(feetY1 - currentGridY);
			if (dist < nearestDist) {
				nearestDist = dist;
				nearest = &elevator;
			}
		}

		LOGI << "Game: elevator input=" << direction << " hocus feetX=" << feetX << " feetY1=" << feetY1
			<< " feetY2=" << feetY2 << " nearest car at (" << nearest->x << ","
			<< nearest->gridY << ") dist=" << nearestDist;
	}
}

void Game::initElevators() {
	// Re-scanned fresh every level start/respawn (loadLevel() rebuilds the
	// collision layer from the level's own data each time), so no explicit
	// reset is needed elsewhere - stale entries just don't survive a reload.
	this->elevators.clear();

	const int16_t leftId = this->map.getElevatorLeftTile();
	if (leftId < 0) {
		return;
	}

	for (int y = 0; y < MAP_HEIGHT; y++) {
		for (int x = 0; x < MAP_WIDTH - 1; x++) {
			if (this->map.getLayer(1).getTile(x, y).getId() == (uint16_t)leftId) {
				this->elevators.push_back({x, y, (float)(y * TILE_SIZE)});
			}
		}
	}

	LOGI << "Game: found " << this->elevators.size() << " elevator car(s), leftTile=" << leftId
		<< " rightTile=" << this->map.getElevatorRightTile();
}

void Game::updateElevators(float dt) {
	if (this->elevators.empty()) {
		return;
	}

	// Manual: "Up or Down Arrow Keys - Moves elevators..." while standing on
	// one. Entity::getTilePosition() is head-level, not feet - confirmed by
	// direct testing that +1 (the offset checkSwitches()/checkTeleports()/etc.
	// use, tolerant of a 1-tile margin since those targets are message/switch
	// tiles that can be reached from a couple of rows) undershot a real
	// elevator car by a full row. Hocus's sprite is 2 tiles tall, so his feet
	// land on head+2; check both +1 and +2 to be safe rather than assume this
	// holds for every level's exact standing alignment.
	const Point headTile = this->hocus.getTilePosition();
	const int feetX = (int)headTile.getX();
	const int feetY1 = (int)headTile.getY() + 1;
	const int feetY2 = (int)headTile.getY() + 2;

	bool hocusOnAnyCar = false;
	this->ridingElevator = false;
	this->ridingElevatorIndex = -1;

	if (this->elevatorInput == 0) {
		this->elevatorInputZeroFrames++;
	}
	else {
		this->elevatorInputZeroFrames = 0;
	}

	// Whether Hocus counts as "on a car" - and therefore gets the floating-
	// sprite render treatment in Game::render(), see the comment there - is
	// purely about his position relative to a car, independent of whether
	// elevatorInput is currently held. Gating this whole detection behind
	// elevatorInput!=0 (an earlier version did) meant that the instant you
	// let go of Up/Down after riding to a stop, render() fell back to the
	// grid's own tile - which sits at a tile-aligned position that generally
	// doesn't exactly match wherever the car's un-aligned float position
	// froze - producing a visible gap between his feet and the car until you
	// pressed the button again and the floating render (which does match)
	// re-engaged, which read as the car "jumping up to meet his feet."
	// Actual movement is still fully gated on elevatorInput below.
	for (size_t i = 0; i < this->elevators.size(); i++) {
		Elevator& elevator = this->elevators[i];
		// gridY is the row holding the car's collision tiles (the row containing
		// its top edge), which is exactly Hocus's feet tile while he rides - his
		// y is pixelY minus his height. Deriving this from pixelY with rounding
		// instead disagreed with that by one row for half of every tile of
		// travel, which is what made a descent lose him on its first frame.
		const int currentGridY = elevator.gridY;
		const bool hocusOnCar = (currentGridY == feetY1 || currentGridY == feetY2) &&
			(feetX == elevator.x || feetX == elevator.x + 1);
		const bool justMounted = hocusOnCar && !elevator.hocusWasOnCar;
		elevator.hocusWasOnCar = hocusOnCar;
		if (!hocusOnCar) {
			continue;
		}
		hocusOnAnyCar = true;
		this->ridingElevator = true;
		this->ridingElevatorIndex = (int)i;

		if (justMounted) {
			// Snap to the exact correct offset (feet flush with the car's top
			// edge, confirmed against the real DOS game) regardless of how he
			// got here - falling onto the car from above vs. stepping up onto
			// it from the side use different position-setting formulas in
			// Game::move() that don't always land at exactly the same
			// sub-pixel offset, and whatever offset exists at mount time is
			// what the rest of the ride preserves unchanged from here on.
			const float correctY = elevator.pixelY - this->hocus.getRect().getSize().getHeight();
			this->hocus.setPosition(Point(this->hocus.getRect().getPosition().getX(), correctY));
		}

		if (this->elevatorInput != 0) {
			const uint16_t leftId = (uint16_t)this->map.getElevatorLeftTile();
			const uint16_t rightId = (uint16_t)this->map.getElevatorRightTile();

			const float previousCarPixelY = elevator.pixelY;
			const bool movingUp = this->elevatorInput > 0;
			// One tile per original game frame.
			float targetPixelY = elevator.pixelY + (movingUp ? -1.0f : 1.0f) * ((float)TILE_SIZE / this->frameDt) * dt;

			// Straight from the DOS original - HOCUS.EXE's per-tick player
			// update (level_run, the block comparing the
			// tile under Hocus's feet against elevator_left_tile/right_tile).
			// The car moves one whole row per tick while Up/Down is held, and
			// the only clearance test is:
			//   up:   both car-width cells of the row Hocus's HEAD is about to
			//         enter (car row - 3 = new car row - 2) must be empty;
			//   down: both car-width cells of the row the CAR is about to
			//         enter (car row + 1) must be empty.
			// Nothing else - the rows in between are Hocus's own body.
			//
			// The port moves in pixels, so: elevator.gridY is the row holding
			// the collision tiles = the row containing the car's TOP edge
			// (floor(pixelY/16)), which is also Hocus's feet tile. A row is
			// tested before any pixel of the car reaches it - going up, the rows
			// the top edge is about to enter; going down, the rows the BOTTOM
			// edge (pixelY+16) is about to enter - and a blocked car snaps flush
			// against the obstacle instead of freezing wherever it happened to
			// be (which used to leave it up to half a tile off-grid).
			const auto isSolid = [&](int checkRow) -> bool {
				if (checkRow < 0 || checkRow >= MAP_HEIGHT) {
					return true;
				}
				return this->map.getLayer(1).getTile(elevator.x, checkRow).isVisible() ||
					this->map.getLayer(1).getTile(elevator.x + 1, checkRow).isVisible();
			};

			int finalGridY = elevator.gridY;
			if (movingUp) {
				const int topRow = (int)std::floor(targetPixelY / TILE_SIZE);
				for (int row = elevator.gridY - 1; row >= topRow; row--) {
					if (isSolid(row - 2)) {
						targetPixelY = (float)((row + 1) * TILE_SIZE);
						break;
					}
					finalGridY = row;
				}
			}
			else {
				const int bottomRow = (int)std::floor((targetPixelY + TILE_SIZE - 1) / TILE_SIZE);
				for (int row = elevator.gridY + 1; row <= bottomRow; row++) {
					if (isSolid(row)) {
						targetPixelY = (float)((row - 1) * TILE_SIZE);
						break;
					}
				}
				finalGridY = (int)std::floor(targetPixelY / TILE_SIZE);
			}

			if (finalGridY != elevator.gridY) {
				this->map.setTile(1, Point((float)elevator.x, (float)elevator.gridY), data::asset::MapLayer::TILE_EMPTY);
				this->map.setTile(1, Point((float)(elevator.x + 1), (float)elevator.gridY), data::asset::MapLayer::TILE_EMPTY);
				this->map.setTile(1, Point((float)elevator.x, (float)finalGridY), leftId);
				this->map.setTile(1, Point((float)(elevator.x + 1), (float)finalGridY), rightId);
				elevator.gridY = finalGridY;
			}

			elevator.pixelY = targetPixelY;

			// Move Hocus by exactly the amount the car actually moved this frame
			// (not the raw, pre-clamp delta - if the row-by-row check above
			// stopped the car short of its target, he should stop with it)
			// rather than recomputing his position from a formula. Whatever
			// offset exists between him and the car right now - normally set by
			// the ordinary ground-landing code the instant before he started
			// riding, so it's already correct - simply carries forward
			// unchanged for the rest of the ride. This is what he asked for:
			// "stay in the standing on top position he's in before the ride
			// starts." An earlier version here derived a fresh position each
			// frame instead, which meant it couldn't preserve that pre-ride
			// position at all; it only existed to work around drift that was
			// actually caused by a since-fixed bug elsewhere (the "start
			// falling" safety net incorrectly firing mid-ride), not by this
			// simpler delta approach itself.
			const float actualDelta = targetPixelY - previousCarPixelY;
			const float newHocusY = this->hocus.getRect().getPosition().getY() + actualDelta;
			this->hocus.setPosition(Point(this->hocus.getRect().getPosition().getX(), newHocusY));

			LOGI << "Elevator ride: input=" << this->elevatorInput << " hocusY=" << newHocusY
				<< " carPixelY=" << elevator.pixelY << " diff=" << (newHocusY - elevator.pixelY);

		}

		// Only one car can be ridden at a time in practice (Hocus occupies one
		// tile row), but break anyway now that we've found the one he's on.
		break;
	}

	// Once he's off the car, hocusTick()'s own fall logic (feet cell empty ->
	// drop 16 px per frame) takes over, exactly as in the original.
	(void)hocusOnAnyCar;
}

void Game::showHint(uint32_t id) {
	this->currentHint = (int)id;
	this->paused = true;

	if (this->soundHint) {
		this->soundHint->play();
	}
}

void Game::hideHint() {
	this->currentHint = -1;
	this->paused = false;
}

void Game::activate() {
	const Point& tilePosition = this->hocus.getTilePosition();

	// Wizard notes (process_event_tiles, item type 10): on Up, a message whose
	// row is Hocus's head row and whose column is within 2 of his (x/2) shows,
	// when one of his body cells holds the note.
	const int hocusCol = this->hocus.dos.xHalf / 2;
	const bool onNote = this->map.getEvent(tilePosition) == data::asset::EventLayer::WIZARD ||
		this->map.getEvent(Point(tilePosition.getX(), tilePosition.getY() + 1)) == data::asset::EventLayer::WIZARD;
	if (onNote) {
		for (int i = 0; i < data::asset::Messages::MESSAGES; i++) {
			const data::asset::Messages::Entry& message = this->map.getMessages().getMessages()[i];
			if (message.y == (int)tilePosition.getY() && std::abs((int)message.x - hocusCol) < 3) {
				showHint(i);
				return;
			}
		}
	}

	// Manual: "Up Arrow - Talk to the Wizard, operate switches" - same button as
	// above, so a switch flips on an explicit press rather than on touch.
	checkSwitches();
}

void Game::hurt(uint8_t health, int cooldownTicks) {
	if (this->hurtCooldownTicks > 0 || this->player.effects.invisibilityTicks > 0) {
		return;
	}

	removeHealth(health);
	this->hurtCooldownTicks = cooldownTicks;

	if (this->soundHit) {
		this->soundHit->play();
	}
}

// ---------------------------------------------------------------------------
// Hocus and the camera
//
// Reverse engineered from HOCUS.EXE's level_run (1ba5:453a,
// the block from the jump start through the walk code) and the camera routine
// (1ba5:29c5). One tick per 20 Hz game frame.
// Units: x in half-tile columns (8 px), y in pixels; his body is the cells
// (row, col..col+1) and (row+1, ...), feet are row+2, col = (x+1)/2.
//
//  jump    edge-triggered when not jumping/falling and the camera didn't just
//          scroll vertically; y follows a 19-entry arc table (25 with the
//          high-jump potion, which is consumed). A solid head cell mid-arc
//          mirrors the index onto the descending half. Past the peak, a solid
//          feet cell ends the jump and snaps y to the row.
//  fall    feet cell empty and not jumping: y += 16 (and it counts as
//          "falling" once the cell below that is empty too).
//  walk    per-column clearance flags: leftOk/rightOk = both body rows clear
//          in the next column (three rows when mid-row); leftStep/rightStep =
//          the next column has a ledge at the lower body row with clearance
//          above -> move and rise one row (automatic step-up). Never while
//          jumping/falling. x moves one half-tile per frame. Left uses the
//          current column (x/2) and right the next ((x+2)/2), asymmetrically,
//          as the original does. Starting/turning left costs one frame
//          (turnLatch).
// ---------------------------------------------------------------------------

namespace {
	// The arc tables themselves (19 and 25 entries) are read from the EXE.
	constexpr int JUMP_PEAK = 9;
	constexpr int HIGH_JUMP_PEAK = 12;
	constexpr int MAP_RIGHT_LIMIT_HALF = 0x1dd;
	constexpr int CAMERA_HOCUS_X_HALF = 20;   // Hocus sits 20 half-tiles (160 px) from the left edge
	constexpr int CAMERA_MAX_X_HALF = 0x1b8;  // 480 - 40 screen half-tiles
	constexpr int CAMERA_MAX_ROW = 0x32;      // 60 - 10 screen rows
}

void Game::hocusTick() {
	Hocus::Dos& d = this->hocus.dos;
	this->hocus.syncFromRect();
	d.movedThisTick = false;

	// Per-frame timers (the original decrements these at the top of its item
	// scan and in the fire block).
	if (this->hurtCooldownTicks > 0) {
		this->hurtCooldownTicks--;
	}
	if (this->player.effects.invisibilityTicks > 0) {
		this->player.effects.invisibilityTicks--;
		if (this->player.effects.invisibilityTicks == 0) {
			play(this->soundHint);
		}
	}

	// A burst of the death and level-end sequences: a puff and a twink
	// around where he stands (level_run's ce8c / iRam0003a39a blocks).
	const auto burst = [&]() {
		const int puffY = d.yPx + dosRandom(10) + 16 - dosRandom(10);
		const int puffX = d.xHalf * 8 + dosRandom(10) + 12 - dosRandom(10);
		spawnPuff(puffX, puffY);
		const int twinkY = d.yPx + dosRandom(12) - dosRandom(12);
		const int twinkX4 = d.xHalf * 2 + dosRandom(3) - dosRandom(3);
		spawnTwink(twinkX4, twinkY);
	};

	// Death (ce8c): he vanishes (ce8e = 30000) into 90 frames of bursts with
	// the kill sound - one on the first frame, then 1 in 4 per frame until
	// the last 25 - and the tally follows.
	if (this->player.getHealth() == 0 && this->deathTicks == 0) {
		this->deathTicks = 90;
		this->player.effects.invisibilityTicks = 30000;
	}
	if (this->deathTicks > 0) {
		this->deathTicks--;
		if ((dosRandom(4) == 0 && this->deathTicks > 25) || this->deathTicks == 89) {
			burst();
			play(this->soundKill);
		}
	}

	// The level-end celebration: the same bursts, 1 in 7 per frame, with
	// the special-item sound.
	if (this->levelEndTicks > 0) {
		this->levelEndTicks--;
		if (this->levelEndTicks > 25 && dosRandom(7) == 0) {
			burst();
			play(this->soundSpecialItem);
		}
		if (this->levelEndTicks == 0) {
			this->levelComplete = true;
		}
	}

	// Teleporting (the original's scripted walk, level_run's else-branch):
	// no control; a 24-frame pause, then fly toward the target at 8 px/frame
	// horizontally and 16 px/frame vertically, then a 15-frame pause.
	if (d.scriptedWalkTimer != 0) {
		d.jumpEdge = false;
		d.fireEdge = false;
		if (d.targetXHalf == d.xHalf && d.targetYPx == d.yPx) {
			if (d.scriptedWalkTimer == 25) {
				play(this->soundHint);
			}
			if (d.scriptedWalkTimer == 40) {
				d.scriptedWalkTimer = 0;
			}
			else {
				d.scriptedWalkTimer++;
			}
		}
		else if (d.scriptedWalkTimer < 25) {
			d.scriptedWalkTimer++;
		}
		if (d.scriptedWalkTimer == 25) {
			if (d.yPx < d.targetYPx) {
				d.yPx += TILE_SIZE;
			}
			if (d.targetYPx < d.yPx) {
				d.yPx -= TILE_SIZE;
			}
			if (d.xHalf < d.targetXHalf) {
				d.xHalf++;
			}
			if (d.targetXHalf < d.xHalf) {
				d.xHalf--;
			}
		}
		this->hocus.writeRect();
		d.morphJitter = dosRandom(2) - dosRandom(2);
		return;
	}

	// The elevator owns his position while it carries him (it moves the car and
	// him together in pixels); the original's elevator code is inside this same
	// block and simply doesn't fall while standing on the car tiles.
	const bool carried = this->ridingElevator && this->elevatorInput != 0;

	const auto cellSolid = [&](int col, int row) -> bool {
		return isSolidCell(col, row);
	};
	const auto cellEmpty = [&](int col, int row) -> bool {
		if (col < 0 || col >= MAP_WIDTH || row < 0 || row >= MAP_HEIGHT) {
			return true;   // the original reads outside the map as whatever is there; treat as open
		}
		return !this->map.getLayer(1).getTile(col, row).isVisible();
	};

	int row = d.yPx / TILE_SIZE;
	const int col = (d.xHalf + 1) / 2;
	bool jumpEnded = false;

	if (!carried) {
		// Jump start
		if (d.jumpEdge && !d.jumping && !this->cameraScrolledVertically && !d.falling) {
			d.jumping = true;
			d.jumpIndex = 0;
			play(this->soundJump);
			if (!this->player.hasSuperJump()) {
				d.highJumpArc = false;
				d.jumpLength = ExeData::get().words(TBL_JUMP);
				d.jumpPeak = JUMP_PEAK;
			}
			else {
				this->player.setSuperJump(false);
				d.highJumpArc = true;
				d.jumpLength = ExeData::get().words(TBL_HIGH_JUMP);
				d.jumpPeak = HIGH_JUMP_PEAK;
				play(this->soundHint);
			}
		}
		d.jumpEdge = false;

		// Jump arc (frozen during the end-of-level celebration)
		if (d.jumping && this->levelEndTicks == 0 && this->deathTicks == 0) {
			const ExeTable table = d.highJumpArc ? TBL_HIGH_JUMP : TBL_JUMP;
			if (d.jumpIndex < d.jumpLength) {
				d.yPx += ExeData::get().word(table, d.jumpIndex);
				row = d.yPx / TILE_SIZE;
				if (cellSolid(col, row)) {
					// Hit the ceiling: continue from the mirrored (descending) index.
					d.jumpIndex = (d.jumpLength - 1) - d.jumpIndex;
					d.yPx += ExeData::get().word(table, d.jumpIndex);
					row = d.yPx / TILE_SIZE;
				}
				d.jumpIndex++;
				if (d.jumpIndex >= d.jumpLength) {
					d.jumping = false;
					jumpEnded = true;
				}
			}
			else {
				d.jumping = false;
				jumpEnded = true;
			}
		}

		// Fall / land
		const int feetRow = row + 2;
		if (cellEmpty(col, feetRow)) {
			if (!d.jumping && this->levelEndTicks == 0 && this->deathTicks == 0) {
				d.yPx += TILE_SIZE;
				if (cellEmpty(col, feetRow + 1)) {
					d.falling = true;
					d.landingSoundPending = true;
				}
			}
		}
		else {
			// Landing (sound 15): a descending jump meeting the floor, or the
			// end of a fall that had two empty rows below.
			if (d.jumping && d.jumpIndex >= d.jumpPeak) {
				d.jumping = false;
				d.yPx = row * TILE_SIZE;
				play(this->soundJump);
			}
			d.falling = false;
			if (d.landingSoundPending) {
				d.landingSoundPending = false;
				play(this->soundJump);
			}
		}
		// An arc that ran its full length while not falling lands too.
		if (jumpEnded && !d.falling) {
			play(this->soundJump);
		}
	}
	else {
		d.jumpEdge = false;
		d.jumping = false;
		d.falling = false;
	}

	// Clearance flags for walking (the original evaluates them every frame).
	bool leftOk = false, leftStep = false, rightOk = false, rightStep = false;
	{
		const int c = d.xHalf / 2;
		const int c2 = (d.xHalf + 2) / 2;
		const bool midRow = (d.yPx % TILE_SIZE) != 0;
		if (!midRow) {
			leftOk = cellEmpty(c, row) && cellEmpty(c, row + 1);
			leftStep = cellEmpty(c, row - 1) && cellEmpty(c, row) && !cellEmpty(c, row + 1);
			rightOk = cellEmpty(c2, row) && cellEmpty(c2, row + 1);
			rightStep = cellEmpty(c2, row - 1) && cellEmpty(c2, row) && !cellEmpty(c2, row + 1);
		}
		else {
			leftOk = cellEmpty(c, row) && cellEmpty(c, row + 1) && cellEmpty(c, row + 2);
			leftStep = cellEmpty(c, row - 1) && cellEmpty(c, row) && !cellEmpty(c, row + 1);
			rightOk = cellEmpty(c2, row) && cellEmpty(c2, row + 1) && cellEmpty(c2, row + 2);
			rightStep = cellEmpty(c2, row - 1) && cellEmpty(c2, row) && !cellEmpty(c2, row + 1);
		}
		if (d.falling || d.jumping) {
			leftStep = false;
			rightStep = false;
		}
	}

	const auto walked = [&]() {
		d.movedThisTick = true;
		// The walk cycle: one frame per step, walkBegin..walkEnd inclusive
		// (hocus_anim_frame++ with the wrap in the movement block).
		d.frame++;
		if (d.frame > this->hocus.getFrames().walkEnd) {
			d.frame = this->hocus.getFrames().walkBegin;
		}
	};

	if (d.keyLeft && d.xHalf != 0) {
		if (!d.turnLatch) {
			d.turnLatch = true;
		}
		else if (!leftOk) {
			if (leftStep) {
				d.xHalf--;
				d.yPx -= TILE_SIZE;
				walked();
			}
		}
		else {
			d.xHalf--;
			walked();
		}
	}
	if (d.keyRight && d.xHalf < MAP_RIGHT_LIMIT_HALF) {
		if (!d.turnLatch) {
			if (!rightOk) {
				if (rightStep) {
					d.xHalf++;
					d.yPx -= TILE_SIZE;
					walked();
				}
			}
			else {
				d.xHalf++;
				walked();
			}
		}
		else {
			d.turnLatch = false;
		}
	}

	if (!carried) {
		this->hocus.writeRect();
	}
	else {
		this->hocus.syncFromRect();
	}

	updateHocusFrame();

	// Twinks around him while he holds a Super jump, and for 25 frames after
	// it is used (cf0e); the laser charge icons blink in 5-frame halves
	// (ce96/ce98). Neither runs while he's invisible (they sit inside the
	// original's ce8e == 0 draw branch).
	if (this->player.effects.invisibilityTicks == 0) {
		if (this->superJumpTwinkTicks != 0) {
			const int y = d.yPx + dosRandom(10) + 20;
			const int x4 = d.xHalf * 2 + dosRandom(7) - 2;
			spawnTwink(x4, y);
			if (!this->player.hasSuperJump()) {
				this->superJumpTwinkTicks--;
			}
		}
		if (this->player.effects.laserShots != 0) {
			this->laserBlinkTimer--;
			if (this->laserBlinkTimer <= 0) {
				this->laserBlinkTimer = 5;
				this->laserBlinkOn = !this->laserBlinkOn;
			}
		}
	}
}

// hocus_anim_frame after a frame's movement (level_run: the idle reset at
// the end of the movement block, then the draw block's pose overrides):
// standing still without fire held shows the stand frame; fire held shows
// the shooting pose (the upward one with Up held) unless he's walking, in
// which case the walk cycle keeps going; a jump or fall overrides all.
void Game::updateHocusFrame() {
	Hocus::Dos& d = this->hocus.dos;
	const Hocus::Frames& f = this->hocus.getFrames();
	if (!d.movedThisTick && !d.keyFire) {
		d.frame = f.stand;
	}
	if (d.scriptedWalkTimer != 0) {
		return;
	}
	if (d.keyFire) {
		if (!d.keyUp || d.movedThisTick) {
			if (!d.movedThisTick) {
				d.frame = f.shoot;
			}
		}
		else {
			d.frame = f.shootUp;
		}
	}
	if (d.jumping) {
		d.frame = d.jumpIndex > d.jumpPeak ? f.fall : f.jump;
	}
	if (d.falling) {
		d.frame = f.fall;
	}
}

void Game::cameraTick(bool snap) {
	const Hocus::Dos& d = this->hocus.dos;
	const int hocusRow = d.yPx / TILE_SIZE;

	// Look up/down (level_run, after the movement block): the Scroll keys
	// (70a3/70a4, or joystick buttons C/D) slide Hocus's screen row one row
	// per frame toward 8 or 0 at once; Up/Down (counters 3a466/3a464) do the
	// same but the row snaps back to 5 until they have been held 10 frames.
	this->lookUpFrames = d.keyUp ? std::min(this->lookUpFrames + 1, 10) : 0;
	this->lookDownFrames = d.keyDown ? std::min(this->lookDownFrames + 1, 10) : 0;
	if ((d.keyScrollDown || this->lookDownFrames != 0) && this->cameraRowOffset > 0) {
		this->cameraRowOffset--;
	}
	if ((d.keyScrollUp || this->lookUpFrames != 0) && this->cameraRowOffset < 8) {
		this->cameraRowOffset++;
	}
	if (!d.keyScrollUp && !d.keyScrollDown && this->lookUpFrames != 10 && this->lookDownFrames != 10) {
		this->cameraRowOffset = 5;
	}

	// Horizontal: follow at one half-tile per frame (snap: jump straight there),
	// keeping Hocus 20 half-tiles from the left edge, clamped to the map.
	const int targetX = d.xHalf - CAMERA_HOCUS_X_HALF;
	if (targetX < 0) {
		this->cameraXHalf = 0;
	}
	else if (d.xHalf + CAMERA_HOCUS_X_HALF >= 0x1e0) {
		this->cameraXHalf = CAMERA_MAX_X_HALF;
	}
	else if (snap) {
		this->cameraXHalf = targetX;
	}
	else if (this->cameraXHalf < targetX) {
		this->cameraXHalf++;
	}
	else if (targetX < this->cameraXHalf) {
		this->cameraXHalf--;
	}

	// Vertical: one row per frame toward (Hocus row - offset), never while
	// jumping; a vertical step blocks starting a jump that same frame.
	this->cameraScrolledVertically = false;
	if (!d.jumping) {
		const int targetRow = hocusRow - this->cameraRowOffset;
		if (hocusRow < this->cameraRowOffset) {
			this->cameraRow = 0;
		}
		else if (snap) {
			this->cameraRow = (hocusRow + this->cameraRowOffset < 0x3c) ? targetRow : CAMERA_MAX_ROW;
		}
		else if (hocusRow + (10 - this->cameraRowOffset) < 0x3c) {
			if (this->cameraRow < targetRow) {
				this->cameraRow++;
				this->cameraScrolledVertically = true;
			}
			else if (targetRow < this->cameraRow) {
				this->cameraRow--;
				this->cameraScrolledVertically = true;
			}
		}
		else {
			this->cameraRow = CAMERA_MAX_ROW;
		}
	}

	this->offset.set((float)(this->cameraXHalf * (TILE_SIZE / 2)), (float)(this->cameraRow * TILE_SIZE));
}

// Items and hazards - the original's item branch of process_event_tiles
// (1ba5:39f3): every game frame
// it scans the two body rows of the three columns around Hocus's head column
// (col-1, col, col+1); an objects-layer value below 0x17 is an index into the
// EXE's item table (name / score / heal / power / type). Effects keyed on the
// table's type byte, with "centre column only" where the original requires
// local_20 == 0:
//   type 0 treasures (score, popup 100..5000) and Heal (heal 10; refused with
//          a "no" sound above 99 health)
//   1  crystal (HUD count; the level ends on the last one)
//   2/9 Hurt / Kill hazards: table damage, 20 / 10 frame cooldown, centre only,
//          never consumed
//   3  Invisible: 400 frames immune         4  Super jump (one jump), centre only
//   5  Super shot: fire power 10 + autofire for 600 frames, centre only
//   6  Smart bomb: no handler in the original's dispatch (left as a plain pickup)
//   7/8 silver / gold key                   10 Wizard note: message on Up, kept
//   11 Laser Shot: +3 piercing shots, centre only   12 two-cell item (consumes both cells)
// Then, for any consumed item: fire power += power (max 10), score += score,
// health += heal, and the cell becomes the level's background tile.
void Game::checkItems() {
	namespace asset = data::asset;

	const Point head = this->hocus.getTilePosition();
	const int headCol = (int)head.getX();
	const int headRow = (int)head.getY();
	// Where a pickup's tag appears: one column left of him, 16 px up.
	const int tagX4 = this->hocus.dos.xHalf * 2 - 1;
	const int tagY = this->hocus.dos.yPx - 16;

	for (int dc = -1; dc <= 1; dc++) {
		const bool centre = dc == 0;
		for (int dr = 0; dr < 2; dr++) {
			const int col = headCol + dc;
			const int row = headRow + dr;
			if (col < 0 || col >= MAP_WIDTH || row < 0 || row >= MAP_HEIGHT) {
				continue;
			}
			const Point cell((float)col, (float)row);
			const uint16_t event = (uint16_t)this->map.getEvent(cell);
			if (event == asset::EventLayer::EMPTY || event >= 0x17) {
				continue;
			}

			const asset::ItemInfo::Entry& item = this->itemInfo.getItems()[event];
			bool keep = false;

			if (item.heal != 0 && this->player.getHealth() > 99) {
				// Health potion at full health: refused, silently.
				keep = true;
			}
			else if (item.heal != 0) {
				if (this->soundPotion) {
					this->soundPotion->play();
				}
			}
			else if (item.score != 0) {
				// Treasure: the special-item sound once every treasure in the
				// level is found (ce6a == ce68; the HUD level number blinks from
				// then on), the score tag, and one of the two pickup sounds.
				this->treasuresFound++;
				if (this->treasuresFound == this->treasuresTotal) {
					this->treasureBlink = 1;
					play(this->soundSpecialItem);
				}
				switch (item.score) {
					case 100: spawnTag(tagX4, tagY, 0); break;
					case 250: spawnTag(tagX4, tagY, 1); break;
					case 500: spawnTag(tagX4, tagY, 2); break;
					case 1000: spawnTag(tagX4, tagY, 3); break;
					case 5000: spawnTag(tagX4, tagY, 4); break;
					default: break;
				}
				if (!this->soundsItem.empty()) {
					this->soundsItem[dosRandom((int)this->soundsItem.size())]->play();
				}
			}
			else {
				switch (item.type) {
					case 1:
						addCrystal(1);
						if (this->soundCrystal) {
							this->soundCrystal->play();
						}
						break;
					case 2:
					case 9:
						keep = true;
						if (centre) {
							hurt((uint8_t)damageForDifficulty(dosDifficultyIndex()), item.type == 2 ? 20 : 10);
						}
						break;
					case 3:
						// Invisible: a twink at each corner of him, the hint sound.
						this->player.effects.invisibilityTicks = 400;
						spawnTwink(this->hocus.dos.xHalf * 2 - 1, this->hocus.dos.yPx);
						spawnTwink(this->hocus.dos.xHalf * 2 + 4, this->hocus.dos.yPx);
						spawnTwink(this->hocus.dos.xHalf * 2 - 1, this->hocus.dos.yPx + 16);
						spawnTwink(this->hocus.dos.xHalf * 2 + 4, this->hocus.dos.yPx + 16);
						this->hurtCooldownTicks = 0;
						play(this->soundHint);
						spawnTag(tagX4, tagY, 8);
						break;
					case 4:
						if (!this->player.hasSuperJump() && centre) {
							this->player.setSuperJump(true);
							this->superJumpTwinkTicks = 25;
							play(this->soundSpecialItem);
							spawnTag(tagX4, tagY, 7);
						}
						else {
							keep = true;
						}
						break;
					case 5:
						if (centre) {
							if (this->player.effects.superShotTicks == 0) {
								this->player.effects.savedFirePower = this->player.getFirePower();
								this->player.setFirePower(10);
							}
							this->player.effects.superShotTicks = 600;
							play(this->soundSpecialItem);
							spawnTag(tagX4, tagY, 9);
						}
						else {
							keep = true;
						}
						break;
					case 7:
						addSilverKey();
						play(this->soundSpecialItem);
						spawnTag(tagX4, tagY, 5);
						break;
					case 8:
						addGoldenKey();
						play(this->soundSpecialItem);
						spawnTag(tagX4, tagY, 5);
						break;
					case 10:
						// Wizard note: read with Up (Game::activate()), never consumed.
						keep = true;
						break;
					case 11:
						if (centre) {
							this->player.effects.laserShots += 3;
							play(this->soundSpecialItem);
							this->laserBlinkOn = true;
							this->laserBlinkTimer = 5;
						}
						else {
							keep = true;
						}
						break;
					case 12:
						// Two cells wide; the original also records his position here.
						keep = true;
						this->map.disableEvent(cell);
						this->map.removeTile(0, cell);
						if (col + 1 < MAP_WIDTH) {
							this->map.disableEvent(Point((float)(col + 1), (float)row));
							this->map.removeTile(0, Point((float)(col + 1), (float)row));
						}
						if (this->soundSpecialItem) {
							this->soundSpecialItem->play();
						}
						break;
					default:
						break;
				}
			}

			if (keep) {
				continue;
			}

			if (item.firePower != 0) {
				if (this->soundSpecialItem) {
					this->soundSpecialItem->play();
				}
				this->player.setFirePower((uint8_t)std::min<int>(this->player.getFirePower() + item.firePower, 10));
				if (this->player.effects.superShotTicks != 0) {
					this->player.effects.savedFirePower = std::min<int>(this->player.effects.savedFirePower + item.firePower, 10);
				}
			}
			if (item.score != 0) {
				addScore(item.score);
			}
			if (item.heal != 0) {
				addHealth(item.heal);
			}
			this->map.disableEvent(cell);
			this->map.removeTile(0, cell);
		}
	}
}

// Insert (codes 0x38..0x50, table type 5) and remove (0x51..0x69, table
// type 6) wall triggers - the original (process_event_tiles): standing on
// the trigger cell (any of the 3x2 scanned cells) fires it if RequiredKey is
// 0, or 1/2 with the silver/gold key (consumed). The target rect is queued
// (insert: cells the hidden layer has a tile for; remove: cells the
// foreground has a tile in) and revealed over 10 frames; the trigger cell is
// consumed and its background repainted with RequiredTile (no key) or the
// level's background tile (key used, plus a particle at the trigger).
void Game::checkWallTriggers() {
	constexpr uint16_t INSERT_BASE = 0x38;
	constexpr uint16_t REMOVE_BASE = 0x51;
	constexpr uint16_t TABLE_SIZE = data::asset::ToggleCoordinates::TOGGLES;

	const Point head = this->hocus.getTilePosition();
	for (int dc = -1; dc <= 1; dc++) {
		for (int dr = 0; dr < 2; dr++) {
			const int col = (int)head.getX() + dc;
			const int row = (int)head.getY() + dr;
			if (col < 0 || col >= MAP_WIDTH || row < 0 || row >= MAP_HEIGHT) {
				continue;
			}
			const Point position((float)col, (float)row);
			const uint16_t event = (uint16_t)this->map.getEvent(position);

			bool insert;
			uint32_t index;
			if (event >= INSERT_BASE && event < INSERT_BASE + TABLE_SIZE) {
				insert = true;
				index = event - INSERT_BASE;
			}
			else if (event >= REMOVE_BASE && event < REMOVE_BASE + TABLE_SIZE) {
				insert = false;
				index = event - REMOVE_BASE;
			}
			else {
				continue;
			}

			const data::asset::ToggleCoordinates::Entry& entry = insert
				? this->map.getInsertToggles().getToggles()[index]
				: this->map.getKeyHoleToggles().getToggles()[index];

			int keyUsed = 0;
			if (entry.requiredKey == 1 && this->player.hasSilverKey()) {
				keyUsed = 1;
			}
			else if (entry.requiredKey == 2 && this->player.hasGoldenKey()) {
				keyUsed = 2;
			}
			else if (entry.requiredKey != 0) {
				continue;
			}

			play(this->soundReveal);
			this->tileChangeTicks = 10;
			queueTileChange(entry.upperLeftX, entry.upperLeftY, entry.lowerRightX, entry.lowerRightY, insert);

			this->map.disableEvent(position);
			if (entry.requiredKey == 0) {
				this->map.setTile(0, position, entry.requiredTile);
			}
			else {
				this->map.removeTile(0, position);
				spawnTwink(col * 4, row * TILE_SIZE);
			}
			if (keyUsed == 1) {
				removeSilverKey();
			}
			else if (keyUsed == 2) {
				removeGoldenKey();
			}
			LOGI << (insert ? "Insert" : "Remove") << " trigger fired: index=" << index << " key=" << keyUsed;
		}
	}
}

void Game::checkMonsterTriggers() {
	// Confirmed against HocusEditor's format docs (_map.pas: TObjectType,
	// first_object_type_value): EventLayer tile values in [0x74, 0x74+ENEMIES) are
	// "monster trigger" tiles, not pickups - touching one spawns the enemy group at
	// (value - 0x74) in data::asset::EnemyTrigger. Enemies aren't all present at
	// level start; each group only appears once its trigger tile is touched.
	constexpr uint16_t MONSTER_TRIGGER_BASE = 0x74;

	const auto checkTriggerAt = [this](const Point& position) {
		const uint16_t event = (uint16_t)this->map.getEvent(position);

		if (event < MONSTER_TRIGGER_BASE || event >= MONSTER_TRIGGER_BASE + data::asset::EnemyTrigger::ENEMIES) {
			return;
		}

		const uint32_t groupIndex = event - MONSTER_TRIGGER_BASE;

		if (this->triggeredEnemyGroups[groupIndex]) {
			return;
		}

		this->triggeredEnemyGroups[groupIndex] = true;
		this->map.disableEvent(position);

		// As the original (process_event_tiles, monster-trigger branch): each of
		// the entry's 8 cells whose objects-layer value hasn't been consumed is
		// queued, unless that cell is already an active monster or already
		// queued. The monster's type is read from the objects layer at that cell
		// when it spawns (see spawnMonster()), not from the entry's Types[].
		const data::asset::EnemyTrigger::Entry& group = this->map.getEnemyTrigger().getEntries()[groupIndex];
		for (int j = 0; j < 8; j++) {
			const uint16_t offset = group.offsets[j];
			const Point cell((float)(offset % MAP_WIDTH), (float)(offset / MAP_WIDTH));
			if (this->map.getEvent(cell) == data::asset::EventLayer::EMPTY) {
				continue;
			}
			if (this->pendingMonsterOffsets.size() >= MAX_PENDING_MONSTERS) {
				break;
			}

			bool known = false;
			for (const auto& enemy : this->enemies) {
				if (enemy.monster.spawnTileOffset == offset) {
					known = true;
				}
			}
			for (uint16_t pending : this->pendingMonsterOffsets) {
				if (pending == offset) {
					known = true;
				}
			}
			if (known) {
				continue;
			}

			this->pendingMonsterOffsets.push_back(offset);
		}
	};

	const Point tilePosition = this->hocus.getTilePosition();

	checkTriggerAt(tilePosition);
	checkTriggerAt({tilePosition.getX(), tilePosition.getY() + 1});
}

void Game::checkSwitches() {
	// Confirmed against HocusEditor's format docs the same way monster triggers
	// were (_map.pas: TObjectType, first_object_type_value): EventLayer values in
	// [0x21, 0x21+SWITCHES) are switch tiles, not pickups - standing on one and
	// pressing Up (see Game::activate()) flips switch (value - 0x21), per the
	// manual's "Up Arrow - Talk to the Wizard, operate switches".
	//
	// SwitchCoordinates::Entry::type is HocusEditor's cbxSwitchType combo box
	// (main.dfm): index 0 = "Remove wall", index 1 = "Insert wall" - confirmed
	// against real level data, not guessed: level 1-1's switches are type 0 and
	// their target rect is already filled with the same tile pattern in both the
	// main collision layer and the unused "additional" layer at author time (so
	// there's nothing to reveal - the switch instead needs to clear the rect to
	// the background tile). Level 1-2's switches are type 1 and their rect starts
	// empty in the collision layer with the tile pattern sitting only in the
	// additional layer (so the switch reveals it by copying that pattern in).
	// The original (process_event_tiles, codes 0x21..0x37, on Up, centre
	// column, either body row): flip this lever between the level's
	// switch-down/up tiles in the background layer; then, if every lever cell
	// of the group (up to 4 SwitchOffsets) shows its DesiredTile, the group
	// fires: the target rect is queued for removal (type 0, where the
	// foreground has a tile) or insertion from the hidden layer (otherwise),
	// revealed over 25 frames, and the group is spent. Otherwise a "not yet"
	// sound. The lever cell itself is never consumed.
	constexpr uint16_t SWITCH_BASE = 0x21;
	constexpr uint16_t EMPTY_SLOT = 0xffff;

	const Point head = this->hocus.getTilePosition();
	for (int dr = 0; dr < 2; dr++) {
		const Point position(head.getX(), head.getY() + (float)dr);
		const uint16_t event = (uint16_t)this->map.getEvent(position);
		if (event < SWITCH_BASE || event >= SWITCH_BASE + data::asset::SwitchCoordinates::SWITCHES) {
			continue;
		}
		const uint32_t switchIndex = event - SWITCH_BASE;
		const data::asset::SwitchCoordinates::Entry& entry = this->map.getSwitchCoordinates().getSwitches()[switchIndex];
		const uint8_t down = this->map.getTileAnimationSettings().getSwitchDownTile();
		const uint8_t up = this->map.getTileAnimationSettings().getSwitchUpTile();

		Tile& lever = this->map.getLayer(0).getTile((int)position.getX(), (int)position.getY());
		this->map.setTile(0, position, lever.getId() == down ? up : down);

		if (this->activatedSwitches[switchIndex]) {
			play(this->soundSwitchRefused);
			return;
		}

		bool allSet = true;
		for (int i = 0; i < 4; i++) {
			if (entry.offsets[i] == EMPTY_SLOT) {
				continue;
			}
			const uint16_t id = this->map.getLayer(0).getTile(entry.offsets[i] % MAP_WIDTH, entry.offsets[i] / MAP_WIDTH).getId();
			if ((entry.desiredTile[i] == down && id != down) || (entry.desiredTile[i] == up && id != up)) {
				allSet = false;
			}
		}

		if (!allSet) {
			play(this->soundSwitchRefused);
			return;
		}

		this->activatedSwitches[switchIndex] = true;
		LOGI << "Switch group fired: index=" << switchIndex << " type=" << entry.type;
		play(this->soundReveal);
		this->tileChangeTicks = 25;
		queueTileChange(entry.upperLeftX, entry.upperLeftY, entry.lowerRightX, entry.lowerRightY, entry.type != 0);
		return;
	}
}

void Game::queueTileChange(uint16_t upperLeftX, uint16_t upperLeftY, uint16_t lowerRightX, uint16_t lowerRightY, bool insert) {
	for (uint16_t y = upperLeftY; y <= lowerRightY && y < MAP_HEIGHT; y++) {
		for (uint16_t x = upperLeftX; x <= lowerRightX && x < MAP_WIDTH; x++) {
			const uint32_t index = (uint32_t)y * MAP_WIDTH + x;
			if (insert) {
				if (this->map.getAdditionalLayer().getData()[index] != data::asset::MapLayer::TILE_EMPTY) {
					this->tileChangeBuffer[index] = 2;
				}
			}
			else if (this->map.getLayer(1).getTile(x, y).isVisible()) {
				this->tileChangeBuffer[index] = 1;
			}
		}
	}
}

// The original's reveal (FUN_1ba5_2c81, first half): each game frame, over
// the 21x10 visible window, a pending cell is applied with probability 1/3
// (with a twink) while the countdown runs, and unconditionally once it
// reaches 0. Off-screen cells wait until they scroll into view.
void Game::tileChangeTick() {
	if (this->tileChangeTicks > 0) {
		this->tileChangeTicks--;
	}
	const int cameraCol = this->cameraXHalf / 2;
	for (int dy = 0; dy < 10; dy++) {
		for (int dx = 0; dx < 21; dx++) {
			const int x = cameraCol + dx;
			const int y = this->cameraRow + dy;
			if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT) {
				continue;
			}
			const uint32_t index = (uint32_t)y * MAP_WIDTH + x;
			const uint8_t change = this->tileChangeBuffer[index];
			if (change == 0) {
				continue;
			}
			if (this->tileChangeTicks != 0 && rand() % 3 != 0) {
				continue;
			}
			const Point cell((float)x, (float)y);
			if (change == 1) {
				this->map.setTile(1, cell, data::asset::MapLayer::TILE_EMPTY);
			}
			else {
				this->map.setTile(1, cell, this->map.getAdditionalLayer().getData()[index]);
			}
			this->tileChangeBuffer[index] = 0;
			if (this->tileChangeTicks != 0 && dx < 20) {
				spawnTwink(x * 4, y * TILE_SIZE);
			}
		}
	}
}

void Game::checkTeleports() {
	// Confirmed against real level data (1-2, 1-5, 2-1 all have populated
	// Teleports entries): EventLayer values in [0x17, 0x17+TELEPORTS) are
	// teleport pad tiles - _map.pas's first_object_type_value table puts
	// otTeleport right before otSwitch's already-confirmed 0x21, at 0x17.
	// A pad's two ends (Teleports::Entry::startOffset/endOffset, both flat
	// y*MAP_WIDTH+x offsets) are BOTH tagged with the same event index in the
	// map, and stepping on either one warps to the other - not a fixed
	// "trigger here, arrive there" pair like a one-way door.
	// The original (process_event_tiles, codes 0x17..0x20): only the pad's
	// StartOff cell triggers, and only from the centre column of either body
	// row; it is consumed, and Hocus is flown to EndOff by the scripted walk
	// (hocusTick). One-way.
	constexpr uint16_t TELEPORT_BASE = 0x17;
	Hocus::Dos& d = this->hocus.dos;
	if (d.scriptedWalkTimer != 0) {
		return;
	}

	const Point head = this->hocus.getTilePosition();
	for (int dr = 0; dr < 2; dr++) {
		const Point position(head.getX(), head.getY() + (float)dr);
		const uint16_t event = (uint16_t)this->map.getEvent(position);
		if (event < TELEPORT_BASE || event >= TELEPORT_BASE + data::asset::Teleports::TELEPORTS) {
			continue;
		}
		const data::asset::Teleports::Entry& entry = this->map.getTeleports().getTeleports()[event - TELEPORT_BASE];
		const uint32_t cellOffset = (uint32_t)position.getY() * MAP_WIDTH + (uint32_t)position.getX();
		if (entry.startOffset != cellOffset) {
			continue;
		}

		d.scriptedWalkTimer = 1;
		d.targetYPx = (entry.endOffset / MAP_WIDTH) * TILE_SIZE;
		d.targetXHalf = (entry.endOffset % MAP_WIDTH) * 2;
		d.jumping = false;
		d.falling = false;
		d.yPx = (d.yPx / TILE_SIZE) * TILE_SIZE;
		this->hocus.writeRect();
		this->map.disableEvent(position);
		this->map.removeTile(0, position);
		d.frame = this->hocus.getFrames().stand;
		play(this->soundHint);
		spawnTag(d.xHalf * 2 - 1, d.yPx - 16, 6);
		return;
	}
}

// Hocus's shots - the original's fire block and shot mover in level_run
// (level_run's fire block). Fire is edge-triggered
// (held fire autofires only during the Super shot); a shot needs a free slot
// out of 10 and fewer live shots than the fire power; each Laser Shot pickup
// charge makes the next shot pierce. Holding Up fires upward. A shot starts
// at Hocus's x +-2 columns (+1 for vertical) and y + the sprite's
// projectileY. Each frame: if the cell ahead (and the one below it) is empty
// it moves 16 px, dying past the screen edges; a breakable brick in either of
// those cells breaks (that one tile only) and stops it; anything else solid
// stops it with an impact puff.
void Game::shoot() {
	if (this->paused) {
		return;
	}
	this->hocus.dos.fireEdge = true;
	this->hocus.dos.keyFire = true;
}

void Game::releaseFire() {
	this->hocus.dos.keyFire = false;
}

void Game::hocusShotTick() {
	Hocus::Dos& d = this->hocus.dos;
	Player::Effects& fx = this->player.effects;

	if (d.fireEdge) {
		if (fx.superShotTicks == 0) {
			d.fireEdge = false;
		}
		if (this->hocusShots.size() < 10 && (int)this->hocusShots.size() < this->player.getFirePower()) {
			HocusShot shot;
			if (fx.laserShots > 0) {
				fx.laserShots--;
				shot.piercing = true;
			}
			shot.facingLeft = this->hocus.getDirection() == Entity::LEFT;
			const int hocusX4 = d.xHalf * 2;
			if (!d.keyUp) {
				shot.vertical = false;
				shot.x4 = hocusX4 + (shot.facingLeft ? -2 : 2);
				shot.y = d.yPx + this->hocus.getProjectileY();
			}
			else {
				shot.vertical = true;
				shot.x4 = hocusX4 + 1;
				shot.y = d.yPx;
			}
			this->hocusShots.push_back(shot);
			// Sound 0, or 13 (the monsters' shot) while the Super shot runs.
			play(fx.superShotTicks == 0 ? this->soundShot : this->soundEnemyShot);
		}
	}

	if (fx.superShotTicks > 0) {
		fx.superShotTicks--;
		if (fx.superShotTicks == 0) {
			this->player.setFirePower((uint8_t)fx.savedFirePower);
		}
	}

	const uint8_t brick = this->map.getTileAnimationSettings().getShootableTile();
	const int camX4 = (int)this->offset.getX() / 4;
	const int camY = (int)this->offset.getY();

	// A breakable brick goes silently, with a twink at the given spot.
	const auto breakBrick = [&](int col, int row, int twinkX4, int twinkY) -> bool {
		if (col < 0 || col >= MAP_WIDTH || row < 0 || row >= MAP_HEIGHT) {
			return false;
		}
		Tile& tile = this->map.getLayer(1).getTile(col, row);
		if (!tile.isVisible() || tile.getId() != brick) {
			return false;
		}
		this->map.setTile(1, Point((float)col, (float)row), data::asset::MapLayer::TILE_EMPTY);
		spawnTwink(twinkX4, twinkY);
		return true;
	};

	auto it = this->hocusShots.begin();
	while (it != this->hocusShots.end()) {
		HocusShot& shot = *it;
		bool remove = false;

		if (!shot.vertical) {
			// Cell the shot is entering: one column ahead of its own on the
			// right, its own column on the left; both that row and the next.
			const int col = shot.x4 / 4 + (shot.facingLeft ? 0 : 1);
			const int ahead = shot.facingLeft ? -1 : 1;
			const int row = shot.y / TILE_SIZE;
			// Rightward: the two columns ahead; leftward: the column ahead only.
			bool broke;
			if (!shot.facingLeft) {
				broke = breakBrick(col, row, shot.x4 + 2, shot.y - 8) || breakBrick(col, row + 1, shot.x4 + 2, shot.y + 8) ||
					breakBrick(col + ahead, row, shot.x4 + 6, shot.y - 8) || breakBrick(col + ahead, row + 1, shot.x4 + 6, shot.y + 8);
			}
			else {
				broke = breakBrick(col + ahead, row, shot.x4 - 2, shot.y - 8) || breakBrick(col + ahead, row + 1, shot.x4 - 2, shot.y + 8);
			}
			if (broke) {
				remove = true;
			}
			else if (!isSolidCell(col, row) && !isSolidCell(col + ahead, row)) {
				shot.x4 += shot.facingLeft ? -4 : 4;
				const int screenX4 = shot.x4 - camX4;
				if ((shot.facingLeft ? screenX4 < -3 : screenX4 >= 80) || shot.y - camY < 0) {
					remove = true;
				}
				else if (dosRandom(10) > 2) {
					// The trail: a sinking spark behind it, 7 frames in 10.
					spawnTrail(shot.x4, shot.y + 5);
				}
			}
			else {
				// A solid tile: a puff at the point of impact.
				spawnPuff(shot.x4 * 4 + (shot.facingLeft ? 1 : 24), shot.y + (shot.facingLeft ? 2 : 10));
				remove = true;
			}
		}
		else {
			const int col = shot.x4 / 4 + (shot.facingLeft ? 0 : 1);
			const int row = std::max(shot.y / TILE_SIZE - 1, 0);
			if (breakBrick(col, row, shot.x4 - 1, shot.y - 15) || breakBrick(col + 1, row, shot.x4 + 3, shot.y - 15)) {
				remove = true;
			}
			else if (!isSolidCell(col, row)) {
				shot.y -= TILE_SIZE;
				if (shot.y - camY < 0) {
					remove = true;
				}
				else if (dosRandom(10) > 2) {
					spawnTrail(shot.x4 + (shot.facingLeft ? 2 : 0), shot.y + 5);
				}
			}
			else {
				spawnPuff(shot.x4 * 4 + 8, shot.y + 2);
				remove = true;
			}
		}

		if (remove) {
			it = this->hocusShots.erase(it);
		}
		else {
			++it;
		}
	}
}

// ---------------------------------------------------------------------------
// Monsters
//
// Everything from here to updateEnemies() is the original
// HOCUS.EXE monster code, reverse engineered (the reverse-engineering notes,
// "Monsters"):
//
//   spawnPendingMonsters / spawnMonster  <- spawn_pending (1ba5:2e9d), spawn (1ba5:04d3)
//   monsterBehaviourTick                 <- monster_update (1ba5:1277), one slot per call
//   monsterShoot                         <- the shared "fire" block + projectile spawn (1ba5:0922)
//   enemyProjectileTick                  <- projectile mover (1ba5:0a81) + hit test (1ba5:0006)
//   contact / Hocus-shot hits / despawn  <- 1ba5:00d3, 1ba5:0267, 1ba5:0439
//
// It runs one tick per original game frame (MONSTER_TICK_DT, 20 Hz). Units
// are the original's: x and widths in 4-pixel columns ("x4"), y and heights in
// pixels, velocities per tick. Its random(n) is rand() % n here.
//
// Behaviour byte (monster_info Behavior), as the original dispatches it:
//   0  walker; turns at walls/ledges; 1-in-20 per tick turns toward Hocus;
//      if it can shoot and is facing him, 1-in-shootDelay per tick it aims
//      (freezes 9 ticks) then fires
//   1  walker; 1-in-20 per tick turns toward Hocus and rushes for 10-24 ticks
//   2  hopper: random 1-column / 4-px steps, new direction every 5-34 ticks
//      or on touching a wall (toward Hocus horizontally if TargetPlayer)
//   3  hopper like 2 (always random), faces Hocus after choosing
//   4  static
//   5  static, always faces Hocus (turret)
//   6  hopper like 3 but double horizontal speed, faces Hocus first
//   7  walker that hops (-6..+6 px arc) when one of Hocus's shots gets within
//      10 columns; turns at walls/ledges only while airborne
//   8, 99  special routines (a hovering shooter and the end boss) - not yet
//      ported; treated as static
// Normal monsters only move on every other tick; a type-1 rush moves every tick.
// ---------------------------------------------------------------------------

namespace {
	int dosRandom(int n) {
		return n > 0 ? rand() % n : 0;
	}

	// The per-difficulty damage table of the EXE holds the (negative) health
	// change; the original's difficulty index is 0..2.
	int damageForDifficulty(int difficulty) {
		return -ExeData::get().word(TBL_DAMAGE, difficulty);
	}

	// The end boss's phase tables: cell, facing and the health thresholds;
	// a new phase starts with the health its predecessor's threshold named.
	int bossPhaseX(int phase) { return ExeData::get().word(TBL_BOSS_X, phase); }
	int bossPhaseY(int phase) { return ExeData::get().word(TBL_BOSS_Y, phase); }
	int bossPhaseFacingLeft(int phase) { return ExeData::get().word(TBL_BOSS_FACING, phase); }
	int bossPhaseThreshold(int phase) { return ExeData::get().word(TBL_BOSS_THRESHOLD, phase); }
	int bossPhaseHealth(int phase) { return phase == 0 ? 0 : ExeData::get().word(TBL_BOSS_THRESHOLD, phase - 1); }
	// Objects-layer value at a monster's cell = this + its monster_info index.
	constexpr int MONSTER_INFO_EVENT_BASE = 106;
	constexpr int BOSS_HEALTH = 21;         // maxHealth >= this: contact kills, health bar in the original
	constexpr int NEVER_DESPAWN_HEALTH = 500;
	constexpr int SPAWN_PUFF_TICKS = 20;
	constexpr int SHOOT_WINDUP_TICKS = 9;
}

int Game::dosDifficultyIndex() const {
	switch (this->player.getDifficulty()) {
		case EASY: return 0;
		case HARD: return 2;
		default: return 1;
	}
}

bool Game::isSolidCell(int col, int row) {
	// The original indexes the layer unchecked; outside the map counts as solid.
	if (col < 0 || col >= MAP_WIDTH || row < 0 || row >= MAP_HEIGHT) {
		return true;
	}
	return this->map.getLayer(1).getTile(col, row).isVisible();
}

bool Game::monsterBoxTouchesSolid(const Enemy::Monster& m) {
	// The box scan the original runs right after moving: first column shifted
	// one right when facing right, width4/4 columns (+1 when off-grid),
	// height/16 rows (+1 when off-grid); any foreground tile in it = touching.
	const int col = m.x4 / 4 + (m.facingLeft ? 0 : 1);
	const int row = m.y / 16;
	const int cols = m.width4 / 4 + (m.x4 % 4 != 0 ? 1 : 0);
	const int rows = m.height / 16 + (m.y % 16 != 0 ? 1 : 0);
	for (int r = 0; r < rows; r++) {
		for (int c = 0; c < cols; c++) {
			if (isSolidCell(col + c, row + r)) {
				return true;
			}
		}
	}
	return false;
}

void Game::spawnPendingMonsters() {
	auto it = this->pendingMonsterOffsets.begin();
	while (it != this->pendingMonsterOffsets.end()) {
		if (this->enemies.size() >= MAX_MONSTERS) {
			return;
		}
		if (spawnMonster(*it)) {
			it = this->pendingMonsterOffsets.erase(it);
		}
		else {
			++it;
		}
	}
}

// Returns true when the pending entry is done with (spawned, or nothing to
// spawn there any more); false to retry later.
bool Game::spawnMonster(uint16_t tileOffset) {
	const int col = tileOffset % MAP_WIDTH;
	const int row = tileOffset / MAP_WIDTH;
	const Point cell((float)col, (float)row);

	const uint16_t event = (uint16_t)this->map.getEvent(cell);
	if (event == data::asset::EventLayer::EMPTY) {
		return true;
	}
	const int infoIndex = (int)event - MONSTER_INFO_EVENT_BASE;
	if (infoIndex < 0 || infoIndex >= data::asset::TileProperties::PROPERTIES) {
		LOGW << "Monster cell " << col << "," << row << " has non-monster objects value " << event;
		return true;
	}
	return spawnMonsterAt(infoIndex, col * 4, row * TILE_SIZE, tileOffset);
}

bool Game::spawnMonsterAt(int infoIndex, int x4, int y, uint16_t tileOffset) {
	const int col = x4 / 4;
	const int row = y / TILE_SIZE;
	if (!this->monsterSpawner) {
		return true;
	}

	Enemy enemy;
	if (!this->monsterSpawner((uint16_t)infoIndex, enemy)) {
		return true;
	}

	const data::asset::TileProperties::Entry& info = this->map.getTileProperties().getProperties()[infoIndex];
	Enemy::Monster& m = enemy.monster;
	m.behaviour = (int16_t)info.behaviour;
	m.infoIndex = infoIndex;
	m.spawnTileOffset = tileOffset;
	m.x4 = x4;
	m.y = y;
	m.width4 = (int)enemy.getRect().getSize().getWidth() / 4;
	m.height = (int)enemy.getRect().getSize().getHeight();
	m.projectileXOffset = (int16_t)info.projectileXOffset;
	m.targetPlayer = info.targetPlayer != 0;
	m.shootOnlyFacing = info.unk2 != 0;
	m.shootProjectiles = info.shootProjectiles != 0;
	m.type8Timer = (int16_t)info.unk4;
	m.projectileHSpeed4 = (int16_t)info.projectileHSpeed;
	m.projectileVSpeed = (int16_t)info.projectileVSpeed;
	m.projectileHoming = m.targetPlayer;
	m.projectileWobbly = info.wobblyProjectiles != 0;

	// Health gains 2 per difficulty step; the negative sentinels are kept as-is.
	const int health = (int16_t)info.health;
	m.health = health >= 0 ? health + 2 * dosDifficultyIndex() : health;
	m.maxHealth = m.health;

	const int hocusX4 = (int)this->hocus.getRect().getPosition().getX() / 4;
	m.facingLeft = m.x4 > hocusX4;
	if (m.behaviour == 2) {
		m.facingLeft = false;
	}
	m.animFrame = 0;
	m.hitFlash = 0;
	m.spawnTicks = SPAWN_PUFF_TICKS;
	m.freezeTicks = 1;
	m.burstTicks = 0;
	m.jumpArc = Enemy::NO_JUMP;
	m.shootWindup = 0;
	m.shooting = false;

	switch (m.behaviour) {
		case 0: case 1: case 7:
			m.velX4 = m.facingLeft ? -1 : 1;
			m.velY = 0;
			break;
		case 2: case 3:
			do {
				m.velX4 = dosRandom(2) - dosRandom(2);
				m.velY = (dosRandom(2) - dosRandom(2)) * 4;
			} while (m.velX4 == 0 && m.velY == 0);
			m.moveTimer = dosRandom(20);
			break;
		case 6:
			do {
				m.velX4 = dosRandom(2) - dosRandom(2);
				m.velY = (dosRandom(2) - dosRandom(2)) * 4;
			} while (m.velX4 == 0);
			m.moveTimer = dosRandom(20);
			break;
		case 4:
			m.velX4 = 0;
			m.velY = 0;
			m.facingLeft = false;
			break;
		case 8:
			m.targetX4 = m.x4;
			m.type8Timer = dosRandom(10) + 10;
			break;
		default:
			m.velX4 = 0;
			m.velY = 0;
			break;
	}

	enemy.setPosition(Point((float)(m.x4 * 4), (float)m.y));
	enemy.showFrame(m.animFrame, m.facingLeft);
	LOGD << "Monster spawned: info=" << infoIndex << " behaviour=" << m.behaviour << " health=" << m.health
		<< " cell=(" << col << "," << row << ")";
	this->enemies.push_back(std::move(enemy));
	return true;
}

void Game::killMonster(Enemy& enemy, int puffs) {
	Enemy::Monster& m = enemy.monster;
	if (m.dead) {
		return;
	}
	m.dead = true;
	// The original marks the cell consumed so the monster never respawns, and
	// counts the kill. The score bonus is this port's own addition.
	this->map.disableEvent(Point((float)(m.spawnTileOffset % MAP_WIDTH), (float)(m.spawnTileOffset / MAP_WIDTH)));
	addScore(1000);
	play(this->soundKill);
	// Three more puffs on top of the hit's own (hproj_hit_monsters); the
	// "kill everything" monster's victims get one each (mon_kill_all).
	for (int i = 0; i < puffs; i++) {
		spawnPuff(m.x4 * 4 + m.width4 * 2, m.y + m.height / 2);
	}
}

// The projectile spawn (1ba5:0922): one of 8 slots, or fails.
bool Game::fireMonsterProjectile(const Enemy& enemy, int x4, int y, bool horizontal, int delay, int hocusX4) {
	if (this->enemyProjectiles.size() >= MAX_ENEMY_PROJECTILES) {
		return false;
	}
	const Enemy::Monster& m = enemy.monster;
	const Enemy::Frames& f = enemy.getFrames();

	Game::EnemyProjectile shot;
	shot.delay = delay;
	shot.facingLeft = false;
	int xOffset = 0;
	if (horizontal) {
		if (hocusX4 < x4) {
			shot.facingLeft = true;
		}
		else {
			xOffset = m.width4;
		}
	}
	shot.width4 = f.projectileWidth4;
	shot.height = f.projectileHeight + 3;
	shot.frame = f.projectile;
	shot.frameEnd = f.projectileEnd;
	shot.frameBase = f.projectile;
	shot.x4 = x4 + xOffset;
	shot.y = y + f.projectileY;
	shot.hspeed4 = m.projectileHSpeed4;
	shot.vspeed = m.projectileVSpeed;
	shot.homing = m.projectileHoming;
	shot.wobbly = m.projectileWobbly;
	shot.frames = enemy.getProjectileFrames(shot.facingLeft);
	this->enemyProjectiles.push_back(std::move(shot));
	return true;
}

// The shared "fire" block at the end of every monster tick.
void Game::monsterShoot(Enemy& enemy) {
	Enemy::Monster& m = enemy.monster;
	const Enemy::Frames& f = enemy.getFrames();
	const int shootDelay = (int)this->map.getPlayerCoordinates().getShootDelay();
	const int hocusX4 = (int)this->hocus.getRect().getPosition().getX() / 4;

	const bool randomShot = m.shootProjectiles && dosRandom(shootDelay) == 0 && m.behaviour != 0;
	if (!randomShot && m.shootWindup != 1) {
		return;
	}

	int delay = (f.shootEnd - f.shootBegin) + 1;
	m.shooting = true;
	bool fired;
	if (m.projectileHSpeed4 == 0) {
		// No horizontal speed: dropped from the bottom edge.
		fired = fireMonsterProjectile(enemy,m.x4 + m.projectileXOffset, m.y + m.height, false, delay, hocusX4);
	}
	else if (!m.shootOnlyFacing) {
		fired = fireMonsterProjectile(enemy,m.x4, m.y, true, delay, hocusX4);
	}
	else {
		if (m.behaviour == 5) {
			delay = f.shootEnd - f.shootBegin;
		}
		const bool hocusLeft = hocusX4 < m.x4;
		fired = (hocusLeft == m.facingLeft) &&
			fireMonsterProjectile(enemy,m.x4, m.y, true, delay, hocusX4);
	}
	if (fired && this->soundEnemyShot) {
		this->soundEnemyShot->play();
	}
	if (!fired) {
		m.shooting = false;
	}
	if (f.shootBegin != -1) {
		m.animFrame = f.shootBegin;
	}
}

void Game::monsterBehaviourTick(Enemy& enemy) {
	Enemy::Monster& m = enemy.monster;
	const Enemy::Frames& f = enemy.getFrames();
	const int hocusX4 = (int)this->hocus.getRect().getPosition().getX() / 4;

	const auto finishTick = [&]() {
		if (m.jumpArc != Enemy::NO_JUMP && f.jump != -1) {
			m.animFrame = f.jump;
		}
		if (m.hitFlash > 0) {
			m.hitFlash--;
		}
		enemy.showFrame(m.animFrame, m.facingLeft);
		// hitFlash's flicker is applied in render() (the original alternates
		// its normal and "flash" blitters each tick while it counts down).
	};

	if (m.spawnTicks > 0) {
		// Materialising: a particle now and then around the centre, nothing else.
		if (dosRandom(5) == 0) {
			const int y = m.y + m.height / 2 + dosRandom(12) - dosRandom(12);
			const int x4 = m.x4 + m.width4 / 2 + dosRandom(3) - dosRandom(3);
			spawnTwink(x4, y);
		}
		m.spawnTicks--;
		enemy.setPosition(Point((float)(m.x4 * 4), (float)m.y));
		return;
	}

	if (m.behaviour == 8) {
		// Hovering shooter (mon_type8_update, 1ba5:0ce8): no gravity. Frames 0-1
		// face right, 2-3 face left. Every 10-19 frames it drifts one column
		// toward its spawn column (randomly either way once within 5 columns),
		// stepping the frame with the direction. On frame 1, 1-in-10 per
		// frame, it fires horizontally if Hocus is to its left.
		if (m.animFrame < 2) {
			m.facingLeft = false;
		}
		else {
			m.facingLeft = true;
		}
		if (m.animFrame == 1 && dosRandom(10) == 0 && hocusX4 < m.x4) {
			// mproj_spawn plays sound 13 and this routine then plays 12 over it
			// (single channel): 12 is what's heard.
			if (fireMonsterProjectile(enemy, m.x4, m.y, true, 0, hocusX4)) {
				play(this->soundMonsterShot);
			}
		}
		if (m.type8Timer == 0) {
			m.type8Timer = dosRandom(10) + 10;
			int step;
			if (m.x4 < m.targetX4) {
				step = (m.targetX4 - m.x4 < 5 && dosRandom(2) != 0) ? -1 : 1;
			}
			else {
				step = (m.x4 - m.targetX4 < 5 && dosRandom(2) == 0) ? -1 : 1;
			}
			m.x4 += step;
			m.animFrame -= step;   // moving right counts the frame down, left counts it up
		}
		if (m.animFrame > 3) {
			m.animFrame = 0;
		}
		if (m.animFrame < 0) {
			m.animFrame = 3;
		}
		m.type8Timer--;
		enemy.setPosition(Point((float)(m.x4 * 4), (float)m.y));
		if (m.hitFlash > 0) {
			m.hitFlash--;
		}
		enemy.showFrame(m.animFrame < 2 ? m.animFrame : m.animFrame - 2, m.facingLeft);
		return;
	}

	if (m.behaviour == 99) {
		// End boss (mon_type99_update, 1ba5:1019). Faces Hocus. State machine:
		// idle (timer 5-49 frames) -> wind-up (frame 1, 4 frames) -> fire ->
		// idle. Health below the phase threshold (600/400/200) starts a
		// 20-frame phase change (frame 2), after which it re-spawns at the
		// next phase's cell (tables DS:2bb6/2bbe/2bce) with that phase's health.
		m.facingLeft = hocusX4 < m.x4;
		if (m.bossTimer == 0) {
			if (m.bossState == 0) {
				m.animFrame = 1;
				m.bossState = 1;
				m.bossTimer = 4;
			}
			else if (m.bossState == 1) {
				m.bossState = 2;
				m.bossTimer = 1;
				if (fireMonsterProjectile(enemy, m.x4, m.y, true, 0, hocusX4) && this->soundEnemyShot) {
					this->soundEnemyShot->play();
				}
			}
			else if (m.bossState == 2) {
				m.animFrame = 0;
				m.bossState = 0;
				m.bossTimer = dosRandom(45) + 5;
			}
		}
		else {
			m.bossTimer--;
		}
		if (this->bossPhase < 4 && m.health < bossPhaseThreshold(this->bossPhase)) {
			if (m.bossState < 3) {
				m.bossState = 3;
				m.bossTimer = 20;
				m.animFrame = 2;
				play(this->soundHint);
			}
			else if (m.bossTimer == 0) {
				// The respawn happens in monsterTick() after this loop (spawning
				// here would reallocate the vector being iterated).
				this->bossPhase++;
				this->bossRespawnPhase = std::min(this->bossPhase, 3);
				m.dead = true;
				return;
			}
		}
		enemy.setPosition(Point((float)(m.x4 * 4), (float)m.y));
		if (m.hitFlash > 0) {
			m.hitFlash--;
		}
		enemy.showFrame(m.animFrame, m.facingLeft);
		return;
	}

	if (m.shootWindup > 0) {
		// Aiming (type 0): frozen on the shoot frame, fires on the way down.
		m.shootWindup--;
		monsterShoot(enemy);
		finishTick();
		return;
	}
	if (m.freezeTicks > 0) {
		m.freezeTicks--;
		monsterShoot(enemy);
		finishTick();
		return;
	}

	const int prevX4 = m.x4;
	const int prevY = m.y;
	bool touching = false;

	if (m.jumpArc == Enemy::NO_JUMP) {
		m.x4 += m.velX4;
		m.y += m.velY;
		touching = monsterBoxTouchesSolid(m);
	}
	else {
		m.y += m.jumpArc;
		m.jumpArc++;
		if (m.jumpArc == 7) {
			m.jumpArc = Enemy::NO_JUMP;
			m.animFrame = 0;
		}
	}

	const auto faceHocus = [&]() {
		m.facingLeft = hocusX4 < m.x4;
	};
	const auto walkTowardFacing = [&]() {
		m.velX4 = m.facingLeft ? -1 : 1;
	};
	// Walkers: also stop at the edge of their platform - the cell under the
	// leading foot (first column facing left, last column facing right) must
	// be solid. Then a wall or ledge undoes the move and turns them around.
	const auto walker = [&]() {
		const int cols = m.width4 / 4 + (m.x4 % 4 != 0 ? 1 : 0);
		const int rows = m.height / 16 + (m.y % 16 != 0 ? 1 : 0);
		const int footCol = m.x4 / 4 + (m.facingLeft ? 0 : cols - 1);
		if (!isSolidCell(footCol, m.y / 16 + rows)) {
			touching = true;
		}
		if (touching) {
			m.x4 = prevX4;
			m.y = prevY;
			m.facingLeft = !m.facingLeft;
			walkTowardFacing();
		}
	};
	const auto hopper = [&](bool towardHocusX, bool doubleX) {
		if (m.moveTimer < 1) {
			touching = true;
		}
		if (touching) {
			m.x4 = prevX4;
			m.y = prevY;
			m.moveTimer = dosRandom(30) + 5;
			do {
				if (towardHocusX) {
					m.velX4 = hocusX4 < m.x4 ? -dosRandom(2) : dosRandom(2);
				}
				else {
					m.velX4 = dosRandom(2) - dosRandom(2);
				}
				m.velY = (dosRandom(2) - dosRandom(2)) * 4;
				if (doubleX) {
					m.velX4 *= 2;
				}
			} while (doubleX ? (m.velX4 == 0) : (m.velX4 == 0 && m.velY == 0));
		}
	};

	switch (m.behaviour) {
		case 0: {
			walker();
			const bool facingAway = (hocusX4 < m.x4 || m.facingLeft) && (m.x4 <= hocusX4 || !m.facingLeft);
			if (facingAway || !m.shootProjectiles || dosRandom((int)this->map.getPlayerCoordinates().getShootDelay()) != 0) {
				if (dosRandom(20) == 0) {
					faceHocus();
					walkTowardFacing();
				}
			}
			else {
				if (f.shootBegin != -1) {
					m.animFrame = f.shootBegin;
				}
				m.shootWindup = SHOOT_WINDUP_TICKS;
			}
			break;
		}
		case 1:
			walker();
			if (dosRandom(20) == 0) {
				faceHocus();
				m.burstTicks = dosRandom(15) + 10;
				if (f.shootBegin != -1) {
					m.animFrame = f.shootBegin;
				}
				walkTowardFacing();
			}
			break;
		case 2:
			hopper(m.targetPlayer, false);
			m.moveTimer--;
			break;
		case 3:
			hopper(false, false);
			if (touching) {
				faceHocus();
			}
			m.moveTimer--;
			break;
		case 6:
			if (m.moveTimer < 1) {
				touching = true;
			}
			faceHocus();
			hopper(false, true);
			m.moveTimer--;
			break;
		case 7:
			if (m.jumpArc == Enemy::NO_JUMP) {
				for (const HocusShot& shot : this->hocusShots) {
					if (shot.x4 - m.x4 < 10) {
						m.jumpArc = -6;
					}
				}
			}
			else {
				walker();
				if (dosRandom(20) == 0) {
					faceHocus();
					walkTowardFacing();
				}
			}
			break;
		case 5:
			faceHocus();
			break;
		default:
			break;
	}

	// Animation: walk cycle, or the shoot frames while shooting / rushing.
	m.animFrame++;
	if (m.burstTicks == 0) {
		if (!m.shooting) {
			if (m.animFrame > f.walkEnd) {
				m.animFrame = f.walkBegin;
			}
		}
		else if (m.animFrame > f.shootEnd) {
			m.shooting = false;
			m.animFrame = f.walkBegin;
		}
	}
	else if (f.shootBegin == -1) {
		if (m.animFrame > f.walkEnd) {
			m.animFrame = f.walkBegin;
		}
	}
	else if (m.animFrame > f.shootEnd) {
		m.animFrame = f.shootBegin;
	}

	// Every other tick unless rushing.
	if (m.burstTicks == 0) {
		m.freezeTicks = 1;
	}
	else {
		m.freezeTicks = 0;
		m.burstTicks--;
	}

	monsterShoot(enemy);
	enemy.setPosition(Point((float)(m.x4 * 4), (float)m.y));
	finishTick();
}

void Game::enemyProjectileTick() {
	const int hocusX4 = (int)this->hocus.getRect().getPosition().getX() / 4;
	const int hocusY = (int)this->hocus.getRect().getPosition().getY();
	const int camX4 = (int)this->offset.getX() / 4;
	const int camY = (int)this->offset.getY();

	auto it = this->enemyProjectiles.begin();
	while (it != this->enemyProjectiles.end()) {
		EnemyProjectile& shot = *it;
		if (shot.delay > 0) {
			shot.delay--;
			++it;
			continue;
		}

		// Hocus's hurt box: his middle two columns, rows 6..25 of his sprite.
		const bool hitsHocus =
			shot.x4 <= hocusX4 + 3 && shot.x4 + shot.width4 - 1 >= hocusX4 + 2 &&
			shot.y <= hocusY + 25 && shot.y + shot.height - 1 >= hocusY + 6;
		if (hitsHocus) {
			hurt((uint8_t)damageForDifficulty(dosDifficultyIndex()));
			it = this->enemyProjectiles.erase(it);
			continue;
		}

		const int screenX4 = shot.x4 - camX4;
		const int screenY = shot.y - camY;
		const bool offScreen = screenX4 < -10 || screenX4 > 80 || screenY < -20 || screenY > 160;
		const bool hitTile = isSolidCell(shot.x4 / 4, shot.y / 16);
		if (offScreen || hitTile) {
			if (hitTile) {
				spawnPuff(shot.x4 * 4 + 8, shot.y + 8);
			}
			it = this->enemyProjectiles.erase(it);
			continue;
		}

		if (shot.frame < shot.frameEnd) {
			shot.frame++;
		}
		shot.x4 += shot.facingLeft ? -shot.hspeed4 : shot.hspeed4;
		if (shot.homing) {
			if (shot.x4 < hocusX4) {
				shot.x4++;
			}
			else if (hocusX4 < shot.x4) {
				shot.x4--;
			}
		}
		shot.y += shot.vspeed;
		if (shot.wobbly) {
			shot.y += dosRandom(2) - dosRandom(2);
		}
		++it;
	}
}

void Game::monsterTick() {
	spawnPendingMonsters();

	for (auto& enemy : this->enemies) {
		if (!enemy.monster.dead) {
			monsterBehaviourTick(enemy);
		}
	}

	if (this->bossRespawnPhase >= 0) {
		// End boss phase change (mon_type99_update): the next phase appears at
		// its table cell with that phase's health and facing.
		const int phase = this->bossRespawnPhase;
		this->bossRespawnPhase = -1;
		const uint16_t offset = (uint16_t)(bossPhaseY(phase) * MAP_WIDTH + bossPhaseX(phase));
		const size_t before = this->enemies.size();
		spawnMonsterAt(0, bossPhaseX(phase) * 4, bossPhaseY(phase) * TILE_SIZE, offset);
		if (this->enemies.size() > before) {
			Enemy::Monster& next = this->enemies.back().monster;
			next.health = bossPhaseHealth(phase);
			next.maxHealth = next.health;
			next.facingLeft = bossPhaseFacingLeft(phase) != 0;
			next.velX4 = 0;
			next.bossTimer = dosRandom(50) + 5;
			next.bossState = 0;
		}
	}

	enemyProjectileTick();

	// Contact with Hocus (1ba5:00d3): a boss-class monster kills outright.
	const int hocusX4 = (int)this->hocus.getRect().getPosition().getX() / 4;
	const int hocusY = (int)this->hocus.getRect().getPosition().getY();
	for (auto& enemy : this->enemies) {
		const Enemy::Monster& m = enemy.monster;
		if (m.dead || m.spawnTicks > 0) {
			continue;
		}
		const bool overlaps =
			m.x4 <= hocusX4 + 3 && m.x4 + m.width4 - 1 >= hocusX4 + 2 &&
			m.y <= hocusY + 25 && m.y + m.height - 1 >= hocusY + 6;
		if (!overlaps) {
			continue;
		}
		if (m.maxHealth < BOSS_HEALTH) {
			hurt((uint8_t)damageForDifficulty(dosDifficultyIndex()));
		}
		else {
			hurt(this->player.getHealth());
		}
	}

	// Far off-screen monsters are dropped (1ba5:0439) - their cell keeps its
	// objects value, so another trigger listing it can bring them back.
	const int camX4 = (int)this->offset.getX() / 4;
	const int camY = (int)this->offset.getY();
	for (auto& enemy : this->enemies) {
		Enemy::Monster& m = enemy.monster;
		if (m.dead || m.maxHealth >= NEVER_DESPAWN_HEALTH) {
			continue;
		}
		if (m.x4 < camX4 - 50 || m.x4 > camX4 + 120 || m.y < camY - 120 || m.y > camY + 510) {
			m.dead = true;
		}
	}

	this->enemies.erase(
		std::remove_if(this->enemies.begin(), this->enemies.end(), [](const Enemy& enemy) { return enemy.monster.dead; }),
		this->enemies.end()
	);
}

void Game::updateEnemies(float dt) {
	(void)dt;
	// Hocus's shots vs monsters (1ba5:0267): a shot hits when its column is
	// within the monster's box and its y+2 is within the box's rows. A Laser
	// Shot pierces (keeps flying) and kills outright.
	bool killAll = false;

	for (auto& enemy : this->enemies) {
		Enemy::Monster& m = enemy.monster;
		if (m.dead || m.spawnTicks > 0) {
			continue;
		}

		for (auto it = this->hocusShots.begin(); it != this->hocusShots.end(); ++it) {
			const HocusShot& shot = *it;
			const int shotY = shot.y + 2;
			if (shot.x4 < m.x4 || shot.x4 > m.x4 + m.width4 - 1 || shotY < m.y || shotY > m.y + m.height - 1) {
				continue;
			}
			const bool piercing = shot.piercing;

			spawnPuff(m.x4 * 4 + m.width4 * 2, m.y + m.height / 2);
			if (!piercing) {
				this->hocusShots.erase(it);
			}

			if (m.health == Enemy::HEALTH_KILLS_ALL_ON_HIT) {
				m.health = Enemy::HEALTH_INVULNERABLE;
				killAll = true;
			}
			else if (m.health != Enemy::HEALTH_INVULNERABLE) {
				if (m.health == 0 || piercing) {
					killMonster(enemy);
				}
				else {
					m.health--;
					m.hitFlash = 10;
				}
			}
			break;
		}
	}

	if (killAll) {
		for (auto& enemy : this->enemies) {
			killMonster(enemy, 1);
		}
	}

	this->enemies.erase(
		std::remove_if(this->enemies.begin(), this->enemies.end(), [](const Enemy& enemy) { return enemy.monster.dead; }),
		this->enemies.end()
	);
}

bool Game::isShowingHint() const {
	return this->currentHint != -1;
}