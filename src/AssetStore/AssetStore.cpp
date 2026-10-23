#include "AssetStore.h"
#include "../Logger/Logger.h"
#include "SDL_image.h"

AssetStore::AssetStore(){
	Logger::Log("AssetStore constructor called");
}

AssetStore::~AssetStore(){
	ClearAssets();
	Logger::Log("AssetStore destructor called");
}

void AssetStore::ClearAssets(){
	for(auto texture : textures){
		SDL_DestroyTexture(texture.second);
	}
	textures.clear();

	for(auto font : fonts){
		TTF_CloseFont(font.second);
	}
	fonts.clear();

	//stop everything first: nothing may be freed while a channel or the music is still playing it
	Mix_HaltChannel(-1);        // -1 = all channels
	Mix_HaltMusic();
	for (auto sound : sounds) {
		Mix_FreeChunk(sound.second);
	}
	sounds.clear();

	for (auto mus : music) {
		Mix_FreeMusic(mus.second);
	}
	music.clear();

	textureFacing.clear();
}

void AssetStore::AddTexture(SDL_Renderer* renderer , const std::string& assetId, const std::string& filePath, double facingDegrees){
	SDL_Surface* surface = IMG_Load(filePath.c_str());
	if (surface == nullptr) {
		Logger::Warn("Texture: " + assetId + " encountered problem during loading from '" + filePath + "' return " + IMG_GetError());
		return;
	}
	SDL_Texture* texture = SDL_CreateTextureFromSurface( renderer,surface);
	SDL_FreeSurface(surface);

	//Add facing of texture
	textureFacing.emplace(assetId, facingDegrees);

	//Add texture to the map
	textures.emplace(assetId, texture);
	Logger::Log("New texture added to the Assets Store with id = " + assetId);
}

SDL_Texture* AssetStore::GetTexture(const std::string& assetId){
	auto it = textures.find(assetId);
	if (it == textures.end()) {
		return nullptr;
	}
	return it->second;
}

double AssetStore::GetTextureFacing(const std::string& assetId) const {
	auto it = textureFacing.find(assetId);
	if (it == textureFacing.end()) {
		return 0.0;
	}
	return it->second;
}

void AssetStore::AddFont(const std::string& assetId, const std::string& filePath, int fontSize){
	TTF_Font* font = TTF_OpenFont(filePath.c_str(), fontSize);
	if (font == nullptr) {
		Logger::Warn("Font: " + assetId + " encounterd problem during loading from '" + filePath + "' check filePath or fontSize, return " + TTF_GetError());
		return;
	}
	fonts.emplace(assetId, font);
	Logger::Log("New font assets loaded to the asset store, id: " + assetId);
}

TTF_Font* AssetStore::GetFont(const std::string& assetId){
	auto it = fonts.find(assetId);
	if (it == fonts.end()) {
		return nullptr;
	}
	return it->second;
}

void AssetStore::AddSound(const std::string& assetId, const std::string& filePath) {
	Mix_Chunk* chunk = Mix_LoadWAV(filePath.c_str());

	if (chunk == nullptr) {
		Logger::Warn("Sound: " + assetId + " encountered problem during loading from '" + filePath + "' ,return " + Mix_GetError());
		return;
	}
	sounds.emplace(assetId, chunk);
	Logger::Log("New sound assets loaded to the asset store, id: " + assetId);
}

Mix_Chunk* AssetStore::GetSound(const std::string& assetId) {
	auto it = sounds.find(assetId);
	if (it == sounds.end()) {
		return nullptr;
	}
	return it->second;
}

void AssetStore::AddMusic(const std::string& assetId, const std::string& filePath) {
	Mix_Music* mus = Mix_LoadMUS(filePath.c_str());

	if (mus == nullptr) {
		Logger::Warn("Music: " + assetId + " encountered problem during loadind from '" + filePath + "' ,return " + Mix_GetError());
		return;
	}
	music.emplace(assetId, mus);
	Logger::Log("New music assets loaded to the asset store, id: " + assetId);
}

Mix_Music* AssetStore::GetMusic(const std::string& assetId) {
	auto it = music.find(assetId);
	if (it == music.end()) {
		return nullptr;
	}
	return it->second;
}

