/*
 * Based on the LearnOpenGL "Skeletal Animation" chapter
 * (https://learnopengl.com/Guest-Articles/2020/Skeletal-Animation,
 *  https://github.com/JoeyDeVries/LearnOpenGL)
 * Copyright (c) Joey de Vries and LearnOpenGL contributors.
 * Licensed under CC BY-NC 4.0 (https://creativecommons.org/licenses/by-nc/4.0/),
 * see third_party_licenses/CC-BY-NC-4.0.txt
 *
 * Modified by nettory0, 2026: ported to Vulkan, renamed types, integrated into the engine.
 */
#pragma once
#include "en_animation_structs.h"


namespace en_io
{
void loadModel(std::string const& path, en_model::CModel& model, std::unordered_map<std::string, en_model::SBoneInfo>& aBoneInfoMap, int& boneCount);
void loadModel2(std::string const& path, en_model::CModel& model, int& boneCount);
void loadAnimation(const std::string& animationPath, std::unordered_map<std::string, en_model::SBoneInfo>& boneInfoMap, int& boneCount, en_model::CAnimator& animator);
}