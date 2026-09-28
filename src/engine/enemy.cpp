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

#include <algorithm>
#include <string>
#include "enemy.h"
#include "provider/provider.h"
#include "texturetools.h"

using namespace pocus;

namespace {
	// This format's frame-index fields are signed, with -1 (0xffff) meaning "not
	// present" - confirmed against HocusEditor (_spritefile.pas) and by the
	// original's own checks (e.g. it tests the shoot frame against -1 before use).
	int asSigned(uint16_t v) {
		return v == 0xffff ? -1 : (int)v;
	}

	std::string frameStateName(int frame, bool facingLeft) {
		return (facingLeft ? "f_l_" : "f_r_") + std::to_string(frame);
	}
}

void Enemy::setSprite(const data::asset::Sprite& sprite, Texture& sheet, const data::asset::Palette& palette) {
	const Color colorKey = color::pink;
	const Color flashColor = paletteColor(palette, 0x70);

	const uint32_t width = sprite.header.width4;
	const uint32_t height = sprite.header.height;

	this->frames.walkBegin = asSigned(sprite.header.walkFrameBegin);
	this->frames.walkEnd = asSigned(sprite.header.walkFrameEnd);
	this->frames.jump = asSigned(sprite.header.jumpFrame);
	this->frames.shootBegin = asSigned(sprite.header.shootDashFrameBegin);
	this->frames.shootEnd = asSigned(sprite.header.shootDashFrameEnd);
	this->frames.projectile = asSigned(sprite.header.projectileFrame);
	this->frames.projectileEnd = asSigned(sprite.header.projectileUnknown);
	this->frames.projectileWidth4 = sprite.header.projectileWidth4;
	this->frames.projectileHeight = sprite.header.projectileHeight;
	this->frames.projectileY = asSigned(sprite.header.projectileY);

	// HocusEditor's loader scrubs every frame slot beyond each sprite's actual
	// highest real frame because the original data has leftover garbage there
	// (e.g. a dev-reference label baked into the sheet). Same bound here.
	int maxUsedFrame = 0;
	for (int f : { asSigned(sprite.header.standFrame2), this->frames.walkEnd, asSigned(sprite.header.fallFrame),
			this->frames.shootEnd, this->frames.projectileEnd, this->frames.jump }) {
		maxUsedFrame = std::max(maxUsedFrame, f);
	}
	this->frameCount = std::min<int>(maxUsedFrame + 1, data::asset::Sprite::COLUMNS);
	if (this->frames.walkBegin < 0 || this->frames.walkBegin >= this->frameCount) {
		this->frames.walkBegin = 0;
	}
	if (this->frames.walkEnd < this->frames.walkBegin || this->frames.walkEnd >= this->frameCount) {
		this->frames.walkEnd = this->frames.walkBegin;
	}

	// Enemy sprites don't carry distinct left-facing art (unlike Hocus); the
	// engine mirrors the east frames. Confirmed by dumping Devil Dan's sheet:
	// its "west" row decodes to the baked-in "MON3.PCX" label.
	const auto mirrorHorizontal = [colorKey](Texture& source) -> std::unique_ptr<Texture> {
		const uint32_t w = source.getWidth();
		const uint32_t h = source.getHeight();
		auto mirrored = Provider::provideTexture(w, h);
		if (w == 0 || h == 0) {
			return mirrored;
		}
		uint8_t r, g, b, a;
		for (uint32_t y = 0; y < h; y++) {
			for (uint32_t x = 0; x < w; x++) {
				source.getPixel(y * w + (w - 1 - x), &r, &g, &b, &a);
				mirrored->setPixel(y * w + x, r, g, b, a);
			}
		}
		mirrored->setColorKey(colorKey.red, colorKey.green, colorKey.blue);
		return mirrored;
	};

	// One state per frame per facing, so Game can show the exact frame index
	// the original's animation logic lands on.
	for (int frame = 0; frame < this->frameCount; frame++) {
		auto east = sheet.extract((uint32_t)frame * width, 0, width, height, &colorKey);
		auto west = mirrorHorizontal(*east);
		addState(frameStateName(frame, false) + "_flash", Animation::createFromFrame(makeSilhouette(*east, colorKey, flashColor)));
		addState(frameStateName(frame, true) + "_flash", Animation::createFromFrame(makeSilhouette(*west, colorKey, flashColor)));
		addState(frameStateName(frame, false), Animation::createFromFrame(std::move(east)));
		addState(frameStateName(frame, true), Animation::createFromFrame(std::move(west)));
	}

	// Projectile frames projectile..projectileEnd, each a cell of the sheet
	// with the projectile's own size (see Hocus::setSprite for the layout note).
	if (this->frames.projectile >= 0) {
		const int last = std::max(this->frames.projectile, this->frames.projectileEnd);
		for (int frame = this->frames.projectile; frame <= last && frame < data::asset::Sprite::COLUMNS; frame++) {
			auto east = sheet.extract((uint32_t)frame * width, 0,
				sprite.header.projectileWidth4, sprite.header.projectileHeight, &colorKey);
			auto west = mirrorHorizontal(*east);
			this->projectileFramesRight.emplace_back(std::move(east));
			this->projectileFramesLeft.emplace_back(std::move(west));
		}
	}

	showFrame(this->frames.walkBegin, false);
	this->setRect(Rect(getRect().getPosition(), Size(sprite.header.width4, sprite.header.height)));
}

void Enemy::render(Renderer& renderer, const Point& offset, bool flash) {
	if (!flash) {
		Entity::render(renderer, offset);
		return;
	}
	Animation* silhouette = findState(getCurrentStateId() + "_flash");
	if (!silhouette) {
		Entity::render(renderer, offset);
		return;
	}
	const Point& position = getRect().getPosition();
	silhouette->render(renderer, Point(position.getX() - offset.getX(), position.getY() - offset.getY()));
}

void Enemy::showFrame(int frame, bool facingLeft) {
	if (frame < 0 || frame >= this->frameCount) {
		frame = this->frames.walkBegin;
	}
	setDirection(facingLeft ? LEFT : RIGHT);
	setCurrentState(frameStateName(frame, facingLeft));
}

int Enemy::getFrameCount() const {
	return this->frameCount;
}

const Enemy::Frames& Enemy::getFrames() const {
	return this->frames;
}

const std::vector<std::shared_ptr<Texture>>& Enemy::getProjectileFrames(bool facingLeft) const {
	return facingLeft ? this->projectileFramesLeft : this->projectileFramesRight;
}
