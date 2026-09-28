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

#include <memory>
#include "pcx.h"
#include "../../provider/provider.h"

using namespace pocus::data::asset;

bool Pcx::loadFromStream(const char *stream, uint32_t length) {
	this->data.resize(length);
	for (uint32_t i = 0; i < length; i++) {
		this->data[i] = stream[i];
	}

	return true;
}

void Pcx::release() {

}

std::unique_ptr<pocus::Texture> Pcx::createTexture() {
	std::unique_ptr<Texture> texture = Provider::provideTexture();

	// loadFromStream()'s success was previously discarded here, so a caller
	// handed malformed/non-PCX bytes (e.g. a FAT slot that doesn't actually
	// hold a PCX - confirmed happening for a few of episode 4's later
	// backgrounds) got back a Texture that LOOKED valid (non-null pointer)
	// but had a null SDL surface underneath, crashing the first time
	// anything called getWidth()/getHeight()/render on it. Surface the
	// failure as a null return instead so callers can check it.
	if (!texture->loadFromStream(reinterpret_cast<char*>(this->data.data()), data.size())) {
		return nullptr;
	}

	return std::move(texture);
}