/*
This source file is part of Rigs of Rods
Copyright 2005-2012 Pierre-Michel Ricordel
Copyright 2007-2012 Thomas Fischer

For more information, see http://www.rigsofrods.com/

Rigs of Rods is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License version 3, as
published by the Free Software Foundation.

Rigs of Rods is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rigs of Rods.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "InputEngine.h"

// some gcc fixes
#if OGRE_PLATFORM == OGRE_PLATFORM_LINUX
#pragma GCC diagnostic ignored "-Wfloat-equal"
#endif //OGRE_PLATFORM_LINUX

#include <Ogre.h>
#include <OgreStringConverter.h>
#include <OgreException.h>

#include "AdvancedOgreFramework.h"
#include "errorutils.h"
#include "RoRWindowEventUtilities.h"
#include "Settings.h"

#ifndef NOOGRE
#include "Console.h"
#include "gui_manager.h"
#include "language.h"
#else
#define _L(x) x
#endif

const char *mOISDeviceType[6] = {"Unknown Device", "Keyboard", "Mouse", "JoyStick", "Tablet", "Other Device"};

// LOOOONG list of possible events. see the struct type for the structure ;)
eventInfo_t eventInfo[] = {
	{
		"AIRPLANE_STEER_RIGHT",
		EV_AIRPLANE_STEER_RIGHT,
		"Keyboard RIGHT",
		_L("steer right")
	},
	{
		"AIRPLANE_BRAKE",
		EV_AIRPLANE_BRAKE,
		"Keyboard B",
		_L("normal brake for an aircraft")
	},
	{
		"AIRPLANE_ELEVATOR_DOWN",
		EV_AIRPLANE_ELEVATOR_DOWN,
		"Keyboard DOWN",
		_L("pull the elevator down in an aircraft.")
	},
	{
		"AIRPLANE_ELEVATOR_UP",
		EV_AIRPLANE_ELEVATOR_UP,
		"Keyboard UP",
		_L("pull the elevator up in an aircraft.")
	},
	{
		"AIRPLANE_FLAPS_FULL",
		EV_AIRPLANE_FLAPS_FULL,
		"Keyboard CTRL+2",
		_L("full flaps in an aircraft.")
	},
	{
		"AIRPLANE_FLAPS_LESS",
		EV_AIRPLANE_FLAPS_LESS,
		"Keyboard EXPL+1",
		_L("one step less flaps.")
	},
	{
		"AIRPLANE_FLAPS_MORE",
		EV_AIRPLANE_FLAPS_MORE,
		"Keyboard EXPL+2",
		_L("one step more flaps.")
	},
	{
		"AIRPLANE_FLAPS_NONE",
		EV_AIRPLANE_FLAPS_NONE,
		"Keyboard CTRL+1",
		_L("no flaps.")
	},
	{
		"AIRPLANE_PARKING_BRAKE",
		EV_AIRPLANE_PARKING_BRAKE,
		"Keyboard P",
		_L("airplane parking brake.")

	},
	{
		"AIRPLANE_REVERSE",
		EV_AIRPLANE_REVERSE,
		"Keyboard R",
		_L("reverse the turboprops")
	},
	{
		"AIRPLANE_RUDDER_LEFT",
		EV_AIRPLANE_RUDDER_LEFT,
		"Keyboard Z",
		_L("rudder left")
	},
	{
		"AIRPLANE_RUDDER_RIGHT",
		EV_AIRPLANE_RUDDER_RIGHT,
		"Keyboard X",
		_L("rudder right")
	},
	{
		"AIRPLANE_STEER_LEFT",
		EV_AIRPLANE_STEER_LEFT,
		"Keyboard LEFT",
		_L("steer left")
	},
	{
		"AIRPLANE_STEER_RIGHT",
		EV_AIRPLANE_STEER_RIGHT,
		"Keyboard RIGHT",
		_L("steer right")
	},
	{
		"AIRPLANE_THROTTLE_AXIS",
		EV_AIRPLANE_THROTTLE_AXIS,
		"None",
		_L("throttle axis. Only use this if you have fitting hardware :) (i.e. a Slider)")
	},
	{
		"AIRPLANE_THROTTLE_DOWN",
		EV_AIRPLANE_THROTTLE_DOWN,
		"Keyboard EXPL+PGDOWN",
		_L("decreases the airplane thrust")
	},
	{
		"AIRPLANE_THROTTLE_FULL",
		EV_AIRPLANE_THROTTLE_FULL,
		"Keyboard CTRL+PGUP",
		_L("full thrust")
	},
	{
		"AIRPLANE_THROTTLE_NO",
		EV_AIRPLANE_THROTTLE_NO,
		"Keyboard CTRL+PGDOWN",
		_L("no thrust")
	},
	{
		"AIRPLANE_THROTTLE_UP",
		EV_AIRPLANE_THROTTLE_UP,
		"Keyboard EXPL+PGUP",
		_L("increase the airplane thrust")
	},
	{
		"AIRPLANE_TOGGLE_ENGINES",
		EV_AIRPLANE_TOGGLE_ENGINES,
		"Keyboard CTRL+HOME",
		_L("switch all engines on / off")
	},
	{
		"BOAT_CENTER_RUDDER",
		EV_BOAT_CENTER_RUDDER,
		"Keyboard DOWN",
		_L("center the rudder")
	},
	{
		"BOAT_REVERSE",
		EV_BOAT_REVERSE,
		"Keyboard UP",
		_L("no thrust")
	},
	{
		"BOAT_STEER_LEFT",
		EV_BOAT_STEER_LEFT,
		"Keyboard LEFT",
		_L("steer left a step")
	},
	{
		"BOAT_STEER_LEFT_AXIS",
		EV_BOAT_STEER_LEFT_AXIS,
		"None",
		_L("steer left (analog value!)")
	},
	{
		"BOAT_STEER_RIGHT",
		EV_BOAT_STEER_RIGHT,
		"Keyboard RIGHT",
		_L("steer right a step")
	},
	{
		"BOAT_STEER_RIGHT_AXIS",
		EV_BOAT_STEER_RIGHT_AXIS,
		"None",
		_L("steer right (analog value!)")
	},
	{
		"BOAT_THROTTLE_AXIS",
		EV_BOAT_THROTTLE_AXIS,
		"None",
		_L("throttle axis. Only use this if you have fitting hardware :) (i.e. a Slider)")
	},
	{
		"BOAT_THROTTLE_DOWN",
		EV_BOAT_THROTTLE_DOWN,
		"Keyboard PGDOWN",
		_L("decrease throttle")
	},
	{
		"BOAT_THROTTLE_UP",
		EV_BOAT_THROTTLE_UP,
		"Keyboard PGUP",
		_L("increase throttle")
	},
	{
		"CAELUM_DECREASE_TIME",
		EV_CAELUM_DECREASE_TIME,
		"Keyboard EXPL+SUBTRACT",
		_L("decrease day-time")
	},
	{
		"CAELUM_DECREASE_TIME_FAST",
		EV_CAELUM_DECREASE_TIME_FAST,
		"Keyboard SHIFT+SUBTRACT",
		_L("decrease day-time a lot faster")
	},
	{
		"CAELUM_INCREASE_TIME",
		EV_CAELUM_INCREASE_TIME,
		"Keyboard EXPL+ADD",
		_L("increase day-time")
	},
	{
		"CAELUM_INCREASE_TIME_FAST",
		EV_CAELUM_INCREASE_TIME_FAST,
		"Keyboard SHIFT+ADD",
		_L("increase day-time a lot faster")
	},
	{
		"CAMERA_CHANGE",
		EV_CAMERA_CHANGE,
		"Keyboard EXPL+C",
		_L("change camera mode")
	},
	{
		"CAMERA_LOOKBACK",
		EV_CAMERA_LOOKBACK,
		"Keyboard NUMPAD1",
		_L("look back (toggles between normal and lookback)")
	},
	{
		"CAMERA_RESET",
		EV_CAMERA_RESET,
		"Keyboard NUMPAD5",
		_L("reset the camera position")
	},
	{
		"CAMERA_ROTATE_DOWN",
		EV_CAMERA_ROTATE_DOWN,
		"Keyboard NUMPAD2",
		_L("rotate camera down")
	},
	{
		"CAMERA_ROTATE_LEFT",
		EV_CAMERA_ROTATE_LEFT,
		"Keyboard NUMPAD4",
		_L("rotate camera left")
	},
	{
		"CAMERA_ROTATE_RIGHT",
		EV_CAMERA_ROTATE_RIGHT,
		"Keyboard NUMPAD6",
		_L("rotate camera right")
	},
	{
		"CAMERA_ROTATE_UP",
		EV_CAMERA_ROTATE_UP,
		"Keyboard NUMPAD8",
		_L("rotate camera up")
	},
	{
		"CAMERA_ZOOM_IN",
		EV_CAMERA_ZOOM_IN,
		"Keyboard EXPL+NUMPAD9",
		_L("zoom camera in")
	},
	{
		"CAMERA_ZOOM_IN_FAST",
		EV_CAMERA_ZOOM_IN_FAST,
		"Keyboard SHIFT+NUMPAD9",
		_L("zoom camera in faster")
	},
	{
		"CAMERA_ZOOM_OUT",
		EV_CAMERA_ZOOM_OUT,
		"Keyboard EXPL+NUMPAD3",
		_L("zoom camera out")
	},
	{
		"CAMERA_ZOOM_OUT_FAST",
		EV_CAMERA_ZOOM_OUT_FAST,
		"Keyboard SHIFT+NUMPAD3",
		_L("zoom camera out faster")
	},
	{
		"CHARACTER_BACKWARDS",
		EV_CHARACTER_BACKWARDS,
		"Keyboard S",
		_L("step backwards with the character")
	},
	{
		"CHARACTER_FORWARD",
		EV_CHARACTER_FORWARD,
		"Keyboard W",
		_L("step forward with the character")
	},
	{
		"CHARACTER_JUMP",
		EV_CHARACTER_JUMP,
		"Keyboard SPACE",
		_L("let the character jump")
	},
	{
		"CHARACTER_LEFT",
		EV_CHARACTER_LEFT,
		"Keyboard LEFT",
		_L("rotate character left")
	},
	{
		"CHARACTER_RIGHT",
		EV_CHARACTER_RIGHT,
		"Keyboard RIGHT",
		_L("rotate character right")
	},
	{
		"CHARACTER_RUN",
		EV_CHARACTER_RUN,
		"Keyboard SHIFT+W",
		_L("let the character run")
	},
	{
		"CHARACTER_SIDESTEP_LEFT",
		EV_CHARACTER_SIDESTEP_LEFT,
		"Keyboard A",
		_L("sidestep to the left")
	},
	{
		"CHARACTER_SIDESTEP_RIGHT",
		EV_CHARACTER_SIDESTEP_RIGHT,
		"Keyboard D",
		_L("sidestep to the right")
	},
	{
		"COMMANDS_01",
		EV_COMMANDS_01,
		"Keyboard EXPL+F1",
		_L("Command 1")
	},
	{
		"COMMANDS_02",
		EV_COMMANDS_02,
		"Keyboard EXPL+F2",
		_L("Command 2")
	},
	{
		"COMMANDS_03",
		EV_COMMANDS_03,
		"Keyboard EXPL+F3",
		_L("Command 3")
	},
	{
		"COMMANDS_04",
		EV_COMMANDS_04,
		"Keyboard EXPL+F4",
		_L("Command 4")
	},
	{
		"COMMANDS_05",
		EV_COMMANDS_05,
		"Keyboard EXPL+F5",
		_L("Command 5")
	},
	{
		"COMMANDS_06",
		EV_COMMANDS_06,
		"Keyboard EXPL+F6",
		_L("Command 6")
	},
	{
		"COMMANDS_07",
		EV_COMMANDS_07,
		"Keyboard EXPL+F7",
		_L("Command 7")
	},
	{
		"COMMANDS_08",
		EV_COMMANDS_08,
		"Keyboard EXPL+F8",
		_L("Command 8")
	},
	{
		"COMMANDS_09",
		EV_COMMANDS_09,
		"Keyboard EXPL+F9",
		_L("Command 9")
	},
	{
		"COMMANDS_10",
		EV_COMMANDS_10,
		"Keyboard EXPL+F10",
		_L("Command 10")
	},
	{
		"COMMANDS_11",
		EV_COMMANDS_11,
		"Keyboard EXPL+F11",
		_L("Command 11")
	},
	{
		"COMMANDS_12",
		EV_COMMANDS_12,
		"Keyboard EXPL+F12",
		_L("Command 12")
	},
	{
		"COMMANDS_13",
		EV_COMMANDS_13,
		"Keyboard EXPL+CTRL+F1",
		_L("Command 13")
	},
	{
		"COMMANDS_14",
		EV_COMMANDS_14,
		"Keyboard EXPL+CTRL+F2",
		_L("Command 14")
	},
	{
		"COMMANDS_15",
		EV_COMMANDS_15,
		"Keyboard EXPL+CTRL+F3",
		_L("Command 15")
	},
	{
		"COMMANDS_16",
		EV_COMMANDS_16,
		"Keyboard EXPL+CTRL+F4",
		_L("Command 16")
	},
	{
		"COMMANDS_17",
		EV_COMMANDS_17,
		"Keyboard EXPL+CTRL+F5",
		_L("Command 17")
	},
	{
		"COMMANDS_18",
		EV_COMMANDS_18,
		"Keyboard EXPL+CTRL+F6",
		_L("Command 18")
	},
	{
		"COMMANDS_19",
		EV_COMMANDS_19,
		"Keyboard EXPL+CTRL+F7",
		_L("Command 19")
	},
	{
		"COMMANDS_20",
		EV_COMMANDS_20,
		"Keyboard EXPL+CTRL+F8",
		_L("Command 20")
	},
	{
		"COMMANDS_21",
		EV_COMMANDS_21,
		"Keyboard EXPL+CTRL+F9",
		_L("Command 21")
	},
	{
		"COMMANDS_22",
		EV_COMMANDS_22,
		"Keyboard EXPL+CTRL+F10",
		_L("Command 22")
	},
	{
		"COMMANDS_23",
		EV_COMMANDS_23,
		"Keyboard EXPL+CTRL+F11",
		_L("Command 23")
	},
	{
		"COMMANDS_24",
		EV_COMMANDS_24,
		"Keyboard EXPL+CTRL+F12",
		_L("Command 24")
	},
	{
		"COMMANDS_25",
		EV_COMMANDS_25,
		"Keyboard EXPL+ALT+F1",
		_L("Command 25")
	},
	{
		"COMMANDS_26",
		EV_COMMANDS_26,
		"Keyboard EXPL+ALT+F2",
		_L("Command 26")
	},
	{
		"COMMANDS_27",
		EV_COMMANDS_27,
		"Keyboard EXPL+ALT+F3",
		_L("Command 27")
	},
	{
		"COMMANDS_28",
		EV_COMMANDS_28,
		"Keyboard EXPL+ALT+F4",
		_L("Command 28")
	},
	{
		"COMMANDS_29",
		EV_COMMANDS_29,
		"Keyboard EXPL+ALT+F5",
		_L("Command 29")
	},
	{
		"COMMANDS_30",
		EV_COMMANDS_30,
		"Keyboard EXPL+ALT+F6",
		_L("Command 30")
	},
	{
		"COMMANDS_31",
		EV_COMMANDS_31,
		"Keyboard EXPL+ALT+F7",
		_L("Command 31")
	},
	{
		"COMMANDS_32",
		EV_COMMANDS_32,
		"Keyboard EXPL+ALT+F8",
		_L("Command 32")
	},
	{
		"COMMANDS_33",
		EV_COMMANDS_33,
		"Keyboard EXPL+ALT+F9",
		_L("Command 33")
	},
	{
		"COMMANDS_34",
		EV_COMMANDS_34,
		"Keyboard EXPL+ALT+F10",
		_L("Command 34")
	},
	{
		"COMMANDS_35",
		EV_COMMANDS_35,
		"Keyboard EXPL+ALT+F11",
		_L("Command 35")
	},
	{
		"COMMANDS_36",
		EV_COMMANDS_36,
		"Keyboard EXPL+ALT+F12",
		_L("Command 36")
	},
	{
		"COMMANDS_37",
		EV_COMMANDS_37,
		"Keyboard EXPL+CTRL+ALT+F1",
		_L("Command 37")
	},
	{
		"COMMANDS_38",
		EV_COMMANDS_38,
		"Keyboard EXPL+CTRL+ALT+F2",
		_L("Command 38")
	},
	{
		"COMMANDS_39",
		EV_COMMANDS_39,
		"Keyboard EXPL+CTRL+ALT+F3",
		_L("Command 39")
	},
	{
		"COMMANDS_40",
		EV_COMMANDS_40,
		"Keyboard EXPL+CTRL+ALT+F4",
		_L("Command 40")
	},
	{
		"COMMANDS_41",
		EV_COMMANDS_41,
		"Keyboard EXPL+CTRL+ALT+F5",
		_L("Command 41")
	},
	{
		"COMMANDS_42",
		EV_COMMANDS_42,
		"Keyboard EXPL+CTRL+ALT+F6",
		_L("Command 42")
	},
	{
		"COMMANDS_43",
		EV_COMMANDS_43,
		"Keyboard EXPL+CTRL+ALT+F7",
		_L("Command 43")
	},
	{
		"COMMANDS_44",
		EV_COMMANDS_44,
		"Keyboard EXPL+CTRL+ALT+F8",
		_L("Command 44")
	},
	{
		"COMMANDS_45",
		EV_COMMANDS_45,
		"Keyboard EXPL+CTRL+ALT+F9",
		_L("Command 45")
	},
	{
		"COMMANDS_46",
		EV_COMMANDS_46,
		"Keyboard EXPL+CTRL+ALT+F10",
		_L("Command 46")
	},
	{
		"COMMANDS_47",
		EV_COMMANDS_47,
		"Keyboard EXPL+CTRL+ALT+F11",
		_L("Command 47")
	},
	{
		"COMMANDS_48",
		EV_COMMANDS_48,
		"Keyboard EXPL+CTRL+ALT+F12",
		_L("Command 48")
	},
	{
		"COMMON_CONSOLEDISPLAY",
		EV_COMMON_CONSOLEDISPLAY,
		"Keyboard EXPL+GRAVE",
		_L("show / hide the console")
	},
	{
		"COMMON_CONSOLEMODE",
		EV_COMMON_CONSOLEMODE,
		"Keyboard EXPL+CTRL+GRAVE",
		_L("toggle appearance of console")
	},
	{
		"COMMON_ENTER_CHATMODE",
		EV_COMMON_ENTER_CHATMODE,
		"Keyboard Y",
		_L("enter the chat")
	},
	{
		"COMMON_SEND_CHAT",
		EV_COMMON_SEND_CHAT,
		"Keyboard RETURN",
		_L("sends the entered text")
	},
	{
		"COMMON_ENTER_OR_EXIT_TRUCK",
		EV_COMMON_ENTER_OR_EXIT_TRUCK,
		"Keyboard RETURN",
		_L("enter or exit a truck")
	},
	{
		"COMMON_HIDE_GUI",
		EV_COMMON_HIDE_GUI,
		"Keyboard EXPL+U",
		_L("hide all GUI elements")
	},
	{
		"COMMON_LOCK",
		EV_COMMON_LOCK,
		"Keyboard EXPL+L",
		_L("connect hook node to a node in close proximity")
	},
	{
		"COMMON_AUTOLOCK",
		EV_COMMON_AUTOLOCK,
		"Keyboard EXPL+ALT+L",
		_L("unlock autolock hook node")
	},
	{
		"COMMON_ROPELOCK",
		EV_COMMON_ROPELOCK,
		"Keyboard EXPL+CTRL+L",
		_L("connect a rope to a node in close proximity")
	},
	{
		"COMMON_OUTPUT_POSITION",
		EV_COMMON_OUTPUT_POSITION,
		"Keyboard H",
		_L("write current position to log (you can open the logfile and reuse the position)")
	},
	{
		"COMMON_PRESSURE_LESS",
		EV_COMMON_PRESSURE_LESS,
		"Keyboard LBRACKET",
		_L("decrease tire pressure (note: only very few trucks support this)")
	},
	{
		"COMMON_PRESSURE_MORE",
		EV_COMMON_PRESSURE_MORE,
		"Keyboard RBRACKET",
		_L("increase tire pressure (note: only very few trucks support this)")
	},
	{
		"COMMON_QUIT_GAME",
		EV_COMMON_QUIT_GAME,
		"Keyboard EXPL+ESCAPE",
		_L("exit the game")
	},
	{
		"COMMON_REPAIR_TRUCK",
		EV_COMMON_REPAIR_TRUCK,
		"Keyboard BACK",
		_L("repair truck")
	},
	{
		"COMMON_RESCUE_TRUCK",
		EV_COMMON_RESCUE_TRUCK,
		"Keyboard EXPL+R",
		_L("teleport to rescue truck")
	},
	{
		"COMMON_RESET_TRUCK",
		EV_COMMON_RESET_TRUCK,
		"Keyboard I",
		_L("reset truck to original starting position")
	},
	{
		"COMMON_SCREENSHOT",
		EV_COMMON_SCREENSHOT,
		"Keyboard EXPL+SYSRQ",
		_L("take a screenshot")
	},
	{
		"COMMON_SCREENSHOT_BIG",
		EV_COMMON_SCREENSHOT_BIG,
		"Keyboard EXPL+CTRL+SYSRQ",
		_L("take a big screenshot (3 times the screen size)")
	},
	{
		"COMMON_SAVE_TERRAIN",
		EV_COMMON_SAVE_TERRAIN,
		"Keyboard EXPL+ALT+SHIF+CTRL+M",
		_L("save the currently loaded terrain to a mesh file")
	},
	{
		"COMMON_SECURE_LOAD",
		EV_COMMON_SECURE_LOAD,
		"Keyboard O",
		_L("tie a load to the truck")
	},
	{
		"COMMON_SHOW_SKELETON",
		EV_COMMON_SHOW_SKELETON,
		"Keyboard K",
		_L("toggle skeleton display mode")
	},
	{
		"COMMON_START_TRUCK_EDITOR",
		EV_COMMON_START_TRUCK_EDITOR,
		"Keyboard EXPL+SHIFT+Y",
		_L("start the old truck editor")
	},
	{
		"COMMON_TOGGLE_CUSTOM_PARTICLES",
		EV_COMMON_TOGGLE_CUSTOM_PARTICLES,
		"Keyboard G",
		_L("toggle particle cannon")
	},
	{
		"COMMON_TOGGLE_MAT_DEBUG",
		EV_COMMON_TOGGLE_MAT_DEBUG,
		"",
		_L("debug purpose - dont use")
	},
	{
		"COMMON_TOGGLE_RENDER_MODE",
		EV_COMMON_TOGGLE_RENDER_MODE,
		"Keyboard E",
		_L("toggle render mode (solid, wireframe and points)")
	},
	{
		"COMMON_TOGGLE_REPLAY_MODE",
		EV_COMMON_TOGGLE_REPLAY_MODE,
		"Keyboard J",
		_L("enable or disable replay mode")
	},
	{
		"COMMON_TOGGLE_STATS",
		EV_COMMON_TOGGLE_STATS,
		"Keyboard EXPL+F",
		_L("toggle Ogre statistics (FPS etc.)")
	},
	{
		"COMMON_TOGGLE_TRUCK_BEACONS",
		EV_COMMON_TOGGLE_TRUCK_BEACONS,
		"Keyboard M",
		_L("toggle truck beacons")
	},
	{
		"COMMON_TOGGLE_TRUCK_LIGHTS",
		EV_COMMON_TOGGLE_TRUCK_LIGHTS,
		"Keyboard N",
		_L("toggle truck front lights")
	},
	{
		"COMMON_TRUCK_INFO",
		EV_COMMON_TRUCK_INFO,
		"Keyboard EXPL+T",
		_L("toggle truck HUD")
	},
	{
		"COMMON_FOV_LESS",
		EV_COMMON_FOV_LESS,
		"Keyboard EXPL+NUMPAD7",
		_L("decreases the current FOV value")
	},
	{
		"COMMON_FOV_MORE",
		EV_COMMON_FOV_MORE,
		"Keyboard EXPL+CTRL+NUMPAD7",
		_L("increase the current FOV value")
	},
	{
		"GRASS_LESS",
		EV_GRASS_LESS,
		"",
		_L("EXPERIMENTAL: remove some grass")
	},
	{
		"GRASS_MORE",
		EV_GRASS_MORE,
		"",
		_L("EXPERIMENTAL: add some grass")
	},
	{
		"GRASS_MOST",
		EV_GRASS_MOST,
		"",
		_L("EXPERIMENTAL: set maximum amount of grass")
	},
	{
		"GRASS_NONE",
		EV_GRASS_NONE,
		"",
		_L("EXPERIMENTAL: remove grass completely")
	},
	{
		"GRASS_SAVE",
		EV_GRASS_SAVE,
		"",
		_L("EXPERIMENTAL: save changes to the grass density image")
	},
	{
		"MENU_DOWN",
		EV_MENU_DOWN,
		"Keyboard DOWN",
		_L("select next element in current category")
	},
	{
		"MENU_LEFT",
		EV_MENU_LEFT,
		"Keyboard LEFT",
		_L("select previous category")
	},
	{
		"MENU_RIGHT",
		EV_MENU_RIGHT,
		"Keyboard RIGHT",
		_L("select next category")
	},
	{
		"MENU_SELECT",
		EV_MENU_SELECT,
		"Keyboard EXPL+RETURN",
		_L("select focussed item and close menu")
	},
	{
		"MENU_UP",
		EV_MENU_UP,
		"Keyboard UP",
		_L("select previous element in current category")
	},
	{
		"SURVEY_MAP_TOGGLE_VIEW",
		EV_SURVEY_MAP_TOGGLE_VIEW,
		"Keyboard EXPL+TAB",
		_L("toggle map modes")
	},
	{
		"SURVEY_MAP_TOGGLE_ALPHA",
		EV_SURVEY_MAP_ALPHA,
		"Keyboard EXPL+CTRL+TAB",
		_L("toggle translucency of overview-map")
	},
	{
		"SURVEY_MAP_ZOOM_IN",
		EV_SURVEY_MAP_ZOOM_IN,
		"Keyboard EXPL+SHIFT+CTRL+TAB",
		_L("zoom in")
	},
	{
		"SURVEY_MAP_ZOOM_OUT",
		EV_SURVEY_MAP_ZOOM_OUT,
		"Keyboard EXPL+SHIFT+TAB",
		_L("zoom out")
	},
	{
		"TERRAINEDITOR_BUILT",
		EV_TERRAINEDITOR_BUILT,
		"Keyboard SPACE",
		_L("place currently selected object at current position")
	},
	{
		"TERRAINEDITOR_PITCHBACKWARD",
		EV_TERRAINEDITOR_PITCHBACKWARD,
		"Keyboard EXPL+DELETE",
		_L("pitch object backward")
	},
	{
		"TERRAINEDITOR_PITCHFOREWARD",
		EV_TERRAINEDITOR_PITCHFOREWARD,
		"Keyboard END",
		_L("pitch object foreward")
	},
	{
		"TERRAINEDITOR_ROTATELEFT",
		EV_TERRAINEDITOR_ROTATELEFT,
		"Keyboard INSERT",
		_L("rotate object left")
	},
	{
		"TERRAINEDITOR_ROTATERIGHT",
		EV_TERRAINEDITOR_ROTATERIGHT,
		"Keyboard HOME",
		_L("rotate object right")
	},
	{
		"TERRAINEDITOR_SELECTROAD",
		EV_TERRAINEDITOR_SELECTROAD,
		"Keyboard B",
		_L("switch to road laying mode")
	},
	{
		"TERRAINEDITOR_TOGGLEOBJECT",
		EV_TERRAINEDITOR_TOGGLEOBJECT,
		"Keyboard V",
		_L("toggle between available objects")
	},
	{
		"TERRAINEDITOR_TOGGLEROADTYPE",
		EV_TERRAINEDITOR_TOGGLEROADTYPE,
		"Keyboard EXPL+CTRL+T",
		_L("toggle between available road types")
	},
	{
		"TRUCK_ACCELERATE",
		EV_TRUCK_ACCELERATE,
		"Keyboard UP",
		_L("accelerate the truck")
	},
	{
		"TRUCK_AUTOSHIFT_DOWN",
		EV_TRUCK_AUTOSHIFT_DOWN,
		"Keyboard PGDOWN",
		_L("shift automatic transmission one gear down")
	},
	{
		"TRUCK_AUTOSHIFT_UP",
		EV_TRUCK_AUTOSHIFT_UP,
		"Keyboard PGUP",
		_L("shift automatic transmission one gear up")
	},
	{
		"TRUCK_BLINK_LEFT",
		EV_TRUCK_BLINK_LEFT,
		"Keyboard COMMA",
		_L("toggle left direction indicator (blinker)")
	},
	{
		"TRUCK_BLINK_RIGHT",
		EV_TRUCK_BLINK_RIGHT,
		"Keyboard PERIOD",
		_L("toggle right direction indicator (blinker)")
	},
	{
		"TRUCK_BLINK_WARN",
		EV_TRUCK_BLINK_WARN,
		"Keyboard MINUS",
		_L("toggle all direction indicators")
	},
	{
		"TRUCK_BRAKE",
		EV_TRUCK_BRAKE,
		"Keyboard DOWN",
		_L("brake")
	},
	{
		"TRUCK_HORN",
		EV_TRUCK_HORN,
		"Keyboard H",
		_L("truck horn")
	},
	{
		"TRUCK_LIGHTTOGGLE1",
		EV_TRUCK_LIGHTTOGGLE1,
		"Keyboard EXPL+CTRL+1",
		_L("toggle custom light 1")
	},
	{
		"TRUCK_LIGHTTOGGLE2",
		EV_TRUCK_LIGHTTOGGLE2,
		"Keyboard EXPL+CTRL+2",
		_L("toggle custom light 2")
	},
	{
		"TRUCK_LIGHTTOGGLE3",
		EV_TRUCK_LIGHTTOGGLE3,
		"Keyboard EXPL+CTRL+3",
		_L("toggle custom light 3")
	},
	{
		"TRUCK_LIGHTTOGGLE4",
		EV_TRUCK_LIGHTTOGGLE4,
		"Keyboard EXPL+CTRL+4",
		_L("toggle custom light 4")
	},
	{
		"TRUCK_LIGHTTOGGLE5",
		EV_TRUCK_LIGHTTOGGLE5,
		"Keyboard EXPL+CTRL+5",
		_L("toggle custom light 5")
	},
	{
		"TRUCK_LIGHTTOGGLE6",
		EV_TRUCK_LIGHTTOGGLE6,
		"Keyboard EXPL+CTRL+6",
		_L("toggle custom light 6")
	},
	{
		"TRUCK_LIGHTTOGGLE7",
		EV_TRUCK_LIGHTTOGGLE7,
		"Keyboard EXPL+CTRL+7",
		_L("toggle custom light 7")
	},
	{
		"TRUCK_LIGHTTOGGLE8",
		EV_TRUCK_LIGHTTOGGLE8,
		"Keyboard EXPL+CTRL+8",
		_L("toggle custom light 8")
	},
	{
		"TRUCK_LIGHTTOGGLE9",
		EV_TRUCK_LIGHTTOGGLE9,
		"Keyboard EXPL+CTRL+9",
		_L("toggle custom light 9")
	},
	{
		"TRUCK_LIGHTTOGGLE10",
		EV_TRUCK_LIGHTTOGGLE10,
		"Keyboard EXPL+CTRL+0",
		_L("toggle custom light 10")
	},
	{
		"TRUCK_MANUAL_CLUTCH",
		EV_TRUCK_MANUAL_CLUTCH,
		"Keyboard LSHIFT",
		_L("manual clutch (for manual transmission)")
	},
	{
		"TRUCK_PARKING_BRAKE",
		EV_TRUCK_PARKING_BRAKE,
		"Keyboard P",
		_L("toggle parking brake")
	},
	{
		"TRUCK_ANTILOCK_BRAKE",
		EV_TRUCK_ANTILOCK_BRAKE,
		"Keyboard EXPL+SHIFT+B",
		_L("toggle antilock brake")
	},
	{
		"TRUCK_TRACTION_CONTROL",
		EV_TRUCK_TRACTION_CONTROL,
		"Keyboard EXPL+SHIFT+T",
		_L("toggle traction control")
	},
	{
		"TRUCK_CRUISE_CONTROL",
		EV_TRUCK_CRUISE_CONTROL,
		"Keyboard EXPL+SPACE",
		_L("toggle cruise control")
	},
	{
		"TRUCK_CRUISE_CONTROL_READJUST",
		EV_TRUCK_CRUISE_CONTROL_READJUST,
		"Keyboard EXPL+SHIFT+SPACE",
		_L("match target speed / rpm with current truck speed / rpm")
	},
	{
		"TRUCK_CRUISE_CONTROL_ACCL",
		EV_TRUCK_CRUISE_CONTROL_ACCL,
		"Keyboard EXPL+SHIFT+R",
		_L("increase target speed / rpm")
	},
	{
		"TRUCK_CRUISE_CONTROL_DECL",
		EV_TRUCK_CRUISE_CONTROL_DECL,
		"Keyboard EXPL+SHIFT+F",
		_L("decrease target speed / rpm")
	},
	{
		"TRUCK_SHIFT_DOWN",
		EV_TRUCK_SHIFT_DOWN,
		"Keyboard Z",
		_L("shift one gear down in manual transmission mode")
	},
	{
		"TRUCK_SHIFT_NEUTRAL",
		EV_TRUCK_SHIFT_NEUTRAL,
		"Keyboard D",
		_L("shift to neutral gear in manual transmission mode")
	},
	{
		"TRUCK_SHIFT_UP",
		EV_TRUCK_SHIFT_UP,
		"Keyboard A",
		_L("shift one gear up in manual transmission mode")
	},
	{
		"TRUCK_SHIFT_GEAR_REVERSE",
		EV_TRUCK_SHIFT_GEAR_REVERSE,
		"",
		_L("shift directly to reverse gear")
	},
	{
		"TRUCK_SHIFT_GEAR1",
		EV_TRUCK_SHIFT_GEAR1,
		"",
		_L("shift directly to first gear")
	},
	{
		"TRUCK_SHIFT_GEAR2",
		EV_TRUCK_SHIFT_GEAR2,
		"",
		_L("shift directly to second gear")
	},
	{
		"TRUCK_SHIFT_GEAR3",
		EV_TRUCK_SHIFT_GEAR3,
		"",
		_L("shift directly to third gear")
	},
	{
		"TRUCK_SHIFT_GEAR4",
		EV_TRUCK_SHIFT_GEAR4,
		"",
		_L("shift directly to fourth gear")
	},
	{
		"TRUCK_SHIFT_GEAR5",
		EV_TRUCK_SHIFT_GEAR5,
		"",
		_L("shift directly to 5th gear")
	},
	{
		"TRUCK_SHIFT_GEAR6",
		EV_TRUCK_SHIFT_GEAR6,
		"",
		_L("shift directly to 6th gear")
	},
	{
		"TRUCK_SHIFT_GEAR7",
		EV_TRUCK_SHIFT_GEAR7,
		"",
		_L("shift directly to 7th gear")
	},
	{
		"TRUCK_SHIFT_GEAR8",
		EV_TRUCK_SHIFT_GEAR8,
		"",
		_L("shift directly to 8th gear")
	},
	{
		"TRUCK_SHIFT_GEAR9",
		EV_TRUCK_SHIFT_GEAR9,
		"",
		_L("shift directly to 9th gear")
	},
	{
		"TRUCK_SHIFT_GEAR10",
		EV_TRUCK_SHIFT_GEAR10,
		"",
		_L("shift directly to 10th gear")
	},
	{
		"TRUCK_SHIFT_GEAR11",
		EV_TRUCK_SHIFT_GEAR11,
		"",
		_L("shift directly to 11th gear")
	},
	{
		"TRUCK_SHIFT_GEAR12",
		EV_TRUCK_SHIFT_GEAR12,
		"",
		_L("shift directly to 12th gear")
	},

	{
		"TRUCK_SHIFT_GEAR13",
		EV_TRUCK_SHIFT_GEAR13,
		"",
		_L("shift directly to 13th gear")
	},
	{
		"TRUCK_SHIFT_GEAR14",
		EV_TRUCK_SHIFT_GEAR14,
		"",
		_L("shift directly to 14th gear")
	},
	{
		"TRUCK_SHIFT_GEAR15",
		EV_TRUCK_SHIFT_GEAR15,
		"",
		_L("shift directly to 15th gear")
	},
	{
		"TRUCK_SHIFT_GEAR16",
		EV_TRUCK_SHIFT_GEAR16,
		"",
		_L("shift directly to 16th gear")
	},
	{
		"TRUCK_SHIFT_GEAR17",
		EV_TRUCK_SHIFT_GEAR17,
		"",
		_L("shift directly to 17th gear")
	},
	{
		"TRUCK_SHIFT_GEAR18",
		EV_TRUCK_SHIFT_GEAR18,
		"",
		_L("shift directly to 18th gear")
	},
	{
		"TRUCK_SHIFT_LOWRANGE",
		EV_TRUCK_SHIFT_LOWRANGE,
		"",
		_L("sets low range (1-6) for H-shaft")
	},
	{
		"TRUCK_SHIFT_MIDRANGE",
		EV_TRUCK_SHIFT_MIDRANGE,
		"",
		_L("sets middle range (7-12) for H-shaft")
	},
	{
		"TRUCK_SHIFT_HIGHRANGE",
		EV_TRUCK_SHIFT_HIGHRANGE,
		"",
		_L("sets high range (13-18) for H-shaft")
	},
	{
		"TRUCK_STARTER",
		EV_TRUCK_STARTER,
		"Keyboard S",
		_L("hold to start the engine")
	},
	{
		"TRUCK_STEER_LEFT",
		EV_TRUCK_STEER_LEFT,
		"Keyboard LEFT",
		_L("steer left")
	},
	{
		"TRUCK_STEER_RIGHT",
		EV_TRUCK_STEER_RIGHT,
		"Keyboard RIGHT",
		_L("steer right")
	},
	{
		"TRUCK_SWITCH_SHIFT_MODES",
		EV_TRUCK_SWITCH_SHIFT_MODES,
		"Keyboard Q",
		_L("toggle between transmission modes")
	},
	{
		"TRUCK_TOGGLE_AXLE_LOCK",
		EV_TRUCK_TOGGLE_AXLE_LOCK,
		"Keyboard W",
		_L("Cycle between available differental models")
	},
	{
		"TRUCK_TOGGLE_CONTACT",
		EV_TRUCK_TOGGLE_CONTACT,
		"Keyboard X",
		_L("toggle ignition")
	},

	// "new" commands
	{
		"COMMON_SHOWTRUCKTOOL",
		EV_COMMON_SHOWTRUCKTOOL,
		"Keyboard EXPL+SHIFT+T",
		_L("show truck tool")
	},
	{
		"COMMON_RELOAD_ROADS",
		EV_COMMON_RELOAD_ROADS,
		"",
		_L("reloads the roads (experimental)")
	},
	{
		"COMMON_FULLSCREEN_TOGGLE",
		EV_COMMON_FULLSCREEN_TOGGLE,
		"Keyboard EXPL+ALT+RETURN",
		_L("toggle between windowed and fullscreen mode")
	},
	{
		"CAMERA_FREE_MODE_FIX",
		EV_CAMERA_FREE_MODE_FIX,
		"Keyboard EXPL+ALT+C",
		_L("fix the camera to a position")
	},
	{
		"CAMERA_FREE_MODE",
		EV_CAMERA_FREE_MODE,
		"Keyboard EXPL+SHIFT+C",
		_L("enable / disable free camera mode")
	},
	{
		"TRUCK_LEFT_MIRROR_LEFT",
		EV_TRUCK_LEFT_MIRROR_LEFT,
		"Keyboard EXPL+SEMICOLON",
		_L("move left mirror to the left")
	},
	{
		"TRUCK_LEFT_MIRROR_RIGHT",
		EV_TRUCK_LEFT_MIRROR_RIGHT,
		"Keyboard EXPL+CTRL+SEMICOLON",
		_L("move left mirror to the right")
	},
	{
		"TRUCK_RIGHT_MIRROR_LEFT",
		EV_TRUCK_RIGHT_MIRROR_LEFT,
		"Keyboard EXPL+COLON",
		_L("more right mirror to the left")
	},
	{
		"TRUCK_RIGHT_MIRROR_RIGHT",
		EV_TRUCK_RIGHT_MIRROR_RIGHT,
		"Keyboard EXPL+CTRL+COLON",
		_L("move right mirror to the right")
	},
	{
		"COMMON_REPLAY_FORWARD",
		EV_COMMON_REPLAY_FORWARD,
		"Keyboard EXPL+RIGHT",
		_L("more replay forward")
	},
	{
		"COMMON_REPLAY_BACKWARD",
		EV_COMMON_REPLAY_BACKWARD,
		"Keyboard EXPL+LEFT",
		_L("more replay backward")
	},
	{
		"COMMON_REPLAY_FAST_FORWARD",
		EV_COMMON_REPLAY_FAST_FORWARD,
		"Keyboard EXPL+SHIFT+RIGHT",
		_L("move replay fast forward")
	},
	{
		"COMMON_REPLAY_FAST_BACKWARD",
		EV_COMMON_REPLAY_FAST_BACKWARD,
		"Keyboard EXPL+SHIFT+LEFT",
		_L("move replay fast backward")
	},
	{
		"AIRPLANE_AIRBRAKES_NONE",
		EV_AIRPLANE_AIRBRAKES_NONE,
		"Keyboard CTRL+3",
		_L("no airbrakes")
	},
	{
		"AIRPLANE_AIRBRAKES_FULL",
		EV_AIRPLANE_AIRBRAKES_FULL,
		"Keyboard CTRL+4",
		_L("full airbrakes")
	},
	{
		"AIRPLANE_AIRBRAKES_LESS",
		EV_AIRPLANE_AIRBRAKES_LESS,
		"Keyboard EXPL+3",
		_L("less airbrakes")
	},
	{
		"AIRPLANE_AIRBRAKES_MORE",
		EV_AIRPLANE_AIRBRAKES_MORE,
		"Keyboard EXPL+4",
		_L("more airbrakes")
	},
	{
		"AIRPLANE_THROTTLE",
		EV_AIRPLANE_THROTTLE,
		"",
		_L("airplane throttle")
	},
	{
		"COMMON_TRUCK_REMOVE",
		EV_COMMON_TRUCK_REMOVE,
		"Keyboard EXPL+CTRL+SHIFT+DELETE",
		_L("delete current truck")
	},
	{
		"COMMON_NETCHATDISPLAY",
		EV_COMMON_NETCHATDISPLAY,
		"Keyboard EXPL+SHIFT+U",
		_L("display or hide net chat")
	},
	{
		"COMMON_NETCHATMODE",
		EV_COMMON_NETCHATMODE,
		"Keyboard EXPL+CTRL+U",
		_L("toggle between net chat display modes")
	},
	{
		"CHARACTER_ROT_UP",
		EV_CHARACTER_ROT_UP,
		"Keyboard UP",
		_L("rotate view up")
	},
	{
		"CHARACTER_ROT_DOWN",
		EV_CHARACTER_ROT_DOWN,
		"Keyboard DOWN",
		_L("rotate view down")
	},
	{
		"CHARACTER_UP",
		EV_CHARACTER_UP,
		"Keyboard PGUP",
		_L("move character up")
	},
	{
		"CHARACTER_DOWN",
		EV_CHARACTER_DOWN,
		"Keyboard PGDOWN",
		_L("move character down")
	},

	// now position storage
	{ "TRUCK_SAVE_POS1", EV_TRUCK_SAVE_POS1, "Keyboard EXPL+ALT+CTRL+1", _L("save position as slot 1") },
	{ "TRUCK_SAVE_POS2", EV_TRUCK_SAVE_POS2, "Keyboard EXPL+ALT+CTRL+2", _L("save position as slot 2") },
	{ "TRUCK_SAVE_POS3", EV_TRUCK_SAVE_POS3, "Keyboard EXPL+ALT+CTRL+3", _L("save position as slot 3") },
	{ "TRUCK_SAVE_POS4", EV_TRUCK_SAVE_POS4, "Keyboard EXPL+ALT+CTRL+4", _L("save position as slot 4") },
	{ "TRUCK_SAVE_POS5", EV_TRUCK_SAVE_POS5, "Keyboard EXPL+ALT+CTRL+5", _L("save position as slot 5") },
	{ "TRUCK_SAVE_POS6", EV_TRUCK_SAVE_POS6, "Keyboard EXPL+ALT+CTRL+6", _L("save position as slot 6") },
	{ "TRUCK_SAVE_POS7", EV_TRUCK_SAVE_POS7, "Keyboard EXPL+ALT+CTRL+7", _L("save position as slot 7") },
	{ "TRUCK_SAVE_POS8", EV_TRUCK_SAVE_POS8, "Keyboard EXPL+ALT+CTRL+8", _L("save position as slot 8") },
	{ "TRUCK_SAVE_POS9", EV_TRUCK_SAVE_POS9, "Keyboard EXPL+ALT+CTRL+9", _L("save position as slot 9") },
	{ "TRUCK_SAVE_POS10", EV_TRUCK_SAVE_POS10, "Keyboard EXPL+ALT+CTRL+0", _L("save position as slot 10") },

	{ "TRUCK_LOAD_POS1", EV_TRUCK_LOAD_POS1, "Keyboard EXPL+ALT+1", _L("load position under slot 1") },
	{ "TRUCK_LOAD_POS2", EV_TRUCK_LOAD_POS2, "Keyboard EXPL+ALT+2", _L("load position under slot 2") },
	{ "TRUCK_LOAD_POS3", EV_TRUCK_LOAD_POS3, "Keyboard EXPL+ALT+3", _L("load position under slot 3") },
	{ "TRUCK_LOAD_POS4", EV_TRUCK_LOAD_POS4, "Keyboard EXPL+ALT+4", _L("load position under slot 4") },
	{ "TRUCK_LOAD_POS5", EV_TRUCK_LOAD_POS5, "Keyboard EXPL+ALT+5", _L("load position under slot 5") },
	{ "TRUCK_LOAD_POS6", EV_TRUCK_LOAD_POS6, "Keyboard EXPL+ALT+6", _L("load position under slot 6") },
	{ "TRUCK_LOAD_POS7", EV_TRUCK_LOAD_POS7, "Keyboard EXPL+ALT+7", _L("load position under slot 7") },
	{ "TRUCK_LOAD_POS8", EV_TRUCK_LOAD_POS8, "Keyboard EXPL+ALT+8", _L("load position under slot 8") },
	{ "TRUCK_LOAD_POS9", EV_TRUCK_LOAD_POS9, "Keyboard EXPL+ALT+9", _L("load position under slot 9") },
	{ "TRUCK_LOAD_POS10", EV_TRUCK_LOAD_POS10, "Keyboard EXPL+ALT+0", _L("load position under slot 10") },


	{ "DOF_TOGGLE", EV_DOF_TOGGLE, "Keyboard EXPL+CTRL+D", _L("turn on Depth of Field on or off") },
	{ "DOF_DEBUG", EV_DOF_DEBUG, "Keyboard EXPL+ALT+D", _L("turn on the Depth of field debug view") },
	{ "DOF_DEBUG_TOGGLE_FOCUS_MODE", EV_DOF_DEBUG_TOGGLE_FOCUS_MODE, "Keyboard EXPL+SPACE", _L("toggle the DOF focus mode") },
	{ "DOF_DEBUG_ZOOM_IN", EV_DOF_DEBUG_ZOOM_IN, "Keyboard EXPL+Q", _L("zoom in") },
	{ "DOF_DEBUG_ZOOM_OUT", EV_DOF_DEBUG_ZOOM_OUT, "Keyboard EXPL+Z", _L("zoom in") },
	{ "DOF_DEBUG_APERTURE_MORE", EV_DOF_DEBUG_APERTURE_MORE, "Keyboard EXPL+1", _L("more aperture") },
	{ "DOF_DEBUG_APERTURE_LESS", EV_DOF_DEBUG_APERTURE_LESS, "Keyboard EXPL+2", _L("less aperture") },
	{ "DOF_DEBUG_FOCUS_IN", EV_DOF_DEBUG_FOCUS_IN, "Keyboard EXPL+3", _L("move focus in") },
	{ "DOF_DEBUG_FOCUS_OUT", EV_DOF_DEBUG_FOCUS_OUT, "Keyboard EXPL+4", _L("move focus out") },

	{ "TRUCKEDIT_RELOAD", EV_TRUCKEDIT_RELOAD, "Keyboard EXPL+SHIFT+CTRL+R", _L("reload truck") },
	
	{ "TOGGLESHADERS", EV_TOGGLESHADERS, "Keyboard EXPL+SHIFT+CTRL+S", _L("toggle shader usage") },

	// the end, DO NOT MODIFY
	{"", -1, "", ""},
};


#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
#define strnlen(str,len) strlen(str)
#endif

//Use this define to signify OIS will be used as a DLL
//(so that dll import/export macros are in effect)
#define OIS_DYNAMIC_LIB
#include <OIS.h>

#if OGRE_PLATFORM == OGRE_PLATFORM_LINUX
#include <X11/Xlib.h>
//#include <linux/LinuxMouse.h>
#endif

#ifndef NOOGRE
#include "Console.h"
#endif

using namespace std;
using namespace Ogre;
using namespace OIS;

// Constructor takes a RenderWindow because it uses that to determine input context
InputEngine::InputEngine() : mInputManager(0), mMouse(0), mKeyboard(0), mForceFeedback(0), captureMode(false), mappingLoaded(false), free_joysticks(0), inputsChanged(true), renderWindow(0)
{
	for(int i=0;i<MAX_JOYSTICKS;i++) mJoy[i]=0;
#ifndef NOOGRE
	LOG("*** Loading OIS ***");
#endif
	initAllKeys();
}

InputEngine::~InputEngine()
{
#ifndef NOOGRE
	LOG("*** Terminating destructor ***");
#endif
	destroy();
}

void InputEngine::destroy()
{
	if( mInputManager )
	{
#ifndef NOOGRE
		LOG("*** Terminating OIS ***");
#endif
		if(mMouse)
		{
			mInputManager->destroyInputObject( mMouse );
			mMouse=0;
		}
		if(mKeyboard)
		{
			mInputManager->destroyInputObject( mKeyboard );
			mKeyboard=0;
		}
		if(mJoy)
		{
			for(int i=0;i<MAX_JOYSTICKS;i++)
			{
				if(!mJoy[i]) continue;
				mInputManager->destroyInputObject(mJoy[i]);
				mJoy[i] = 0;
			}
		}

		OIS::InputManager::destroyInputSystem(mInputManager);
		mInputManager = 0;
	}
}


bool InputEngine::setup(RenderWindow* rw, Ogre::String hwnd, bool capture, bool capturemouse, int _grabMode, bool captureKbd)
{
	renderWindow = rw;
	grabMode = _grabMode;

	// grab mode override in embedded mode
	if(OgreFramework::getSingleton().isEmbedded())
	{
		grabMode = GRAB_DYNAMICALLY;
#ifndef NOOGRE
		LOG("*** EMBEDDED INPUT MODE ***");
#endif
	}
	
#ifndef NOOGRE
	LOG("*** Initializing OIS ***");
#endif
	//try to delete old ones first (linux can only handle one at a time)
	destroy();
	captureMode = capture;
	if(captureMode)
	{
		//size_t hWnd = 0;
		//win->getCustomAttribute("WINDOW", &hWnd);
		ParamList pl;
		
#if 0 //OGRE_PLATFORM == OGRE_PLATFORM_LINUX
		// we get the ogre way of defining the handle, extract the window HWND only
		int screen=0,app=0,windowhandle=0;
		sscanf(hwnd.c_str(), "%d:%d:%d", &screen, &app, &windowhandle);
		hwnd = TOSTRING(windowhandle);
		printf("OIS windowhandle = %s\n", hwnd.c_str());
#endif // LINUX
		
		pl.insert(OIS::ParamList::value_type("WINDOW", hwnd));
		if(grabMode == GRAB_ALL)
		{
		} else if(grabMode == GRAB_DYNAMICALLY || grabMode == GRAB_NONE)
		{
#if OGRE_PLATFORM == OGRE_PLATFORM_LINUX
			pl.insert(OIS::ParamList::value_type("x11_mouse_hide", "false"));
			pl.insert(OIS::ParamList::value_type("XAutoRepeatOn", "false"));
			pl.insert(OIS::ParamList::value_type("x11_mouse_grab", "false"));
			pl.insert(OIS::ParamList::value_type("x11_keyboard_grab", "false"));
#else
			pl.insert(OIS::ParamList::value_type("w32_mouse", "DISCL_FOREGROUND"));
			pl.insert(OIS::ParamList::value_type("w32_mouse", "DISCL_NONEXCLUSIVE"));
			pl.insert(OIS::ParamList::value_type("w32_keyboard", "DISCL_FOREGROUND"));
			pl.insert(OIS::ParamList::value_type("w32_keyboard", "DISCL_NONEXCLUSIVE"));
#endif // LINUX
		}

#ifndef NOOGRE
		LOG("*** OIS WINDOW: "+hwnd);
#endif //NOOGRE

		mInputManager = OIS::InputManager::createInputSystem(pl);

#ifndef NOOGRE
		//Print debugging information
		unsigned int v = mInputManager->getVersionNumber();
		LOG("OIS Version: " + TOSTRING(v>>16) + String(".") + TOSTRING((v>>8) & 0x000000FF) + String(".") + TOSTRING(v & 0x000000FF));
		LOG("+ Release Name: " + mInputManager->getVersionName());
		LOG("+ Manager: " + mInputManager->inputSystemName());
		LOG("+ Total Keyboards: " + TOSTRING(mInputManager->getNumberOfDevices(OISKeyboard)));
		LOG("+ Total Mice: " + TOSTRING(mInputManager->getNumberOfDevices(OISMouse)));
		LOG("+ Total JoySticks: " + TOSTRING(mInputManager->getNumberOfDevices(OISJoyStick)));

		//List all devices
		OIS::DeviceList list = mInputManager->listFreeDevices();
		for(OIS::DeviceList::iterator i = list.begin(); i != list.end(); ++i )
			LOG("* Device: " + String(mOISDeviceType[i->first]) + String(" Vendor: ") + i->second);
#endif //NOOGRE

		//Create all devices (We only catch joystick exceptions here, as, most people have Key/Mouse)
		mKeyboard=0;
		if(captureKbd)
		{
			try
			{
				mKeyboard = static_cast<Keyboard*>(mInputManager->createInputObject( OISKeyboard, true ));
				mKeyboard->setTextTranslation(OIS::Keyboard::Unicode);
			} catch(OIS::Exception &ex)
			{
				LOG(String("Exception raised on keyboard creation: ") + String(ex.eText));
			}
		}


		try
		{
			//This demo uses at most 10 joysticks - use old way to create (i.e. disregard vendor)
			int numSticks = std::min(mInputManager->getNumberOfDevices(OISJoyStick), 10);
			free_joysticks=0;
			for(int i = 0; i < numSticks; ++i)
			{
				mJoy[i] = (JoyStick*)mInputManager->createInputObject(OISJoyStick, true);
				mJoy[i]->setEventCallback(this);
				free_joysticks++;
				//create force feedback too
				//here, we take the first device we can get, but we could take a device index
				if (!mForceFeedback) mForceFeedback = (OIS::ForceFeedback*)mJoy[i]->queryInterface(OIS::Interface::ForceFeedback );

#ifndef NOOGRE
				LOG("Creating Joystick " + TOSTRING(i + 1) + " (" + mJoy[i]->vendor() + ")");
				LOG("* Axes: " + TOSTRING(mJoy[i]->getNumberOfComponents(OIS_Axis)));
				LOG("* Sliders: " + TOSTRING(mJoy[i]->getNumberOfComponents(OIS_Slider)));
				LOG("* POV/HATs: " + TOSTRING(mJoy[i]->getNumberOfComponents(OIS_POV)));
				LOG("* Buttons: " + TOSTRING(mJoy[i]->getNumberOfComponents(OIS_Button)));
				LOG("* Vector3: " + TOSTRING(mJoy[i]->getNumberOfComponents(OIS_Vector3)));
#endif //NOOGRE
			}
		}
#ifndef NOOGRE
		catch(OIS::Exception &ex)
		{
			LOG(String("Exception raised on joystick creation: ") + String(ex.eText));
		}
#else  //NOOGRE
		catch(...)
		{
		}
#endif //NOOGRE

		if(capturemouse)
		{
			try
			{
				mMouse = static_cast<Mouse*>(mInputManager->createInputObject( OISMouse, true ));
			} catch(OIS::Exception &ex)
			{
				LOG(String("Exception raised on mouse creation: ") + String(ex.eText));
			}
		}

		//Set initial mouse clipping size
		//windowResized(win);

		// set Callbacks
		if(mKeyboard)
			mKeyboard->setEventCallback(this);
		if(capturemouse && mMouse)
		{
			mMouse->setEventCallback(this);

			// init states (not required for keyboard)
			mouseState = mMouse->getMouseState();
		}
		if(free_joysticks)
		{
			for(int i=0;i<free_joysticks;i++)
				joyState[i] = mJoy[i]->getJoyStickState();
		}

		// set the mouse to the middle of the screen, hackish!
#if WIN32
		// under linux, this will not work and the cursor will never reach (0,0)
		if(mMouse && renderWindow)
		{
			OIS::MouseState &mutableMouseState = const_cast<OIS::MouseState &>(mMouse->getMouseState());
			mutableMouseState.X.abs = renderWindow->getWidth()  * 0.5f;
			mutableMouseState.Y.abs = renderWindow->getHeight() * 0.5f;
		}
#endif // WIN32
	}
	//this becomes more and more convoluted!
#ifdef NOOGRE
	// we will load the mapping manually
#else //NOOGRE
	if(!mappingLoaded)
	{
		if (!loadMapping())
			return false;
	}
#endif //NOOGRE
	return true;
}

void InputEngine::grabMouse(bool enable)
{
	static int lastmode = -1;
	if(!mMouse)
		return;
#if OGRE_PLATFORM == OGRE_PLATFORM_LINUX
	if((enable && lastmode == 0) || (!enable && lastmode == 1) || (lastmode == -1))
	{
		LOG("*** mouse grab: " + TOSTRING(enable));
		//((LinuxMouse *)mMouse)->grab(enable);
		lastmode = enable?1:0;
	}
#endif
}

void InputEngine::hideMouse(bool visible)
{
	static int mode = -1;
	if(!mMouse)
		return;
#if OGRE_PLATFORM == OGRE_PLATFORM_LINUX
	if((visible && mode == 0) || (!visible && mode == 1) || mode == -1)
	{
		//((LinuxMouse *)mMouse)->hide(visible);
		mode = visible?1:0;
	}
#endif
}

void InputEngine::setMousePosition(int x, int y, bool padding)
{
	if(!mMouse)
		return;
#if OGRE_PLATFORM == OGRE_PLATFORM_LINUX
	// padding ensures that the mouse has a safety area at the window's borders
//	((LinuxMouse *)mMouse)->setMousePosition(x, y, padding);
#endif
}

OIS::MouseState InputEngine::getMouseState()
{
	static float mX=999999, mY=999999;
	static int mode = -1;
	if(mode == -1)
	{
#ifndef NOOGRE
		// try to find the correct location!
		mX = FSETTING("MouseSensX", 1);
		mY = FSETTING("MouseSensY", 1);
		LOG("Mouse X sens: " + TOSTRING((Real)mX));
		LOG("Mouse Y sens: " + TOSTRING((Real)mY));
		mode = 1;
		if(mX == 0 || mY ==0)
			mode = 2;
#else
		// no scaling without ogre
		mode = 2;
#endif
	}

	OIS::MouseState m;
	if(mMouse)
	{
		m = mMouse->getMouseState();
		if(mode == 1)
		{
			m.X.rel = (int)((float)m.X.rel * mX);
			m.Y.rel = (int)((float)m.X.rel * mY);
		}
	}
	return m;
}

Ogre::String InputEngine::getKeyNameForKeyCode(OIS::KeyCode keycode)
{
	if(keycode == KC_LSHIFT || keycode == KC_RSHIFT)
		return "SHIFT";
	if(keycode == KC_LCONTROL || keycode == KC_RCONTROL)
		return "CTRL";
	if(keycode == KC_LMENU || keycode == KC_RMENU)
		return "ALT";
	for(allit = allkeys.begin();allit != allkeys.end();allit++)
	{
		if(allit->second == keycode)
			return allit->first;
	}
	return "unknown";
}

void InputEngine::Capture()
{
	if(mKeyboard) mKeyboard->capture();
	if(mMouse) mMouse->capture();
	if(free_joysticks)
		for(int i=0;i<free_joysticks;i++)
			if(mJoy[i]) mJoy[i]->capture();
}

void InputEngine::windowResized(RenderWindow* rw)
{
	if(!mMouse)
		return;
	//update mouse area
	unsigned int width, height, depth;
	int left, top;
	rw->getMetrics(width, height, depth, left, top);
	const OIS::MouseState &ms = mMouse->getMouseState();
	ms.width = width;
	ms.height = height;
#ifdef USE_MYGUI
	GUIManager::getSingleton().windowResized(rw);
#endif //MYGUI
}

/* --- Joystik Events ------------------------------------------ */
bool InputEngine::buttonPressed( const OIS::JoyStickEvent &arg, int button )
{
	inputsChanged=true;
	//LOG("*** buttonPressed " + TOSTRING(button));
	int i = arg.device->getID();
	if (i < 0 || i >= MAX_JOYSTICKS) i = 0;
	joyState[i] = arg.state;
	return true;
}

