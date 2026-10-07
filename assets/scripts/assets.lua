-- assets/scripts/assets.lua
-- Every texture and font the game uses. Level files load this with dofile.
-- The first entry MUST be [0] =  (LevelLoader counts from 0)

return {
	-- tilemaps
	[0] =
	{type = "texture", id = "tilemap-texture-day",   file = "./assets/tilemaps/jungle.png"},
	{type = "texture", id = "tilemap-texture-night", file = "./assets/tilemaps/jungle-night.png"},

	-- units and props
	{type = "texture", id = "chopper-texture", file = "./assets/images/chopper-spritesheet.png"},
	{type = "texture", id = "takeoff-texture", file = "./assets/images/landing-base.png"},
	{type = "texture", id = "tank-texture",    file = "./assets/images/tank-panther-right.png"},
	{type = "texture", id = "truck-texture",   file = "./assets/images/truck-ford-right.png"},
	{type = "texture", id = "tree-texture",    file = "./assets/images/tree.png"},
	{type = "texture", id = "radar-texture",   file = "./assets/images/radar.png"},

	-- projectiles
	{type = "texture", id = "bullet-texture",    file = "./assets/images/bullet.png"},
	{type = "texture", id = "iron-sword-bullet", file = "./assets/images/weapons/iron-sword-bullet.png"},
	{type = "texture", id = "black-shield-shot", file = "./assets/images/ability/shield/black-shield-shot.png"},

	-- item icons
	{type = "texture", id = "iron-sword",    file = "./assets/images/weapons/iron-sword.png"},
	{type = "texture", id = "golden-ring",   file = "./assets/images/special_items/rings/golden-ring.png"},
	{type = "texture", id = "golden-amulet", file = "./assets/images/special_items/amulets/golden-amulet.png"},
	{type = "texture", id = "horse-trinket", file = "./assets/images/special_items/trinkets/horse.png"},
	{type = "texture", id = "black-shield",  file = "./assets/images/ability/shield/black-shield.png"},
	{type = "texture", id = "black-helm",    file = "./assets/images/armor/helm/black-helm.png"},
	{type = "texture", id = "black-chest",   file = "./assets/images/armor/chest/black-chest.png"},
	{type = "texture", id = "black-legs",    file = "./assets/images/armor/legs/black-legs.png"},
	{type = "texture", id = "black-boots",   file = "./assets/images/armor/feet/black-boots.png"},

	-- fonts
	{type = "font", id = "charriot-font", file = "./assets/fonts/charriot.ttf", font_size = 10},
}