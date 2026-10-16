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

	-- loot bags/banners
	{type = "texture", id = "common-loot-bag",   file = "./assets/images/loot_bags/common-loot-bag.png"},
	{type = "texture", id = "epic-loot-bag",   file = "./assets/images/loot_bags/epic-loot-bag.png"},
	{type = "texture", id = "legendary-loot-bag",   file = "./assets/images/loot_bags/legendary-loot-bag.png"},
	{type = "texture", id = "legendary-loot-banner",   file = "./assets/images/loot_bags/legendary-loot-banner.png"},

	-- fonts
	{type = "font", id = "charriot-font", file = "./assets/fonts/charriot.ttf", font_size = 10},

	-- sounds
	{type = "sound", id = "bag-drop",    file = "./assets/sounds/bag-drop.wav"},
	{type = "sound", id = "bag-open",    file = "./assets/sounds/bag-open.wav"},
	{type = "sound", id = "birds-1",    file = "./assets/sounds/birds-1.wav"},
	{type = "sound", id = "birds-2",    file = "./assets/sounds/birds-2.wav"},
	{type = "sound", id = "birds-3",    file = "./assets/sounds/birds-3.wav"},
	{type = "sound", id = "enemy-hurt",    file = "./assets/sounds/enemy-hurt.wav"},
	{type = "sound", id = "fire-loop",    file = "./assets/sounds/fire-loop.wav"},
	{type = "sound", id = "footstep-grass",    file = "./assets/sounds/footstep-grass.wav"},
	{type = "sound", id = "footstep-ice",    file = "./assets/sounds/footstep-ice.wav"},
	{type = "sound", id = "footstep-snow",    file = "./assets/sounds/footstep-snow.wav"},
	{type = "sound", id = "legendary-aura",    file = "./assets/sounds/legendary-aura.wav"},
	{type = "sound", id = "level-up",    file = "./assets/sounds/level-up.wav"},
	{type = "sound", id = "player-hurt",    file = "./assets/sounds/player-hurt.wav"},
	{type = "sound", id = "sword-swing",    file = "./assets/sounds/sword-swing.wav"},
	{type = "sound", id = "ui-equip",    file = "./assets/sounds/ui-equip.wav"},
	{type = "sound", id = "ui-unequip",    file = "./assets/sounds/ui-unequip.wav"},
	{type = "sound", id = "wind-1",    file = "./assets/sounds/wind-1.wav"},
	{type = "sound", id = "wind-2",    file = "./assets/sounds/wind-2.wav"},
	{type = "sound", id = "wind-3",    file = "./assets/sounds/wind-3.wav"},

}