#pragma once
#include "en_animation_structs.h"


namespace en_io
{
void loadModel(std::string const& path, en_model::CModel& model, std::unordered_map<std::string, en_model::SBoneInfo>& aBoneInfoMap, int& boneCount);
void loadModel2(std::string const& path, en_model::CModel& model, int& boneCount);
void loadAnimation(const std::string& animationPath, std::unordered_map<std::string, en_model::SBoneInfo>& boneInfoMap, int& boneCount, en_model::CAnimator& animator);
}