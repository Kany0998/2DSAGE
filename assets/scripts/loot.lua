LootBags = {
    common    = { texture = "common-loot-bag",     rank = 1, lifetime_seconds = 30, drop_sound = "bag-drop" },
    uncommon    = { texture = "uncommon-loot-bag",     rank = 2, lifetime_seconds = 40, drop_sound = "bag-drop" },
    epic      = { texture = "epic-loot-bag",    rank = 3, lifetime_seconds = 45, drop_sound = "bag-drop" },
    legendary = { texture = "legendary-loot-bag", rank = 4, lifetime_seconds = 60, drop_sound = "bag-drop",
                  announce_image = "legendary-loot-banner", announce_seconds = 4, announce_scale = 8,
                  announce_sound = "legendary-aura" },
}

LootTables = {
    goblin_boss = {
        { item = "golden_ring", chance = 0.15 },
        { item = "black_chest", chance = 0.15 },
        { item = "black_legs", chance = 0.15 },
        { item = "black_boots", chance = 0.15 },
        { item = "black_helm", chance = 0.15 },
        { item = "golden_amulet", chance = 0.15 },
        { item = "shield_of_war", chance = 0.15 },
        { item = "horse_trinket", chance = 0.15 }
    },

    desert_goblin = {
        { item = "desert_sword", chance = 0.10 },
        { item = "desert_chest", chance = 0.10 },
        { item = "desert_legs", chance = 0.10 },
        { item = "desert_boots", chance = 0.10 },
        { item = "desert_helm", chance = 0.10 },
        { item = "desert_shield", chance = 0.10 },
        { item = "golden_amulet", chance = 0.02 },
        { item = "horse_trinket", chance = 0.01 }
    },

    taiga_goblin = {
        { item = "taiga_sword", chance = 0.10 },
        { item = "taiga_chest", chance = 0.10 },
        { item = "taiga_legs", chance = 0.10 },
        { item = "taiga_boots", chance = 0.10 },
        { item = "taiga_helm", chance = 0.10 },
        { item = "taiga_shield", chance = 0.10 },
        { item = "golden_amulet", chance = 0.03 },
        { item = "horse_trinket", chance = 0.02 }
    },

    sawanna_goblin = {
        { item = "sawanna_sword", chance = 0.15 },
        { item = "sawanna_chest", chance = 0.15 },
        { item = "sawanna_legs", chance = 0.15 },
        { item = "sawanna_boots", chance = 0.15 },
        { item = "sawanna_helm", chance = 0.15 },
        { item = "sawanna_shield", chance = 0.15 },
        { item = "golden_amulet", chance = 0.05 },
        { item = "horse_trinket", chance = 0.03 }
    },
}

LootSettings = { pickup_range_tiles = 2, default_bag = "common", blink_seconds = 5, bag_scale = 6, player_drop_bag = "common" }