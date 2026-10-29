#include "Controls.h"
#include "../Logger/Logger.h"
#include <string>

//Keys the game already uses (movement, ability, debug, quit). A window key on one
//of them would fire both actions, so Lua is not allowed to pick them.
static bool IsReservedKey(SDL_Keycode key){
	return key == SDLK_w || key == SDLK_a || key == SDLK_s || key == SDLK_d
		|| key == SDLK_SPACE || key == SDLK_x || key == SDLK_ESCAPE;
}

//One key from the Lua Controls table, or the fallback if it is missing, unknown or reserved
static SDL_Keycode ReadKey(sol::state& lua, const char* action, SDL_Keycode fallback){
	sol::optional<sol::table> controlsTable = lua["Controls"];
	if (!controlsTable) {
		return fallback;   //no Controls table at all: defaults, nothing to warn about
	}

	const std::string name = (*controlsTable)[action].get_or(std::string(""));
	if (name.empty()) {
		return fallback;   //this action not set in Lua: its default
	}

	const SDL_Keycode key = SDL_GetKeyFromName(name.c_str());
	if (key == SDLK_UNKNOWN) {
		Logger::Warn("controls: unknown key '" + name + "' for " + action +
			", keeping " + SDL_GetKeyName(fallback));
		return fallback;
	}

	if (IsReservedKey(key)) {
		Logger::Warn("controls: key '" + name + "' for " + action +
			" is already used by the game, keeping " + SDL_GetKeyName(fallback));
		return fallback;
	}

	return key;
}

Controls Controls::Load(sol::state& lua){
	Controls controls;
	controls.equipment = ReadKey(lua, "equipment", controls.equipment);
	controls.backpack = ReadKey(lua, "backpack", controls.backpack);
	controls.character = ReadKey(lua, "character", controls.character);
	controls.options = ReadKey(lua, "options", controls.options);
	controls.editor = ReadKey(lua, "editor", controls.editor);
	return controls;
}