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

#ifndef TEXTURETOOLS_H
#define TEXTURETOOLS_H

#include <memory>
#include "texture.h"
#include "color.h"
#include "data/asset/palette.h"

namespace pocus {

// A copy of a colour-keyed sprite frame with every opaque pixel replaced by
// one colour - what HOCUS.EXE's "flash" blitters (1ba5:2896 and friends)
// draw: they write the same palette index for every sprite pixel instead
// of the sprite's own. Used for the hurt / super-shot flashes of Hocus and
// the hit flash of monsters.
std::unique_ptr<Texture> makeSilhouette(Texture& source, const Color& colorKey, const Color& fill);

// A palette entry as a Color.
Color paletteColor(const data::asset::Palette& palette, uint8_t index);

}

#endif // TEXTURETOOLS_H
