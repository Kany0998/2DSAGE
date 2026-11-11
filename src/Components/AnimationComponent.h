#ifndef ANIMATIONCOMPONENT_H
#define ANIMATIONCOMPONENT_H

#include<SDL.h>
#include<string>

struct AnimationComponent
{
	int numFrames;
	int currentFrame;
	int frameSpeedRate;
	bool isLoop;
	bool onlyWhenMoving;
	int startTime;

	//attack clip: settings read once from lua
	std::string attackAssetId = "";
	int attackNumFrames = 1;
	int attackSpeedRate = 1;

	//attack clip: state changed by animationSystem while it plays
	bool isAttacking = false;
	int attackStartTime = 0;
	int attackRow = 0;
	bool attackFacesLeft = false;
	std::string walkAssetId = "";
	int attackDurationMs = 0;
	int facingLockUntil = 0;		// until this time (SDL ticks) the shot direction decides facing

	AnimationComponent(int numFrames = 1, int frameSpeedRate = 1, bool isLoop = true, bool onlyWhenMoving = false){
		this->numFrames = numFrames;
		this->currentFrame = 1;
		this->frameSpeedRate = frameSpeedRate;
		this->isLoop = isLoop;
		this->onlyWhenMoving = onlyWhenMoving;
		this->startTime = SDL_GetTicks();
	}
};

#endif
