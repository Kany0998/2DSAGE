Abilities = 
{
	radial_burst = 
	{
		mana_cost = 200,
		cooldown_seconds = 1.0,
		cast_sound = "",
		shot_count = 20,
		shot_speed = 30,
		range_tiles = 10,
		min_damage = 20,
		max_damage = 40,
		texture = "black-shield-shot",
		on_cast = function(caster, target_x, target_y, ability) 
			for i = 0, ability.shot_count - 1 do
				local angle = 2 * math.pi * i / ability.shot_count
				local velocity_x = math.cos(angle) * ability.shot_speed
				local velocity_y = math.sin(angle) * ability.shot_speed
				spawn_projectile{
					owner = caster,
					x = target_x,
					y = target_y,
					velocity_x = velocity_x,
					velocity_y = velocity_y,
					damage = roll_damage(caster, ability.min_damage, ability.max_damage),
					range_tiles = ability.range_tiles,
					texture = ability.texture,
				}
			end
		end,
	},
	cone_shot = 
	{
		mana_cost = 200,
		cooldown_seconds = 1.0,
		cast_sound = "",
		shot_count = 5,
		shot_speed = 200,
		range_tiles = 3,
		min_damage = 200,
		max_damage = 300,
		spread_degrees = 40,
		texture = "black-shield-shot",
		on_cast = function(caster, target_x, target_y, ability) 
			local caster_x, caster_y = get_center(caster)
			local direction = math.atan(target_y - caster_y, target_x - caster_x)
			local spread = math.rad(ability.spread_degrees)
			local count = ability.shot_count
			for i = 0, ability.shot_count - 1 do
				
				local angle = direction
				if count > 1 then
					angle = direction - spread / 2 + spread * i / (count - 1)
				end
				
				local velocity_x = math.cos(angle) * ability.shot_speed
				local velocity_y = math.sin(angle) * ability.shot_speed

				spawn_projectile{
					owner = caster,
					x = caster_x,
					y = caster_y,
					velocity_x = velocity_x,
					velocity_y = velocity_y,
					damage = roll_damage(caster, ability.min_damage, ability.max_damage),
					range_tiles = ability.range_tiles,
					texture = ability.texture,
				}
			end
		end,
	},
}
