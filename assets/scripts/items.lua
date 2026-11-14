Items = {
	iron_sword = 
	{
		name = "Iron Sword",
		tag = "weapon",
		icon = "iron-sword",
		bonuses = {attack = 2},
		weapon = 
			{ 
				min_damage = 30,
				max_damage = 70,
				projectile_texture = "iron-sword-bullet",
                projectile_speed = 500,
				shots_per_second = 2.0,
				range_tiles = 3.5,
				shot_sound = "sword-swing",
				pellet_count = 1,
				spread_degrees = 0
			},
		bag = "common",
	},

	desert_sword = 
	{
		name = "Desert Sword",
		tag = "weapon",
		icon = "desert-sword",
		bonuses = {attack = 5},
		weapon = 
			{ 
				min_damage = 50,
				max_damage = 90,
				projectile_texture = "desert-sword-bullet",
                projectile_speed = 500,
				shots_per_second = 2.0,
				range_tiles = 3.5,
				shot_sound = "sword-swing",
				pellet_count = 1,
				spread_degrees = 0
			},
		bag = "common",
	},

	taiga_sword = 
	{
		name = "Taiga Sword",
		tag = "weapon",
		icon = "taiga-sword",
		bonuses = {defense = 5},
		weapon = 
			{ 
				min_damage = 40,
				max_damage = 80,
				projectile_texture = "taiga-sword-bullet",
                projectile_speed = 500,
				shots_per_second = 2.0,
				range_tiles = 3.5,
				shot_sound = "sword-swing",
				pellet_count = 2,
				spread_degrees = 10.0
			},
		bag = "uncommon",
	},

	sawanna_sword = 
	{
		name = "Sawanna Sword",
		tag = "weapon",
		icon = "sawanna-sword",
		bonuses = {vitality = 6},
		weapon = 
			{ 
				min_damage = 40,
				max_damage = 110,
				projectile_texture = "sawanna-sword-bullet",
                projectile_speed = 500,
				shots_per_second = 2.0,
				range_tiles = 3.5,
				shot_sound = "sword-swing",
				pellet_count = 3,
				spread_degrees = 12.0
			},
		bag = "epic",
	},

	golden_ring = 
	{ 
		name = "Golden Ring",
		tag = "ring",
		icon = "golden-ring",
        bonuses = { defense = 20 } ,
		bag = "legendary"
	},

	shield_of_war = 
	{ 
		name = "Shield Of War",
		tag = "special_ability",
        icon = "black-shield",
		bonuses = { defense = 15, attack = 10 },
		bag = "legendary",
		ability = "cone_shot",
		ability_stats = 
		{
			mana_cost = 100,
			shot_count = 5,
			shot_speed = 400,
			cooldown_seconds = 1,
			min_damage = 150,
			max_damage = 200,
			range_tiles = 2,
			texture = "black-shield-shot",
			cast_sound = "goblin-idle-3"
		}	
	},

	legendary_shield = 
	{
		name = "Legendary Shield",
		tag = "special_ability",
        icon = "black-shield",
		bonuses = { defense = 55, attack = 60 },
		bag = "legendary",
		ability = "radial_burst",
		ability_stats = 
		{
			mana_cost = 100,
			shot_count = 30,
			shot_speed = 200,
			min_damage = 160,
			max_damage = 160,
			texture = "iron-sword-bullet",
			cast_sound = "goblin-idle-1"
		}
	},

	desert_shield = 
	{ 
		name = "Desert Shield",
		tag = "special_ability",
        icon = "desert-shield",
		bonuses = { speed = 10},
		bag = "common",
		ability = "cone_shot",
		ability_stats = 
		{
			mana_cost = 100,
			shot_count = 2,
			shot_speed = 400,
			cooldown_seconds = 1,
			min_damage = 60,
			max_damage = 80,
			range_tiles = 3,
			texture = "desert-shield-shot"
		}	
	},

	taiga_shield = 
	{ 
		name = "Taiga Shield",
		tag = "special_ability",
        icon = "taiga-shield",
		bonuses = { dexterity = 10},
		bag = "uncommon",
		ability = "cone_shot",
		ability_stats = 
		{
			mana_cost = 100,
			shot_count = 3,
			shot_speed = 400,
			cooldown_seconds = 1,
			min_damage = 90,
			max_damage = 120,
			range_tiles = 3,
			texture = "taiga-shield-shot"
		}	
	},

	sawanna_shield = 
	{ 
		name = "Sawanna Shield",
		tag = "special_ability",
        icon = "sawanna-shield",
		bonuses = { wisdom = 5, vitality = 5},
		bag = "epic",
		ability = "cone_shot",
		ability_stats = 
		{
			mana_cost = 100,
			shot_count = 4,
			shot_speed = 400,
			cooldown_seconds = 1,
			min_damage = 110,
			max_damage = 140,
			range_tiles = 3,
			texture = "desert-shield-shot"
		}	
	},


	black_helm = 
	{ 
		name = "Black Helm",
		tag = "helm",
		icon = "black-helm",
        bonuses = { dexterity = 40 },
		bag = "legendary"
	},

	desert_helm = 
	{ 
		name = "Desert Helm",
		tag = "helm",
		icon = "desert-helm",
        bonuses = { dexterity = 5, defense = 3 },
		bag = "common"
	},

	taiga_helm = 
	{ 
		name = "Taiga Helm",
		tag = "helm",
		icon = "taiga-helm",
        bonuses = { dexterity = 8, defense = 6 },
		bag = "uncommon"
	},

	sawanna_helm = 
	{ 
		name = "Sawanna Helm",
		tag = "helm",
		icon = "sawanna-helm",
        bonuses = { dexterity = 10, defense = 8, vitality = 10 },
		bag = "epic"
	},

	black_chest = 
	{ 
		name = "Black Chest",
		tag = "chest",
		icon = "black-chest",
        bonuses = { defense = 60 },
		bag = "legendary"
	},

	desert_chest = 
	{ 
		name = "Desert Chest",
		tag = "chest",
		icon = "desert-chest",
        bonuses = { defense = 5 },
		bag = "common"
	},

	taiga_chest = 
	{ 
		name = "Taiga Chest",
		tag = "chest",
		icon = "taiga-chest",
        bonuses = { defense = 10 },
		bag = "uncommon"
	},

	sawanna_chest = 
	{ 
		name = "Sawanna Chest",
		tag = "chest",
		icon = "sawanna-chest",
        bonuses = { defense = 20 },
		bag = "epic"
	},

	black_legs = 
	{ 
		name = "Black Legs",
		tag = "legs",
		icon = "black-legs",
        bonuses = { vitality = 40, speed = 5 },
		bag = "legendary"
	},

	desert_legs = 
	{ 
		name = "Desert Legs",
		tag = "legs",
		icon = "desert-legs",
        bonuses = { dexterity = 2, speed = 2 },
		bag = "common"
	},

	taiga_legs = 
	{ 
		name = "Taiga Legs",
		tag = "legs",
		icon = "taiga-legs",
        bonuses = { dexterity = 4, speed = 4 },
		bag = "uncommon"
	},

	sawanna_legs = 
	{ 
		name = "Sawanna Legs",
		tag = "legs",
		icon = "sawanna-legs",
        bonuses = { dexterity = 10, speed = 10 },
		bag = "epic"
	},

	black_boots = 
	{ 
		name = "Black Boots",
		tag = "feet",
		icon = "black-boots",
        bonuses = { wisdom = 10, speed = 25 },
		bag = "legendary"
	},

	desert_boots = 
	{ 
		name = "Desert Boots",
		tag = "feet",
		icon = "desert-boots",
        bonuses = { wisdom = 2, speed = 2 },
		bag = "common"
	},

	taiga_boots = 
	{ 
		name = "Taiga Boots",
		tag = "feet",
		icon = "taiga-boots",
        bonuses = { wisdom = 6, speed = 4 },
		bag = "uncommon"
	},

	sawanna_boots = 
	{ 
		name = "Sawanna Boots",
		tag = "feet",
		icon = "sawanna-boots",
        bonuses = { wisdom = 10, speed = 6 },
		bag = "epic"
	},
	
	golden_amulet = 
	{ 
		name = "Golden Amulet",
		tag = "amulet",
		icon = "golden-amulet",
        bonuses = { defense = 3, vitality = 3, attack = 3, wisdom = 3, speed = 3, dexterity = 3 },
		bag = "epic"
	},

	horse_trinket = 
	{ 
		name = "Horse Trinket",
		tag = "trinket",
		icon = "horse-trinket",
        bonuses = { defense = 5, vitality = 5, attack = 5, wisdom = 5, speed = 5, dexterity = 5 },
		bag = "legendary"
	},

}
