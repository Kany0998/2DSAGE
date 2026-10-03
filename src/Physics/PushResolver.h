#ifndef PUSHRESOLVER_H
#define PUSHRESOLVER_H

#include "../ECS/ECS.h"
#include "../Components/TransformComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/BoxColliderComponent.h"
#include "../Components/PushableComponent.h"
#include "../Components/MovementTypeComponent.h"
#include "../Components/ProjectileComponent.h"
#include "../TileMap/TileMap.h"
#include "../Utils/EntityGeometry.h"
#include <algorithm>
#include <cmath>
#include <vector>

class PushResolver{

	public:
		
		//All crates the mover would overlap if it moved to the candidate position
		static void CollectSolidBlockers(Registry& registry, Entity mover, double candidateX, double candidateY, std::vector<Entity>& outBlockers, bool alongX) {
			outBlockers.clear();

			if (!mover.HasComponent<BoxColliderComponent>()) {
				return;
			}

			const auto& moverTransform = mover.GetComponent<TransformComponent>();
			const Box candidateBox = BoxAt(mover, candidateX, candidateY);
			const Box currentBox = BoxAt(mover, moverTransform.position.x, moverTransform.position.y);

			for (auto rawSolid : registry.Raw().view<PushableComponent, BoxColliderComponent, TransformComponent>()) {
				Entity solid(rawSolid, &registry);

				if (solid == mover) {
					continue;
				}

				const auto& solidTransform = solid.GetComponent<TransformComponent>();
				const Box solidBox = BoxAt(solid, solidTransform.position.x, solidTransform.position.y);

				if (!Overlaps(candidateBox, solidBox)) {
					continue;
				}

				if (Overlaps(currentBox, solidBox) && MoveEscapes(currentBox, candidateBox, solidBox, alongX)) {
					continue;
				}

				outBlockers.push_back(solid);

			}
		}


		//How far the mover may travel along one axis this frame: the whole delta when nothing
		//is in the way, otherwise a share of it, and every crate in the way moves that same
		//share - so the two stay in contact without ever overlapping
		static double PushAlongAxis(Registry& registry, const TileMap& tileMap, Entity mover, double candidateX, double candidateY, double delta, bool alongX, double deltaTime) {

			//Not moving on this axis - skips the crate scan for everything standing still
			if (delta == 0.0) {
				return 0.0;
			}

			std::vector<Entity> blockers;
			CollectSolidBlockers(registry, mover, candidateX, candidateY, blockers, alongX);

			if (blockers.empty()) {
				return delta;
			}

			const auto& moverRigidbody = mover.GetComponent<RigidBodyComponent>();

			//Full distance - shared is not known yet, it depends on the mass
			const double fullDeltaX = alongX ? delta : 0.0;
			const double fullDeltaY = alongX ? 0.0 : delta;

			double totalMass = 0.0;
			std::vector<Entity> massVisited;

			for (const auto& blocker : blockers) {
				//A crate with no body has no mass to share the push with - treat it as a wall
				if (!blocker.HasComponent<RigidBodyComponent>()) {
					return 0.0;
				}

				totalMass += RowMass(registry, blocker, fullDeltaX, fullDeltaY, alongX, massVisited);
			}
			

			//Heavier rows, smaller share for both
			const double shared = delta * moverRigidbody.mass / (moverRigidbody.mass + totalMass);

			//How far this crate would move on each axis - shared on the pushed one, nothing on the other
			const double blockerDeltaX = alongX ? shared : 0.0;
			const double blockerDeltaY = alongX ? 0.0 : shared;

			std::vector<Entity> checkVisited;
			for (const auto& blocker : blockers) {

				if (!CanMoveSolid(registry, tileMap, blocker, blockerDeltaX, blockerDeltaY, alongX, checkVisited)) {
					return 0.0;
				}
			}

			std::vector<Entity> moveVisited;
			for (const auto& blocker : blockers) {
				MoveSolid(registry, tileMap, blocker, blockerDeltaX, blockerDeltaY, alongX, moveVisited, deltaTime);
			}

			return shared;
		}

