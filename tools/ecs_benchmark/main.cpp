// ECS benchmark for the thesis: the custom ECS the engine started with (copied
// from branch legacy-custom-ecs into namespace legacy) against the current
// EnTT-based Registry, on the same synthetic workloads. The game uses none of
// this; it is a separate executable (ecs_benchmark).
//
// Implementations compared:
//   legacy - the old pattern: System::GetSystemEntities() (a copy of the list)
//            and entity.GetComponent<T>() (pool lookup through unordered_map)
//   engine - the current pattern in the systems: registry.Raw().view<...>(),
//            the Entity wrapper and entity.GetComponent<T>()
//   entt   - EnTT used directly (view.each() / storage.get()), without the
//            wrapper - what the engine could gain by bypassing it
//
// Fairness: the same components (the engine's TransformComponent and
// RigidBodyComponent), the same data from a fixed seed, and the same loop body
// in every implementation. The legacy copy has its log calls removed; the
// engine's ECS.cpp is compiled unchanged against a no-op Logger, so it still
// formats one log string per created and per destroyed entity - the churn test
// is conservative for EnTT.
//
// Build in Release and run it from the folder of the exe. Results are printed
// and written to ./benchmarks/ecs_benchmark.csv and ./benchmarks/ecs_membership.csv.
// "--quick" runs a short version for checking that everything works.

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <numeric>
#include <random>
#include <string>
#include <vector>

#include "ECS/ECS.h"
#include "Components/TransformComponent.h"
#include "Components/RigidBodyComponent.h"
#include "legacy/LegacyECS.h"

namespace{
	// Stands in for ProjectileComponent in the churn test. owner is the id of the
	// entity it was added to, so the membership check can see when a lookup
	// returns another entity's component.
	struct BulletComponent{
		int owner = -1;
		BulletComponent(int owner = -1) : owner(owner) {}
	};

	// Empty tag, like ActiveComponent in the simulation culling.
	struct ActiveComponent {};

	constexpr float DeltaTime = 1.0f / 60.0f;
	constexpr unsigned Seed = 2026;

	struct Settings{
		std::vector<int> entityCounts{ 1000, 10000, 100000 };
		int iterationRuns = 20;        // timed runs per case, after one warm-up run
		int framesPerRun = 100;        // movement updates per run
		int lookupRuns = 20;
		int lookupsPerRun = 1000000;   // random GetComponent calls per run
		int churnRuns = 5;
		int churnFramesPerRun = 20;    // create + destroy cycles per run
		int churnWarmupFrames = 5;
		int bulletsPerFrame = 1000;
		int membershipEntities = 10000;
	};

	using Clock = std::chrono::steady_clock;

	template <typename Work>
	double TimeMs(Work&& work){
		const auto start = Clock::now();
		work();
		const auto end = Clock::now();
		return std::chrono::duration<double, std::milli>(end - start).count();
	}

	struct Stats{
		double avg = 0.0;
		double min = 0.0;
		double max = 0.0;
	};

	Stats Summarise(const std::vector<double>& values){
		Stats stats;
		if (values.empty()){
			return stats;
		}
		stats.min = *std::min_element(values.begin(), values.end());
		stats.max = *std::max_element(values.begin(), values.end());
		stats.avg = std::accumulate(values.begin(), values.end(), 0.0) / static_cast<double>(values.size());
		return stats;
	}

	struct ResultRow{
		std::string test;
		std::string implementation;
		int entities = 0;
		std::string unit;        // what one measured value is ("ms per frame", ...)
		int runs = 0;
		Stats ms;
		double nsPerItem = 0.0;  // ms.avg spread over the entities or lookups
		double checksum = 0.0;   // keeps the work observable; similar across implementations
	};

	struct MembershipRow{
		std::string check;
		int entities = 0;
		int expected = 0;
		int legacySees = 0;
		int legacyWrongReads = 0;
		int engineSees = 0;
		int engineWrongReads = 0;
	};

