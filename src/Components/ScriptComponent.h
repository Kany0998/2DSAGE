#ifndef SCRIPTCOMPONENT_H
#define SCRIPTCOMPONENT_H

#include <sol/sol.hpp>

struct ScriptComponent
{
	sol::protected_function func;
	bool failed = false;

	ScriptComponent(sol::protected_function func = sol::lua_nil)
	{
		this->func = func;
	}
};

#endif