bool InputEngine::buttonReleased( const OIS::JoyStickEvent &arg, int button )
{
	inputsChanged=true;
	//LOG("*** buttonReleased " + TOSTRING(button));
	int i = arg.device->getID();
	if (i < 0 || i >= MAX_JOYSTICKS) i = 0;
	joyState[i] = arg.state;
	return true;
}

bool InputEngine::axisMoved( const OIS::JoyStickEvent &arg, int axis )
{
	inputsChanged=true;
	//LOG("*** axisMoved " + TOSTRING(axis) + " / " + TOSTRING((int)(arg.state.mAxes[axis].abs / (float)(mJoy->MAX_AXIS/100))));
	int i = arg.device->getID();
	if (i < 0 || i >= MAX_JOYSTICKS) i = 0;
	joyState[i] = arg.state;
	return true;
}

bool InputEngine::sliderMoved( const OIS::JoyStickEvent &arg, int )
{
	inputsChanged=true;
	//LOG("*** sliderMoved");
	int i = arg.device->getID();
	if (i < 0 || i >= MAX_JOYSTICKS) i = 0;
	joyState[i] = arg.state;
	return true;
}

bool InputEngine::povMoved( const OIS::JoyStickEvent &arg, int )
{
	inputsChanged=true;
	//LOG("*** povMoved");
	int i = arg.device->getID();
	if (i < 0 || i >= MAX_JOYSTICKS) i = 0;
	joyState[i] = arg.state;
	return true;
}

