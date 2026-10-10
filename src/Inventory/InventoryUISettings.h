#ifndef INVENTORYUISETTINGS_H
#define INVENTORYUISETTINGS_H

#include <sol/sol.hpp>

struct InventoryUISettings {
    int iconSize = 24;          //canvas every item icon is drawn on, in pixels
    int iconScale = 2;          //whole number, so pixels stay square
    int backpackColumns = 8;    //slots per row in the backpack grid (step 11)

    float SlotPixels() const { return static_cast<float>(iconSize * iconScale); }   //48 by default

    static InventoryUISettings Load(sol::state& lua);
};

#endif // !INVENTORYUISETTINGS_H
