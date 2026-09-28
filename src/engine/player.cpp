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

#include "player.h"

using namespace pocus;

uint8_t Player::getCrystals() const {
	return crystals;
}

void Player::setCrystals(uint8_t crystals) {
	Player::crystals = crystals;
}

uint32_t Player::getScore() const {
	return score;
}

void Player::setScore(uint32_t score) {
	Player::score = score;
}

uint8_t Player::getHealth() const {
	return health;
}

void Player::setHealth(uint8_t health) {
	Player::health = health;
}

uint8_t Player::getLevel() const {
	return level;
}

void Player::setLevel(uint8_t level) {
	Player::level = level;
}

uint8_t Player::getEpisode() const {
	return episode;
}

void Player::setEpisode(uint8_t episode) {
	Player::episode = episode;
}

bool Player::hasSilverKey() const {
	return silverKey;
}

void Player::setSilverKey(bool silverKey) {
	Player::silverKey = silverKey;
}

bool Player::hasGoldenKey() const {
	return goldKey;
}

void Player::setGoldKey(bool goldKey) {
	Player::goldKey = goldKey;
}

uint8_t Player::getFirePower() const {
	return this->firePower;
}

void Player::setFirePower(uint8_t power) {
	Player::firePower = power;
}

bool Player::hasSuperJump() const {
	return this->superJump;
}

void Player::setSuperJump(bool superJump) {
	Player::superJump = superJump;
}

Difficulty_t Player::getDifficulty() const {
	return this->difficulty;
}

void Player::setDifficulty(Difficulty_t difficulty) {
	Player::difficulty = difficulty;
}
