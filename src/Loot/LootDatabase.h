#ifndef LOOTDATABASE_H
#define LOOTDATABASE_H

#include "LootDefinition.h"
#include "../Inventory/ItemDatabase.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <sol/sol.hpp>


class LootDatabase {
    public:


        void Load(sol::state& lua, const std::string& path, const ItemDatabase& items);

        const BagTier* FindBag(const std::string& id) const;

        const std::vector<LootEntry>* FindTable(const std::string& id) const;

        const BagTier* BagFor(const ItemDefinition& item) const;   //item's bag, or the default

        const BagTier* PlayerDropBag() const;   //bag for player drops, nullptr = use the item's own bag

        float PickupRangeTiles() const { return pickupRangeTiles; } //how close player needs to be to open a bag

        int BlinkMs() const { return blinkMs; }     //the time left until bag is destroyed, starts blinking

        int BagScale() const { return bagScale; }   //Scale of bag sprite

    private:
        
        std::unordered_map<std::string, BagTier> bags;
        std::unordered_map<std::string, std::vector<LootEntry>> tables;     //Loot table of enemy
        std::string defaultBag;             //lowest tier bag
        float pickupRangeTiles = 2.0f;      //how close player need to be in tiles to pick up bag
        int blinkMs = 5000;                 //start blinking 5000 ms before destruction
        int bagScale = 1;                   //scale of bag sprite
        std::string playerDropBag;          //tier for items the player drops; empty = item's own bag
};



#endif // !LOOTDATABASE_H
