#ifndef ASSETSTORE_H
#define ASSETSTORE_H

#include <map>
#include <string>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>

class AssetStore
{
	private:
		std::map <std::string, SDL_Texture*> textures;
		std::map <std::string, double> textureFacing;
		std::map <std::string, TTF_Font*> fonts;
		std::map <std::string, Mix_Chunk*> sounds;		//effects: decoded once, fully in memory
		std::map <std::string, Mix_Music*> music;		//music: streamed from the file while it plays
	public:
		AssetStore();
		~AssetStore();

		void ClearAssets();

		void AddTexture(SDL_Renderer* renderer ,const std::string& assetId , const std::string& filePath, double facingDegrees = 0.0);
		SDL_Texture* GetTexture(const std::string& assetId);
		double GetTextureFacing(const std::string& assetId) const;

		void AddFont(const std::string& assetId, const std::string& filePath, int fontSize);
		TTF_Font* GetFont(const std::string& assetId);

		void AddSound(const std::string& assetId, const std::string& filePath);
		Mix_Chunk* GetSound(const std::string& assetId);

		void AddMusic(const std::string& assetId, const std::string& filePath);
		Mix_Music* GetMusic(const std::string& assetId);

};

#endif