/* --- Key Events ------------------------------------------ */
bool InputEngine::keyPressed( const OIS::KeyEvent &arg )
{
#ifdef USE_MYGUI
	if(GUIManager::getSingleton().keyPressed(arg))
		return true;
#endif //MYGUI

	//LOG("*** keyPressed");
	if(keyState[arg.key] != 1)
		inputsChanged=true;
	keyState[arg.key] = 1;

	return true;
}

bool InputEngine::keyReleased( const OIS::KeyEvent &arg )
{
#ifdef USE_MYGUI
	if(GUIManager::getSingleton().keyReleased(arg))
		return true;
#endif //MYGUI
	//LOG("*** keyReleased");
	if(keyState[arg.key] != 0)
		inputsChanged=true;
	keyState[arg.key] = 0;
	return true;
}

/* --- Mouse Events ------------------------------------------ */
bool InputEngine::mouseMoved( const OIS::MouseEvent &arg )
{
#ifdef USE_MYGUI
	if(GUIManager::getSingleton().mouseMoved(arg))
		return true;
#endif //MYGUI
	//LOG("*** mouseMoved");
	inputsChanged=true;
	mouseState = arg.state;
	return true;
}

bool InputEngine::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
#ifdef USE_MYGUI
	if(GUIManager::getSingleton().mousePressed(arg, id))
		return true;
