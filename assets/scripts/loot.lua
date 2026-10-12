LootBags = {
    common    = { texture = "common-loot-bag",     rank = 1, lifetime_seconds = 30, drop_sound = "bag-drop" },
    epic      = { texture = "epic-loot-bag",    rank = 2, lifetime_seconds = 45, drop_sound = "bag-drop" },
    legendary = { texture = "legendary-loot-bag", rank = 3, lifetime_seconds = 60, drop_sound = "bag-drop",
                  announce_image = "legendary-loot-banner", announce_seconds = 4, announce_scale = 4,
                  announce_sound = "legendary-aura" },
}

LootTables = {
    forest_tank = {
        { item = "iron_sword",  chance = 0.30 },    -- 30 %
        { item = "golden_ring", chance = 0.10 },
        { item = "black_chest", chance = 0.05 },
        { item = "black_legs", chance = 0.05 },
        { item = "black_boots", chance = 0.05 },
        { item = "black_helm", chance = 0.05 },
        { item = "golden_amulet", chance = 0.15 },
        { item = "shield_of_war", chance = 0.10 },
        { item = "horse_trinket", chance = 0.45 }
    },
}

LootSettings = { pickup_range_tiles = 2, default_bag = "common", blink_seconds = 5, bag_scale = 1 }