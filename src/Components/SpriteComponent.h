#ifndef SPRITECOMPONENT_H
#define SPRITECOMPONENT_H

#include <string>
#include <SDL.h>

struct SpriteComponent
{
	std::string assetId;
	int width;
	int height;
	SDL_Rect srcRect;
	int layer;
	SDL_RendererFlip flip;
	bool isFixed;

	//Recomended use for layers
	/*
		LAYER_TILEMAP, = 0
		LAYER_VEGETATION, = 1
		LAYER_OBSTACLE, = 2
		LAYER_ENEMIES, = 3
		LAYER_PLAYER_ON_GROUND, = 4 
		LAYER_BOSS, = 5
		LAYER_ENEMIES_IN_AIR, = 6
		LAYER_PLAYER_IN_AIR, = 7
		LAYER_PROJECTILES, = 8
		LAYER_GUI = 9
	*/

	SpriteComponent(std::string assetId = "",int width = 0, int height = 0, int layer = 0,bool isFixed = false,  int srcRectX = 0, int srcRectY = 0)
	{
		this->assetId = assetId;
		this->width = width;
		this->height = height;
		this->layer = layer;
		this->srcRect = { srcRectX, srcRectY, width, height };
		this->flip = SDL_FLIP_NONE;
		this->isFixed = isFixed;
	}
};


#endif
