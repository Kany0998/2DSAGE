#ifndef LOOTDEFINITION_H
#define LOOTDEFINITION_H

#include <string>

struct BagTier {
    std::string id;                 //Tier name
    std::string texture;            //Bag sprite
    std::string dropSound;          //played on every drop (audio still not developed)
    std::string announceImage;      //banner shown top-left when this bag drops (legendary), empty = no banner
    std::string announceSound;      //extra sound for special bags

    int rank = 1;                   //the higher the better after the kill we show highest-ranked bag
    int lifetimeMs = 30000;         //How long bag stays on the ground before destruction
    int announceMs = 4000;          //How long banner exist
    int announceScale = 1;          //How big banner is
};   


struct LootEntry { 
    std::string itemId;         //item from items.lua
    float chance = 0.0f;        //drop chance rolled on every kill
};



#endif // !LOOTDEFINITION_H
