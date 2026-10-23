#ifndef AUDIOSYSTEM_H
#define AUDIOSYSTEM_H

#include "../ECS/ECS.h"
#include "../Components/TransformComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/SoundComponent.h"
#include "../Components/ProgressionComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../AssetStore/AssetStore.h"
#include "../Audio/AudioSettings.h"
#include "../Audio/LevelAmbience.h"
#include "../Audio/AudioStats.h"
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
#include "../Events/UISoundEvent.h"
#include <SDL.h>
#include <SDL_mixer.h>
#include <glm/glm.hpp>
#include <string>
#include <random>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <algorithm>

class AudioSystem {
	
	public:
		AudioSystem(Registry& registry, AssetStore& assetStore, AudioSettings& audioSettings,
			const SDL_Rect& camera, const TileMap& tileMap, const LootDatabase& lootDatabase, std::unique_ptr<EventBus>& eventBus, AudioStats& audioStats, bool audioReady) :
			registry(registry), assetStore(assetStore), audioSettings(audioSettings),
			camera(camera), tileMap(tileMap), lootDatabase(lootDatabase), eventBus(eventBus), audioStats(audioStats), audioReady(audioReady) {

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
			eventBus->SubscribeToEvent<UISoundEvent>(this, &AudioSystem::OnUISound);
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
			auto& soundComp = event.victim.GetComponent<SoundComponent>();

			if (event.victim.HasComponent<TransformComponent>() && event.victim.HasComponent<SpriteComponent>()) {
				const std::string& sound = soundComp.death;
				const glm::vec2 victimCenter = EntityCenter(event.victim.GetComponent<TransformComponent>(), event.victim.GetComponent<SpriteComponent>());
				PlayAt(sound, victimCenter);
			}

			
			if (soundComp.footstepChannel != -1) {
				Mix_HaltChannel(soundComp.footstepChannel);
				soundComp.footstepChannel = -1;
			}

			if (soundComp.loopChannel != -1) {
				Mix_HaltChannel(soundComp.loopChannel);
				soundComp.loopChannel = -1;
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
			if (event.caster.HasComponent<TransformComponent>() && event.caster.HasComponent<SpriteComponent>()) {
				const std::string& sound = (event.soundId.empty() && event.caster.HasComponent<SoundComponent>())
					? event.caster.GetComponent<SoundComponent>().cast : event.soundId;
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

		void OnUISound(UISoundEvent& event) {

			auto it = audioSettings.ui.find(event.action);
			if (it == audioSettings.ui.end()) {
				return;
			}

			Play2D(it->second);
		}

		//effect at a place in the world
		void PlayAt(const std::string& soundId, glm::vec2 worldPosition) {

			if (!audioReady || soundId.empty()) {
				return;
			}
			counting.requested++;

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
				counting.outOfRange++;
				return;
			}

			//checked only after the range test, so a far-away copy can't block a near one
			bool repGapPas = RepeatGapPassed(soundId);
			if (!repGapPas) {
				counting.skippedGap++;
				return;
			}

			//every channel busy: skipped, that's the channel budget working
			int channel = Mix_PlayChannel(-1, sound, 0);
			if (channel == -1) {
				counting.noChannel++;
				return;
			}

			ApplyPosition(channel, worldPosition, listener, static_cast<float>(range), audioSettings.master * audioSettings.effects);
			counting.played++;
		}

		//effect without a place (UI)
		void Play2D(const std::string& soundId) {

			if (!audioReady || soundId.empty()) {
				return;
			}
			counting.requested++;

			Mix_Chunk* sound = FindSound(soundId);
			if (sound == nullptr) {
				return;
			}

			bool repGapPas = RepeatGapPassed(soundId);
			if (!repGapPas) {
				counting.skippedGap++;
				return;
			}
			//channels keep their effects after a sound ends: reset to centre, full volume
			int channel = Mix_PlayChannel(-1, sound, 0);
			if (channel == -1) {
				counting.noChannel++;
				return;
			}

			Mix_Volume(channel, static_cast<int>(audioSettings.effects * audioSettings.master * MIX_MAX_VOLUME));
			Mix_SetPanning(channel, 255, 255);
			Mix_SetDistance(channel, 0);
			counting.played++;
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

			//walking loops: the ground's step sound while the entity moves
			for (auto rawEntity : registry.Raw().view<SoundComponent, TransformComponent, SpriteComponent, RigidBodyComponent>()) {
				Entity entity(rawEntity, &registry);

				auto& sound = entity.GetComponent<SoundComponent>();
				if (!sound.footsteps) {
					continue;
				}

				//is moving taking velocity, not actual velocity becuse sliding should be silent becuse no step is taken
				const auto& rigidBody = entity.GetComponent<RigidBodyComponent>();
				const bool moving = glm::length(rigidBody.velocity) > 0.0f;

				//which sound from tile should be played
				std::string wantedStepSound;
				if (moving) {
					const auto& transform = entity.GetComponent<TransformComponent>();
					const auto& sprite = entity.GetComponent<SpriteComponent>();
					const float feetX = transform.position.x + sprite.width * transform.scale.x / 2.0f;
					const float feetY = transform.position.y + sprite.height * transform.scale.y -1.0f;
					wantedStepSound = tileMap.StepSoundAtWorld(feetX, feetY);
					if (wantedStepSound.empty()) {
						wantedStepSound = audioSettings.defaultStepSound;
					}
				}

				//if wantedStepSound is diffrent than sound.footStep stop old loop start new one
				if (wantedStepSound != sound.footstepSound) {
					sound.footstepSound = wantedStepSound;
					if (sound.footstepChannel != -1) {
						//fade out sound
						Mix_FadeOutChannel(sound.footstepChannel, 100);
						sound.footstepChannel = -1;
					}

					if (!wantedStepSound.empty()) {
						Mix_Chunk* stepSound = FindSound(wantedStepSound);
						if (stepSound != nullptr) {
							int channel = Mix_PlayChannel(-1, stepSound, -1);
							if (channel != -1) {
								//channels keep their old effects, the player's steps are centred, full volume
								Mix_SetPanning(channel, 255, 255);
								Mix_SetDistance(channel, 0);
								sound.footstepChannel = channel;
							}
						}
					}
				}

				//keep the running loop in line with the effects/master sliders
				if (sound.footstepChannel != -1) {
					Mix_Volume(sound.footstepChannel, static_cast<int>(audioSettings.effects* audioSettings.master* MIX_MAX_VOLUME));
				}
			}

			//looping sources (campfires...), only the nearest maxLoopSources within hearing range play
			const glm::vec2 listener = ListenerPosition();
			const float range = audioSettings.hearingRangeTiles * static_cast<float>(tileMap.TileWorldSize());
			const float ambienceVolume = audioSettings.master * audioSettings.ambience;

			//candidates: every loop source within range, with its distance
			std::vector<std::pair<float, entt::entity>> inRange;
			for (auto rawEntity : registry.Raw().view<SoundComponent, TransformComponent, SpriteComponent>()) {
				
				Entity entity(rawEntity, &registry);
				const auto& sound = entity.GetComponent<SoundComponent>();
				if (sound.loop.empty()) {
					continue;
				}

				const glm::vec2 center = EntityCenter(entity.GetComponent<TransformComponent>(), entity.GetComponent<SpriteComponent>());
				const float distanceToSoruce = glm::length(center - listener);
				if (distanceToSoruce < range) {
					inRange.push_back({ distanceToSoruce, rawEntity });
				}
			}
			counting.loopsInRange = static_cast<int>(inRange.size());

			//nearest first only the first maxLoopSources are kept
			std::sort(inRange.begin(), inRange.end(), [](const auto& a, const auto& b) {return a.first < b.first;});
			std::unordered_set<entt::entity> keep;

			for (size_t i = 0; i < inRange.size() && i < static_cast<size_t>(audioSettings.maxLoopSources); i++) {
				keep.insert(inRange[i].second);
			}

			//every loop source: start, update or stop
			for (auto rawEntity : registry.Raw().view<SoundComponent, TransformComponent, SpriteComponent>()) {
				
				Entity entity(rawEntity, &registry);
				auto& sound = entity.GetComponent<SoundComponent>();
				if (sound.loop.empty()) {
					continue;
				}

				const bool kept = keep.count(rawEntity) > 0;

				if (kept) {
					//not playing yet: start it
					if (sound.loopChannel == -1) {
						Mix_Chunk* loopSound = FindSound(sound.loop);
						if (loopSound != nullptr) {
							sound.loopChannel = Mix_PlayChannel(-1, loopSound, -1);		//if -1 all channels are taken
						}
					}
					//playing: follow the listener every frame 
					if (sound.loopChannel != -1) {
						const glm::vec2 center = EntityCenter(entity.GetComponent<TransformComponent>(), entity.GetComponent<SpriteComponent>());
						ApplyPosition(sound.loopChannel, center, listener, range, ambienceVolume);
					}
				}

				else {
					if (sound.loopChannel != -1) {
						//out of range, or a nearer source took its place: fade out and let the channel go
						Mix_FadeOutChannel(sound.loopChannel, 200);
						sound.loopChannel = -1;
					}
				}
			}

			counting.loopsPlaying = static_cast<int>(keep.size());

			if (now - statsSecondStart >= 1000) {
				counting.channelsBusy = Mix_Playing(-1);
				audioStats = counting;
				counting = {};
				statsSecondStart = now;
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

		//volume, distance and pan of a channel for a sound at worldPosition, heard from listener
		void ApplyPosition(int channel, glm::vec2 worldPosition, glm::vec2 listener, float range, float volume) {
			const float distance = glm::length(worldPosition - listener);
			const float t = std::min(distance / range, 1.0f);     //0 = at the listener, 1 = edge of hearing range

			Mix_Volume(channel, static_cast<int>(volume * MIX_MAX_VOLUME));

			//0 = at the listener, 255 = edge of the hearing range
			Mix_SetDistance(channel, static_cast<Uint8>(t * 255.0f));

			//-1 = left edge of the screen, +1 = right; the far ear keeps 40% so nothing sounds one-eared
			const float side = std::clamp((worldPosition.x - listener.x) / (camera.w / 2.0f), -1.0f, 1.0f);
			Uint8 left = static_cast<Uint8>(255 * (1.0 - 0.6f * std::max(side, 0.0f)));
			Uint8 right = static_cast<Uint8>(255 * (1.0 - 0.6f * std::max(-side, 0.0f)));

			Mix_SetPanning(channel, left, right);
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
		AudioStats& audioStats;
		bool audioReady = false;

		LevelAmbience ambience;			//the current level's ambience
		std::mt19937 rng{ std::random_device{}() };
		float appliedAmbienceVolume = -1.0f;
		bool ambienceLoopPlaying = false;	//whether channel 0 holds the loop

		AudioStats counting;		//the current second, still running
		Uint32 statsSecondStart = 0;	//when the current second began

};


#endif