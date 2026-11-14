-- Goblin boss creaton and behaviour loaded in luaLoadLevel so every level load resets it state

local BOSS = {
	calm_speed = 100,
	calm_stop = 3,		--tiles
	angry_speed = 200,
	angry_stop = 1,
	shot_velocity = { x = 200, y = 200 },
	angry_bullet_speed_factor = 1.5,
	-- health % trigered 0.00 - 1.00
	spin_below = 0.75,
	angry_bellow = 0.50,
	--roll chance attack duration in seconds
	spin_roll_every = 3.0,
	spin_chance = 0.5,
	spin_duration = 5.0,
	burst_every = 0.5,
	burst_bullets = 20,
	spin_bullet_speed = 250,
}

local boss_state = {}   -- one entry per boss, by entity id; local = reset on every load


-- One radial burst: burst_bullets bullets evenly around the boss.
-- The ring starts at the angle to the player, so one bullet always flies straight at them.
local function fire_ring (entity, angry)
	local boss_x, boss_y = get_center(entity)
	local player_x, player_y, found = get_player_center()

	local base = 0		--first shot angle
	if found then
		base = math.atan(player_y - boss_y, player_x - boss_x)
	end

	local speed = BOSS.spin_bullet_speed
	if angry then
		speed = speed * BOSS.angry_bullet_speed_factor
	end

	for i = 0, BOSS.burst_bullets - 1 do
		local angle = base + i * (2 * math.pi / BOSS.burst_bullets)
		spawn_projectile{
			owner = entity,
			x = boss_x, y = boss_y,
			velocity_x = math.cos(angle) * speed,
			velocity_y = math.sin(angle) * speed,
			damage = roll_damage(entity, 100, 160),
			lifetime_seconds = 3,
			texture = "taiga-goblin-bullet",
		}
	end
end


local function update(entity, delta_time, elapsed_ms)
	local id = entity:get_id()
	local state = boss_state[id]

	if state == nil then
		state = {angry = false, spinning = false, spin_left = 0, burst_left = 0, roll_left = BOSS.spin_roll_every}
		boss_state[id] = state
	end

	local hp, max_hp = get_health(entity)
	local ratio = hp / max_hp

	-- angry phase triggered once in the first frame where boss hp <= 50%
	if not state.angry and ratio <= BOSS.angry_bellow then
		state.angry = true
		set_ai_movement(entity, BOSS.angry_speed, BOSS.angry_stop)
		set_projectile_velocity(entity, BOSS.shot_velocity.x * BOSS.angry_bullet_speed_factor, BOSS.shot_velocity.y * BOSS.angry_bullet_speed_factor)
	end
	
	--spin attack
	if state.spinning then
		set_velocity(entity, 0, 0)

		state.burst_left = state.burst_left - delta_time
		if state.burst_left <= 0 then
			state.burst_left = BOSS.burst_every
			fire_ring(entity, state.angry)
		end

		state.spin_left = state.spin_left - delta_time
		if state.spin_left <= 0 then                     -- end of spin
			state.spinning = false
			set_sprite_animation(entity, "goblin-boss-walk", 4, 4, true)
			set_emitter_paused(entity, false)
			set_ai_paused(entity, false)
			state.roll_left = BOSS.spin_roll_every
		end
		
	elseif ratio <= BOSS.spin_below then
		state.roll_left = state.roll_left - delta_time
		if state.roll_left <= 0 then
			state.roll_left = BOSS.spin_roll_every
			if math.random() < BOSS.spin_chance then     -- start of spin
				state.spinning = true
				state.spin_left = BOSS.spin_duration
				state.burst_left = 0                     -- first ring right away
				set_ai_paused(entity, true)
				set_emitter_paused(entity, true)
				set_sprite_animation(entity, "goblin-boss-spin-attack", 4, 8, false)
			end
		end
	end
end

local function make(x, y)

	return {
		--goblin boss test
		group = "enemies",
			components =
			{
				transform =
				{
					position = {x = x, y = y},
					scale = {x = 6.0, y = 6.0},
					rotation = 0.0 --deg
				},
				rigidbody =
				{
					velocity = {x = 0, y = 0},
					mass = 20.0
				},
				sprite = 
				{
					texture_asset_id = "goblin-boss-walk",
					width = 32,
					height = 32,
					layer  = 3,
					fixed = false,
					src_rect_x = 0,
					src_rect_y = 0
				},
				animation = 
				{
					num_frames = 4,
					speed_rate = 4,
					is_loop = true,
					only_when_moving = true,
					attack = 
					{ 
						attack_texture_asset_id = "goblin-boss-basic-attack", 
						attack_frames = 2, 
						attack_speed_rate = 2 
					}
				},
				boxcollider =
				{
					width = 8,
					height = 8,
					offset = {x = 12, y = 12}
				},
				health =
				{
					health_points = 50000,
					max_health_points = 50000
				},
				experience_reward =
				{
					experience = 50000
				},
				sounds = 
				{ 
					hurt = "enemy-hurt",
					death = "goblin-idle-3",
					idle = "goblin-idle-1",
					idle_min_seconds = 5,
					idle_max_seconds = 12
				},
				projectile_emitter =
				{
					projectile_velocity = {x = BOSS.shot_velocity.x, y = BOSS.shot_velocity.y},
					projectile_duration = 10, --sec
					repeat_frequency = 1.0,
					projectile_damage = 250,
					aim_at_player = true,
					attack_range = 12,
					friendly = false,
					projectile_texture = "taiga-goblin-bullet",
					pellet_count = 5,
					spread_degrees = 90
				},
				movement_type =
				{
					type = "ground"
				},
				ai =
				{
					detection_range = 8,
					stop_distance = BOSS.calm_stop,
					leash_range = 20,
					move_speed = BOSS.calm_speed,
					patrol_radius = 3,   -- tiles, measured from the spawn point
					patrol_pause = 2.0  -- seconds to wait after reaching a point

				},
				on_update_script = 
				{ 
					[0] = update 
				},
				loot = { table = "goblin_boss" }
			}
		}
end

return {make = make}