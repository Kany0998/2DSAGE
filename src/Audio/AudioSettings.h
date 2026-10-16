#ifndef AUDIOSETTINGS_H
#define AUDIOSETTINGS_H

#include <sol/sol.hpp>
#include <unordered_map>
#include <string>

struct AudioSettings {
    
    float master = 0.5f;                                //0..1, multiplies everything
    float music = 0.5f;
    float effects = 0.5f;
    float ambience = 0.5f;
    float hearingRangeTiles = 10.0f;                    //farther than this: not played
    int channels = 16;                                   //how many effects can play at once
    int repeatGapMs = 40;                                //the same sound again sooner is skipped
    std::unordered_map<std::string, std::string>ui;     //action  sound id, e.g. "equip"  "ui-equip"

    static AudioSettings Load(sol::state& lua);
};

#endif // !AUDIOSETTINGS_H
