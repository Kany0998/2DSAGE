#ifndef CONTROLS_H
#define CONTROLS_H

#include <SDL.h>
#include <sol/sol.hpp>


struct Controls {
    SDL_Keycode equipment = SDLK_i;
    SDL_Keycode backpack = SDLK_b;
    SDL_Keycode character = SDLK_c;
    SDL_Keycode options = SDLK_o;

    static Controls Load(sol::state& lua);   //reads the Lua Controls table, keeps a default for anything wrong
};

#endif