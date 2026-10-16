#ifndef AUDIOSYSTEM_H
#define AUDIOSYSTEM_H

#include "../ECS/ECS.h"
#include "../Components/TransformComponent.h"
#include "../Components/SpriteComponent.h"
#include "../AssetStore/AssetStore.h"
#include "../Audio/AudioSettings.h"
#include "../TileMap/TileMap.h"
#include "../Utils/EntityGeometry.h"
#include "../Logger/Logger.h"
#include <SDL.h>
#include <SDL_mixer.h>
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>

class AudioSystem {
	
	public:
		AudioSystem(Registry& registry, AssetStore& assetStore, AudioSettings& audioSettings,
			const SDL_Rect& camera, const TileMap& tileMap, bool audioReady) :
			registry(registry), assetStore(assetStore), audioSettings(audioSettings),
			camera(camera), tileMap(tileMap), audioReady(audioReady) {

			if (audioReady) {
				Mix_AllocateChannels(audioSettings.channels);
			}
		}

		//effect at a place in the world
		void PlayAt(const std::string& soundId, glm::vec2 worldPosition) {

			if (!audioReady || soundId.empty()) {
				return;
			}

			Mix_Chunk* sound = FindSound(soundId);
			if (sound == nullptr) {
				return;
			}

			const glm::vec2 listener = ListenerPosition();
			const float distance = glm::length(worldPosition - listener);
			double tileWorldSize = tileMap.TileWorldSize();
			double range = audioSettings.hearingRangeTiles * tileWorldSize;

			//beyond hearing range: not played at all
			if (distance >= range) {
				return;
			}

			//checked only after the range test, so a far-away copy can't block a near one
			bool repGapPas = RepeatGapPassed(soundId);
			if (!repGapPas) {
				return;
			}

			//every channel busy: skipped, that's the channel budget working
			int channel = Mix_PlayChannel(-1, sound, 0);
			if (channel == -1) {
				return;
			}

			Mix_Volume(channel, static_cast<int>(audioSettings.effects * audioSettings.master * MIX_MAX_VOLUME));
			//0 = at the listener, 255 = edge of the hearing range
			Mix_SetDistance(channel, static_cast<Uint8>(distance / range * 255.0f));

			//-1 = left edge of the screen, +1 = right; the far ear keeps 40% so nothing sounds one-eared
			const float side = std::clamp((worldPosition.x - listener.x) / (camera.w / 2.0f), -1.0f, 1.0f);
			Uint8 left = static_cast<Uint8>(255 * (1.0 - 0.6f * std::max(side, 0.0f)));
			Uint8 right = static_cast<Uint8>(255 * (1.0 - 0.6f * std::max(-side, 0.0f)));
			
			Mix_SetPanning(channel, left, right);
		}

		//effect without a place (UI)
		void Play2D(const std::string& soundId) {

			if (!audioReady || soundId.empty()) {
				return;
			}

			Mix_Chunk* sound = FindSound(soundId);
			if (sound == nullptr) {
				return;
			}

			bool repGapPas = RepeatGapPassed(soundId);
			if (!repGapPas) {
				return;
			}
			//channels keep their effects after a sound ends: reset to centre, full volume
			int channel = Mix_PlayChannel(-1, sound, 0);
			if (channel == -1) {
				return;
			}

			Mix_Volume(channel, static_cast<int>(audioSettings.effects * audioSettings.master * MIX_MAX_VOLUME));
			Mix_SetPanning(channel, 255, 255);
			Mix_SetDistance(channel, 0);
		}

		//level music
		void PlayMusic(const std::string& musicId) {

			if (!audioReady || musicId.empty()) {
				return;
			}

			Mix_Music* music = assetStore.GetMusic(musicId);
			if (music == nullptr) {
				Logger::Warn("Audio: couldnt find the music: " + musicId);
				return;
			}
			// loop forever fade in 1000ms
			Mix_FadeInMusic(music, -1, 1000);
		}

		//every frame: keeps the music volume current
		void Update() {
			if (!audioReady) {
				return;
			}

			const float wanted = audioSettings.music * audioSettings.master;

			//only talk to the mixer when the value actually changed, not 60 times a second
			if (wanted != appliedMusicVolume) {
				Mix_VolumeMusic(static_cast<int>(wanted * MIX_MAX_VOLUME));
				appliedMusicVolume = wanted;
			}
		}

	private:

		//GetSound + warn ONCE per unknown id
		Mix_Chunk* FindSound(const std::string& soundId) {
			Mix_Chunk* sound = assetStore.GetSound(soundId);
			if (sound == nullptr && warnedIds.insert(soundId).second) {
				Logger::Warn("audio: sound '" + soundId + "' is not in assets.lua, it stays silent");
			}
			return sound;
		}

		//too soon after the same sound? remember the time
		bool RepeatGapPassed(const std::string& soundId) {
			
			
			Uint32 now = SDL_GetTicks();

			auto it = lastPlayed.find(soundId);
			if (it != lastPlayed.end() && now - it->second < static_cast<Uint32>(audioSettings.repeatGapMs)) {
				return false;          //played too recently
			}

			lastPlayed[soundId] = now;
			return true;
		}

		//where the ears are
		glm::vec2 ListenerPosition() const {
			if (registry.HasEntityWithTag("player")) {
				Entity player = registry.GetEntityByTag("player");
				if (player.HasComponent<TransformComponent>() && player.HasComponent<SpriteComponent>()) {
					return EntityCenter(player.GetComponent<TransformComponent>(), player.GetComponent<SpriteComponent>());
				}
			}
			
			return glm::vec2(camera.x + camera.w / 2.0f, camera.y + camera.h / 2.0f);
			
		}

		std::unordered_set<std::string> warnedIds;				//ids already reported as missing
		std::unordered_map<std::string, Uint32> lastPlayed;		//id -> SDL_GetTicks() of the last play
		float appliedMusicVolume = -1.0f;						//what Update last sent to the mixer (-1 = not yet)

		Registry& registry;
		AssetStore& assetStore;
		AudioSettings& audioSettings;
		const SDL_Rect& camera;
		const TileMap& tileMap;
		bool audioReady = false;

};


#endif