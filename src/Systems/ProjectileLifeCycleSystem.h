#ifndef PROJECTILELIFECYCLESYSTEM_H
#define PROJECTILELIFECYCLESYSTEM_H

#include "../ECS/ECS.h"
#include "../Components/ProjectileComponent.h"

class ProjectileLifeCycleSystem
{
    public:
        ProjectileLifeCycleSystem() = default;

        void Update(Registry& registry) {
            for (auto rawEntity : registry.Raw().view<ProjectileComponent>()) {
                Entity entity(rawEntity, &registry);
                auto projectile = entity.GetComponent<ProjectileComponent>();

                // Kill projectiles after they reach their duration limit
                if (SDL_GetTicks() - projectile.startTime > projectile.duration) {
                    entity.Kill();
                }
            }
        }
};

#endif
