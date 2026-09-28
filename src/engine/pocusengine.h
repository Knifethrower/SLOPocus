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

#ifndef POCUSENGINE_H
#define POCUSENGINE_H

#include <memory>

#include "statemanager.h"
#include "renderer.h"
#include "eventhandler.h"
#include "data/data.h"
#include "definitions.h"
#include "audio.h"
#include "config.h"
#include "rules.h"
#include "data/datamanager.h"

namespace pocus {

class PocusEngine {
public:
	PocusEngine(std::unique_ptr<Renderer> renderer, std::unique_ptr<EventHandler> eventHandler, std::unique_ptr<Audio> audio);
	virtual ~PocusEngine() = default;

	int run(int argc, char* argv[]);

protected:
	virtual void createStates(StateManager& stateManager) = 0;
	
	StateManager& getStateManager();
	data::DataManager& getDataManager();
	Rules& getRules();

private:
	bool initialize();
	void loop();
	void release();

	void processStateMessage(State* state);
	float processFrameRate(const Tick& startTick, int delay, int fixedFpsDelay);
	bool loadConfig();
	bool loadRules();
	bool loadExecutable();
	bool loadData();

private:
	StateManager stateManager;
	std::unique_ptr<Renderer> renderer;
	std::unique_ptr<EventHandler> eventHandler;
	std::unique_ptr<Audio> audio;
	data::DataManager dataManger;
	Config config;
	Rules rules;

	bool running { false };
};

}


#endif //POCUSENGINE_H