		//Whether a crate moving to the candidate position would run into a player or an enemy
		static bool IsBlockedByBody(Registry& registry, Entity mover,double candidateX, double candidateY, bool alongX) {

			if (!mover.HasComponent<BoxColliderComponent>()) {
				return false;
			}

			const auto& moverTransform = mover.GetComponent<TransformComponent>();
			const Box candidateBox = BoxAt(mover, candidateX, candidateY);
			const Box currentBox = BoxAt(mover, moverTransform.position.x, moverTransform.position.y);

			for (auto rawBody : registry.Raw().view<BoxColliderComponent, TransformComponent>()) {
				Entity body(rawBody, &registry);

				if (body == mover || body.HasComponent<PushableComponent>() || body.HasComponent<ProjectileComponent>()) {
					continue;
				}

				const auto& bodyTransform = body.GetComponent<TransformComponent>();
				const Box bodyBox = BoxAt(body, bodyTransform.position.x, bodyTransform.position.y);

				if (!Overlaps(candidateBox, bodyBox)) {
					continue;
				}

				if (Overlaps(currentBox, bodyBox) && MoveEscapes(currentBox, candidateBox, bodyBox, alongX)) {
					continue;
				}

				return true; // found the body in the way
			}
			return false; //didint find body in the way
		}

	private:
		//Maximum amount of pushed obsticles in one push, if not used the operations may become too expensive
		static constexpr int maxPushCount = 4;


		//Whether this crate was already handled in the current walk
		static bool AlreadyVisited(const std::vector<Entity>& visited, Entity solid) {
			return std::find(visited.begin(), visited.end(), solid) != visited.end();
		}


		//Cheks if obsticle that we push can enter certain tiles
		static bool SolidEdgeIsClear(const TileMap& tileMap, Entity solid,
			double candidateX, double candidateY, double deltaX, double deltaY, int movementType) {

			const Box solidBox = BoxAt(solid, candidateX, candidateY);
			const double inset = 1.0;
			
			double firstX = 0.0;
			double secondX = 0.0;
			double firstY = 0.0;
			double secondY = 0.0;

			if (std::abs(deltaX) > std::abs(deltaY)) {

				const double edgeX = (deltaX > 0.0) ? solidBox.left + solidBox.width - inset : solidBox.left + inset;
				firstX = edgeX;
				secondX = edgeX;
				firstY = solidBox.top + inset;
				secondY = solidBox.top + solidBox.height - inset;
			}

			else {
				const double edgeY = (deltaY > 0.0) ? solidBox.top + solidBox.height - inset : solidBox.top + inset;
				firstX = solidBox.left + inset;
				secondX = solidBox.left + solidBox.width - inset;
				firstY = edgeY;
				secondY = edgeY;
			}

			return !tileMap.isBlockedAtWorld(firstX, firstY, movementType) && !tileMap.isBlockedAtWorld(secondX, secondY, movementType);
		}


		//Whether the move keeps the mover from going deeper into something it is already inside
		static bool MoveEscapes(const Box& current, const Box& candidate, const Box& other, bool alongX) {

			if (alongX) {
				//How much current and other share horizontally
				const double currentDepth = std::min(current.left + current.width, other.left + other.width) - std::max(current.left, other.left);

				//Same for where the mover wants to be
				const double candidateDepth = std::min(candidate.left + candidate.width, other.left + other.width) - std::max(candidate.left, other.left);

				//<= rather than <: a mover wider than the crate keeps the same depth for its first
				//steps out, and with < it would push the crate along forever instead of leaving it
				return candidateDepth <= currentDepth;
			}

			else {
				//How much current and other share vertically
				const double currentDepth = std::min(current.top + current.height, other.top + other.height) - std::max(current.top, other.top);

				//Same for where the mover wants to be
				const double candidateDepth = std::min(candidate.top + candidate.height, other.top + other.height) - std::max(candidate.top, other.top);
				return candidateDepth <= currentDepth;
			}
		}


		//Checks if row of ceretien length can be moved
		static bool CanMoveSolid(Registry& registry, const TileMap& tileMap, Entity solid,
			double deltaX, double deltaY, bool alongX, std::vector<Entity>& visited) {

			if (AlreadyVisited(visited, solid)) {
				return true;
			}

			visited.push_back(solid);

			if (static_cast<int>(visited.size()) > maxPushCount) {
				return false;
			}

			const auto& solidTransform = solid.GetComponent<TransformComponent>();
			
			const double candidateX = solidTransform.position.x + deltaX;
			const double candidateY = solidTransform.position.y + deltaY;

			const int solidMovementType = solid.HasComponent<MovementTypeComponent>()
				? solid.GetComponent<MovementTypeComponent>().movementType : MovementType::MovementType_Ground;

			//Terrain or a body in the way stops the whole push
			if (!SolidEdgeIsClear(tileMap, solid, candidateX, candidateY, deltaX, deltaY, solidMovementType)
				|| IsBlockedByBody(registry, solid, candidateX, candidateY, alongX)) {
				return false;
			}

			std::vector<Entity> nextBlockers;
			CollectSolidBlockers(registry, solid, candidateX, candidateY, nextBlockers, alongX);

			for (const auto& next : nextBlockers) {
				if (!CanMoveSolid(registry, tileMap, next, deltaX, deltaY, alongX, visited)) {
					return false;
				}
			}
			return true;
		}


