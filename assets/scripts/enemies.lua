-- Island enemies. Each maker builds one entity table; the engine calls it with the
-- spot it picked (the enemy's centre) and the island's difficulty.

-- What each difficulty does to an enemy's base stats
local DIFFICULTY = {
	[1] = { hp = 1.0, damage = 1.0, xp = 1.0 , pellet = 1, deg_spread = 0.0},   -- first island: easy
	[2] = { hp = 1.6, damage = 2.0, xp = 2.0 , pellet = 2, deg_spread = 10.0},   -- second: medium
	[3] = { hp = 2.4, damage = 4.0, xp = 3.0 , pellet = 3, deg_spread = 15.0},   -- third: hard
}

-- Everything that differs between goblin types
local GOBLINS = {
	desert = {
		texture = "desert-goblin-texture", attack = "desert-goblin-attack-texture",
		bullet = "desert-goblin-bullet", loot = "forest_tank",     -- "desert_goblin" after step 1
		hp = 1000, damage = 100, xp = 50,
	},
	sawanna = {
		texture = "sawanna-goblin-texture", attack = "sawanna-goblin-attack-texture",
		bullet = "sawanna-goblin-bullet", loot = "forest_tank",
		hp = 1000, damage = 100, xp = 50,
	},
	taiga = {
		texture = "taiga-goblin-texture", attack = "taiga-goblin-attack-texture",
		bullet = "taiga-goblin-bullet", loot = "forest_tank",
		hp = 1000, damage = 100, xp = 50,
	},
}

local SIZE = 16     -- sprite frame px
local SCALE = 4

local function make_goblin(kind, x, y, difficulty)
	local goblin = GOBLINS[kind]
	local diff = DIFFICULTY[difficulty] or DIFFICULTY[1]
	local hp = math.floor(goblin.hp * diff.hp)

	return {
		group = "enemies",
		components = {
			transform = 
			{
				-- x, y is the centre; the transform wants the top-left corner
				position = 
				{ 
					x = x - SIZE * SCALE / 2, 
					y = y - SIZE * SCALE / 2 
				},
				scale = 
				{ 
					x = SCALE, 
					y = SCALE 
				},
				rotation = 0.0,
			},
			rigidbody = 
			{ 
				velocity = 
				{ 
					x = 0, 
					y = 0 }, 
					mass = 4.0 
			},
			sprite = {
				texture_asset_id = goblin.texture,
				width = SIZE, 
				height = SIZE, 
				layer = 3, 
				fixed = false,
				src_rect_x = 0, 
				src_rect_y = 0,
			},
			animation = 
			{
				num_frames = 4, 
				speed_rate = 4, 
				is_loop = true, 
				only_when_moving = true,
				attack = 
				{ 
					attack_texture_asset_id = goblin.attack, 
					attack_frames = 1, 
					attack_speed_rate = 2 
				},
			},
			boxcollider = 
			{ 
				width = SIZE, 
				height = SIZE, 
				offset = 
				{ 
					x = 0, 
					y = 0 
				} 
			},
			health = 
			{ 
				health_points = hp, 
				max_health_points = hp 
			},
			experience_reward = 
			{ 
				experience = math.floor(goblin.xp * diff.xp) 
			},
			sounds = 
			{
				hurt = "enemy-hurt", 
				death = "goblin-idle-3", 
				idle = "goblin-idle-1",
				idle_min_seconds = 5, 
				idle_max_seconds = 12,
			},
			projectile_emitter = {
				projectile_velocity = { x = 300, y = 100 },
				projectile_duration = 10,
				repeat_frequency = 1,
				projectile_damage = math.floor(goblin.damage * diff.damage),
				aim_at_player = true,
				attack_range = 4,
				friendly = false,
				projectile_texture = goblin.bullet,
				pellet_count = diff.pellet,
				spread_degrees = diff.deg_spread
			},
			movement_type = 
			{ 
				type = "ground" 
			},
			ai = 
			{
				detection_range = 8, 
				stop_distance = 1.0, 
				leash_range = 15,
				move_speed = 100, 
				patrol_radius = 3, 
				patrol_pause = 2.0,
			},
			loot = 
			{ 
				table = goblin.loot 
			},
		}
	}
end

return {
	desert_goblin  = function(x, y, difficulty) return make_goblin("desert",  x, y, difficulty) end,
	sawanna_goblin = function(x, y, difficulty) return make_goblin("sawanna", x, y, difficulty) end,
	taiga_goblin   = function(x, y, difficulty) return make_goblin("taiga",   x, y, difficulty) end,
}