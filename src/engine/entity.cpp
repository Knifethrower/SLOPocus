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

#include <complex>
#include <iostream>
#include "entity.h"

using namespace pocus;

const Rect& Entity::getRect() const {
	return this->rect;
}

void Entity::setRect(const Rect& rect) {
	this->rect = rect;
}

void Entity::addState(const std::string& state, Animation animation) {
	this->states.insert(std::make_pair(state, std::move(animation)));
}

void Entity::setCurrentState(const std::string& state) {
	this->currentStateId = state;
	this->currentState = &this->states.find(state)->second;
}

Animation* Entity::findState(const std::string& state) {
	auto iterator = this->states.find(state);
	return iterator == this->states.end() ? nullptr : &iterator->second;
}

void Entity::render(Renderer &renderer, const Point& offset) {
	Point position = Point(
		this->rect.getPosition().getX() - offset.getX(),
		this->rect.getPosition().getY() - offset.getY()
	);
	
	if (this->currentState) {
		this->currentState->render(renderer, position);
	}
	
	//renderer.drawRect(Rect(position, this->rect.getSize()), color::redAlpha);
}

void Entity::update() {
	if (this->currentState) {
		this->currentState->update(0.f);
	}
	
	if (this->onHit) {
		if (getElapsedTime(this->tickHit) >= this->hitInvulnerableTime) {
			this->onHit = false;
			restoreColor();
		}
		else {
			if (this->blink.update(.0f)) {
				if (this->blink.getIsBlinking()) {
					setOverlayColor(color::red);
				} else {
					restoreColor();
				}
			}
		}
	}
}

void Entity::move(float dt) {
	this->rect.setPosition(
		Point(
			this->rect.getPosition().getX() + (this->velocity.getX() * (float)this->speed * dt),
			this->rect.getPosition().getY() + (this->velocity.getY() * (float)this->speed * dt)
		)
	);
}

const Entity::Direction_t& Entity::getDirection() const {
	return direction;
}

void Entity::setDirection(Entity::Direction_t direction) {
	Entity::direction = direction;
}

Point Entity::getTilePosition() {
	float width = 0.0f;
	if (this->direction == RIGHT) {
		width = TILE_SIZE / 2;
	}
	
	return {
		std::round((this->rect.getPosition().getX() + width) / TILE_SIZE),
		std::ceil(this->rect.getPosition().getY() / TILE_SIZE)
	};
}

const Point &Entity::getVelocity() const {
	return velocity;
}

void Entity::setVelocity(const Point &velocity) {
	Entity::velocity = velocity;
}

float Entity::getSpeed() const {
	return speed;
}

void Entity::setSpeed(float speed) {
	Entity::speed = speed;
}

void Entity::setPosition(const Point& point) {
	this->rect.setPosition(point);
}

void Entity::setX(float x) {
	this->rect.setPosition(Point(x, this->rect.getPosition().getY()));
}

void Entity::setY(float y) {
	this->rect.setPosition(Point(this->rect.getPosition().getX(), y));
}

void Entity::setVelocityX(float x) {
	this->velocity.setX(x);
}

void Entity::setVelocityY(float y) {
	this->velocity.setY(y);
}

std::string Entity::getDirectionName(const Direction_t& direction) {
	switch (direction) {
		case Entity::LEFT: return "left";
		case Entity::RIGHT: return "right";
	}

	return "";
}

const Tick &Entity::getTickCreation() const {
	return tickCreation;
}

void Entity::setTickCreation(const Tick &tickCreation) {
	Entity::tickCreation = tickCreation;
}

void Entity::resetStates() {
	this->states.erase(this->states.begin(), this->states.end());
	this->currentState = nullptr;
	this->currentStateId = "";
}

void Entity::setOverlayColor(const Color& color) {
	// ¯\_(ツ)_/¯
	for (auto& state : this->states) {
		state.second.setOverlayColor(color);
	}
}

void Entity::restoreColor() {
	// ¯\_(ツ)_/¯
	for (auto& state : this->states) {
		state.second.restoreColor();
	}
}

void Entity::hit(uint32_t invulnerableTime) {
	this->hitInvulnerableTime = invulnerableTime;
	this->onHit = true;
	this->tickHit = getNow();
	this->blink.blink(Blink::DEFAULT_DELAY);
}

void Entity::Blink::blink(uint32_t delay) {
	this->delay = delay;
	this->tick = getNow();
	this->isBlinking = true;
}

bool Entity::Blink::update(float dt) {
	if (getElapsedTime(this->tick) >= this->delay) {
		this->tick = getNow();
		this->isBlinking = !this->isBlinking;
		return true;
	}
	
	return false;
}

bool Entity::Blink::getIsBlinking() const {
	return this->isBlinking;
}

bool Entity::isInvulnerable() const {
	return this->onHit;
}