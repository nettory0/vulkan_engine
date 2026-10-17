#include "en_animation_system.h"
#include "en_scene_component.h"

CAnimationSystem::CAnimationSystem(CSceneComponentSystem& scene): m_scene(scene)
{
}

void CAnimationSystem::update(float dt)
{
	auto list = m_scene.componentManager().getEntitiesWithData<std::unique_ptr<en_model::CAnimator>>();
	for(auto& [id, animator] : list)
	{
		animator->updateAnimation(dt);
	}
}