--Load a differnt tilemap image depnds on the time of the day
local current_system_hour = os.date("*t").hour

local map_texute_asset_id

if current_system_hour >= 9 and current_system_hour < 23 then
	map_texute_asset_id = "tilemap-texture-day"
else
	map_texute_asset_id = "tilemap-texture-night"
end

--tables for level 1
Level = {
	--Table of assets

	assets = dofile("./assets/scripts/assets.lua"),

	--map sounds
	music = "",
	ambience = 
	{
		loop = "fire-loop",
		random =
		{
			{sound = "wind-1", min_seconds = 20, max_seconds = 70},
			{sound = "wind-2", min_seconds = 20, max_seconds = 70},
			{sound = "wind-3", min_seconds = 20, max_seconds = 70},
			{sound = "birds-1", min_seconds = 20, max_seconds = 70},
			{sound = "birds-2", min_seconds = 20, max_seconds = 70},
			{sound = "birds-3", min_seconds = 20, max_seconds = 70},
		},
	},

	--map config values

	tilemap =
	{
		map_file = "./assets/tilemaps/jungle.map",
		texture_asset_id = map_texute_asset_id,
		mapNumRows = 20,
		mapNumCols = 25,
		tileSize = 32,
		tileScale = 4.0,

		tile_properties =
		{
			[0] = {tile_id = 21, blocks_ground = false, blocks_flying = false, blocks_projectiles = false, speed_multiplier = 1.0, friction = 0.70, acceleration = 400, step_sound = "footstep-ice"},
			[1] = {tile_id = 14, speed_multiplier = 0.5},   -- shallow water
			[2] = {tile_id = 15, speed_multiplier = 0.7}    -- swamp

		},
	},

	-- table to define entites and components of enetities

	entities =
	{
		[0] =
		{
			--player
			tag = "player",
			components =
			{
				transform =
				{
					position = {x = 1000, y = 1000},
					scale = {x = 3.0, y = 3.0},
					rotation = 0.0 --deg
				},
				rigidbody =
				{
					velocity = {x = 0, y = 0},
					mass = 6.0
				},
				sprite = 
				{
					texture_asset_id = "chopper-texture",
					width = 32,
					height = 32,
					layer = 7,
					fixed = false,
					src_rect_x = 0,
					src_rect_y = 0
				},
				animation = 
				{
					num_frames = 2,
					speed_rate = 10
				},
				boxcollider =
				{
					width = 32,
					height = 32,
					offset = {x = 0, y = 5}
				},
				health =
				{
					health_points = 10,
					max_health_points = 1000
				},
				mana =
				{
					mana_percentage = 1000,
					max_mana_points = 1000
				},
				attributes =
				{
					attack = 50,
					defense = 10000,
					wisdom = 100,
					vitality = 1000,
					speed = 100,
					dexterity = 100
				},
				sounds =
				{
					hurt = "player-hurt",
					level_up = "level-up",
					cast = "wind-3",
					footsteps = true
				},
				projectile_emitter =
				{
					projectile_velocity = {x = 100, y = 100},
					projectile_duration = 10, --sec
					repeat_frequency = 0,
					projectile_damage = 15,
					friendly = true,
					shot_sound = "sword-swing"
				},
				keyboard_controlled =
				{
					up_velocity = {x = 0, y = -100},
					right_velocity = {x = 100, y = 0},
					down_velocity = {x = 0, y = 100},
					left_velocity = {x = -100, y = 0},
					diagnalMovement = true
				},
				progression =
				{
					level = 1,
					experience = 0,
					skill_points = 0,
					max_level = 100,
					experience_base = 100,	--experience needed to clear level 1
					experience_growth = 1.15	--each level costs 15% more than the one before
				},
				movement_type =
				{
					type = "ground"
				},
				camera_follow =
				{
					follow = true
				},
				backpack = 
				{
					capacity = 30,			--0 means unlimited
					items = 
					{
						"golden_ring", "black_helm", "black_chest", "black_boots", "black_legs", "golden_amulet", "horse_trinket", "shield_of_war"
					}
				},
				equipment =
				{
					slots =
					{
						{ name = "Helm",    accepts = "helm",            column = 0, row = 0 },
						{ name = "Amulet",  accepts = "amulet",          column = 0, row = 1 },
						{ name = "Chest",   accepts = "chest",           column = 0, row = 2 },
						{ name = "Legs",    accepts = "legs",            column = 0, row = 3 },
						{ name = "Feet",    accepts = "feet",            column = 0, row = 4 },

						{ name = "Ring",    accepts = "ring",            column = 5, row = 0 },
						{ name = "Trinket", accepts = "trinket",         column = 5, row = 1 },
						{ name = "Ability", accepts = "special_ability", column = 3, row = 5 },

						{ name = "Weapon",  accepts = "weapon",          column = 2, row = 5 },
					},
					equipped = 
					{ 
						Weapon = "iron_sword"
					}
				},
			}
		},
		{
			--tank
			group = "enemies",
			components =
			{
				transform =
				{
					position = {x = 1216, y = 1216},
					scale = {x = 2.0, y = 2.0},
					rotation = 0.0 --deg
				},
				rigidbody =
				{
					velocity = {x = 0, y = 0},
					mass = 4.0
				},
				sprite = 
				{
					texture_asset_id = "tank-texture",
					width = 32,
					height = 32,
					layer  = 3,
					fixed = false,
					src_rect_x = 0,
					src_rect_y = 0
				},
				boxcollider =
				{
					width = 32,
					height = 32,
					offset = {x = 0, y = 0}
				},
				health =
				{
					health_points = 1000,
					max_health_points = 1000
				},
				experience_reward =
				{
					experience = 50
				},
				sounds = 
				{ 
					hurt = "enemy-hurt",
					death = "enemy-hurt"
				},
				projectile_emitter =
				{
					projectile_velocity = {x = 300, y = 100},
					projectile_duration = 10, --sec
					repeat_frequency = 1,
					projectile_damage = 100,
					aim_at_player = true,
					attack_range = 4,
					friendly = false
				},
				movement_type =
				{
					type = "ground"
				},
				ai =
				{
					detection_range = 12,
					stop_distance = 1.0,
					leash_range = 10,
					move_speed = 100,
					patrol_radius = 3,   -- tiles, measured from the spawn point
					patrol_pause = 2.0  -- seconds to wait after reaching a point

				},
				loot = { table = "forest_tank" }
			}

		},
		{
			--tank
			group = "enemies",
			components =
			{
				transform =
				{
					position = {x = 800, y = 400},
					scale = {x = 6.0, y = 6.0},
					rotation = 0.0 --deg
				},
				rigidbody =
				{
					velocity = {x = -100, y = 0},
					mass = 2.5
				},
				sprite = 
				{
					texture_asset_id = "tank-texture",
					width = 32,
					height = 32,
					layer  = 3,
					fixed = false,
					src_rect_x = 0,
					src_rect_y = 0
				},
				boxcollider =
				{
					width = 32,
					height = 32,
					offset = {x = 0, y = 0}
				},
				health =
				{
					health_points = 100,
					max_health_points = 100
				},
				experience_reward =
				{
					experience = 50
				},
				sounds = 
				{ 
					hurt = "enemy-hurt",
					death = "goblin-idle-3" 
				},
				projectile_emitter =
				{
					projectile_velocity = {x = 100, y = 0},
					projectile_duration = 10, --sec
					repeat_frequency = 3,
					projectile_damage = 49,
					aim_at_player = true,
					friendly = false,
					projectile_texture = "black-shield-shot"
				},
				movement_type =
				{
					type = "ground"
				},
				ai =
				{
					detection_range = 6,
					stop_distance = 1,
					leash_range = 10,
					move_speed = 100
				},
				loot = { table = "forest_tank" }
			}

		},
		{
			--tree
			group = "obstacles",
			components =
			{
				transform =
				{
					position = {x = 1650, y = 400},
					scale = {x = 2.0, y = 2.0},
					rotation = 0.0 --deg
				},
				rigidbody =
				{
					velocity = {x = 0, y = 0},
					mass = 1.0
				},
				sprite = 
				{
					texture_asset_id = "tree-texture",
					width = 16,
					height = 32,
					layer  = 2,
					fixed = false,
					src_rect_x = 0,
					src_rect_y = 0
				},
				boxcollider =
				{
					width = 16,
					height = 32,
					offset = {x = 0, y = 0}
				},
				movement_type =
				{
					type = "ground"
				},
				pushable =
				{

				}

			}

		},
		{
			--tree
			group = "obstacles",
			components =
			{
				transform =
				{
					position = {x = 1700, y = 400},
					scale = {x = 2.0, y = 2.0},
					rotation = 0.0 --deg
				},
				rigidbody =
				{
					velocity = {x = 0, y = 0},
					mass = 1.0
				},
				sprite = 
				{
					texture_asset_id = "tree-texture",
					width = 16,
					height = 32,
					layer  = 2,
					fixed = false,
					src_rect_x = 0,
					src_rect_y = 0
				},
				boxcollider =
				{
					width = 16,
					height = 32,
					offset = {x = 0, y = 0}
				},
				movement_type =
				{
					type = "ground"
				},
				pushable =
				{
					
				}

			}

		},
		{
			--tree
			group = "obstacles",
			components =
			{
				transform =
				{
					position = {x = 1750, y = 400},
					scale = {x = 2.0, y = 2.0},
					rotation = 0.0 --deg
				},
				rigidbody =
				{
					velocity = {x = 0, y = 0},
					mass = 1.0
				},
				sprite = 
				{
					texture_asset_id = "tree-texture",
					width = 16,
					height = 32,
					layer  = 2,
					fixed = false,
					src_rect_x = 0,
					src_rect_y = 0
				},
				boxcollider =
				{
					width = 16,
					height = 32,
					offset = {x = 0, y = 0}
				},
				movement_type =
				{
					type = "ground"
				},
				pushable =
				{
					
				}

			}

		},
		{
			--tree
			group = "obstacles",
			components =
			{
				transform =
				{
					position = {x = 1800, y = 400},
					scale = {x = 2.0, y = 2.0},
					rotation = 0.0 --deg
				},
				rigidbody =
				{
					velocity = {x = 0, y = 0},
					mass = 1.0
				},
				sprite = 
				{
					texture_asset_id = "tree-texture",
					width = 16,
					height = 32,
					layer  = 2,
					fixed = false,
					src_rect_x = 0,
					src_rect_y = 0
				},
				boxcollider =
				{
					width = 16,
					height = 32,
					offset = {x = 0, y = 0}
				},
				movement_type =
				{
					type = "ground"
				},
				pushable =
				{
					
				}

			}

		},
		{
			--tree
			group = "obstacles",
			components =
			{
				transform =
				{
					position = {x = 1850, y = 400},
					scale = {x = 2.0, y = 2.0},
					rotation = 0.0 --deg
				},
				rigidbody =
				{
					velocity = {x = 0, y = 0},
					mass = 1.0
				},
				sprite = 
				{
					texture_asset_id = "tree-texture",
					width = 16,
					height = 32,
					layer  = 2,
					fixed = false,
					src_rect_x = 0,
					src_rect_y = 0
				},
				boxcollider =
				{
					width = 16,
					height = 32,
					offset = {x = 0, y = 0}
				},
				movement_type =
				{
					type = "ground"
				},
				pushable =
				{
					
				}

			}

		},
		{
			--tree
			group = "obstacles",
			components =
			{
				transform =
				{
					position = {x = 1900, y = 400},
					scale = {x = 2.0, y = 2.0},
					rotation = 0.0 --deg
				},
				rigidbody =
				{
					velocity = {x = 0, y = 0},
					mass = 1.0
				},
				sprite = 
				{
					texture_asset_id = "tree-texture",
					width = 16,
					height = 32,
					layer  = 2,
					fixed = false,
					src_rect_x = 0,
					src_rect_y = 0
				},
				boxcollider =
				{
					width = 16,
					height = 32,
					offset = {x = 0, y = 0}
				},
				movement_type =
				{
					type = "ground"
				},
				pushable =
				{
					
				}

			}

		},
		{
			--tank
			group = "enemies",
			components =
			{
				transform =
				{
					position = {x = 1000, y = 400},
					scale = {x = 6.0, y = 6.0},
					rotation = 0.0 --deg
				},
				rigidbody =
				{
					velocity = {x = 0, y = 100},
					mass = 7.5
				},
				sprite = 
				{
					texture_asset_id = "truck-texture",
					width = 32,
					height = 32,
					layer  = 3,
					fixed = false,
					src_rect_x = 0,
					src_rect_y = 0
				},
				boxcollider =
				{
					width = 32,
					height = 32,
					offset = {x = 0, y = 0}
				},
				health =
				{
					health_points = 100,
					max_health_points = 100
				},
				experience_reward =
				{
					experience = 50
				},
				sounds = 
				{ 
					hurt = "enemy-hurt" 
				},
				projectile_emitter =
				{
					projectile_velocity = {x = 100, y = 0},
					projectile_duration = 10, --sec
					repeat_frequency = 3,
					aim_at_player = true,
					projectile_damage = 49,
					friendly = false
				},
				movement_type =
				{
					type = "ground"
				},
				on_update_script =
				{
					[0] = 
					function(entity, delta_time, ellapsed_time)
						--print("Siema tutaj maniek od Mc")
					-- make move truck move and turn around
						local current_position_x, current_position_y = get_position(entity)
						local current_velocity_x, current_velocity_y = get_velocity(entity)

						--if it reaches top or bottom of the map
						if current_position_y < 10 or current_position_y > map_height - 32 then
							set_velocity(entity, 0, current_velocity_y * -1); -- flip the entity
						else
							set_velocity(entity, 0, current_velocity_y); -- do not flip entity
						end

						-- set transform rotation to match going up or down
						if(current_velocity_y < 0) then	
							set_rotation(entity, 0) -- point up
							set_projectile_velocity(entity, 0 , -200) --shoot up
						else
							set_rotation(entity, 180) -- point up
							set_projectile_velocity(entity, 0 , 200) --shoot up
						end
					end
				},
				loot = { table = "forest_tank" }

			}

		},
		{
			--tree
			group = "enemies",
			components =
			{
				transform =
				{
					position = {x = 400, y = 200},
					scale = {x = 5.0, y = 5.0},
					rotation = 0.0 --deg
				},
				rigidbody =
				{
					velocity = {x = 100, y = 0},
					mass = 1.5
				},
				sprite = 
				{
					texture_asset_id = "tree-texture",
					width = 16,
					height = 32,
					layer  = 3,
					fixed = false,
					src_rect_x = 0,
					src_rect_y = 0
				},
				boxcollider =
				{
					width = 16,
					height = 32,
					offset = {x = 0, y = 0}
				},
				health =
				{
					health_points = 10000,
					max_health_points = 10000
				},
				experience_reward =
				{
					experience = 50000000
				},
				attributes =
				{
					attack = 150,
					defense = 100,
					wisdom = 10,
					vitality = 10,
					speed = 10,
					dexterity = 10
				},
				sounds = 
				{ 
					hurt = "enemy-hurt" 
				},
				projectile_emitter =
				{
					projectile_velocity = {x = 100, y = 0},
					projectile_duration = 10, --sec
					repeat_frequency = 3,
					projectile_damage = 49,
					friendly = false
				},
				on_update_script =
				{
					[0] = 
					function(entity, delta_time, ellapsed_time)
						--print("Siema tutaj drzewo od druskiego")
						
						--change position of the airplane to follow sine movement
						local new_x = ellapsed_time * 0.09
						local new_y = 200 + (math.sin(ellapsed_time * 0.001)* 50)
						set_position(entity, new_x, new_y)-- set new position to c++
					end
				},
				loot = { table = "forest_tank" }

			}

		}
	}
}

Controls = 
{ 
	equipment = "I",
	backpack = "B", 
	character = "C",
	options = "O"
}

InventoryUI =
{
	icon_size = 24,
	icon_scale = 2,
	backpack_columns = 8
}

AudioSettings =
{
	master = 0.8,
	music = 0.5,
	effects = 0.4,
	ambience = 0.6,
	hearing_range_tiles = 12,
	channels = 32,
	repeat_gap_ms = 40,
	ui = 
	{ 
		equip = "ui-equip", 
		unequip = "ui-unequip", 
		open_bag = "bag-open", 
		take_item = "ui-equip" 
	},
}

--map in pixels
map_width = Level.tilemap.mapNumCols * Level.tilemap.tileSize * Level.tilemap.tileScale
map_height = Level.tilemap.mapNumRows * Level.tilemap.tileSize * Level.tilemap.tileScale
