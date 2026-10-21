#ifndef ABILITYDEFINITION_H
#define ABILITYDEFINITION_H

#include <string>
#include <sol/sol.hpp>

struct AbilityDefinition {
	std::string id;
	sol::protected_function onCast;		//called with (caster, target_x, target_y, stats)
	sol::table data;
};


#endif // !ABILITYDEFINITION_H
