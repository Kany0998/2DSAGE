#ifndef AUDIOSYSTEM_H
#define AUDIOSYSTEM_H

#include "../ECS/ECS.h"
#include "../Components/TransformComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/SoundComponent.h"
#include "../Components/ProgressionComponent.h"
#include "../AssetStore/AssetStore.h"
#include "../Audio/AudioSettings.h"
#include "../Audio/LevelAmbience.h"
#include "../TileMap/TileMap.h"
#include "../Utils/EntityGeometry.h"
#include "../Logger/Logger.h"
#include "../Loot/LootDatabase.h"
#include "../EventBus/EventBus.h"
#include "../Events/EntityKilledEvent.h"
#include "../Events/LootDroppedEvent.h"
#include "../Events/ProjectileFiredEvent.h"
#include "../Events/EntityHurtEvent.h"
#include "../Events/AbilityCastEvent.h"
#include "../Events/LevelUpEvent.h"
#include <SDL.h>
#include <SDL_mixer.h>
#include <glm/glm.hpp>
#include <string>
#include <random>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>

class AudioSystem {
	
	public:
		AudioSystem(Registry& registry, AssetStore& assetStore, AudioSettings& audioSettings,
			const SDL_Rect& camera, const TileMap& tileMap, const LootDatabase& lootDatabase, std::unique_ptr<EventBus>& eventBus, bool audioReady) :
			registry(registry), assetStore(assetStore), audioSettings(audioSettings),
			camera(camera), tileMap(tileMap), lootDatabase(lootDatabase), eventBus(eventBus), audioReady(audioReady) {

			if (audioReady) {
				Mix_AllocateChannels(audioSettings.channels);
			}
		}

		void SubscribeToEvents() {
			eventBus->SubscribeToEvent<LootDroppedEvent>(this, &AudioSystem::OnLootDropped);
			eventBus->SubscribeToEvent<EntityKilledEvent>(this, &AudioSystem::OnEntityKilled);
			eventBus->SubscribeToEvent<ProjectileFiredEvent>(this, &AudioSystem::OnProjectileFired);
			eventBus->SubscribeToEvent<EntityHurtEvent>(this, &AudioSystem::OnEntityHurt);
			eventBus->SubscribeToEvent<AbilityCastEvent>(this, &AudioSystem::OnSpacialAbilityCast);
			eventBus->SubscribeToEvent<LevelUpEvent>(this, &AudioSystem::OnLevelUp);
		}

		void OnLootDropped(LootDroppedEvent& event) {
			
			const BagTier* tier = lootDatabase.FindBag(event.tierId);
			if (tier == nullptr) {
				return;
			}

			PlayAt(tier->dropSound, event.bagPosition);
			if (event.fromKill) {
				PlayAt(tier->announceSound, event.bagPosition);
			}
		}

		void OnEntityKilled(EntityKilledEvent& event) {
			if (!event.victim.HasComponent<SoundComponent>()) {
				return;
			}

			if (event.victim.HasComponent<TransformComponent>() && event.victim.HasComponent<SpriteComponent>()) {
				const std::string& sound = event.victim.GetComponent<SoundComponent>().death;
				const glm::vec2 victimCenter = EntityCenter(event.victim.GetComponent<TransformComponent>(), event.victim.GetComponent<SpriteComponent>());
				PlayAt(sound, victimCenter);
			}
		}

		void OnProjectileFired(ProjectileFiredEvent& event) {
			PlayAt(event.soundId, event.position);
		}

		void OnEntityHurt(EntityHurtEvent& event) {
			if (!event.victim.HasComponent<SoundComponent>()) {
				return;
			}

			if (event.victim.HasComponent<TransformComponent>() && event.victim.HasComponent<SpriteComponent>()) {
				const std::string& sound = event.victim.GetComponent<SoundComponent>().hurt;
				const glm::vec2 victimCenter = EntityCenter(event.victim.GetComponent<TransformComponent>(), event.victim.GetComponent<SpriteComponent>());
				PlayAt(sound, victimCenter);
			}
		}

		void OnSpacialAbilityCast(AbilityCastEvent& event) {
			if (!event.caster.HasComponent<SoundComponent>()) {
				return;
			}

			if (event.caster.HasComponent<TransformComponent>() && event.caster.HasComponent<SpriteComponent>()) {
				const std::string& sound = event.caster.GetComponent<SoundComponent>().cast;
				const glm::vec2 casterCenter = EntityCenter(event.caster.GetComponent<TransformComponent>(), event.caster.GetComponent<SpriteComponent>());
				PlayAt(sound, casterCenter);
			}
		}

