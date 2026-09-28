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

#ifndef STATEMANAGER_H
#define STATEMANAGER_H

#include <memory>
#include <unordered_map>
#include <string>

#include "state.h"

namespace pocus {

class StateManager {
public:
	friend class PocusEngine;
	
public:
	StateManager() = default;

	void quit(int signal);

	bool getQuit() const;

	void release();

	State& addState(const std::string &name, std::unique_ptr<State> state);
	void changeState(const std::string &name);
	State* getCurrentState();
	
	void setStartupState(const std::string& name);
	
	const std::string& getStartupState() const;
	
protected:
	void createStates(data::DataManager& dataManager);

private:
	int quitSignal { -1 };
	std::unordered_map<std::string, std::unique_ptr<State>> states;
	std::string currentStateName;
	State* currentState { nullptr };

};

}

#endif //STATEMANAGER_H
