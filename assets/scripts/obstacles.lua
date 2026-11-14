-- One pushable crate; only the position differs between crates
local function make_crate(x, y)
	return {
		group = "obstacles",
		components = {
			transform = 
			{ 
				position = 
				{ 
					x = x, 
					y = y 
				}, 
				scale = 
				{ 
					x = 3.0, 
					y = 3.0 
				}, 
				rotation = 0.0
			},
			rigidbody = 
			{ 
				velocity = 
				{ 
					x = 0, 
					y = 0 
				}, 
				mass = 1.0 
			},
			sprite = {
				texture_asset_id = "crate-texture",
				width = 16,
				height = 32, 
				layer = 2, 
				fixed = false,
				src_rect_x = 0, 
				src_rect_y = 0,
			},
			boxcollider = 
			{ 
				width = 16, 
				height = 20, 
				offset = 
				{ 
					x = 0, 
					y = 6 
				} 
			},
			movement_type = 
			{ 
				type = "ground" 
			},
			pushable = {},
		}
	}
end

return {make_crate = make_crate}