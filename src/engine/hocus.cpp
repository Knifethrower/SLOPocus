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

#include <cmath>
#include <algorithm>
#include "hocus.h"
#include "definitions.h"
#include "texturetools.h"

using namespace pocus;

namespace {
	// A 0xffff header field means "no such frame".
	int asFrame(uint16_t value, int fallback) {
		return value == 0xffff ? fallback : (int)value;
	}
}

void Hocus::setSprite(const data::asset::Sprite& sprite, Texture& sheet, const data::asset::Palette& palette) {
	const Color colorKey = color::pink;
	const uint32_t width = sprite.header.width4;
	const uint32_t height = sprite.header.height;

	this->frames.stand = asFrame(sprite.header.standFrame, 0);
	this->frames.walkBegin = asFrame(sprite.header.walkFrameBegin, 1);
	this->frames.walkEnd = asFrame(sprite.header.walkFrameEnd, this->frames.walkBegin);
	this->frames.jump = asFrame(sprite.header.jumpFrame, this->frames.stand);
	this->frames.fall = asFrame(sprite.header.fallFrame, this->frames.jump);
	this->frames.shoot = asFrame(sprite.header.shootDashFrameBegin, this->frames.stand);
	// The original draws shootDashFrameBegin + 1 for the upward shot (level_run's
	// draw block), not the header's end field (they coincide for Hocus: 9, 10).
	this->frames.shootUp = this->frames.shoot + 1;

	// Everything before the projectile cells is a pose frame.
	const int firstProjectile = asFrame(sprite.header.projectileFrame, MAX_FRAMES);
	this->frameCount = std::min<int>(std::max(firstProjectile, this->frames.shootUp + 1), MAX_FRAMES);

	const Color hurtColor = paletteColor(palette, 0x70);
	const Color superShotColor = paletteColor(palette, 0x60);
	for (int side = 0; side < 2; side++) {
		const uint32_t y = side == 0 ? 0 : height;
		for (int frame = 0; frame < this->frameCount; frame++) {
			auto texture = sheet.extract((uint32_t)frame * width, y, width, height, &colorKey);
			this->hurtTextures[side][frame] = makeSilhouette(*texture, colorKey, hurtColor);
			this->superShotTextures[side][frame] = makeSilhouette(*texture, colorKey, superShotColor);
			this->frameTextures[side][frame] = std::move(texture);
		}
	}

	// Projectile frames: packed into the sheet at column (projectileFrame * width) -
	// the character's own frame width, not projectileWidth4 - per row (East=right/
	// y=0, West=left/y=height), same as every other frame. projectileY is NOT a
	// sheet coordinate (verified by dumping the decoded sheet to a bitmap and
	// inspecting the actual pixels) - it's a gameplay spawn-position offset for
	// where to place the fired projectile relative to the shooter.
	for (int variant = 0; variant < 4; variant++) {
		const uint32_t projectileCellX = (sprite.header.projectileFrame + variant) * width;
		this->projectileTexturesRight[variant] = sheet.extract(
			projectileCellX, 0,
			sprite.header.projectileWidth4, sprite.header.projectileHeight, &colorKey
		);
		this->projectileTexturesLeft[variant] = sheet.extract(
			projectileCellX, height,
			sprite.header.projectileWidth4, sprite.header.projectileHeight, &colorKey
		);
	}
	this->projectileY = sprite.header.projectileY == 0xffff ? 0 : (int)sprite.header.projectileY;

	this->dos.frame = this->frames.stand;
	this->setRect(Rect(getRect().getPosition(), Size(sprite.header.width4, sprite.header.height)));
}

void Hocus::setMorphSprite(const data::asset::Sprite& sprite, Texture& sheet) {
	const Color colorKey = color::pink;
	const uint32_t width = sprite.header.width4;
	const uint32_t height = sprite.header.height;
	for (int side = 0; side < 2; side++) {
		const uint32_t y = side == 0 ? 0 : height;
		for (int frame = 0; frame < MORPH_FRAMES; frame++) {
			this->morphTextures[side][frame] = sheet.extract((uint32_t)frame * width, y, width, height, &colorKey);
		}
	}
}