#endif //MYGUI
	//LOG("*** mousePressed");
	inputsChanged=true;
	mouseState = arg.state;
	return true;
}

bool InputEngine::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
#ifdef USE_MYGUI
	if(GUIManager::getSingleton().mouseReleased(arg, id))
		return true;
#endif //MYGUI
	//LOG("*** mouseReleased");
	inputsChanged=true;
	mouseState = arg.state;
	return true;
}

/* --- Custom Methods ------------------------------------------ */
void InputEngine::prepareShutdown()
{
#ifndef NOOGRE
	LOG("*** Inputsystem prepare for shutdown ***");
#endif
	destroy();
}


void InputEngine::resetKeys()
{
	for ( std::map<int, bool>::iterator iter = keyState.begin(); iter != keyState.end(); ++iter)
	{
		iter->second = false;
	}
}

bool InputEngine::getEventBoolValue(int eventID)
{
	return (getEventValue(eventID) > 0.5f);
}

bool InputEngine::getEventBoolValueBounce(int eventID, float time)
{
	if(event_times[eventID] > 0)
		return false;
	else
	{
		bool res = getEventBoolValue(eventID);
		if(res) event_times[eventID] = time;
		return res;
	}
}

float InputEngine::getEventBounceTime(int eventID)
{
	return event_times[eventID];
}

