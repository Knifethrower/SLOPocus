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

#ifndef MAP_H
#define MAP_H

#include <memory>
#include "texture.h"
#include "sound.h"
#include "renderer.h"
#include "point.h"
#include "animation.h"
#include "data/asset/level.h"

namespace pocus {

class Tile {
public:
	Tile(uint32_t x, uint32_t y);
	
	[[nodiscard]] uint16_t getId() const;
	void setId(uint16_t id);
	[[nodiscard]] const Point &getTilePosition() const;
	void setTilePosition(const Point &tilePosition);
	[[nodiscard]] bool isVisible() const;
	void setVisible(bool visible);
	Animation& getAnimation();

private:
	Animation animation;
	uint16_t id { 0 };
	Point tilePosition { Point(.0f, .0f) };
	bool visible { true };
};

class Layer {
public:
	Layer(uint32_t width, uint32_t height);
	void create(uint32_t width, uint32_t height);
	Tile& getTile(uint32_t x, uint32_t y);
	[[nodiscard]] bool isVisible() const;
	void setVisible(bool visible);

private:
	uint32_t width, height;
	std::vector<Tile> tiles;
	bool visible { true };
};

class Map {
public:
	class Chunk {
	public:
		Chunk(const Size& size);
		
		std::vector<uint32_t>& getTiles();
		[[nodiscard]] const Size& getSize() const;
		void addTile(uint32_t id);
		
	private:
		Size size;
		std::vector<uint32_t> tiles;
	};
	
public:
	void setMusic(std::unique_ptr<Sound> music);
	void setTileSet(const std::unique_ptr<Texture>& tileSet);
	void setBackground(std::unique_ptr<Texture> texture);
	void start();
	void render(Renderer& renderer, const Point& offset);
	void update(float dt);
	// The original's per-frame tile animation (FUN_1ba5_2c81, second half):
	// only the background layer animates, only on the 21x10 visible window,
	// one step per 20 Hz game frame, per the level's animation table.
	void animateTick(int cameraCol, int cameraRow);
	void create(uint32_t width, uint32_t height);
	
	Layer& getLayer(uint8_t layer);
	Chunk getChunk(const Point& position, const Size& size);
	
	[[nodiscard]] data::asset::PlayerCoordinates &getPlayerCoordinates();
	void setPlayerCoordinates(const data::asset::PlayerCoordinates &playerCoordinates);
	[[nodiscard]] data::asset::TileAnimationSettings &getTileAnimationSettings();
	void setTileAnimationSettings(const data::asset::TileAnimationSettings &tileAnimationSettings);
	[[nodiscard]] data::asset::Messages &getMessages();
	void setMessages(const data::asset::Messages &messages);
	[[nodiscard]] data::asset::Teleports &getTeleports();
	void setTeleports(const data::asset::Teleports &teleports);
	[[nodiscard]] data::asset::SwitchCoordinates &getSwitchCoordinates();
	void setSwitchCoordinates(const data::asset::SwitchCoordinates &switchCoordinates);
	[[nodiscard]] data::asset::ToggleCoordinates &getInsertToggles();
	void setInsertToggles(const data::asset::ToggleCoordinates &insertToggles);
	[[nodiscard]] data::asset::ToggleCoordinates &getKeyHoleToggles();
	void setKeyHoleToggles(const data::asset::ToggleCoordinates &keyHoleToggles);
	[[nodiscard]] data::asset::TileProperties &getTileProperties();
	void setTileProperties(const data::asset::TileProperties &tileProperties);
	[[nodiscard]] data::asset::EnemyTrigger &getEnemyTrigger();
	void setEnemyTrigger(const data::asset::EnemyTrigger &enemyTrigger);
	[[nodiscard]] data::asset::MapLayer &getBackgroundLayer();
	void setBackgroundLayer(const data::asset::MapLayer &backgroundLayer);
	[[nodiscard]] data::asset::MapLayer &getMapLayer();
	void setMapLayer(const data::asset::MapLayer &mapLayer);
	[[nodiscard]] data::asset::MapLayer &getAdditionalLayer();
	void setAdditionalLayer(const data::asset::MapLayer &additionalLayer);
	[[nodiscard]] data::asset::EventLayer& getEventLayer();
	void setEventLayer(const data::asset::EventLayer &eventLayer);
	
	[[nodiscard]] uint32_t getLimitTime() const;
	void setLimitTime(uint32_t limitTime);

	// the EXE elevator table tile ids for this level (see LevelElevator) - a left
	// id of -1 means no elevators. Game scans the collision layer for these
	// at level start to find where the actual elevator cars are.
	[[nodiscard]] int16_t getElevatorLeftTile() const;
	[[nodiscard]] int16_t getElevatorRightTile() const;
	void setElevatorTiles(int16_t leftTile, int16_t rightTile);
	
	[[nodiscard]] uint8_t getCrystals() const;
	
	[[nodiscard]] data::asset::EventLayer::Event_t getEvent(const Point& tile) const;
	void disableEvent(const Point& point);
	
	void removeTile(uint8_t layer, const Point& tile);
	void setTile(uint8_t layer, const Point& tile, uint16_t id);
	void fillRegion(uint8_t layer, const Point& topLeft, const Point& bottomRight, uint16_t id);
	
private:
	std::unique_ptr<Sound> backgroundMusic { nullptr };
	std::unique_ptr<Texture> backgroundImage { nullptr };
	std::vector<std::unique_ptr<Texture>> tileSet;
	std::vector<Animation> animatedTileSet;
	uint32_t width;
	uint32_t height;
	std::vector<Layer> layers;
	std::vector<Animation> animations;
	
	uint8_t crystals;
	
	pocus::data::asset::PlayerCoordinates playerCoordinates;
	pocus::data::asset::TileAnimationSettings tileAnimationSettings;
	pocus::data::asset::Messages messages;
	pocus::data::asset::Teleports teleports;
	pocus::data::asset::SwitchCoordinates switchCoordinates;
	pocus::data::asset::ToggleCoordinates insertToggles;
	pocus::data::asset::ToggleCoordinates keyHoleToggles;
	pocus::data::asset::TileProperties tileProperties;
	pocus::data::asset::EnemyTrigger enemyTrigger;
	pocus::data::asset::MapLayer backgroundLayer;
	pocus::data::asset::MapLayer mapLayer;
	pocus::data::asset::MapLayer additionalLayer;
	pocus::data::asset::EventLayer eventLayer;
	
	uint32_t limitTime;

	int16_t elevatorLeftTile { -1 };
	int16_t elevatorRightTile { -1 };

};

class MapBuilder {
public:
};

}

#endif // MAP_H