	struct StartData{
		glm::vec2 position;
		glm::vec2 velocity;
	};

	// Same starting positions and velocities for every implementation.
	std::vector<StartData> MakeStartData(int count){
		std::mt19937 rng(Seed);
		std::uniform_real_distribution<float> position(0.0f, 10000.0f);
		std::uniform_real_distribution<float> velocity(-200.0f, 200.0f);
		std::vector<StartData> data(static_cast<size_t>(count));
		for (auto& item : data){
			item.position = glm::vec2(position(rng), position(rng));
			item.velocity = glm::vec2(velocity(rng), velocity(rng));
		}
		return data;
	}

	std::vector<int> MakeLookupOrder(int count, int lookups){
		std::mt19937 rng(Seed + 1);
		std::uniform_int_distribution<int> pick(0, count - 1);
		std::vector<int> order(static_cast<size_t>(lookups));
		for (auto& index : order){
			index = pick(rng);
		}
		return order;
	}

	//////////////////////////////// legacy systems ////////////////////////////////

	class LegacyMovementSystem : public legacy::System{
		public:
			LegacyMovementSystem(){
				RequireComponent<TransformComponent>();
				RequireComponent<RigidBodyComponent>();
			}

			void Update(float deltaTime){
				//the original pattern: a copy of the entity list on every call
				for (auto entity : GetSystemEntities()){
					auto& transform = entity.GetComponent<TransformComponent>();
					const auto& rigidbody = entity.GetComponent<RigidBodyComponent>();
					transform.position += rigidbody.velocity * deltaTime;
				}
			}
	};

	//a system that only needs the transform, like the render system
	class LegacyTransformSystem : public legacy::System{
		public:
			LegacyTransformSystem() { RequireComponent<TransformComponent>(); }
	};

	class LegacyBulletSystem : public legacy::System{
		public:
			LegacyBulletSystem() { RequireComponent<BulletComponent>(); }
	};

	class LegacyTransformBulletSystem : public legacy::System{
		public:
			LegacyTransformBulletSystem(){
				RequireComponent<TransformComponent>();
				RequireComponent<BulletComponent>();
			}
	};

	class LegacyActiveSystem : public legacy::System{
		public:
			LegacyActiveSystem(){
				RequireComponent<TransformComponent>();
				RequireComponent<ActiveComponent>();
			}
	};

	//////////////////////////////// 1. iteration ////////////////////////////////

	double LegacyChecksum(LegacyMovementSystem& system){
		double sum = 0.0;
		for (auto entity : system.GetSystemEntities()){
			const auto& transform = entity.GetComponent<TransformComponent>();
			sum += static_cast<double>(transform.position.x) + static_cast<double>(transform.position.y);
		}
		return sum;
	}

	double EngineChecksum(Registry& registry){
		double sum = 0.0;
		for (auto rawEntity : registry.Raw().view<TransformComponent>()){
			const auto& transform = registry.Raw().get<TransformComponent>(rawEntity);
			sum += static_cast<double>(transform.position.x) + static_cast<double>(transform.position.y);
		}
		return sum;
	}

	ResultRow IterateLegacy(int count, const Settings& settings){
		const auto start = MakeStartData(count);
		legacy::Registry registry;
		registry.AddSystem<LegacyMovementSystem>();
		for (const auto& item : start){
			legacy::Entity entity = registry.CreateEntity();
			entity.AddComponent<TransformComponent>(item.position);
			entity.AddComponent<RigidBodyComponent>(item.velocity);
		}
		registry.Update(); //the entities join the systems here

		auto& movement = registry.GetSystem<LegacyMovementSystem>();
		std::vector<double> perFrame;
		for (int run = 0; run <= settings.iterationRuns; run++){
			const double ms = TimeMs([&] {
				for (int frame = 0; frame < settings.framesPerRun; frame++){
					movement.Update(DeltaTime);
				}
			});
			if (run > 0){ //run 0 is the warm-up
				perFrame.push_back(ms / settings.framesPerRun);
			}
		}

		ResultRow row{ "iterate", "legacy", count, "ms per frame", settings.iterationRuns, Summarise(perFrame) };
		row.nsPerItem = row.ms.avg * 1.0e6 / count;
		row.checksum = LegacyChecksum(movement);
		return row;
	}