		void OnLevelUp(LevelUpEvent& event) {
			if (!event.player.HasComponent<SoundComponent>()) {
				return;
			}

			if (event.player.HasComponent<TransformComponent>() && event.player.HasComponent<SpriteComponent>()) {
				const glm::vec2 playerCenter = EntityCenter(event.player.GetComponent<TransformComponent>(), event.player.GetComponent<SpriteComponent>());
				const std::string& sound = event.player.GetComponent<SoundComponent>().levelUp;
				PlayAt(sound, playerCenter);
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

		void StartAmbience(const LevelAmbience& newAmbience) {
			if (!audioReady) {
				return;
			}

			ambience = newAmbience;

			if (!ambience.loop.empty()) {
				if (audioSettings.channels < 2) {
					Logger::Warn("audio: ambience loop '" + ambience.loop + "' needs its own channel, but only 1 is available - not played");
				}

				else {
					//channel 0 belongs to the background from now on: Mix_PlayChannel(-1, ...) will never pick it
					Mix_ReserveChannels(1);

					Mix_Chunk* loopSound = FindSound(ambience.loop);
					if (loopSound != nullptr) {
						//channel 0 (the reserved one), -1 = repeat forever
						if (Mix_PlayChannel(0, loopSound, -1) != -1) {
							Mix_SetPanning(0, 255, 255);      //centred: the background is everywhere
							Mix_SetDistance(0, 0);
							ambienceLoopPlaying = true;
							appliedAmbienceVolume = -1.0f;    //forces Update to set the loop volume next frame
						}
					}
				}
			}
			//random sounds
			const Uint32 now = SDL_GetTicks();
			for (auto& sound : ambience.random) {
				std::uniform_int_distribution<Uint32> delay(sound.minMs, sound.maxMs);
				sound.nextAt = now + delay(rng);
			}
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

			//the background loop's volume
			const float wantedAmbience = audioSettings.ambience * audioSettings.master;
			if (ambienceLoopPlaying && wantedAmbience != appliedAmbienceVolume) 
			{
				Mix_Volume(0, static_cast<int>(wantedAmbience * MIX_MAX_VOLUME));
				appliedAmbienceVolume = wantedAmbience;
			}

			//random ambience sounds that are due
			const Uint32 now = SDL_GetTicks();
			for (auto& sound : ambience.random) {
				
				//waiting for its turn
				if (now < sound.nextAt) {
					continue;
				}

				//pick the next time now, so a sound that can't play waits too
				std::uniform_int_distribution<Uint32> delay(sound.minMs, sound.maxMs);
				sound.nextAt = now + delay(rng);

				Mix_Chunk* chunk = FindSound(sound.soundId);
				if (chunk == nullptr) {
					continue;
				}

				int channel = Mix_PlayChannel(-1, chunk, 0);
				if (channel == -1) {
					continue;
				}


				Mix_Volume(channel, static_cast<int>(wantedAmbience * MIX_MAX_VOLUME));
				Mix_SetDistance(channel, 0);
				std::uniform_real_distribution<float> sideRoll(-0.5f, 0.5f);
				const float side = sideRoll(rng);

				Uint8 left = static_cast<Uint8>(255 * (1.0 - 0.6f * std::max(side, 0.0f)));
				Uint8 right = static_cast<Uint8>(255 * (1.0 - 0.6f * std::max(-side, 0.0f)));

				Mix_SetPanning(channel, left, right);
			}

			//enemies and other entities grumbling now and then, each on its own timer
			for (auto rawEntity : registry.Raw().view<SoundComponent, TransformComponent, SpriteComponent>()) {
				Entity entity(rawEntity, &registry);
				auto& sound = entity.GetComponent<SoundComponent>();

				if (sound.idle.empty()) {
					continue;
				}

				//first time this entity is seen: only DRAW its first time, play nothing
				if (sound.nextIdleAt == 0) {
					std::uniform_int_distribution<Uint32> delay(sound.idleMinMs, sound.idleMaxMs);
					sound.nextIdleAt = now + delay(rng);
					continue;
				}

				if (now < sound.nextIdleAt) {
					continue;
				}

				std::uniform_int_distribution<Uint32> delay(sound.idleMinMs, sound.idleMaxMs);
				sound.nextIdleAt = now + delay(rng);
				const glm::vec2 center = EntityCenter(entity.GetComponent<TransformComponent>(), entity.GetComponent<SpriteComponent>());
				PlayAt(sound.idle, center);
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
		const LootDatabase& lootDatabase;
		std::unique_ptr<EventBus>& eventBus;
		bool audioReady = false;

		LevelAmbience ambience;			//the current level's ambience
		std::mt19937 rng{ std::random_device{}() };
		float appliedAmbienceVolume = -1.0f;
		bool ambienceLoopPlaying = false;	//whether channel 0 holds the loop

};


#endif