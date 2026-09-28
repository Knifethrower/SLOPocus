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

#ifndef ENTITY_H
#define ENTITY_H

#include <unordered_map>
#include <string>
#include "rect.h"
#include "animation.h"

namespace pocus {

class Entity {
public:
	class Blink {
	public:
		enum { DEFAULT_DELAY = 50 };
		
	public:
		void blink(uint32_t delay);
		bool update(float dt);
		[[nodiscard]] bool getIsBlinking() const;
	
	private:
		bool isBlinking { true };
		Tick tick;
		uint32_t delay { DEFAULT_DELAY };
	};
	
public:
	enum Direction_t { LEFT, RIGHT };
	enum { DEFAULT_HIT_TIME = 800 };
	
public:
	[[nodiscard]] const Rect& getRect() const;
	void setRect(const Rect& rect);
	void addState(const std::string& state, Animation animation);
	void setCurrentState(const std::string& state);
	[[nodiscard]] const Direction_t& getDirection() const;
	void setDirection(Direction_t direction);
	
	void render(Renderer& renderer, const Point& offset);
	void update();
	virtual void move(float dt);
	
	Point getTilePosition();
	
	const Point &getVelocity() const;
	
	void setVelocity(const Point &velocity);
	
	float getSpeed() const;
	
	void setSpeed(float speed);
	
	void setPosition(const Point& point);
	
	void setX(float x);
	void setY(float y);
	
	void setVelocityX(float x);
	void setVelocityY(float y);
	
	static std::string getDirectionName(const Direction_t& direction);
	
	const Tick &getTickCreation() const;
	
	void setTickCreation(const Tick &tickCreation);
	
	void resetStates();
	
	void setOverlayColor(const Color& color);
	void restoreColor();
	
	void hit(uint32_t invulnerableTime = DEFAULT_HIT_TIME);
	
	[[nodiscard]] bool isInvulnerable() const;

protected:
	// The animation registered under a state name, or nullptr.
	Animation* findState(const std::string& state);
	[[nodiscard]] const std::string& getCurrentStateId() const { return this->currentStateId; }
	
private:
	Rect rect {};
	std::unordered_map<std::string, Animation> states;
	std::string currentStateId;
	Animation* currentState { nullptr };
	Direction_t direction { RIGHT };
	Point velocity { .0f, .0f };
	float speed { 2.5f };
	Tick tickCreation;
	bool onHit { false };
	Tick tickHit;
	uint32_t hitInvulnerableTime;
	Blink blink;
};

}

#endif // ENTITY_H