void InputEngine::updateKeyBounces(float dt)
{
	for(std::map<int, float>::iterator it=event_times.begin(); it!=event_times.end(); it++)
	{
		if(it->second > 0)
			it->second -= dt;
		//else
		// erase element?
	}
}

float InputEngine::deadZone(float axisValue, float dz)
{
	// no deadzone?
	if(dz < 0.0001f)
		return axisValue;

	// check for deadzone
	if (fabs(axisValue) < dz)
		// dead zone case
		return 0.0f;
	else
		// non-deadzone, remap the remaining part
		return (axisValue - dz) * (1.0f / (1.0f-dz));
}

float InputEngine::axisLinearity(float axisValue, float linearity)
{
	return (axisValue * linearity);
}

float InputEngine::logval(float val)
{
	if (val>0) return log10(1.0/(1.1-val))/1.0;
	if (val==0) return 0;
	return -log10(1.0/(1.1+val))/1.0;
}

void InputEngine::smoothValue(float &ref, float value, float rate)
{
	if(value < -1) value = -1;
	if(value > 1) value = 1;
	// smooth!
	if (ref > value)
	{
		ref -= rate;
		if (ref < value)
			ref = value;
	}
	else if (ref < value)
		ref += rate;
}

String InputEngine::getEventCommand(int eventID)
{
	std::vector<event_trigger_t> t_vec = events[eventID];
	if(t_vec.size() > 0)
		return String(t_vec[0].configline);
	return "";
}

event_trigger_t *InputEngine::getEventBySUID(int suid)
{
	std::map<int, std::vector<event_trigger_t> >::iterator a;
	std::vector<event_trigger_t>::iterator b;
	for(a = events.begin();a != events.end(); a++)
	{
		for(b = a->second.begin();b != a->second.end(); b++)
		{
			if(b->suid == suid)
				return &(*b);
		}
	}
	return 0;
}

bool InputEngine::deleteEventBySUID(int suid)
{
	std::map<int, std::vector<event_trigger_t> >::iterator a;
	std::vector<event_trigger_t>::iterator b;
	for(a = events.begin();a != events.end(); a++)
	{
		for(b = a->second.begin();b != a->second.end(); b++)
		{
			if(b->suid == suid)
			{
				a->second.erase(b);
				return true;
			}
		}
	}
	return false;
}

bool InputEngine::isEventDefined(int eventID)
{
	std::vector<event_trigger_t> t_vec = events[eventID];
	if(t_vec.size() > 0)
	{
		if(t_vec[0].eventtype != ET_NONE)
			return true;
	}
	return false;
}

int InputEngine::getKeboardKeyForCommand(int eventID)
{
	std::vector<event_trigger_t> t_vec = events[eventID];
	for(std::vector<event_trigger_t>::iterator i = t_vec.begin(); i != t_vec.end(); i++)
	{
		event_trigger_t t = *i;
		if(t.eventtype == ET_Keyboard)
			return t.keyCode;
	}
	return -1;
}

bool InputEngine::isEventAnalog(int eventID)
{
	std::vector<event_trigger_t> t_vec = events[eventID];
	if(t_vec.size() > 0)
	{
		//loop through all eventtypes, because we want to find a analog device wether it is the first device or not
		//this means a analog device is always preferred over a digital one
		for (unsigned int i=0;i<t_vec.size();i++)
		{
			if((t_vec[i].eventtype == ET_MouseAxisX \
				|| t_vec[i].eventtype == ET_MouseAxisY \
				|| t_vec[i].eventtype == ET_MouseAxisZ \
				|| t_vec[i].eventtype == ET_JoystickAxisAbs \
				|| t_vec[i].eventtype == ET_JoystickAxisRel \
				|| t_vec[i].eventtype == ET_JoystickSliderX \
				|| t_vec[i].eventtype == ET_JoystickSliderY) \
				//check if value comes from analog device
				//this way, only valid events (e.g. joystick mapped, but unplugged) are recognized as analog events
				&& getEventValue(eventID, true, 2)!=0.0)
				{
					return true;
				}
		}
	}
	return false;
#if 0
	// XXX : TODO fix this problem properly
	std::vector<event_trigger_t> t_vec = events[eventID];
	for(std::vector<event_trigger_t>::iterator i = t_vec.begin(); i != t_vec.end(); i++)
	{
		event_trigger_t t = *i;
		if(i->eventtype == ET_MouseAxisX \
			|| i->eventtype == ET_MouseAxisX \
			|| i->eventtype == ET_MouseAxisY \
			|| i->eventtype == ET_MouseAxisZ \
			|| i->eventtype == ET_JoystickAxisAbs \
			|| i->eventtype == ET_JoystickAxisRel \
			|| i->eventtype == ET_JoystickSliderX \
			|| i->eventtype == ET_JoystickSliderY)
			return true;
	}
	return false;
#endif //0
}