	ResultRow IterateEnTT(int count, const Settings& settings, bool throughWrapper){
		const auto start = MakeStartData(count);
		Registry registry;
		for (const auto& item : start){
			Entity entity = registry.CreateEntity();
			entity.AddComponent<TransformComponent>(item.position);
			entity.AddComponent<RigidBodyComponent>(item.velocity);
		}

		auto moveThroughWrapper = [&registry]() {
			//the pattern the engine's systems use today
			for (auto rawEntity : registry.Raw().view<TransformComponent, RigidBodyComponent>()){
				Entity entity(rawEntity, &registry);
				auto& transform = entity.GetComponent<TransformComponent>();
				const auto& rigidbody = entity.GetComponent<RigidBodyComponent>();
				transform.position += rigidbody.velocity * DeltaTime;
			}
		};
		auto moveDirect = [&registry]() {
			registry.Raw().view<TransformComponent, RigidBodyComponent>().each(
				[](TransformComponent& transform, const RigidBodyComponent& rigidbody) {
					transform.position += rigidbody.velocity * DeltaTime;
				});
		};

		std::vector<double> perFrame;
		for (int run = 0; run <= settings.iterationRuns; run++){
			const double ms = TimeMs([&] {
				for (int frame = 0; frame < settings.framesPerRun; frame++){
					if (throughWrapper){
						moveThroughWrapper();
					}
					else{
						moveDirect();
					}
				}
			});
			if (run > 0){
				perFrame.push_back(ms / settings.framesPerRun);
			}
		}

		ResultRow row{ "iterate", throughWrapper ? "engine" : "entt", count, "ms per frame", settings.iterationRuns, Summarise(perFrame) };
		row.nsPerItem = row.ms.avg * 1.0e6 / count;
		row.checksum = EngineChecksum(registry);
		return row;
	}

	//////////////////////////////// 2. random access ////////////////////////////////

	ResultRow LookupLegacy(int count, const Settings& settings){
		const auto start = MakeStartData(count);
		const auto order = MakeLookupOrder(count, settings.lookupsPerRun);
		legacy::Registry registry;
		registry.AddSystem<LegacyMovementSystem>();
		std::vector<legacy::Entity> entities;
		entities.reserve(static_cast<size_t>(count));
		for (const auto& item : start){
			legacy::Entity entity = registry.CreateEntity();
			entity.AddComponent<TransformComponent>(item.position);
			entity.AddComponent<RigidBodyComponent>(item.velocity);
			entities.push_back(entity);
		}
		registry.Update();

		double sum = 0.0;
		std::vector<double> perRun;
		for (int run = 0; run <= settings.lookupRuns; run++){
			const double ms = TimeMs([&] {
				for (int index : order){
					sum += entities[static_cast<size_t>(index)].GetComponent<TransformComponent>().position.x;
				}
			});
			if (run > 0){
				perRun.push_back(ms);
			}
		}

		ResultRow row{ "random_access", "legacy", count, "ms per " + std::to_string(settings.lookupsPerRun) + " lookups", settings.lookupRuns, Summarise(perRun) };
		row.nsPerItem = row.ms.avg * 1.0e6 / settings.lookupsPerRun;
		row.checksum = sum;
		return row;
	}

