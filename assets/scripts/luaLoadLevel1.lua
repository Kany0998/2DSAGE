--Load a differnt tilemap image depnds on the time of the day
local current_system_hour = os.date("*t").hour

local map_texute_asset_id

if current_system_hour >= 9 and current_system_hour < 19 then
	map_texute_asset_id = "tilemap-texture-day"
else
	map_texute_asset_id = "tilemap-texture-night"
end

--tables for level 1
Level = {
	--Table of assets

	assets =
	{
		[0] =
		{type = "texture", id = "tilemap-texture-day", file = "./assets/tilemaps/jungle.png"},
		{type = "texture", id = "tilemap-texture-night", file = "./assets/tilemaps/jungle-night.png"},
		{type = "texture" , id = "chopper-texture", file="./assets/images/chopper-spritesheet.png"},
		{type = "texture" , id = "takeoff-texture", file="./assets/images/landing-base.png"},
		{type = "texture" , id = "tank-texture", file="./assets/images/tank-panther-right.png"},
		{type = "texture" , id = "truck-texture", file="./assets/images/truck-ford-right.png"},
		{type = "texture" , id = "tree-texture", file="./assets/images/tree.png"},
		{type = "texture" , id = "radar-texture", file="./assets/images/radar.png"},
		{type = "texture" , id = "bullet-texture", file="./assets/images/bullet.png"},
		{type = "font" , id = "charriot-font", file="./assets/fonts/charriot.ttf", font_size = 10}
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
		layer = 0
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
					position = {x = 100, y = 100},
					scale = {x = 3.0, y = 3.0},
					rotation = 0.0 --deg
				},
				rigidbody =
				{
					velocity = {x = 0, y = 0}
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
					health_percentage = 100
				},
				projectile_emitter =
				{
					projectile_velocity = {x = 100, y = 100},
					projectile_duration = 10, --sec
					repeat_frequency = 0,
					hit_percentage_damage = 20,
					friendly = true
				},
				keyboard_controlled =
				{
					up_velocity = {x = 0, y = -200},
					right_velocity = {x = 200, y = 0},
					down_velocity = {x = 0, y = 200},
					left_velocity = {x = -200, y = 0},
					diagnalMovement = true
				},
				camera_follow =
				{
					follow = true
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
					position = {x = 400, y = 400},
					scale = {x = 6.0, y = 6.0},
					rotation = 0.0 --deg
				},
				rigidbody =
				{
					velocity = {x = 100, y = 0}
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
					health_percentage = 100
				},
				projectile_emitter =
				{
					projectile_velocity = {x = 100, y = 0},
					projectile_duration = 10, --sec
					repeat_frequency = 3,
					hit_percentage_damage = 49,
					friendly = false
				},
			}

		},
		{
			--tree
			group = "obstacles",
			components =
			{
				transform =
				{
					position = {x = 800, y = 400},
					scale = {x = 2.0, y = 2.0},
					rotation = 0.0 --deg
				},
				rigidbody =
				{
					velocity = {x = 0, y = 0}
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
					velocity = {x = 0, y = 100}
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
					health_percentage = 100
				},
				projectile_emitter =
				{
					projectile_velocity = {x = 100, y = 0},
					projectile_duration = 10, --sec
					repeat_frequency = 3,
					hit_percentage_damage = 49,
					friendly = false
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
					position = {x = 400, y = 200},
					scale = {x = 5.0, y = 5.0},
					rotation = 0.0 --deg
				},
				rigidbody =
				{
					velocity = {x = 100, y = 0}
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
					health_percentage = 100
				},
				projectile_emitter =
				{
					projectile_velocity = {x = 100, y = 0},
					projectile_duration = 10, --sec
					repeat_frequency = 3,
					hit_percentage_damage = 49,
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
				}

			}

		}
	}
}

--map in pixels
map_width = Level.tilemap.mapNumCols * Level.tilemap.tileSize * Level.tilemap.tileScale
map_height = Level.tilemap.mapNumRows * Level.tilemap.tileSize * Level.tilemap.tileScale