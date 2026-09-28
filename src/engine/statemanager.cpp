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

#include "statemanager.h"
#include "log.h"

using namespace pocus;

void StateManager::release() {
	for (auto& state : this->states) {
		if (state.second) {
			state.second->release();
		}
	}

	this->states.erase(this->states.begin(), this->states.end());

	this->currentState = nullptr;
}

void StateManager::quit(int signal) {
	this->quitSignal = signal;
}

State& StateManager::addState(const std::string &name, std::unique_ptr<State> state) {
	LOGD << "StateManager: state name '" << name << "' inserted.";

	this->states.insert(std::make_pair(name, std::move(state)));

	return *this->states[name];
}

void StateManager::changeState(const std::string &name) {
	LOGI << "StateManager: changeState " << name;
	LOGD << "StateManager: state name '" << name << "' selected.";

	if (this->currentState) {
		this->currentState->onDetach();
	}

	this->currentStateName = name;
	auto iterator = this->states.find(this->currentStateName);
	if (iterator != this->states.end()) {
		this->currentState = iterator->second.get();
		this->currentState->onAttach();
	}
	else {
		LOGE << "StateManager error: state name '" << name << "' not found.";
	}
}

State* StateManager::getCurrentState() {
	return this->currentState;
}

bool StateManager::getQuit() const {
	return this->quitSignal != -1;
}

void StateManager::createStates(data::DataManager& dataManager) {
	for (auto& state : this->states) {
		state.second->onCreate(dataManager);
		
		if (this->currentState == nullptr) {
			changeState(state.first);
		}
	}
}

void StateManager::setStartupState(const std::string& name) {
	this->currentStateName = name;
	auto iterator = this->states.find(this->currentStateName);
	if (iterator != this->states.end()) {
		this->currentState = iterator->second.get();
	}
	else {
		LOGE << "StateManager error: state name '" << name << "' not found.";
	}
}

const std::string& StateManager::getStartupState() const {
	return this->currentStateName;
}