	ResultRow LookupEnTT(int count, const Settings& settings, bool throughWrapper){
		const auto start = MakeStartData(count);
		const auto order = MakeLookupOrder(count, settings.lookupsPerRun);
		Registry registry;
		std::vector<Entity> entities;
		entities.reserve(static_cast<size_t>(count));
		for (const auto& item : start){
			Entity entity = registry.CreateEntity();
			entity.AddComponent<TransformComponent>(item.position);
			entity.AddComponent<RigidBodyComponent>(item.velocity);
			entities.push_back(entity);
		}
		auto& transforms = registry.Raw().storage<TransformComponent>();

		double sum = 0.0;
		std::vector<double> perRun;
		for (int run = 0; run <= settings.lookupRuns; run++){
			const double ms = TimeMs([&] {
				if (throughWrapper){
					for (int index : order){
						sum += entities[static_cast<size_t>(index)].GetComponent<TransformComponent>().position.x;
					}
				}
				else{
					for (int index : order){
						sum += transforms.get(entities[static_cast<size_t>(index)].GetHandle()).position.x;
					}
				}
			});
			if (run > 0){
				perRun.push_back(ms);
			}
		}

		ResultRow row{ "random_access", throughWrapper ? "engine" : "entt", count, "ms per " + std::to_string(settings.lookupsPerRun) + " lookups", settings.lookupRuns, Summarise(perRun) };
		row.nsPerItem = row.ms.avg * 1.0e6 / settings.lookupsPerRun;
		row.checksum = sum;
		return row;
	}

	//////////////////////////////// 3. create and destroy churn ////////////////////////////////
	// Background entities stand still (Transform + RigidBody); every frame the
	// bullets of the previous frame are destroyed and the same number of new
	// ones created, like projectiles in a fight.

	ResultRow ChurnLegacy(int background, const Settings& settings){
		const auto start = MakeStartData(background);
		legacy::Registry registry;
		registry.AddSystem<LegacyMovementSystem>();
		registry.AddSystem<LegacyTransformSystem>();
		registry.AddSystem<LegacyBulletSystem>();
		for (const auto& item : start){
			legacy::Entity entity = registry.CreateEntity();
			entity.AddComponent<TransformComponent>(item.position);
			entity.AddComponent<RigidBodyComponent>(item.velocity);
		}
		registry.Update();

		std::vector<legacy::Entity> bullets;
		bullets.reserve(static_cast<size_t>(settings.bulletsPerFrame));
		auto frame = [&]() {
			for (auto& bullet : bullets){
				bullet.Kill();
			}
			bullets.clear();
			for (int i = 0; i < settings.bulletsPerFrame; i++){
				legacy::Entity bullet = registry.CreateEntity();
				bullet.AddComponent<TransformComponent>(glm::vec2(static_cast<float>(i), 0.0f));
				bullet.AddComponent<RigidBodyComponent>(glm::vec2(300.0f, 0.0f));
				bullet.AddComponent<BulletComponent>(bullet.GetId());
				bullets.push_back(bullet);
			}
			registry.Update();
		};

		for (int warmup = 0; warmup < settings.churnWarmupFrames; warmup++){
			frame();
		}
		std::vector<double> perFrame;
		for (int run = 0; run < settings.churnRuns; run++){
			const double ms = TimeMs([&] {
				for (int i = 0; i < settings.churnFramesPerRun; i++){
					frame();
				}
			});
			perFrame.push_back(ms / settings.churnFramesPerRun);
		}

		ResultRow row{ "churn", "legacy", background, "ms per frame (" + std::to_string(settings.bulletsPerFrame) + " created + destroyed)", settings.churnRuns, Summarise(perFrame) };
		row.nsPerItem = row.ms.avg * 1.0e6 / settings.bulletsPerFrame;
		row.checksum = static_cast<double>(registry.GetSystem<LegacyBulletSystem>().GetSystemEntities().size());
		return row;
	}