void Hocus::render(Renderer& renderer, const Point& offset, Flash flash) {
	const int side = getDirection() == LEFT ? 1 : 0;
	int frame = this->dos.frame;
	if (frame < 0 || frame >= this->frameCount) {
		frame = this->frames.stand;
	}
	Texture* texture = nullptr;
	switch (flash) {
		case FLASH_HURT: texture = this->hurtTextures[side][frame].get(); break;
		case FLASH_SUPER_SHOT: texture = this->superShotTextures[side][frame].get(); break;
		default: texture = this->frameTextures[side][frame].get(); break;
	}
	if (!texture) {
		return;
	}
	const Point& position = getRect().getPosition();
	renderer.drawTexture(*texture, Point(position.getX() - offset.getX(), position.getY() - offset.getY()));
}

void Hocus::renderMorph(Renderer& renderer, const Point& offset) {
	// level_run's scripted-walk branch: frames 0..3 grow in over the 24-frame
	// pause, frame 4 (shaken +-1 px) while flying, then 3..1 back out.
	const int t = this->dos.scriptedWalkTimer;
	int frame;
	int jitter = 0;
	if (t < 5) frame = 0;
	else if (t < 10) frame = 1;
	else if (t < 15) frame = 2;
	else if (t < 20) frame = 3;
	else if (t < 26) { frame = 4; jitter = this->dos.morphJitter; }
	else if (t < 30) frame = 3;
	else if (t < 35) frame = 2;
	else if (t < 40) frame = 1;
	else return;

	const int side = getDirection() == LEFT ? 1 : 0;
	Texture* texture = this->morphTextures[side][frame].get();
	if (!texture) {
		return;
	}
	const Point& position = getRect().getPosition();
	renderer.drawTexture(*texture, Point(position.getX() - offset.getX(), position.getY() + (float)jitter - offset.getY()));
}

void Hocus::renderLaserCharge(Renderer& renderer, const Point& offset, int charges) {
	// Up to three laser cells at x + 4 px, 14 px apart above his head.
	const int count = std::min(charges, 3);
	const Point& position = getRect().getPosition();
	for (int i = 0; i < count; i++) {
		renderer.drawTexture(getProjectileTexture(SHOT_LASER, getDirection()),
			Point(position.getX() + 4.0f - offset.getX(), position.getY() - 14.0f - 14.0f * (float)i - offset.getY()));
	}
}

Texture& Hocus::getProjectileTexture(ShotVariant variant, const Direction_t& direction) {
	return direction == LEFT ? *this->projectileTexturesLeft[variant] : *this->projectileTexturesRight[variant];
}

int Hocus::getProjectileY() const {
	return this->projectileY;
}

void Hocus::startMovement(const Direction_t& direction) {
	this->setDirection(direction);
	if (direction == LEFT) {
		this->dos.keyLeft = true;
	}
	else {
		this->dos.keyRight = true;
	}
}

void Hocus::stopMovement(const Direction_t& direction) {
	if (direction == LEFT) {
		this->dos.keyLeft = false;
	}
	else {
		this->dos.keyRight = false;
	}
	// Facing follows whichever key is still held, else the one just released.
	if (this->dos.keyLeft) {
		this->setDirection(LEFT);
	}
	else if (this->dos.keyRight) {
		this->setDirection(RIGHT);
	}
	else {
		this->setDirection(direction);
	}
}

Point Hocus::getTilePosition() {
	return { (float)((this->dos.xHalf + 1) / 2), (float)(this->dos.yPx / TILE_SIZE) };
}

void Hocus::syncFromRect() {
	const Point& position = getRect().getPosition();
	const float x = (float)(this->dos.xHalf * (TILE_SIZE / 2));
	const float y = (float)this->dos.yPx;
	if (std::abs(position.getX() - x) > 0.5f || std::abs(position.getY() - y) > 0.5f) {
		this->dos.xHalf = (int)std::lround(position.getX() / (TILE_SIZE / 2));
		this->dos.yPx = (int)std::lround(position.getY());
	}
}

void Hocus::writeRect() {
	setPosition(Point((float)(this->dos.xHalf * (TILE_SIZE / 2)), (float)this->dos.yPx));
}
