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

#include "optionsmenu.h"
#include "settings.h"
#include "exedata.h"
#include "engine/audiocontrol.h"
#include "engine/log.h"
#include "screens/message.h"
#include "screens/volume.h"


namespace {
std::string helpSub() { return pocus::ui::padPrompts() ? "Use UP/DOWN to move - START to select" : pocus::ExeData::get().string(pocus::STR_SUB_MENU_HELP); }
std::string ioError() { return pocus::ExeData::get().string(pocus::STR_IO_ERROR); }
// The options item as the EXE's table holds it ("Sound is now off") with the
// state it names replaced by the current one.
std::string stateItem(int index, bool on) {
	std::string item = pocus::ExeData::get().pointerString(pocus::PTR_MENU_OPTIONS, index);
	const size_t space = item.rfind(' ');
	return (space == std::string::npos ? item : item.substr(0, space + 1)) + (on ? "on" : "off");
}
}

void OptionsMenu::applyAudioSettings() {
	const GameSettings& settings = GameSettings::get();
	pocus::AudioControl::setSoundVolume(settings.save.soundVolume);
	pocus::AudioControl::setMusicVolume(settings.save.musicVolume);
	pocus::AudioControl::setSoundEnabled(settings.sound());
	pocus::AudioControl::setMusicEnabled(settings.music());
}

void OptionsMenu::storeSettings(pocus::ui::ScreenHost& host, pocus::ui::ScreenAssets& assets, std::function<void()> then) {
	if (GameSettings::get().store()) {
		if (then) {
			then();
		}
		return;
	}
	host.push(std::make_unique<pocus::ui::MessageScreen>(assets, ioError()), [then](int) {
		if (then) {
			then();
		}
	});
}

// Menu 2: the first three lines report the current state, as the original
// rewrites them ("Sound is now on/off" ...). The original's "Joystick is now
// on/off" and "Define key controls" items are deliberately left out of the
// port's menu (no joystick support; the keys stay as HOCUS.SAV binds them).
void OptionsMenu::buildOptions(pocus::Menu& menu, pocus::ui::ScreenHost& host, pocus::ui::ScreenAssets& assets, const Callbacks& callbacks) {
	GameSettings& settings = GameSettings::get();
	menu.clear();
	menu.setBottomText(helpSub());
	// 1548:0858: leaving the options menu writes HOCUS.SAV.
	menu.setEscapeHandler([&host, &assets, callbacks] {
		storeSettings(host, assets, callbacks.onBack);
	});
	menu.addOption(stateItem(0, settings.sound()), [callbacks] {
		GameSettings::get().setSound(!GameSettings::get().sound());
		pocus::AudioControl::setSoundEnabled(GameSettings::get().sound());
		if (callbacks.onRebuild) {
			callbacks.onRebuild();
		}
	});
	menu.addOption(stateItem(1, settings.music()), [callbacks] {
		GameSettings::get().setMusic(!GameSettings::get().music());
		pocus::AudioControl::setMusicEnabled(GameSettings::get().music());
		if (callbacks.onMusicToggled) {
			callbacks.onMusicToggled();
		}
		if (callbacks.onRebuild) {
			callbacks.onRebuild();
		}
	});
	menu.addOption(pocus::ExeData::get().pointerString(pocus::PTR_MENU_OPTIONS, 2), [&host, &assets, callbacks] {
		host.push(std::make_unique<pocus::ui::VolumeScreen>(assets, GameSettings::get().save, [] { return GameSettings::get().store(); }),
			[&host, &assets, callbacks](int result) {
				if (result == 2) {
					host.push(std::make_unique<pocus::ui::MessageScreen>(assets, ioError()), [callbacks](int) {
						if (callbacks.onRebuild) {
							callbacks.onRebuild();
						}
					});
					return;
				}
				if (callbacks.onRebuild) {
					callbacks.onRebuild();
				}
			});
	});
	menu.addOption(pocus::ExeData::get().pointerString(pocus::PTR_MENU_OPTIONS, 4), [callbacks] {
		if (callbacks.onSpeed) {
			callbacks.onSpeed();
		}
	});
	menu.layoutLikeOriginal();
}

// Menu 7.
void OptionsMenu::buildSpeed(pocus::Menu& menu, std::function<void()> onBack, std::function<void()> onChanged) {
	menu.clear();
	menu.setBottomText(helpSub());
	menu.setEscapeHandler(onBack);
	const std::vector<std::string> names = pocus::ExeData::get().pointerStrings(pocus::PTR_MENU_SPEED);
	menu.addTitle(names[0]);
	for (int i = 0; i < 3; i++) {
		menu.addOption(names[i + 1], [i, onBack, onChanged] {
			// 1548:081f: 790e and 48d0 take the index.
			GameSettings::get().setSpeed(i);
			if (onChanged) {
				onChanged();
			}
			if (onBack) {
				onBack();
			}
		});
	}
	menu.layoutLikeOriginal();
}