	ResultRow ChurnEngine(int background, const Settings& settings){
		const auto start = MakeStartData(background);
		Registry registry;
		for (const auto& item : start){
			Entity entity = registry.CreateEntity();
			entity.AddComponent<TransformComponent>(item.position);
			entity.AddComponent<RigidBodyComponent>(item.velocity);
		}

		std::vector<Entity> bullets;
		bullets.reserve(static_cast<size_t>(settings.bulletsPerFrame));
		auto frame = [&]() {
			for (auto& bullet : bullets){
				bullet.Kill();
			}
			bullets.clear();
			for (int i = 0; i < settings.bulletsPerFrame; i++){
				Entity bullet = registry.CreateEntity();
				bullet.AddComponent<TransformComponent>(glm::vec2(static_cast<float>(i), 0.0f));
				bullet.AddComponent<RigidBodyComponent>(glm::vec2(300.0f, 0.0f));
				bullet.AddComponent<BulletComponent>(bullet.GetId());
				bullets.push_back(bullet);
			}
			registry.Update(); //destroys the bullets killed above
		};

		for (int warmup = 0; warmup < settings.churnWarmupFrames; warmup++){
			frame();
		}
		std::vector<double> perFrame;
		for (int run = 0; run < settings.churnRuns; run++){
			const double ms = TimeMs([&] {
				for (int i = 0; i < settings.churnFramesPerRun; i++){
					frame();
				}
			});
			perFrame.push_back(ms / settings.churnFramesPerRun);
		}

		ResultRow row{ "churn", "engine", background, "ms per frame (" + std::to_string(settings.bulletsPerFrame) + " created + destroyed)", settings.churnRuns, Summarise(perFrame) };
		row.nsPerItem = row.ms.avg * 1.0e6 / settings.bulletsPerFrame;
		row.checksum = static_cast<double>(registry.Raw().storage<BulletComponent>().size());
		return row;
	}

	ResultRow ChurnDirect(int background, const Settings& settings){
		const auto start = MakeStartData(background);
		entt::registry registry;
		for (const auto& item : start){
			const auto entity = registry.create();
			registry.emplace<TransformComponent>(entity, item.position);
			registry.emplace<RigidBodyComponent>(entity, item.velocity);
		}

		std::vector<entt::entity> bullets;
		bullets.reserve(static_cast<size_t>(settings.bulletsPerFrame));
		auto frame = [&]() {
			registry.destroy(bullets.begin(), bullets.end());
			bullets.clear();
			for (int i = 0; i < settings.bulletsPerFrame; i++){
				const auto bullet = registry.create();
				registry.emplace<TransformComponent>(bullet, glm::vec2(static_cast<float>(i), 0.0f));
				registry.emplace<RigidBodyComponent>(bullet, glm::vec2(300.0f, 0.0f));
				registry.emplace<BulletComponent>(bullet, static_cast<int>(entt::to_integral(bullet)));
				bullets.push_back(bullet);
			}
		};

		for (int warmup = 0; warmup < settings.churnWarmupFrames; warmup++){
			frame();
		}
		std::vector<double> perFrame;
		for (int run = 0; run < settings.churnRuns; run++){
			const double ms = TimeMs([&] {
				for (int i = 0; i < settings.churnFramesPerRun; i++){
					frame();
				}
			});
			perFrame.push_back(ms / settings.churnFramesPerRun);
		}

		ResultRow row{ "churn", "entt", background, "ms per frame (" + std::to_string(settings.bulletsPerFrame) + " created + destroyed)", settings.churnRuns, Summarise(perFrame) };
		row.nsPerItem = row.ms.avg * 1.0e6 / settings.bulletsPerFrame;
		row.checksum = static_cast<double>(registry.storage<BulletComponent>().size());
		return row;
	}

	//////////////////////////////// 4. membership after runtime changes ////////////////////////////////
	// Not a timing: what a system sees after components are added or removed
	// once an entity already exists. The legacy registry assigns an entity to
	// its systems only once, in Update() after creation; later AddComponent and
	// RemoveComponent only change the signature bits. The culling in the engine
	// adds and removes ActiveComponent at runtime, so it needs the EnTT behaviour.

