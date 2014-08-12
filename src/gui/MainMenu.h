/*
 * Copyright 2014 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Arx Libertatis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arx Libertatis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Arx Libertatis.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ARX_GUI_MAINMENU_H
#define ARX_GUI_MAINMENU_H

#include <string>

#include "math/Types.h"

// FIXME remove this
const std::string AUTO_RESOLUTION_STRING = "Automatic";

enum MENUSTATE
{
	MAIN,
	RESUME_GAME,
	NEW_QUEST,
	NEW_QUEST_ENTER_GAME,
	EDIT_QUEST,
	EDIT_QUEST_LOAD,
	EDIT_QUEST_SAVE,
	EDIT_QUEST_SAVE_CONFIRM,
	MULTIPLAYER,
	OPTIONS,
	OPTIONS_VIDEO,
	OPTIONS_VIDEO_RESOLUTION,
	OPTIONS_AUDIO,
	OPTIONS_INPUT,
	OPTIONS_INPUT_CUSTOMIZE_KEYS_1,
	OPTIONS_INPUT_CUSTOMIZE_KEYS_2,
	CREDITS,
	BACK,
	QUIT,
	NOP,
	OPTIONS_LOD,
	OPTIONS_OTHERDETAILS,
	OPTIONS_VIDEO_RENDERER_OPENGL,
	OPTIONS_VIDEO_RENDERER_AUTOMATIC,
	OPTIONS_AUDIO_BACKEND_OPENAL,
	OPTIONS_AUDIO_BACKEND_AUTOMATIC,
	SAVE_QUEST_0 = 100,
	SAVE_QUEST_1 = 101,
	SAVE_QUEST_2 = 102,
	SAVE_QUEST_3 = 103,
	SAVE_QUEST_4 = 104,
	SAVE_QUEST_5 = 105,
	SAVE_QUEST_6 = 106,
	SAVE_QUEST_7 = 107,
	SAVE_QUEST_8 = 108,
	SAVE_QUEST_9 = 109,
	OPTIONS_VIDEO_RESOLUTION_0 = 200,
	OPTIONS_AUDIO_VOLUME = 300,
	OPTIONS_INPUT_KEY_0 = 400
};

void MainMenuLeftCreate(MENUSTATE eMenuState);

#endif // ARX_GUI_MAINMENU_H