float InputEngine::getEventValue(int eventID, bool pure, int valueSource)
{
	float returnValue = 0;
	std::vector<event_trigger_t> t_vec = events[eventID];
	float value=0;
	for(std::vector<event_trigger_t>::iterator i = t_vec.begin(); i != t_vec.end(); i++)
	{
		event_trigger_t t = *i;

		if (valueSource==0 || valueSource==1)
		{
			switch(t.eventtype)
			{
				case ET_NONE:
					break;
				case ET_Keyboard:
					if(!keyState[t.keyCode])
						break;

					// only use explicite mapping, if two keys with different modifiers exist, i.e. F1 and SHIFT+F1.
					// check for modificators
					if (t.explicite)
					{
						if(t.ctrl != (keyState[KC_LCONTROL] || keyState[KC_RCONTROL]))
							break;
						if(t.shift != (keyState[KC_LSHIFT] || keyState[KC_RSHIFT]))
							break;
						if(t.alt != (keyState[KC_LMENU] || keyState[KC_RMENU]))
							break;
					} else {
						if(t.ctrl && !(keyState[KC_LCONTROL] || keyState[KC_RCONTROL]))
							break;
						if(t.shift && !(keyState[KC_LSHIFT] || keyState[KC_RSHIFT]))
							break;
						if(t.alt && !(keyState[KC_LMENU] || keyState[KC_RMENU]))
							break;
					}
					value = 1;
					break;
				case ET_MouseButton:
					//if(t.mouseButtonNumber == 0)
					// TODO: FIXME
					value = mouseState.buttonDown(MB_Left);
					break;
				case ET_JoystickButton:
					{
						if(t.joystickNumber > free_joysticks || !mJoy[t.joystickNumber])
						{
							value=0;
							continue;
						}
						if(t.joystickButtonNumber >= (int)mJoy[t.joystickNumber]->getNumberOfComponents(OIS_Button))
						{
#ifndef NOOGRE
							LOG("*** Joystick has not enough buttons for mapping: need button "+TOSTRING(t.joystickButtonNumber) + ", availabe buttons: "+TOSTRING(mJoy[t.joystickNumber]->getNumberOfComponents(OIS_Button)));
#endif
							value=0;
							continue;
						}
						value = joyState[t.joystickNumber].mButtons[t.joystickButtonNumber];
					}
					break;
				case ET_JoystickPov:
					{
						if(t.joystickNumber > free_joysticks || !mJoy[t.joystickNumber])
						{
							value=0;
							continue;
						}
						if(t.joystickPovNumber >= (int)mJoy[t.joystickNumber]->getNumberOfComponents(OIS_POV))
						{
#ifndef NOOGRE
							LOG("*** Joystick has not enough POVs for mapping: need POV "+TOSTRING(t.joystickPovNumber) + ", availabe POVs: "+TOSTRING(mJoy[t.joystickNumber]->getNumberOfComponents(OIS_POV)));
#endif
							value=0;
							continue;
						}
						if(joyState[t.joystickNumber].mPOV[t.joystickPovNumber].direction & t.joystickPovDirection)
							value = 1;
						else
							value = 0;
					}
					break;
			}

		}
		if (valueSource==0 || valueSource==2)
		{
			switch (t.eventtype)
			{
				case ET_MouseAxisX:
					value = mouseState.X.abs / 32767;
					break;
				case ET_MouseAxisY:
					value = mouseState.Y.abs / 32767;
					break;
				case ET_MouseAxisZ:
					value = mouseState.Z.abs / 32767;
					break;
				
				case ET_JoystickAxisRel:
				case ET_JoystickAxisAbs:
					{
						if(t.joystickNumber > free_joysticks || !mJoy[t.joystickNumber])
						{
							value=0;
							continue;
						}
						if(t.joystickAxisNumber >= (int)joyState[t.joystickNumber].mAxes.size())
						{
	#ifndef NOOGRE
							LOG("*** Joystick has not enough axis for mapping: need axe "+TOSTRING(t.joystickAxisNumber) + ", availabe axis: "+TOSTRING(joyState[t.joystickNumber].mAxes.size()));
	#endif
							value=0;
							continue;
						}
						Axis axe = joyState[t.joystickNumber].mAxes[t.joystickAxisNumber];

						if(t.eventtype == ET_JoystickAxisRel)
						{
							value = (float)axe.rel / (float)mJoy[t.joystickNumber]->MAX_AXIS;
						}
						else
						{
							value = (float)axe.abs / (float)mJoy[t.joystickNumber]->MAX_AXIS;
							switch(t.joystickAxisRegion)
							{
							case 0:
								// normal case, full axis used
								value = (value + 1)/2;
								break;
							case -1:
								// lower range used
								if(value > 0)
									value = 0;
								else
									value = -value;
								break;
							case 1:
								// upper range used
								if(value < 0)
									value = 0;
								break;
							}

							if(t.joystickAxisHalf)
							{
								// XXX: TODO: write this
								//float a = (double)((value+1.0)/2.0);
								//float b = (double)(1.0-(value+1.0)/2.0);
								//LOG("half: "+TOSTRING(value)+" / "+TOSTRING(a)+" / "+TOSTRING(b));
								//no dead zone in half axis
								value = (1.0 + value) / 2.0;
								if (t.joystickAxisReverse)
									value = 1.0 - value;
								if(!pure)
									value = axisLinearity(value, t.joystickAxisLinearity);
							}else
							{
								//LOG("not half: "+TOSTRING(value)+" / "+TOSTRING(deadZone(value, t.joystickAxisDeadzone)) +" / "+TOSTRING(t.joystickAxisDeadzone) );
								if (t.joystickAxisReverse)
									value = 1-value;
								if(!pure)
									// no deadzone when using oure value
									value = deadZone(value, t.joystickAxisDeadzone);
								if(!pure)
									value = axisLinearity(value, t.joystickAxisLinearity);
							}
							// digital mapping of analog axis
							if(t.joystickAxisUseDigital)
								if(value >= 0.5)
									value = 1;
								else
									value = 0;
						}
					}
					break;
				case ET_JoystickSliderX:
				case ET_JoystickSliderY:
					{
						if(t.joystickNumber > free_joysticks || !mJoy[t.joystickNumber])
						{
							value=0;
							continue;
						}
						if(t.eventtype == ET_JoystickSliderX)
							value = (float)joyState[t.joystickNumber].mSliders[t.joystickSliderNumber].abX / (float)mJoy[t.joystickNumber]->MAX_AXIS;
						else if(t.eventtype == ET_JoystickSliderY)
							value = (float)joyState[t.joystickNumber].mSliders[t.joystickSliderNumber].abY / (float)mJoy[t.joystickNumber]->MAX_AXIS;
						value = (value + 1)/2; // full axis
						if(t.joystickSliderReverse)
							value = 1.0 - value; // reversed
					}
					break;
			}
		}
		// only return if grater zero, otherwise check all other bombinations
		if(value > returnValue)
			returnValue = value;
	}
	return returnValue;
}

bool InputEngine::isKeyDown(OIS::KeyCode key)
{
	if(!mKeyboard) return false;
	return this->mKeyboard->isKeyDown(key);
}

bool InputEngine::isKeyDownValueBounce(OIS::KeyCode mod, float time)
{
	if(event_times[-mod] > 0)
		return false;
	else
	{
		bool res = isKeyDown(mod);
		if(res) event_times[-mod] = time;
		return res;
	}
}

Ogre::String InputEngine::getDeviceName(event_trigger_t evt)
{
	switch(evt.eventtype)
	{
	case ET_NONE:
		return "None";
	case ET_Keyboard:
		return "Keyboard";
	case ET_MouseButton:
	case ET_MouseAxisX:
	case ET_MouseAxisY:
	case ET_MouseAxisZ:
		return "Mouse";
	case ET_JoystickButton:
	case ET_JoystickAxisAbs:
	case ET_JoystickAxisRel:
	case ET_JoystickPov:
	case ET_JoystickSliderX:
	case ET_JoystickSliderY:
		return "Joystick: " + getJoyVendor(evt.joystickNumber);
	}
	return "unknown";
}

Ogre::String InputEngine::getEventTypeName(int type)
{
	switch(type)
	{
	case ET_NONE: return "None";
	case ET_Keyboard: return "Keyboard";
	case ET_MouseButton: return "MouseButton";
	case ET_MouseAxisX: return "MouseAxisX";
	case ET_MouseAxisY: return "MouseAxisY";
	case ET_MouseAxisZ: return "MouseAxisZ";
	case ET_JoystickButton: return "JoystickButton";
	case ET_JoystickAxisAbs: return "JoystickAxis";
	case ET_JoystickAxisRel: return "JoystickAxis";
	case ET_JoystickPov: return "JoystickPov";
	case ET_JoystickSliderX: return "JoystickSliderX";
	case ET_JoystickSliderY: return "JoystickSliderY";
	}
	return "unknown";
}

void InputEngine::addEvent(int eventID, event_trigger_t t)
 {
	static int counter=0;
	counter++;
	t.suid = counter;

	if(eventID == -1)
		//unknown event, discard
		return;
	if(events.find(eventID) == events.end())
	{
		events[eventID] = std::vector<event_trigger_t>();
		events[eventID].clear();
	}
	events[eventID].push_back(t);
}

bool InputEngine::processLine(char *line)
{
	static Ogre::String cur_comment = "";

	char eventName[256]="", evtype[256]="";
	const char delimiters[] = "+";
	size_t linelen = strnlen(line, 1024);
	enum eventtypes eventtype = ET_NONE;

	int joyNo = 0;
	float defaultDeadzone = 0.1f;
	float defaultLinearity = 1.0f;
	if (line[0]==';' || linelen < 5)
	{
		cur_comment += line;;
		return false;
	}
	sscanf(line, "%s %s", eventName, evtype);
	if(strnlen(eventName, 255) == 0 || strnlen(evtype, 255) == 0)
		return false;

	if(!strncmp(evtype, "Keyboard", 8))              eventtype = ET_Keyboard;
	else if(!strncmp(evtype, "MouseButton", 10))     eventtype = ET_MouseButton;
	else if(!strncmp(evtype, "MouseAxisX", 9))       eventtype = ET_MouseAxisX;
	else if(!strncmp(evtype, "MouseAxisY", 9))       eventtype = ET_MouseAxisY;
	else if(!strncmp(evtype, "MouseAxisZ", 9))       eventtype = ET_MouseAxisZ;
	else if(!strncmp(evtype, "JoystickButton", 14))  eventtype = ET_JoystickButton;
	else if(!strncmp(evtype, "JoystickAxis", 12))    eventtype = ET_JoystickAxisAbs;
	//else if(!strncmp(evtype, "JoystickAxis", 250)) eventtype = ET_JoystickAxisRel;
	else if(!strncmp(evtype, "JoystickPov", 11))     eventtype = ET_JoystickPov;
	else if(!strncmp(evtype, "JoystickSlider", 14))  eventtype = ET_JoystickSliderX;
	else if(!strncmp(evtype, "JoystickSliderX", 15)) eventtype = ET_JoystickSliderX;
	else if(!strncmp(evtype, "JoystickSliderY", 15)) eventtype = ET_JoystickSliderY;
	else if(!strncmp(evtype, "None", 4))             eventtype = ET_NONE;

	switch(eventtype)
	{
	case ET_Keyboard:
		{
			bool alt   = false;
			bool shift = false;
			bool ctrl  = false;
			bool expl  = false;

			char *keycode            = 0;
			char  keycodes[256]      = {};
			char  keycodes_work[256] = {};

			OIS::KeyCode key = KC_UNASSIGNED;

			sscanf(line, "%s %s %s", eventName, evtype, keycodes);
			// separate all keys and construct the key combination
			//LOG("try to add key: " + String(eventName)+","+ String(evtype)+","+String(keycodes));
			strncpy(keycodes_work, keycodes, 255);
			keycodes_work[255] = '\0';
			char *token = strtok(keycodes_work, delimiters);

			while (token != NULL)
			{
				if (strncmp(token, "SHIFT", 5) == 0)
					shift=true;
				else if (strncmp(token, "CTRL", 4) == 0)
					ctrl=true;
				else if (strncmp(token, "ALT", 3) == 0)
					alt=true;
				else if (strncmp(token, "EXPL", 4) == 0)
					expl=true;
				keycode = token;
				token = strtok(NULL, delimiters);
			}

			allit = allkeys.find(keycode);
			if(allit == allkeys.end())
			{
#ifndef NOOGRE
				LOG("unknown key: " + string(keycodes));
#endif
				key = KC_UNASSIGNED;
			} else
			{
				//LOG("found key: " + string(keycode) + " = " + TOSTRING((int)key));
				key = allit->second;
			}
			int eventID = resolveEventName(String(eventName));
			if(eventID == -1) return false;
			event_trigger_t t_key = newEvent();
			//memset(&t_key, 0, sizeof(event_trigger_t));
			t_key.eventtype = ET_Keyboard;
			t_key.shift     = shift;
			t_key.ctrl      = ctrl;
			t_key.alt       = alt;
			t_key.keyCode   = key;
			t_key.explicite = expl;
			
			strncpy(t_key.configline, keycodes, 128);
			strncpy(t_key.group, getEventGroup(eventName).c_str(), 128);
			strncpy(t_key.tmp_eventname, eventName, 128);

			strncpy(t_key.comments, cur_comment.c_str(), 1024);
			addEvent(eventID, t_key);

			return true;
		}
	case ET_JoystickButton:
		{
			int buttonNo = 0;
			char tmp2[256] = {};
			sscanf(line, "%s %s %d %d %s", eventName, evtype, &joyNo, &buttonNo, tmp2);
			event_trigger_t t_joy = newEvent();
			//memset(&t_joy, 0, sizeof(event_trigger_t));
			int eventID = resolveEventName(String(eventName));
			if(eventID == -1) return false;
			t_joy.eventtype = ET_JoystickButton;
			t_joy.joystickNumber = joyNo;
			t_joy.joystickButtonNumber = buttonNo;
			if(!strcmp(tmp2, "!NEW!"))
			{
				strncpy(t_joy.configline, tmp2, 128);
			} else
			{
				char tmp[256] = {};
				sprintf(tmp, "%d", buttonNo);
				strncpy(t_joy.configline, tmp, 128);
			}
			strncpy(t_joy.group, getEventGroup(eventName).c_str(), 128);
			strncpy(t_joy.tmp_eventname, eventName, 128);
			strncpy(t_joy.comments, cur_comment.c_str(), 1024);
			cur_comment = "";
			addEvent(eventID, t_joy);
			return true;
		}
	case ET_JoystickAxisRel:
	case ET_JoystickAxisAbs:
		{
			int axisNo = 0;
			char options[256] = {};
			sscanf(line, "%s %s %d %d %s", eventName, evtype, &joyNo, &axisNo, options);
			int eventID = resolveEventName(String(eventName));
			if(eventID == -1) return false;

			bool half       = false;
			bool reverse    = false;
			bool linear     = false;
			bool relative   = false;
			bool usedigital = false;
			float deadzone  = defaultDeadzone;
			float linearity = defaultLinearity;
			int jAxisRegion = 0;
			//  0 = all
			// -1 = lower
			//  1 = upper
			char tmp[256] = {};
			strcpy(tmp, options);
			tmp[255] = '\0';
			char *token = strtok(tmp, delimiters);
			while (token != NULL)
			{
				if (strncmp(token, "HALF", 4) == 0)
					half=true;
				else if (strncmp(token, "REVERSE", 7) == 0)
					reverse=true;
				else if (strncmp(token, "LINEAR", 6) == 0)
					linear=true;
				else if (strncmp(token, "UPPER", 5) == 0)
					jAxisRegion = 1;
				else if (strncmp(token, "LOWER", 5) == 0)
					jAxisRegion = -1;
				else if (strncmp(token, "RELATIVE", 8) == 0)
					relative=true;
				else if (strncmp(token, "DIGITAL", 7) == 0)
					usedigital=true;
				else if (strncmp(token, "DEADZONE", 8) == 0 && strnlen(token, 250) > 9)
				{
					char tmp2[256] = {};
					strcpy(tmp2,token+9);
					deadzone = atof(tmp2);
					//LOG("got deadzone: " + TOSTRING(deadzone)+", "+String(tmp2));
				}
				else if (strncmp(token, "LINEARITY", 9) == 0 && strnlen(token, 250) > 10)
				{
					char tmp2[256] = {};
					strcpy(tmp2,token+10);
					linearity = atof(tmp2);
				}
				token = strtok(NULL, delimiters);
			}

			if(relative)
				eventtype = ET_JoystickAxisRel;

			event_trigger_t t_joy = newEvent();
			//memset(&t_joy, 0, sizeof(event_trigger_t));
			t_joy.eventtype = eventtype;
			t_joy.joystickAxisRegion = jAxisRegion;
			t_joy.joystickAxisUseDigital = usedigital;
			t_joy.joystickAxisDeadzone = deadzone;
			t_joy.joystickAxisHalf = half;
			t_joy.joystickAxisLinearity = linearity;
			t_joy.joystickAxisReverse = reverse;
			t_joy.joystickAxisNumber = axisNo;
			t_joy.joystickNumber = joyNo;
			strncpy(t_joy.configline, options, 128);
			strncpy(t_joy.group, getEventGroup(eventName).c_str(), 128);
			strncpy(t_joy.tmp_eventname, eventName, 128);
			strncpy(t_joy.comments, cur_comment.c_str(), 1024);
			cur_comment = "";
			addEvent(eventID, t_joy);
			//LOG("added axis: " + TOSTRING(axisNo));
			return true;
		}
	case ET_NONE:
		{
			int eventID = resolveEventName(String(eventName));
			if(eventID == -1) return false;
			event_trigger_t t_none = newEvent();
			t_none.eventtype = eventtype;
			//t_none.configline = "";
			strncpy(t_none.group, getEventGroup(eventName).c_str(), 128);
			strncpy(t_none.tmp_eventname, eventName, 128);
			strncpy(t_none.comments, cur_comment.c_str(), 1024);
			cur_comment = "";
			addEvent(eventID, t_none);
			return true;
		}
	case ET_MouseButton:
	case ET_MouseAxisX:
	case ET_MouseAxisY:
	case ET_MouseAxisZ:
		// no mouse support D:
		return false;
	case ET_JoystickPov:
		{
			int povNumber = 0;
			char dir[256] = {};
			sscanf(line, "%s %s %d %d %s", eventName, evtype, &joyNo, &povNumber, dir);
			int eventID = resolveEventName(String(eventName));
			if(eventID == -1) return false;

			int direction = OIS::Pov::Centered;
			if(!strcmp(dir, "North"))     direction = OIS::Pov::North;
			if(!strcmp(dir, "South"))     direction = OIS::Pov::South;
			if(!strcmp(dir, "East"))      direction = OIS::Pov::East;
			if(!strcmp(dir, "West"))      direction = OIS::Pov::West;
			if(!strcmp(dir, "NorthEast")) direction = OIS::Pov::NorthEast;
			if(!strcmp(dir, "SouthEast")) direction = OIS::Pov::SouthEast;
			if(!strcmp(dir, "NorthWest")) direction = OIS::Pov::NorthWest;
			if(!strcmp(dir, "SouthWest")) direction = OIS::Pov::SouthWest;

			event_trigger_t t_pov = newEvent();
			t_pov.eventtype = eventtype;
			t_pov.joystickNumber = joyNo;
			t_pov.joystickPovNumber = povNumber;
			t_pov.joystickPovDirection = direction;

			strncpy(t_pov.group, getEventGroup(eventName).c_str(), 128);
			strncpy(t_pov.tmp_eventname, eventName, 128);
			strncpy(t_pov.comments, cur_comment.c_str(), 1024);
			cur_comment = "";
			addEvent(eventID, t_pov);
			//LOG("added axis: " + TOSTRING(axisNo));
			return true;
		}
	case ET_JoystickSliderX:
	case ET_JoystickSliderY:
		{
			int sliderNumber = 0;
			char options[256] = {};
			char type;
			sscanf(line, "%s %s %d %c %d %s", eventName, evtype, &joyNo, &type, &sliderNumber, options);
			int eventID = resolveEventName(String(eventName));
			if(eventID == -1) return false;

			bool reverse=false;
			char tmp[256] = {};
			strncpy(tmp, options, 255);
			tmp[255] = '\0';
			char *token = strtok(tmp, delimiters);
			while (token != NULL)
			{
				if (strncmp(token, "REVERSE", 7) == 0)
					reverse=true;

				token = strtok(NULL, delimiters);
			}

			event_trigger_t t_slider = newEvent();

			if(type == 'Y' || type == 'y')
				eventtype = ET_JoystickSliderY;
			else if(type == 'X' || type == 'x')
				eventtype = ET_JoystickSliderX;

			t_slider.eventtype = eventtype;
			t_slider.joystickNumber = joyNo;
			t_slider.joystickSliderNumber = sliderNumber;
			t_slider.joystickSliderReverse = reverse;
			// TODO: add region support to sliders!
			t_slider.joystickSliderRegion = 0;
			strncpy(t_slider.configline, options, 128);
			strncpy(t_slider.group, getEventGroup(eventName).c_str(), 128);
			strncpy(t_slider.tmp_eventname, eventName, 128);
			strncpy(t_slider.comments, cur_comment.c_str(), 1024);
			cur_comment = "";
			addEvent(eventID, t_slider);
			//LOG("added axis: " + TOSTRING(axisNo));
			return true;
		}
	default:
		return false;
	}
}

