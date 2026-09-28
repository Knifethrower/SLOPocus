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

#include "fade.h"

#include <utility>
#include <iostream>

using namespace pocus;

void Fade::start(const Fade_t& type, std::function<void()> onFinished) {
	this->onFinished = std::move(onFinished);
	this->type = type;
	this->running = true;
	this->tickStart = getNow();

	switch (type) {
		case FADE_IN:
			this->alpha = 255;
			break;
		case FADE_OUT:
			this->alpha = 0;
			break;
	}
	
	this->color.alpha = (uint8_t)this->alpha;
}

bool Fade::isRunning() const {
	return this->running;
}

void Fade::stop() {
	this->running = false;
}

void Fade::update(float dt) {
	if (!this->running) {
		return;
	}
	
	switch (type) {
		case FADE_IN:
			this->alpha -= this->speed * dt;
			if (this->alpha < 0.0f) {
				this->alpha = 0.0f;
				this->running = false;
			}
			
			break;
		case FADE_OUT:
			this->alpha += this->speed * dt;
			if (this->alpha > 255.0f) {
				this->alpha = 255.0f;
				this->running = false;
			}
			
			break;
	}
	
	this->color.alpha = (uint8_t)this->alpha;

	if (!this->running) {
		if (this->onFinished) {
			this->onFinished();
		}
	}
}

void Fade::render(Renderer& renderer) {
	if (this->color.alpha == 0) {
		return;
	}
	renderer.drawRect(Rect(Point(0, 0), Size(-1, -1)), this->color);
}

void Fade::setSpeed(float speed) {
	this->speed = speed;
	
	if (this->speed < 0.0f) {
		this->speed = 0.001f;
	}
}

float Fade::getSpeed() const {
	return this->speed;
}

void Fade::setColor(const Color& color) {
	this->color = color;
}

void Fade::setOnFinished(std::function<void()> onFinished) {
	this->onFinished = std::move(onFinished);
}
