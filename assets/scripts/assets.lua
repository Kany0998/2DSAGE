-- assets/scripts/assets.lua
-- Every texture and font the game uses. Level files load this with dofile.
-- The first entry MUST be [0] =  (LevelLoader counts from 0)

return {
	-- tilemaps
	[0] =
	{type = "texture", id = "tilemap-texture-day",   file = "./assets/tilemaps/jungle.png"},
	{type = "texture", id = "tilemap-texture-night", file = "./assets/tilemaps/jungle-night.png"},
	{type = "texture", id = "tilemap-texture-test", file = "./assets/tilemaps/Map_tiles.png"},

	-- units and props
	{type = "texture", id = "chopper-texture", file = "./assets/images/chopper-spritesheet.png"},
	{type = "texture", id = "takeoff-texture", file = "./assets/images/landing-base.png"},
	{type = "texture", id = "tank-texture",    file = "./assets/images/tank-panther-right.png"},
	{type = "texture", id = "truck-texture",   file = "./assets/images/truck-ford-right.png"},
	{type = "texture", id = "tree-texture",    file = "./assets/images/tree.png"},
	{type = "texture", id = "crate-texture",    file = "./assets/images/obstacles/crate.png"},
	{type = "texture", id = "radar-texture",   file = "./assets/images/radar.png"},

	{type = "texture", id = "goblin-player-texture", file = "./assets/images/player/goblin-spritesheet.png"},
	{type = "texture", id = "goblin-player-attack-texture", file = "./assets/images/player/goblin-attack-spritesheet.png"},

	{type = "texture", id = "desert-goblin-texture",   file = "./assets/images/enemies/goblins/desert-goblin.png"},
	{type = "texture", id = "desert-goblin-attack-texture",   file = "./assets/images/enemies/goblins/desert-goblin-attack.png"},

	{type = "texture", id = "taiga-goblin-texture",   file = "./assets/images/enemies/goblins/taiga-goblin.png"},
	{type = "texture", id = "taiga-goblin-attack-texture",   file = "./assets/images/enemies/goblins/taiga-goblin-attack.png"},

	{type = "texture", id = "sawanna-goblin-texture",   file = "./assets/images/enemies/goblins/sawanna-goblin.png"},
	{type = "texture", id = "sawanna-goblin-attack-texture",   file = "./assets/images/enemies/goblins/sawanna-goblin-attack.png"},


	--goblin boss
	{type = "texture", id = "goblin-boss-walk",   file = "./assets/images/enemies/bosses/goblin-boss-walk.png"},
	{type = "texture", id = "goblin-boss-basic-attack",   file = "./assets/images/enemies/bosses/goblin-boss-basic-attack.png"},
	{type = "texture", id = "goblin-boss-spin-attack",   file = "./assets/images/enemies/bosses/goblin-boss-spin-attack.png"},


	-- projectiles (enemies)
	{type = "texture", id = "bullet-texture",    file = "./assets/images/bullet.png"},
	{type = "texture", id = "desert-goblin-bullet",    file = "./assets/images/enemies/goblins/desert-goblin-bullet.png"},
	{type = "texture", id = "taiga-goblin-bullet",    file = "./assets/images/enemies/goblins/taiga-goblin-bullet.png"},
	{type = "texture", id = "sawanna-goblin-bullet",    file = "./assets/images/enemies/goblins/sawanna-goblin-bullet.png"},


	-- item icons
	
	-- weapons and it's bullets
	{type = "texture", id = "iron-sword",    file = "./assets/images/weapons/iron-sword.png"},
	{type = "texture", id = "iron-sword-bullet", file = "./assets/images/weapons/iron-sword-bullet.png"},

	{type = "texture", id = "desert-sword",    file = "./assets/images/weapons/desert-sword.png"},
	{type = "texture", id = "desert-sword-bullet", file = "./assets/images/weapons/desert-sword-bullet.png"},

	{type = "texture", id = "taiga-sword",    file = "./assets/images/weapons/taiga-sword.png"},
	{type = "texture", id = "taiga-sword-bullet", file = "./assets/images/weapons/taiga-sword-bullet.png"},

	{type = "texture", id = "sawanna-sword",    file = "./assets/images/weapons/sawanna-sword.png"},
	{type = "texture", id = "sawanna-sword-bullet", file = "./assets/images/weapons/sawanna-sword-bullet.png"},


	-- special abilites and it's bullets
	{type = "texture", id = "black-shield",  file = "./assets/images/ability/shield/black-shield.png"},
	{type = "texture", id = "black-shield-shot", file = "./assets/images/ability/shield/black-shield-shot.png", facing_degrees = 45.0},

	{type = "texture", id = "desert-shield",  file = "./assets/images/ability/shield/desert-shield.png"},
	{type = "texture", id = "desert-shield-shot", file = "./assets/images/ability/shield/desert-shield-shot.png", facing_degrees = 45.0},

	{type = "texture", id = "taiga-shield",  file = "./assets/images/ability/shield/taiga-shield.png"},
	{type = "texture", id = "taiga-shield-shot", file = "./assets/images/ability/shield/taiga-shield-shot.png", facing_degrees = 45.0},

	{type = "texture", id = "sawanna-shield",  file = "./assets/images/ability/shield/sawanna-shield.png"},
	{type = "texture", id = "sawanna-shield-shot", file = "./assets/images/ability/shield/sawanna-shield-shot.png", facing_degrees = 45.0},

	-- helms
	{type = "texture", id = "black-helm",    file = "./assets/images/armor/helm/black-helm.png"},
	{type = "texture", id = "desert-helm",    file = "./assets/images/armor/helm/desert-helm.png"},
	{type = "texture", id = "taiga-helm",    file = "./assets/images/armor/helm/taiga-helm.png"},
	{type = "texture", id = "sawanna-helm",    file = "./assets/images/armor/helm/sawanna-helm.png"},


	-- chests
	{type = "texture", id = "black-chest",   file = "./assets/images/armor/chest/black-chest.png"},
	{type = "texture", id = "desert-chest",   file = "./assets/images/armor/chest/desert-chest.png"},
	{type = "texture", id = "taiga-chest",   file = "./assets/images/armor/chest/taiga-chest.png"},
	{type = "texture", id = "sawanna-chest",   file = "./assets/images/armor/chest/sawanna-chest.png"},


	-- legs
	{type = "texture", id = "black-legs",    file = "./assets/images/armor/legs/black-legs.png"},
	{type = "texture", id = "desert-legs",    file = "./assets/images/armor/legs/desert-legs.png"},
	{type = "texture", id = "taiga-legs",    file = "./assets/images/armor/legs/taiga-legs.png"},
	{type = "texture", id = "sawanna-legs",    file = "./assets/images/armor/legs/sawanna-legs.png"},


	-- boots
	{type = "texture", id = "black-boots",   file = "./assets/images/armor/feet/black-boots.png"},
	{type = "texture", id = "desert-boots",   file = "./assets/images/armor/feet/desert-boots.png"},
	{type = "texture", id = "taiga-boots",   file = "./assets/images/armor/feet/taiga-boots.png"},
	{type = "texture", id = "sawanna-boots",   file = "./assets/images/armor/feet/sawanna-boots.png"},


	-- special-items
	{type = "texture", id = "golden-ring",   file = "./assets/images/special_items/rings/golden-ring.png"},
	{type = "texture", id = "golden-amulet", file = "./assets/images/special_items/amulets/golden-amulet.png"},
	{type = "texture", id = "horse-trinket", file = "./assets/images/special_items/trinkets/horse.png"},


	-- loot bags/banners
	{type = "texture", id = "common-loot-bag",   file = "./assets/images/loot_bags/common-loot-bag.png"},
	{type = "texture", id = "uncommon-loot-bag",   file = "./assets/images/loot_bags/uncommon-loot-bag.png"},
	{type = "texture", id = "epic-loot-bag",   file = "./assets/images/loot_bags/epic-loot-bag.png"},
	{type = "texture", id = "legendary-loot-bag",   file = "./assets/images/loot_bags/legendary-loot-bag.png"},
	{type = "texture", id = "legendary-loot-banner",   file = "./assets/images/loot_bags/legendary-loot-banner.png"},


	--sound making sprites (campfire, etc.)
	{type = "texture", id = "campfire-texture",   file = "./assets/images/sound_sprites/campfire.png"},


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
	{type = "sound", id = "goblin-idle-1",    file = "./assets/sounds/goblin-idle-1.wav"},
	{type = "sound", id = "goblin-idle-2",    file = "./assets/sounds/goblin-idle-2.wav"},
	{type = "sound", id = "goblin-idle-3",    file = "./assets/sounds/goblin-idle-3.wav"},


	-- music
	{type = "music", id = "test-music",    file = "./assets/sounds/wind-3.wav"},
}