int InputEngine::getCurrentJoyButton(int &joystickNumber, int &button)
{
	for(int j=0;j<free_joysticks;j++)
	{
		for(int i=0;i<(int)joyState[j].mButtons.size();i++)
		{
			if(joyState[j].mButtons[i])
			{
				joystickNumber = j;
				button = i;
				return 1;
			}
		}
	}
	return 0;
}

int InputEngine::getCurrentPovValue(int &joystickNumber, int &pov, int &povdir)
{
	for(int j=0; j<free_joysticks; j++)
	{
		for(int i=0; i<MAX_JOYSTICK_POVS; i++)
		{
			if(joyState[j].mPOV[i].direction != Pov::Centered)
			{
				joystickNumber = j;
				pov = i;
				povdir = joyState[j].mPOV[i].direction;
				return 1;
			}
		}
	}
	return 0;
}


event_trigger_t InputEngine::newEvent()
{
	event_trigger_t res;
	memset(&res, 0, sizeof(event_trigger_t));
	return res;
}

int InputEngine::getJoyComponentCount(OIS::ComponentType type, int joystickNumber)
{
	if(joystickNumber > free_joysticks || !mJoy[joystickNumber]) return 0;
	return mJoy[joystickNumber]->getNumberOfComponents(type);
}

std::string InputEngine::getJoyVendor(int joystickNumber)
{
	if(joystickNumber > free_joysticks || !mJoy[joystickNumber]) return "unknown";
	return mJoy[joystickNumber]->vendor();
}

JoyStickState *InputEngine::getCurrentJoyState(int joystickNumber)
{
	if(joystickNumber > free_joysticks) return 0;
	return &joyState[joystickNumber];
}


int InputEngine::getCurrentKeyCombo(Ogre::String *combo)
{
	std::map<int, bool>::iterator i;
	int keyCounter = 0;
	int modCounter = 0;

	// list all modificators first
	for(i = keyState.begin();i!=keyState.end();i++)
	{
		if(i->second)
		{
			if(i->first != KC_LSHIFT && i->first != KC_RSHIFT && i->first != KC_LCONTROL && i->first != KC_RCONTROL && i->first != KC_LMENU && i->first != KC_RMENU)
				continue;
			modCounter++;
			Ogre::String keyName = getKeyNameForKeyCode((OIS::KeyCode)i->first);
			if(*combo == "")
				*combo = keyName;
			else
				*combo = *combo + "+" + keyName;
		}
	}

	// now list all keys
	for(i = keyState.begin();i!=keyState.end();i++)
	{
		if(i->second)
		{
			if(i->first == KC_LSHIFT || i->first == KC_RSHIFT || i->first == KC_LCONTROL || i->first == KC_RCONTROL || i->first == KC_LMENU || i->first == KC_RMENU)
				continue;
			Ogre::String keyName = getKeyNameForKeyCode((OIS::KeyCode)i->first);
			if(*combo == "")
				*combo = keyName;
			else
				*combo = *combo + "+" + keyName;
			keyCounter++;
		}
	}

	//
	if(modCounter > 0 && keyCounter == 0)
	{
		return -modCounter;
	} else if(keyCounter==0 && modCounter == 0)
	{
		*combo = "(Please press a key)";
		return 0;
	}
	return keyCounter;
}

Ogre::String InputEngine::getEventGroup(Ogre::String eventName)
{
	const char delimiters[] = "_";
	char tmp[256] = {};
	strncpy(tmp, eventName.c_str(), 255);
	tmp[255] = '\0';
	char *token = strtok(tmp, delimiters);
	while (token != NULL)
	{
		return Ogre::String(token);
	}
	return "";
}

bool InputEngine::appendLineToConfig(std::string line, std::string outfile)
{
	FILE *f = fopen(const_cast<char *>(outfile.c_str()),"a");
	if(!f)
		return false;
	fprintf(f, "%s\n", line.c_str());
	fclose(f);
	return true;
}

bool InputEngine::reloadConfig(std::string outfile)
{
	events.clear();
	loadMapping(outfile);
	return true;
}

bool InputEngine::updateConfigline(event_trigger_t *t)
{
	if(t->eventtype != ET_JoystickAxisAbs && t->eventtype != ET_JoystickSliderX && t->eventtype != ET_JoystickSliderY)
		return false;
	bool isSlider = (t->eventtype == ET_JoystickSliderX || t->eventtype == ET_JoystickSliderY);
	strcpy(t->configline, "");

	if (isSlider)
	{
		if(t->joystickSliderReverse && !strlen(t->configline))
			strcat(t->configline, "REVERSE");
		else if(t->joystickSliderReverse && strlen(t->configline))
			strcat(t->configline, "+REVERSE");

		if(t->joystickSliderRegion==1 && !strlen(t->configline))
			strcat(t->configline, "UPPER");
		else if(t->joystickSliderRegion==1 && strlen(t->configline))
			strcat(t->configline, "+UPPER");
		else if(t->joystickSliderRegion==-1 && !strlen(t->configline))
			strcat(t->configline, "LOWER");
		else if(t->joystickSliderRegion==-1 && strlen(t->configline))
			strcat(t->configline, "+LOWER");

		// is this is a slider, ignore the rest
		return true;
	}

	if(t->joystickAxisReverse && !strlen(t->configline))
		strcat(t->configline, "REVERSE");
	else if(t->joystickAxisReverse && strlen(t->configline))
		strcat(t->configline, "+REVERSE");

	if(t->joystickAxisRegion==1 && !strlen(t->configline))
		strcat(t->configline, "UPPER");
	else if(t->joystickAxisRegion==1 && strlen(t->configline))
		strcat(t->configline, "+UPPER");
	else if(t->joystickAxisRegion==-1 && !strlen(t->configline))
		strcat(t->configline, "LOWER");
	else if(t->joystickAxisRegion==-1 && strlen(t->configline))
		strcat(t->configline, "+LOWER");


	if(fabs(t->joystickAxisDeadzone-0.1) > 0.0001f)
	{
		char tmp[256] = {};
		sprintf(tmp, "DEADZONE=%0.2f", t->joystickAxisDeadzone);
		if(strlen(t->configline))
		{
			strcat(t->configline, "+");
			strcat(t->configline, tmp);
		}
		else
			strcat(t->configline, tmp);
	}
	if(fabs(1.0f - t->joystickAxisLinearity) > 0.01f)
	{
		char tmp[256] = {};
		sprintf(tmp, "LINEARITY=%0.2f", t->joystickAxisLinearity);
		if(strlen(t->configline))
		{
			strcat(t->configline, "+");
			strcat(t->configline, tmp);
		}
		else
			strcat(t->configline, tmp);
	}
	return true;
}

bool InputEngine::saveMapping(Ogre::String outfile, Ogre::String hwnd, int joyNum)
{
	// -10 = all
	// -2  = keyboard
	// -3  = mouse
	// >0 joystick
	FILE *f = fopen(const_cast<char *>(outfile.c_str()),"w");
	if(!f)
		return false;

	bool created=false;
	/*
	// we dont need to capture the mouse anymore
	if(mInputManager && !captureMode && hwnd>0)
	{
		destroy();
		setup(hwnd, true, true);
		created=true;
	}
	*/
	if(!mInputManager && !hwnd.empty())
	{
		destroy();
		setup(renderWindow, hwnd, false, false);
		created=true;
	}
	/*
	// heap problems below, removed
	if(mInputManager)
	{
		// some helpful information
		unsigned int v = mInputManager->getVersionNumber();
		fprintf(f, ";==== Input System Information Start\n");
		fprintf(f, ";OIS Version: %d.%d.%d\n",(v>>16), ((v>>8) & 0x000000FF), (v & 0x000000FF));
		fprintf(f, ";OIS Release Name: '%s'\n", mInputManager->getVersionName().c_str());
		fprintf(f, ";OIS Manager: %s\n", mInputManager->inputSystemName().c_str());
		fprintf(f, ";Total Keyboards: %d\n", mInputManager->getNumberOfDevices(OISKeyboard));
		fprintf(f, ";Total Mice: %d\n", mInputManager->getNumberOfDevices(OISMouse));
		fprintf(f, ";Total JoySticks: %d\n", mInputManager->getNumberOfDevices(OISJoyStick));
		OIS::DeviceList list = mInputManager->listFreeDevices();
		for(OIS::DeviceList::iterator i = list.begin(); i != list.end(); ++i )
			fprintf(f, "; OIS Device: %s Vendor: '%s'\n",mOISDeviceType[i->first], i->second.c_str());
		for(int i = 0; i < mInputManager->getNumberOfDevices(OISJoyStick); ++i)
		{
			if(joyNum != -1 && i != joyNum) continue;
			if(!mJoy[i]) continue;
			fprintf(f, ";*Joystick %d '%s'\n", i, mJoy[i]->vendor().c_str());
			if(mJoy[i]->getNumberOfComponents(OIS_Axis)>0)    fprintf(f, ";**Axes: %d\n", mJoy[i]->getNumberOfComponents(OIS_Axis));
			if(mJoy[i]->getNumberOfComponents(OIS_Slider)>0)  fprintf(f, ";**Sliders: %d\n", mJoy[i]->getNumberOfComponents(OIS_Slider));
			if(mJoy[i]->getNumberOfComponents(OIS_POV)>0)     fprintf(f, ";**POV/HATs: %d\n", mJoy[i]->getNumberOfComponents(OIS_POV));
			if(mJoy[i]->getNumberOfComponents(OIS_Button)>0)  fprintf(f, ";**Buttons: %d\n", mJoy[i]->getNumberOfComponents(OIS_Button));
			if(mJoy[i]->getNumberOfComponents(OIS_Vector3)>0) fprintf(f, ";**Vector3: %d\n", mJoy[i]->getNumberOfComponents(OIS_Vector3));
		}
		fprintf(f, ";==== Input System Information End\n");

		if(created)
			destroy();
	}
	*/

	int counter = 0;
	char curGroup[128] = "";
	std::map<int, std::vector<event_trigger_t> > controls = getEvents();
	std::map<int, std::vector<event_trigger_t> >::iterator mapIt;
	std::vector<event_trigger_t>::iterator vecIt;
	for(mapIt = controls.begin(); mapIt != controls.end(); mapIt++)
	{
		std::vector<event_trigger_t> vec = mapIt->second;

		for(vecIt = vec.begin(); vecIt != vec.end(); vecIt++, counter++)
		{
			// filters
			if(vecIt->eventtype == ET_Keyboard && joyNum != -10 && joyNum != -2) continue;
			if((vecIt->eventtype == ET_MouseAxisX || vecIt->eventtype == ET_MouseAxisY || vecIt->eventtype == ET_MouseAxisZ) && joyNum != -10 && joyNum != -3) continue;
			if((vecIt->eventtype == ET_JoystickAxisAbs || vecIt->eventtype == ET_JoystickAxisRel || vecIt->eventtype == ET_JoystickButton || vecIt->eventtype == ET_JoystickPov || vecIt->eventtype == ET_JoystickSliderX || vecIt->eventtype == ET_JoystickSliderY) && joyNum>=0 && vecIt->joystickNumber != joyNum) continue;

			if(strcmp(vecIt->group, curGroup))
			{
				strncpy(curGroup, vecIt->group, 128);
				// group title:
				fprintf(f, "\n; %s\n", curGroup);
			}

			// no user comments for now!
			//if(vecIt->comments!="")
			//	fprintf(f, "%s", vecIt->comments.c_str());

			// print event name
			fprintf(f, "%-30s ", eventIDToName(mapIt->first).c_str());
			// print event type
			fprintf(f, "%-20s ", getEventTypeName(vecIt->eventtype).c_str());

			if(vecIt->eventtype == ET_Keyboard)
			{
				fprintf(f, "%s ", vecIt->configline);
			} else if(vecIt->eventtype == ET_JoystickAxisAbs || vecIt->eventtype == ET_JoystickAxisRel)
			{
				fprintf(f, "%d ", vecIt->joystickNumber);
				fprintf(f, "%d ", vecIt->joystickAxisNumber);
				fprintf(f, "%s ", vecIt->configline);
			} else if(vecIt->eventtype == ET_JoystickSliderX || vecIt->eventtype == ET_JoystickSliderY)
			{
				fprintf(f, "%d ", vecIt->joystickNumber);
				char type = 'X';
				if(vecIt->eventtype == ET_JoystickSliderY)
					type = 'Y';
				fprintf(f, "%c ", type);
				fprintf(f, "%d ", vecIt->joystickSliderNumber);
				fprintf(f, "%s ", vecIt->configline);
			} else if(vecIt->eventtype == ET_JoystickButton)
			{
				fprintf(f, "%d ", vecIt->joystickNumber);
				fprintf(f, "%d ", vecIt->joystickButtonNumber);
			} else if(vecIt->eventtype == ET_JoystickPov)
			{
				const char *dirStr = "North";
				if(vecIt->joystickPovDirection == OIS::Pov::North)     dirStr = "North";
				if(vecIt->joystickPovDirection == OIS::Pov::South)     dirStr = "South";
				if(vecIt->joystickPovDirection == OIS::Pov::East)      dirStr = "East";
				if(vecIt->joystickPovDirection == OIS::Pov::West)      dirStr = "West";
				if(vecIt->joystickPovDirection == OIS::Pov::NorthEast) dirStr = "NorthEast";
				if(vecIt->joystickPovDirection == OIS::Pov::SouthEast) dirStr = "SouthEast";
				if(vecIt->joystickPovDirection == OIS::Pov::NorthWest) dirStr = "NorthWest";
				if(vecIt->joystickPovDirection == OIS::Pov::SouthWest) dirStr = "SouthWest";

				fprintf(f, "%d %d %s", vecIt->joystickNumber, vecIt->joystickPovNumber, dirStr);
			}
			// end this line
			fprintf(f, "\n");
		}
	}
	fclose(f);
	return true;
}

