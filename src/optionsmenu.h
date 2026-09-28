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

#ifndef OPTIONSMENU_H
#define OPTIONSMENU_H

#include <functional>
#include "engine/menu.h"
#include "screens/screen.h"

// The original's "Change game options" menu (menu 2, handlers at 1548:0745)
// and its "Game playing speed" sub-menu (menu 7), shared by the main menu
// and the in-game ESC menu. Sound/music/joystick toggle their HOCUS.SAV
// flags, Volume control and Define key controls open their screens, and
// leaving the menu writes HOCUS.SAV back (1548:0858).
class OptionsMenu {
public:
	struct Callbacks {
		std::function<void()> onBack;            // ESC: the caller returns to its previous menu
		std::function<void()> onSpeed;           // "Game playing speed" chosen
		std::function<void()> onMusicToggled;    // music flag changed (start/stop the caller's tune)
		std::function<void()> onRebuild;         // the texts changed, redraw this menu
	};

	static void buildOptions(pocus::Menu& menu, pocus::ui::ScreenHost& host, pocus::ui::ScreenAssets& assets, const Callbacks& callbacks);
	static void buildSpeed(pocus::Menu& menu, std::function<void()> onBack, std::function<void()> onChanged);

	// Pushes the saved volumes and on/off switches into the mixer.
	static void applyAudioSettings();
	// Writes HOCUS.SAV; on failure shows the original's "I/O Error - HOCUS.SAV" box.
	static void storeSettings(pocus::ui::ScreenHost& host, pocus::ui::ScreenAssets& assets, std::function<void()> then);
};

#endif // OPTIONSMENU_H
