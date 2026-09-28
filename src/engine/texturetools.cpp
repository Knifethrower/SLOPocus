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

#include "texturetools.h"
#include "provider/provider.h"

using namespace pocus;

std::unique_ptr<Texture> pocus::makeSilhouette(Texture& source, const Color& colorKey, const Color& fill) {
	const uint32_t w = source.getWidth();
	const uint32_t h = source.getHeight();
	auto silhouette = Provider::provideTexture(w, h);
	if (w == 0 || h == 0) {
		return silhouette;
	}
	uint8_t r, g, b, a;
	for (uint32_t i = 0; i < w * h; i++) {
		source.getPixel(i, &r, &g, &b, &a);
		if (r == colorKey.red && g == colorKey.green && b == colorKey.blue) {
			silhouette->setPixel(i, colorKey.red, colorKey.green, colorKey.blue, 255);
		}
		else {
			silhouette->setPixel(i, fill.red, fill.green, fill.blue, 255);
		}
	}
	silhouette->setColorKey(colorKey.red, colorKey.green, colorKey.blue);
	return silhouette;
}

Color pocus::paletteColor(const data::asset::Palette& palette, uint8_t index) {
	const data::asset::PaletteColor& c = palette.colors[index % 128];
	return Color { c.r, c.g, c.b, 255 };
}
