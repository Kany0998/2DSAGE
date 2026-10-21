Items = {
	iron_sword = 
	{
		name = "Iron Sword",
		tag = "weapon",
		icon = "iron-sword",
		bonuses = {attack = 10},
		weapon = 
			{ 
				min_damage = 30,
				max_damage = 70,
				projectile_texture = "iron-sword-bullet",
                projectile_speed = 500,
				shots_per_second = 2.0,
				range_tiles = 2 ,
				shot_sound = "sword-swing"
			},
		bag = "common",
	},

	golden_ring = 
	{ 
		name = "Golden Ring",
		tag = "ring",
		icon = "golden-ring",
        bonuses = { defense = 20 } ,
		bag = "epic"
	},

	shield_of_war = 
	{ 
		name = "Shield Of War",
		tag = "special_ability",
        icon = "black-shield",
		bonuses = { defense = 15, attack = 60 },
		bag = "legendary",
		ability = "radial_burst",
		ability_stats = 
		{
			mana_cost = 100,
			shot_count = 12,
			shot_speed = 40,
			cooldown_seconds = 1,
			min_damage = 60,
			max_damage = 100,
			texture = "black-shield-shot",
			cast_sound = "goblin-idle-3"
		}	
	},

	legendary_shield = 
	{
		name = "Legendary Shield",
		tag = "special_ability",
        icon = "iron-sword-bullet",
		bonuses = { defense = 55, attack = -60 },
		bag = "legendary",
		ability = "radial_burst",
		ability_stats = 
		{
			mana_cost = 100,
			shot_count = 12,
			shot_speed = 40,
			min_damage = 160,
			max_damage = 160,
			texture = "black-shield-shot",
			cast_sound = "goblin-idle-1"
		}
	},


	black_helm = 
	{ 
		name = "Black Helm",
		tag = "helm",
		icon = "black-helm",
        bonuses = { dexterity = 400 },
		bag = "legendary"
	},

	black_chest = 
	{ 
		name = "Black Chest",
		tag = "chest",
		icon = "black-chest",
        bonuses = { defense = 60, speed = -10 },
		bag = "legendary"
	},

	black_legs = 
	{ 
		name = "Black Legs",
		tag = "legs",
		icon = "black-legs",
        bonuses = { vitality = 40, speed = 5 },
		bag = "legendary"
	},

	black_boots = 
	{ 
		name = "Black Boots",
		tag = "feet",
		icon = "black-boots",
        bonuses = { wisdom = 10, speed = 25 },
		bag = "legendary"
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
		name = "Trojan Horse Trinket",
		tag = "trinket",
		icon = "horse-trinket",
        bonuses = { defense = 1, vitality = 1, attack = 1, wisdom = 1, speed = 1, dexterity = 1 },
		bag = "common"
	},

}