	std::vector<MembershipRow> MembershipCheck(int count){
		std::vector<MembershipRow> rows;
		const int changed = (count + 9) / 10; //every 10th entity

		//legacy
		legacy::Registry legacyRegistry;
		legacyRegistry.AddSystem<LegacyTransformBulletSystem>();
		legacyRegistry.AddSystem<LegacyActiveSystem>();
		std::vector<legacy::Entity> legacyEntities;
		for (int i = 0; i < count; i++){
			legacy::Entity entity = legacyRegistry.CreateEntity();
			entity.AddComponent<TransformComponent>();
			entity.AddComponent<BulletComponent>(entity.GetId());
			legacyEntities.push_back(entity);
		}
		legacyRegistry.Update();
		for (int i = 0; i < count; i += 10){
			legacyEntities[static_cast<size_t>(i)].RemoveComponent<BulletComponent>();
		}
		legacyRegistry.Update();
		const auto legacyListed = legacyRegistry.GetSystem<LegacyTransformBulletSystem>().GetSystemEntities();
		int legacyWrong = 0;
		for (auto entity : legacyListed){
			//a removed component is still looked up; the pool then hands back another entity's data
			if (entity.GetComponent<BulletComponent>().owner != entity.GetId()){
				legacyWrong++;
			}
		}
		for (int i = 0; i < count; i += 10){
			legacyEntities[static_cast<size_t>(i)].AddComponent<ActiveComponent>();
		}
		legacyRegistry.Update();
		const int legacyActive = static_cast<int>(legacyRegistry.GetSystem<LegacyActiveSystem>().GetSystemEntities().size());

		//engine (EnTT)
		Registry engineRegistry;
		std::vector<Entity> engineEntities;
		for (int i = 0; i < count; i++){
			Entity entity = engineRegistry.CreateEntity();
			entity.AddComponent<TransformComponent>();
			entity.AddComponent<BulletComponent>(entity.GetId());
			engineEntities.push_back(entity);
		}
		for (int i = 0; i < count; i += 10){
			engineEntities[static_cast<size_t>(i)].RemoveComponent<BulletComponent>();
		}
		engineRegistry.Update();
		int engineListed = 0;
		int engineWrong = 0;
		for (auto rawEntity : engineRegistry.Raw().view<TransformComponent, BulletComponent>()){
			engineListed++;
			if (engineRegistry.Raw().get<BulletComponent>(rawEntity).owner != static_cast<int>(entt::to_integral(rawEntity))){
				engineWrong++;
			}
		}
		for (int i = 0; i < count; i += 10){
			engineEntities[static_cast<size_t>(i)].AddComponent<ActiveComponent>();
		}
		engineRegistry.Update();
		int engineActive = 0;
		for ([[maybe_unused]] auto rawEntity : engineRegistry.Raw().view<TransformComponent, ActiveComponent>()){
			engineActive++;
		}

		rows.push_back({ "component removed after creation (Transform + Bullet)", count, count - changed,
			static_cast<int>(legacyListed.size()), legacyWrong, engineListed, engineWrong });
		rows.push_back({ "component added after creation (Transform + Active)", count, changed,
			legacyActive, 0, engineActive, 0 });
		return rows;
	}

	//////////////////////////////// output ////////////////////////////////

	void PrintRow(const ResultRow& row){
		std::printf("  %-14s %-7s %7d  avg %10.4f  min %10.4f  max %10.4f ms  %9.2f ns/item\n",
			row.test.c_str(), row.implementation.c_str(), row.entities, row.ms.avg, row.ms.min, row.ms.max, row.nsPerItem);
	}

	std::string CompilerName(){
	#if defined(_MSC_VER)
			return "MSVC " + std::to_string(_MSC_VER);
	#elif defined(__clang__)
			return std::string("Clang ") + __clang_version__;
	#elif defined(__GNUC__)
			return std::string("GCC ") + __VERSION__;
	#else
			return "unknown compiler";
	#endif
	}