void InputEngine::completeMissingEvents()
{
	if(!mappingLoaded) return;
	int i=-1;
	while(i!=EV_MODE_LAST)
	{
		i++;
		if(events.find(eventInfo[i].eventID) == events.end())
		{
			if(eventInfo[i].defaultKey.empty()) continue;
			if(eventInfo[i].defaultKey == "None") continue;
#ifndef NOOGRE
			LOG("event mapping not existing, using default: " + eventInfo[i].name);
#endif
			// not existing, insert default
			char tmp[256] = "";
			sprintf(tmp, "%s %s", eventInfo[i].name.c_str(), eventInfo[i].defaultKey.c_str());
			processLine(tmp);
		}
	}
}


bool InputEngine::loadMapping(Ogre::String outfile, bool append)
{
	char line[1025] = "";

	if(!append)
	{
		// clear everything
		resetKeys();
		events.clear();
	}

#ifndef NOOGRE
	LOG("Loading input mapping...");
	{
		DataStreamPtr ds = ResourceGroupManager::getSingleton().openResource(outfile, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		while (!ds->eof())
		{
			size_t size = 1024;
			if(ds->tell() + size >= ds->size())
				size = ds->size()-ds->tell();
			if(ds->tell() >= ds->size())
				break;
			size_t readnum = ds->readLine(line, size);
			if(readnum > 5)
				processLine(line);
		}
	}
#else
	FILE *f = fopen(outfile.c_str(), "r");
	if(!f)
		return false;
	while(fgets(line, 1024, f)!=NULL)
	{
		if(strnlen(line, 1024) > 5)
			processLine(line);
	}
	fclose(f);

#endif

	mappingLoaded = true;

#ifndef NOOGRE
	LOG("key map successfully loaded!");
#endif
	completeMissingEvents();
	return true;
}

int InputEngine::resolveEventName(Ogre::String eventName)
{
	int i=0;
	while(i!=EV_MODE_LAST)
	{
		if(eventInfo[i].name == eventName)
			return eventInfo[i].eventID;
		i++;
	}
#ifndef NOOGRE
	LOG("unknown event (ignored): " + eventName);
#endif
	return -1;
}

Ogre::String InputEngine::eventIDToName(int eventID)
{
	int i=0;
	while(i!=EV_MODE_LAST)
	{
		if(eventInfo[i].eventID == eventID)
			return eventInfo[i].name;
		i++;
	}
	return "unknown";
}

void InputEngine::initAllKeys()
{
	allkeys["0"] = KC_0;
	allkeys["1"] = KC_1;
	allkeys["2"] = KC_2;
	allkeys["3"] = KC_3;
	allkeys["4"] = KC_4;
	allkeys["5"] = KC_5;
	allkeys["6"] = KC_6;
	allkeys["7"] = KC_7;
	allkeys["8"] = KC_8;
	allkeys["9"] = KC_9;
	allkeys["A"] = KC_A ;
	allkeys["ABNT_C1"] = KC_ABNT_C1;
	allkeys["ABNT_C2"] = KC_ABNT_C2;
	allkeys["ADD"] = KC_ADD;
	allkeys["APOSTROPHE"] = KC_APOSTROPHE;
	allkeys["APPS"] = KC_APPS;
	allkeys["AT"] = KC_AT;
	allkeys["AX"] = KC_AX;
	allkeys["B"] = KC_B;
	allkeys["BACK"] = KC_BACK;
	allkeys["BACKSLASH"] = KC_BACKSLASH;
	allkeys["C"] = KC_C;
	allkeys["CALCULATOR"] = KC_CALCULATOR;
	allkeys["CAPITAL"] = KC_CAPITAL;
	allkeys["COLON"] = KC_COLON;
	allkeys["COMMA"] = KC_COMMA;
	allkeys["CONVERT"] = KC_CONVERT;
	allkeys["D"] = KC_D;
	allkeys["DECIMAL"] = KC_DECIMAL;
	allkeys["DELETE"] = KC_DELETE;
	allkeys["DIVIDE"] = KC_DIVIDE;
	allkeys["DOWN"] = KC_DOWN;
	allkeys["E"] = KC_E;
	allkeys["END"] = KC_END;
	allkeys["EQUALS"] = KC_EQUALS;
	allkeys["ESCAPE"] = KC_ESCAPE;
	allkeys["F"] = KC_F;
	allkeys["F1"] = KC_F1;
	allkeys["F10"] = KC_F10;
	allkeys["F11"] = KC_F11;
	allkeys["F12"] = KC_F12;
	allkeys["F13"] = KC_F13;
	allkeys["F14"] = KC_F14;
	allkeys["F15"] = KC_F15;
	allkeys["F2"] = KC_F2;
	allkeys["F3"] = KC_F3;
	allkeys["F4"] = KC_F4;
	allkeys["F5"] = KC_F5;
	allkeys["F6"] = KC_F6;
	allkeys["F7"] = KC_F7;
	allkeys["F8"] = KC_F8;
	allkeys["F9"] = KC_F9;
	allkeys["G"] = KC_G;
	allkeys["GRAVE"] = KC_GRAVE;
	allkeys["H"] = KC_H;
	allkeys["HOME"] = KC_HOME;
	allkeys["I"] = KC_I;
	allkeys["INSERT"] = KC_INSERT;
	allkeys["J"] = KC_J;
	allkeys["K"] = KC_K;
	allkeys["KANA"] = KC_KANA;
	allkeys["KANJI"] = KC_KANJI;
	allkeys["L"] = KC_L;
	allkeys["LBRACKET"] = KC_LBRACKET;
	allkeys["LCONTROL"] = KC_LCONTROL;
	allkeys["LEFT"] = KC_LEFT;
	allkeys["LMENU"] = KC_LMENU;
	allkeys["LSHIFT"] = KC_LSHIFT;
	allkeys["LWIN"] = KC_LWIN;
	allkeys["M"] = KC_M;
	allkeys["MAIL"] = KC_MAIL;
	allkeys["MEDIASELECT"] = KC_MEDIASELECT;
	allkeys["MEDIASTOP"] = KC_MEDIASTOP;
	allkeys["MINUS"] = KC_MINUS;
	allkeys["MULTIPLY"] = KC_MULTIPLY;
	allkeys["MUTE"] = KC_MUTE;
	allkeys["MYCOMPUTER"] = KC_MYCOMPUTER;
	allkeys["N"] = KC_N;
	allkeys["NEXTTRACK"] = KC_NEXTTRACK;
	allkeys["NOCONVERT"] = KC_NOCONVERT;
	allkeys["NUMLOCK"] = KC_NUMLOCK;
	allkeys["NUMPAD0"] = KC_NUMPAD0;
	allkeys["NUMPAD1"] = KC_NUMPAD1;
	allkeys["NUMPAD2"] = KC_NUMPAD2;
	allkeys["NUMPAD3"] = KC_NUMPAD3;
	allkeys["NUMPAD4"] = KC_NUMPAD4;
	allkeys["NUMPAD5"] = KC_NUMPAD5;
	allkeys["NUMPAD6"] = KC_NUMPAD6;
	allkeys["NUMPAD7"] = KC_NUMPAD7;
	allkeys["NUMPAD8"] = KC_NUMPAD8;
	allkeys["NUMPAD9"] = KC_NUMPAD9;
	allkeys["NUMPADCOMMA"] = KC_NUMPADCOMMA;
	allkeys["NUMPADENTER"] = KC_NUMPADENTER;
	allkeys["NUMPADEQUALS"] = KC_NUMPADEQUALS;
	allkeys["O"] = KC_O;
	allkeys["OEM_102"] = KC_OEM_102;
	allkeys["P"] = KC_P;
	allkeys["PAUSE"] = KC_PAUSE;
	allkeys["PERIOD"] = KC_PERIOD;
	allkeys["PGDOWN"] = KC_PGDOWN;
	allkeys["PGUP"] = KC_PGUP;
	allkeys["PLAYPAUSE"] = KC_PLAYPAUSE;
	allkeys["POWER"] = KC_POWER;
	allkeys["PREVTRACK"] = KC_PREVTRACK;
	allkeys["Q"] = KC_Q;
	allkeys["R"] = KC_R;
	allkeys["RBRACKET"] = KC_RBRACKET;
	allkeys["RCONTROL"] = KC_RCONTROL;
	allkeys["RETURN"] = KC_RETURN;
	allkeys["RIGHT"] = KC_RIGHT;
	allkeys["RMENU"] = KC_RMENU;
	allkeys["RSHIFT"] = KC_RSHIFT;
	allkeys["RWIN"] = KC_RWIN;
	allkeys["S"] = KC_S;
	allkeys["SCROLL"] = KC_SCROLL;
	allkeys["SEMICOLON"] = KC_SEMICOLON;
	allkeys["SLASH"] = KC_SLASH;
	allkeys["SLEEP"] = KC_SLEEP;
	allkeys["SPACE"] = KC_SPACE;
	allkeys["STOP"] = KC_STOP;
	allkeys["SUBTRACT"] = KC_SUBTRACT;
	allkeys["SYSRQ"] = KC_SYSRQ;
	allkeys["T"] = KC_T;
	allkeys["TAB"] = KC_TAB;
	allkeys["U"] = KC_U;
	//allkeys["UNASSIGNED"] = KC_UNASSIGNED;
	allkeys["UNDERLINE"] = KC_UNDERLINE;
	allkeys["UNLABELED"] = KC_UNLABELED;
	allkeys["UP"] = KC_UP;
	allkeys["V"] = KC_V;
	allkeys["VOLUMEDOWN"] = KC_VOLUMEDOWN;
	allkeys["VOLUMEUP"] = KC_VOLUMEUP;
	allkeys["W"] = KC_W;
	allkeys["WAKE"] = KC_WAKE;
	allkeys["WEBBACK"] = KC_WEBBACK;
	allkeys["WEBFAVORITES"] = KC_WEBFAVORITES;
	allkeys["WEBFORWARD"] = KC_WEBFORWARD;
	allkeys["WEBHOME"] = KC_WEBHOME;
	allkeys["WEBREFRESH"] = KC_WEBREFRESH;
	allkeys["WEBSEARCH"] = KC_WEBSEARCH;
	allkeys["WEBSTOP"] = KC_WEBSTOP;
	allkeys["X"] = KC_X;
	allkeys["Y"] = KC_Y;
	allkeys["YEN"] = KC_YEN;
	allkeys["Z"] = KC_Z;
}

void InputEngine::setupDefault(Ogre::RenderWindow *win, Ogre::String inputhwnd)
{
	// setup input
	int inputGrabMode=GRAB_ALL;
	String inputGrabSetting = SSETTING("Input Grab", "All");

	if     (inputGrabSetting == "All")
		inputGrabMode = GRAB_ALL;
	else if(inputGrabSetting == "Dynamically")
		inputGrabMode = GRAB_DYNAMICALLY;
	else if(inputGrabSetting == "None")
		inputGrabMode = GRAB_NONE;

	if(!OgreFramework::getSingleton().isEmbedded())
	{

		// start input engine
		size_t hWnd = 0;
		win->getCustomAttribute("WINDOW", &hWnd);

		INPUTENGINE.setup(win, TOSTRING(hWnd), true, true, inputGrabMode);
	} else
	{

	#if OGRE_PLATFORM == OGRE_PLATFORM_LINUX
		size_t windowHnd = 0;
		std::ostringstream windowHndStr;
		win->getCustomAttribute("GLXWINDOW", &windowHnd );
		windowHndStr << windowHnd;
		printf("#### GLXWINDOW = %s\n", windowHndStr.str().c_str());
		INPUTENGINE.setup(win, windowHndStr.str(), true, true, GRAB_NONE);
	#else
		INPUTENGINE.setup(win, inputhwnd, true, true, GRAB_NONE);
	#endif
	}
}

Ogre::String InputEngine::getKeyForCommand( int eventID )
{
	std::map<int, std::vector<event_trigger_t> >::iterator it = events.find(eventID);

	if(it == events.end()) return String();
	if(it->second.empty()) return String();
	
	std::vector<event_trigger_t>::iterator it2 = it->second.begin();
	return getKeyNameForKeyCode((OIS::KeyCode)it2->keyCode);
}
