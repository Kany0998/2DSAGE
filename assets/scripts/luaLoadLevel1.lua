--Load a differnt tilemap image depnds on the time of the day
local current_system_hour = os.date("*t").hour

local map_texute_asset_id

if current_system_hour >= 9 and current_system_hour < 23 then
	map_texute_asset_id = "tilemap-texture-day"
else
	map_texute_asset_id = "tilemap-texture-night"
end

map_texute_asset_id = "tilemap-texture-test"

local goblin_boss = dofile("./assets/scripts/goblin_boss.lua")
local enemies = dofile("./assets/scripts/enemies.lua")
local obstacles = dofile("./assets/scripts/obstacles.lua")


-- One enemy type per island difficulty grows from island to island.
-- Each island has 4 batch centres (the centre of a circle, in world pixels).
local ISLANDS = 
{
	{ make = enemies.desert_goblin,  difficulty = 1, centres = { {x = 1600, y = 350}, {x = 1200, y = 1800}, {x = 2300, y = 2200}, {x = 2000, y = 1300} } },
	{ make = enemies.taiga_goblin,   difficulty = 2, centres = { {x = 5700, y = 700}, {x = 6700, y = 1000}, {x = 6800, y = 2000}, {x = 5500, y = 2500}, {x = 4700, y = 2100}, {x = 5500, y = 1800}, {x = 5000, y = 3000}, {x = 4000, y = 3000} } },
	{ make = enemies.sawanna_goblin, difficulty = 3, centres = { {x = 9500, y = 4400}, {x = 8700, y = 5300}, {x = 9700, y = 6000}, {x = 10000, y = 5000} } },	
}
local BATCH = { min = 2, max = 4, radius = 300 }