	void WriteCsv(const std::vector<ResultRow>& rows, const std::vector<MembershipRow>& membership){
		std::filesystem::create_directories("benchmarks");

		std::ofstream results("benchmarks/ecs_benchmark.csv");
		results << "test,implementation,entities,unit,runs,avg_ms,min_ms,max_ms,ns_per_item,checksum\n";
		results.setf(std::ios::fixed);
		results.precision(6);
		for (const auto& row : rows){
			results << row.test << ',' << row.implementation << ',' << row.entities << ",\"" << row.unit << "\","
				<< row.runs << ',' << row.ms.avg << ',' << row.ms.min << ',' << row.ms.max << ','
				<< row.nsPerItem << ',' << row.checksum << '\n';
		}

		std::ofstream check("benchmarks/ecs_membership.csv");
		check << "check,entities,expected,legacy_sees,legacy_wrong_reads,engine_sees,engine_wrong_reads\n";
		for (const auto& row : membership){
			check << '"' << row.check << "\"," << row.entities << ',' << row.expected << ',' << row.legacySees << ','
				<< row.legacyWrongReads << ',' << row.engineSees << ',' << row.engineWrongReads << '\n';
		}

		std::printf("\nResults written to %s and %s\n",
			std::filesystem::absolute("benchmarks/ecs_benchmark.csv").string().c_str(),
			std::filesystem::absolute("benchmarks/ecs_membership.csv").string().c_str());
	}
}

int main(int argc, char** argv){
	Settings settings;
	const bool quick = argc > 1 && std::string(argv[1]) == "--quick";
	if (quick){
		settings.entityCounts = { 1000, 10000 };
		settings.iterationRuns = 3;
		settings.framesPerRun = 10;
		settings.lookupRuns = 3;
		settings.lookupsPerRun = 100000;
		settings.churnRuns = 2;
		settings.churnFramesPerRun = 3;
		settings.churnWarmupFrames = 1;
		settings.membershipEntities = 1000;
	}

		std::printf("2DSAGE ECS benchmark: legacy custom ECS vs EnTT (%s)%s\n", CompilerName().c_str(), quick ? " - quick run" : "");
	#ifndef NDEBUG
		std::printf("WARNING: this is not a Release build, the timings are not representative.\n");
	#endif

	std::vector<ResultRow> rows;

	std::printf("\n1. Iteration: position += velocity * dt over every entity (%d frames per run, %d runs)\n",
		settings.framesPerRun, settings.iterationRuns);
	for (int count : settings.entityCounts){
		for (const auto& row : { IterateLegacy(count, settings), IterateEnTT(count, settings, true), IterateEnTT(count, settings, false) }){
			PrintRow(row);
			rows.push_back(row);
		}
	}

	std::printf("\n2. Random access: GetComponent<Transform> for random entities (%d lookups per run, %d runs)\n",
		settings.lookupsPerRun, settings.lookupRuns);
	for (int count : settings.entityCounts){
		for (const auto& row : { LookupLegacy(count, settings), LookupEnTT(count, settings, true), LookupEnTT(count, settings, false) }){
			PrintRow(row);
			rows.push_back(row);
		}
	}

	std::printf("\n3. Churn: %d bullets destroyed and created per frame next to N standing entities (%d frames per run, %d runs)\n",
		settings.bulletsPerFrame, settings.churnFramesPerRun, settings.churnRuns);
	for (int count : settings.entityCounts){
		for (const auto& row : { ChurnLegacy(count, settings), ChurnEngine(count, settings), ChurnDirect(count, settings) }){
			PrintRow(row);
			rows.push_back(row);
		}
	}

	std::printf("\n4. Membership after runtime component changes (%d entities, every 10th changed)\n", settings.membershipEntities);
	const auto membership = MembershipCheck(settings.membershipEntities);
	for (const auto& row : membership){
		std::printf("  %-55s expected %6d | legacy sees %6d (wrong reads %5d) | EnTT sees %6d (wrong reads %d)\n",
			row.check.c_str(), row.expected, row.legacySees, row.legacyWrongReads, row.engineSees, row.engineWrongReads);
	}

	WriteCsv(rows, membership);
	return 0;
}