		//Moves whole row if CanMoveSolid allows it
		static void MoveSolid(Registry& registry,const TileMap& tileMap, Entity solid,
			double deltaX, double deltaY, bool alongX, std::vector<Entity>& visited, double deltaTime) {
			
			if (AlreadyVisited(visited, solid)) {
				return;
			}

			visited.push_back(solid);

			auto& solidTransform = solid.GetComponent<TransformComponent>();

			const double candidateX = solidTransform.position.x + deltaX;
			const double candidateY = solidTransform.position.y + deltaY;

			std::vector<Entity> nextBlockers;
			CollectSolidBlockers(registry, solid, candidateX, candidateY, nextBlockers, alongX);

			for (const auto& next : nextBlockers) {
				MoveSolid(registry, tileMap, next, deltaX, deltaY, alongX, visited, deltaTime);
			}

			solidTransform.position.x += static_cast<float>(deltaX);
			solidTransform.position.y += static_cast<float>(deltaY);
			GiveGlide(tileMap, solid, deltaX, deltaY, deltaTime);
		}



		static double RowMass(Registry& registry, Entity solid, double deltaX, double deltaY, bool alongX, std::vector<Entity>& visited) {
			if (AlreadyVisited(visited, solid)) {
				return 0.0;
			}

			visited.push_back(solid);

			//Past the limit - CanMoveSolid will refuse this push, so stop walking
			if (static_cast<int>(visited.size()) > maxPushCount) {
				return 0.0;
			}
			

			double totalMass = solid.HasComponent<RigidBodyComponent>() ? solid.GetComponent<RigidBodyComponent>().mass : 0.0;
			const auto& solidTransform = solid.GetComponent<TransformComponent>();

			const double candidateX = solidTransform.position.x + deltaX;
			const double candidateY = solidTransform.position.y + deltaY;
			
			std::vector<Entity> nextBlockers;
			CollectSolidBlockers(registry, solid, candidateX, candidateY, nextBlockers, alongX);

			for (const auto& next : nextBlockers) {
				totalMass += RowMass(registry, next, deltaX, deltaY, alongX, visited);
			}
			return totalMass;
		}



		static void GiveGlide(const TileMap& tileMap, Entity solid, double deltaX, double deltaY, double deltaTime) {
			if (deltaTime <= 0.0) {
				return;
			}

			if (!solid.HasComponent<RigidBodyComponent>() || !solid.HasComponent<SpriteComponent>()) {
				return;
			}


			const auto& solidTransform = solid.GetComponent<TransformComponent>();
			const auto& solidSprite = solid.GetComponent<SpriteComponent>();

			const glm::vec2 solidCenter = EntityCenter(solidTransform, solidSprite);
			
			const int solidRow = tileMap.rowAt(solidCenter.y);
			const int solidCol = tileMap.colAt(solidCenter.x);

			if (tileMap.AccelerationAt(solidCol, solidRow) <= 0.0) {
				return;
			}

			
			auto& solidRigidBody = solid.GetComponent<RigidBodyComponent>();

			if (std::abs(deltaX) > std::abs(deltaY)) {
				const float pushed = static_cast<float>(deltaX / deltaTime);
					
				if (pushed * solidRigidBody.actualVelocity.x < 0.0f 
					|| std::abs(pushed) > std::abs(solidRigidBody.actualVelocity.x)) {
					solidRigidBody.actualVelocity.x = pushed;
				}
			}

			else {
				const float pushed = static_cast<float>(deltaY / deltaTime);

				if (pushed * solidRigidBody.actualVelocity.y < 0.0f
					|| std::abs(pushed) > std::abs(solidRigidBody.actualVelocity.y)) {
						solidRigidBody.actualVelocity.y = pushed;
				}
			}
			

		}
};

#endif // !PUSHRESOLVER_H