--tables for level 1
Level = {
	--Table of assets

	assets = dofile("./assets/scripts/assets.lua"),

	--map sounds
	music = "",
	ambience = 
	{
		loop = "",
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
		map_file = "./assets/tilemaps/LevelOne.map",
		texture_asset_id = map_texute_asset_id,
		mapNumRows = 20,
		mapNumCols = 25,
		tileSize = 16,
		tileScale = 6.0,

		tile_properties =
		{
			[0] = {tile_id = 33, blocks_ground = true, blocks_flying = false, blocks_projectiles = false, speed_multiplier = 1.0, friction = 1.00, acceleration = 100, step_sound = "footstep-ice"},
			[1] = {tile_id = 81, speed_multiplier = 1.0, friction = 0.50, acceleration = 250, step_sound = "footstep-ice"},
			[2] = {tile_id = 9, speed_multiplier = 1.2},   -- plains
			[3] = {tile_id = 38, speed_multiplier = 1.5},   -- stone bridge
			[4] = {tile_id = 46, blocks_ground = true, blocks_projectiles = true},   -- boss arena walls
		},
	},


	--pathfinder benchmark tests

	pathfinding_benchmark = 
	{
		runs = 20,
		routes = 
		{
			{ name =  "short", from = {col = 11, row = 6}, to = {col =8 ,row = 10}},
			{ name =  "long", from = {col =9, row = 12}, to = {col =23,row = 12}},
			{ name =  "obstacle", from = {col =11, row = 6}, to = {col =21, row = 6}},
			{ name =  "water", from = {col =1,row = 7}, to = {col =7,row = 7}},
			{ name =  "unreachable", from = {col =11,row = 6}, to = {col =3,row = 1}},
			{ name = "shore", from = {col = 11, row = 6}, to = {col = 21, row = 7} },
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
					position = {x = 594, y = 500},
					scale = {x = 4.0, y = 4.0},
					rotation = 0.0 --deg
				},
				rigidbody =
				{
					velocity = {x = 0, y = 0},
					mass = 6.0
				},
				sprite = 
				{
					texture_asset_id = "goblin-player-texture",
					width = 16,
					height = 16,
					layer = 7,
					fixed = false,
					src_rect_x = 0,
					src_rect_y = 0
				},
				animation = 
				{
					num_frames = 4,
					speed_rate = 8,
					is_loop = true,
					only_when_moving = true,
					attack = 
					{ 
						attack_texture_asset_id = "goblin-player-attack-texture", 
						attack_frames = 1, 
						attack_speed_rate = 4 
					}
				},
				boxcollider =
				{
					width = 16,
					height = 16,
					offset = {x = 0, y = 0}
				},
				health =
				{
					health_points = 100,
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
					defense = 1,
					wisdom = 100,
					vitality = 100,
					speed = 100,
					dexterity = 100
				},
				sounds =
				{
					hurt = "player-hurt",
					death = "goblin-idle-1",
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
					experience_growth = 1.10	--each level costs 15% more than the one before
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
						"golden_ring"
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
		goblin_boss.make(3070, 7366),
		{
			--campfire
			group = "obstacles",
			components =
			{
				transform =
				{
					position = {x = 1100, y = 900},
					scale = {x = 4.0, y = 4.0},
					rotation = 0.0 --deg
				},
				sprite = 
				{
					texture_asset_id = "campfire-texture",
					width = 19,
					height = 27,
					layer  = 2,
					fixed = false,
					src_rect_x = 0,
					src_rect_y = 0
				},
				sounds =
				{
					loop = "fire-loop"
				}

			}

		},
	}
}

Controls = 
{ 
	equipment = "I",
	backpack = "B", 
	character = "C",
	options = "O",
	editor = "E"
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
	default_step_sound = "footstep-grass",
	max_loop_sources = 5,
	ui = 
	{ 
		equip = "ui-equip", 
		unequip = "ui-unequip", 
		open_bag = "bag-open", 
		take_item = "ui-equip",
		move_item = "ui-equip"
	},
}

SimulationSettings =
{
    enable = true,
    activate_radius_tiles = 18,
    deactivate_radius_tiles = 24,
    check_interval_seconds = 0.25,
}

FlankingSettings =
{
    enable = true,
    penalty_tiles_cost = 4.0,
    ignore_radius_tiles = 5,
}

--map in pixels
map_width = Level.tilemap.mapNumCols * Level.tilemap.tileSize * Level.tilemap.tileScale
map_height = Level.tilemap.mapNumRows * Level.tilemap.tileSize * Level.tilemap.tileScale


stress_enemies = 0                      -- 0 = off; e.g. 500 or 2000 for the measurement

local tile_world_size = Level.tilemap.tileSize * Level.tilemap.tileScale

local function MakeStressTank(xpos, ypos)
    return {                             -- a NEW table on every call
        group = "enemies",
        components =
			{
				transform =
				{
					position = {x = xpos, y = ypos},
					scale = {x = 6.0, y = 6.0},
					rotation = 0.0 --deg
				},
				rigidbody =
				{
					velocity = {x = 0, y = 0},
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
					death = "goblin-idle-3",
					idle = "goblin-idle-2",
					idle_min_seconds = 5,
					idle_max_seconds = 12
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
					detection_range = 5,
					stop_distance = 1.0,
					leash_range = 8,
					move_speed = 100,
					patrol_radius = 3,
					patrol_pause = 2.0
				},
				loot = { table = "forest_tank" }
			}
    }
end

for i = 0, stress_enemies - 1 do
    local copy_x = i % 40
    local copy_y = math.floor(i / 40)
    local col = copy_x * 25 + 18          -- local (18, 12): walkable in every copy
    local row = copy_y * 20 + 12
    table.insert(Level.entities, MakeStressTank(col * tile_world_size, row * tile_world_size))
end


flank_test = true   -- flank_40x40.map: 4 chasers in the right yard
if flank_test then
    local chaser_cells = { {33, 14}, {34, 14}, {35, 15}, {35, 16}, {32, 19}, {33, 20}, {33, 18} }
    for _, cell in ipairs(chaser_cells) do
        local tank = MakeStressTank(cell[1] * tile_world_size, cell[2] * tile_world_size)
        tank.components.transform.scale = {x = 2.0, y = 2.0}   -- fits a 1-tile lane
        tank.components.ai.detection_range = 38                 -- they see you at once
        tank.components.ai.leash_range = 52                     -- both lanes inside the leash
		tank.components.ai.move_speed = 50
        table.insert(Level.entities, tank)
    end
end


-- One flat list for the engine: 3 islands x 4 centres = 12 batches
Level.enemy_batches = {}
for _, island in ipairs(ISLANDS) do
	for _, centre in ipairs(island.centres) do
		table.insert(Level.enemy_batches, {
			make = island.make,
			difficulty = island.difficulty,
			x = centre.x,
			y = centre.y,
			min = BATCH.min,
			max = BATCH.max, 
			radius = BATCH.radius,
		})
	end
end


local CRATES = {
	{ x = 824, y = 5472 },
	{ x = 924, y = 5520 },
	{ x = 1024, y = 5568 },
	{ x = 1124, y = 5520 },
	{ x = 1224, y = 5472 },
	{ x = 1324, y = 5520 },
	{ x = 1424, y = 5568 },
	{ x = 1524, y = 5520 },
	{ x = 1624, y = 5472 },
	{ x = 1724, y = 5520 },
}

for _, p in ipairs(CRATES) do
	table.insert(Level.entities, obstacles.make_crate(p.x, p.y))
end