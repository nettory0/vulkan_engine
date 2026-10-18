#pragma once
class CSceneComponentSystem;

class CAnimationSystem
{
public:
    explicit CAnimationSystem(CSceneComponentSystem& scene);

    void update(float dt);

private:
    CSceneComponentSystem& m_scene;
};