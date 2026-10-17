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
#include "en_animation_load.h"
#include <format>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/quaternion.h>
#include <assimp/vector3.h>
#include <assimp/matrix4x4.h>
#include "co_geom_primitives.h"
#include "co_logging.h"


namespace en_io
{

class AssimpGLMHelpers
{
public:
	static inline glm::mat4 convertMatrixToGLMFormat(const aiMatrix4x4& from);
	static inline glm::vec3 getGLMVec(const aiVector3D& vec)
	{
		return glm::vec3(vec.x, vec.y, vec.z);
	}
	static inline glm::vec4 getGLMColor(const aiColor4D& color)
	{
		return glm::vec4(color.r, color.g, color.b, color.a);
	}
	static inline glm::quat getGLMQuat(const aiQuaternion& pOrientation)
	{
		return glm::quat(pOrientation.w, pOrientation.x, pOrientation.y, pOrientation.z);
	}
};

inline glm::mat4 AssimpGLMHelpers::convertMatrixToGLMFormat(const aiMatrix4x4& from)
{
	glm::mat4 to;
	for(int i{0}; i < 4; i++)
	{
		for(int j{0}; j < 4; j++)
			to[i][j] = from[j][i];
	}
	return to;
}

en_model::CMeshAnimated processMeshAnimated(aiMesh* mesh, int& boneCount, std::unordered_map<std::string, en_model::SBoneInfo>& boneInfoMap);
en_model::CMeshStatic processMeshStatic(aiMesh* mesh, int& boneCount);


void processNode(aiNode* node, const aiScene* scene, en_model::CNodeMesh& nodeMesh);
void extractBoneWeightForVertices(std::vector<en_model::SVertexBone>& vertices, aiMesh* mesh, int& boneCount, std::unordered_map<std::string, en_model::SBoneInfo>& boneInfoMap);

void readMissingBones(const aiAnimation* animation, std::unordered_map<std::string, en_model::SBoneInfo>& aBone, int& boneCount, en_model::CAnimation& anim);
void readHierarchyData(en_model::SAssimpNodeData& dest, const aiNode* src, en_model::CAnimation& anim);
en_model::CAnimation createAnimation(const aiAnimation* animation, std::unordered_map<std::string, en_model::SBoneInfo>& boneInfoMap, int& boneCount, const aiMatrix4x4& globalTransformationIn, const aiNode* root);
en_model::CBone createBone(const std::string& name, int ID, const aiNodeAnim* channel);
void setVertexBoneDataToDefault(en_model::SVertexBone& vertex);
void setVertexBoneData(en_model::SVertexBone& vertex, int boneID, float weight);

void loadAnimation(const std::string& animationPath, std::unordered_map<std::string, en_model::SBoneInfo>& boneInfoMap, int& boneCount, en_model::CAnimator& animator)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
	assert(scene && scene->mRootNode);
	std::string strRun{"Run"};

	for(int t{0}; t < scene->mNumAnimations; t++)
	{
		auto* an = scene->mAnimations[t];
		animator.m_aAnimation.push_back(createAnimation(an, boneInfoMap, boneCount, scene->mRootNode->mTransformation, scene->mRootNode));
		std::string strAn{scene->mAnimations[t]->mName.C_Str()};
		common::logInfo(std::format("animation name {} {}", strAn.c_str(), strRun.c_str()));
	}

	if(animator.m_aAnimation.size() == 0)
	{
		animator.m_activeAnimation = -1;
		animator.m_currentTime = 0;
	}

	animator.m_finalBoneMatrices.reserve(cMaxBones);

	for(int i = 0; i < cMaxBones; i++)
		animator.m_finalBoneMatrices.push_back(glm::mat4(1.0f));
}

void processMeshAnimated(const aiScene* scene, std::vector<std::unique_ptr<en_model::CMeshBase>>& aMeshes, int& boneCount, std::unordered_map<std::string, en_model::SBoneInfo>& boneInfoMap)
{
	aMeshes.reserve(scene->mNumMeshes);
	for(unsigned int i = 0; i < scene->mNumMeshes; i++)
	{
		// the node object only contains indices to index the actual objects in the scene. 
		// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
		aiMesh* mesh = scene->mMeshes[i];
		aMeshes.push_back(std::make_unique<en_model::CMeshAnimated>(processMeshAnimated(mesh, boneCount, boneInfoMap)));
	}
}

void processMeshStatic(const aiScene* scene, std::vector<std::unique_ptr<en_model::CMeshBase>>& aMeshes, int& boneCount)
{
	aMeshes.reserve(scene->mNumMeshes);
	for(unsigned int i = 0; i < scene->mNumMeshes; i++)
	{
		// the node object only contains indices to index the actual objects in the scene. 
		// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
		aiMesh* mesh = scene->mMeshes[i];
		aMeshes.push_back(std::make_unique<en_model::CMeshStatic>(processMeshStatic(mesh, boneCount)));
	}
}

void loadModel(std::string const& path, en_model::CModel& model, std::unordered_map<std::string, en_model::SBoneInfo>& aBoneInfoMap, int& boneCount)
{
	// read file via ASSIMP
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);
	// check for errors
	if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
	{
		common::logError(std::format("ERROR::ASSIMP:: {}", importer.GetErrorString()));
		return;
	}

	processMeshAnimated(scene, model.m_aMeshes, boneCount, aBoneInfoMap);

	// process ASSIMP's root node recursively
	processNode(scene->mRootNode, scene, *model.m_pRoot.get());
	en_model::traverse(model.m_pRoot.get());
	if(scene->HasMaterials())
	{
		for(int t{0}; t < scene->mNumMaterials; t++)
		{
			model.m_aMaterial.push_back({});
			auto& newMat = model.m_aMaterial.back();
			aiMaterial* material = scene->mMaterials[t];
			aiColor3D color(0.f, 0.f, 0.f);
			material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
			newMat.diffuse = glm::vec4(color.r, color.g, color.b, 1.);

			material->Get(AI_MATKEY_COLOR_AMBIENT, color);
			newMat.ambient = glm::vec4(color.r, color.g, color.b, 1.);

			material->Get(AI_MATKEY_COLOR_SPECULAR, color);
			newMat.specular = glm::vec4(color.r, color.g, color.b, 1.);

			material->Get(AI_MATKEY_COLOR_EMISSIVE, color);
			newMat.emission = glm::vec4(color.r, color.g, color.b, 1.);
			aiString textPath;
			if(scene->HasTextures() && material->GetTexture(aiTextureType_DIFFUSE, 0, &textPath) == AI_SUCCESS)
			{
				std::string textP(textPath.C_Str());
				if(textP[0] == '*')
				{
					common::logInfo(textP);
					auto ind = std::stoi(textP.substr(1));
					newMat.textureIndex = ind;
				}
			}
		}
	}
	else
	{
		common::logInfo("No material");
	}

	if(scene->HasTextures())
	{
		common::logInfo(std::format("textures {}", scene->mNumTextures));
		for(int t{0}; t < scene->mNumTextures; t++)
		{
			auto* pTexture = scene->mTextures[t];
			model.m_aTexture.push_back({});
			auto& newTexture = model.m_aTexture.back();
			if(pTexture->mHeight == 0)
			{
				common::logInfo(" compressed");
				newTexture.m_data = en_vk_utils::loadTextureFromMemory(newTexture.w, newTexture.h, newTexture.texChannels, (unsigned char*)pTexture->pcData, pTexture->mWidth);
			}
			else
			{
				common::logInfo(" uncompressed");
			}

		}
	}
}

void loadModel2(std::string const& path, en_model::CModel& model, int& boneCount)
{
	// read file via ASSIMP
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);
	// check for errors
	if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
	{
		common::logError(std::format("ERROR::ASSIMP:: {}", importer.GetErrorString()));
		return;
	}

	processMeshStatic(scene, model.m_aMeshes, boneCount);

	// process ASSIMP's root node recursively
	processNode(scene->mRootNode, scene, *model.m_pRoot.get());
	en_model::traverse(model.m_pRoot.get());
	if(scene->HasMaterials())
	{
		for(int t{0}; t < scene->mNumMaterials; t++)
		{
			model.m_aMaterial.push_back({});
			auto& newMat = model.m_aMaterial.back();
			aiMaterial* material = scene->mMaterials[t];
			aiColor3D color(0.f, 0.f, 0.f);
			material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
			newMat.diffuse = glm::vec4(color.r, color.g, color.b, 1.);

			material->Get(AI_MATKEY_COLOR_AMBIENT, color);
			newMat.ambient = glm::vec4(color.r, color.g, color.b, 1.);

			material->Get(AI_MATKEY_COLOR_SPECULAR, color);
			newMat.specular = glm::vec4(color.r, color.g, color.b, 1.);

			material->Get(AI_MATKEY_COLOR_EMISSIVE, color);
			newMat.emission = glm::vec4(color.r, color.g, color.b, 1.);
			aiString textPath;

			if(scene->HasTextures() && material->GetTexture(aiTextureType_DIFFUSE, 0, &textPath) == AI_SUCCESS)
			{
				std::string textP(textPath.C_Str());
				if(textP[0] == '*')
				{
					common::logInfo(textP);
					auto ind = std::stoi(textP.substr(1));
					newMat.textureIndex = ind;
				}
			}
		}
	}
	else
	{
		common::logInfo("No material\n");
	}

	if(scene->HasTextures())
	{
		common::logInfo(std::format("textures {}", scene->mNumTextures));
		for(int t{0}; t < scene->mNumTextures; t++)
		{
			auto* pTexture = scene->mTextures[t];
			model.m_aTexture.push_back({});
			auto& newTexture = model.m_aTexture.back();
			if(pTexture->mHeight == 0)
			{
				common::logInfo(" compressed \n");
				newTexture.m_data = en_vk_utils::loadTextureFromMemory(newTexture.w, newTexture.h, newTexture.texChannels, (unsigned char*)pTexture->pcData, pTexture->mWidth);
			}
			else
			{
				common::logInfo(" uncompressed \n");
			}

		}
	}
}

void processNode(aiNode* node, const aiScene* scene, en_model::CNodeMesh& nodeMesh)
{
	for(unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		nodeMesh.m_aChild.push_back(std::make_unique<en_model::CNodeMesh>());
		nodeMesh.m_aChild.back()->m_meshId = node->mMeshes[i];
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
	}

	for(unsigned int i = 0; i < node->mNumChildren; i++)
	{
		nodeMesh.m_aChild.push_back(std::make_unique<en_model::CNodeMesh>());
		processNode(node->mChildren[i], scene, *nodeMesh.m_aChild.back().get());
	}
}

void setVertexBoneDataToDefault(en_model::SVertexBone& vertex)
{
	for(int i = 0; i < MAX_BONE_INFLUENCE; i++)
	{
		vertex.m_boneIDs[i] = -1;
		vertex.m_weights[i] = 0.0f;
	}
}

en_model::CMeshAnimated processMeshAnimated(aiMesh* mesh, int& boneCount, std::unordered_map<std::string, en_model::SBoneInfo>& boneInfoMap)
{
	std::vector<en_model::SVertexBone> vertices;
	std::vector<unsigned int> indices;
	vertices.reserve(mesh->mNumVertices);
	indices.reserve(mesh->mNumFaces);
	for(unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		en_model::SVertexBone vertex;
		setVertexBoneDataToDefault(vertex);
		vertex.pos = AssimpGLMHelpers::getGLMVec(mesh->mVertices[i]);

		if(mesh->HasNormals())
			vertex.normal = AssimpGLMHelpers::getGLMVec(mesh->mNormals[i]);
		else
			vertex.normal = glm::vec3(0.0f);
		if(mesh->HasVertexColors(0))
			vertex.color = AssimpGLMHelpers::getGLMColor(mesh->mColors[0][i]);
		else
			vertex.color = glm::vec4(1);
		if(mesh->mTextureCoords[0])
		{
			vertex.tex.x = mesh->mTextureCoords[0][i].x;
			vertex.tex.y = mesh->mTextureCoords[0][i].y;
		}
		else
			vertex.tex = glm::vec2(0.0f, 0.0f);

		vertices.push_back(vertex);
	}


	for(unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace& face = mesh->mFaces[i];
		for(unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	extractBoneWeightForVertices(vertices, mesh, boneCount, boneInfoMap);
	return en_model::CMeshAnimated(vertices, indices, mesh->mMaterialIndex);
}

en_model::CMeshStatic processMeshStatic(aiMesh* mesh, int& boneCount)
{
	std::vector<commons_struct::SVertex> vertices;
	std::vector<unsigned int> indices;
	vertices.reserve(mesh->mNumVertices);
	indices.reserve(mesh->mNumFaces);
	for(unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		commons_struct::SVertex vertex;
		//setVertexBoneDataToDefault(vertex);
		vertex.pos = glm::vec4{AssimpGLMHelpers::getGLMVec(mesh->mVertices[i]), 0};

		if(mesh->HasNormals())
			vertex.normal = glm::vec4{AssimpGLMHelpers::getGLMVec(mesh->mNormals[i]), 0};
		else
			vertex.normal = glm::vec4(0.0f);
		if(mesh->HasVertexColors(0))
			vertex.color = AssimpGLMHelpers::getGLMColor(mesh->mColors[0][i]);
		else
			vertex.color = glm::vec4(1);
		if(mesh->mTextureCoords[0])
		{
			vertex.texCoord.x = mesh->mTextureCoords[0][i].x;
			vertex.texCoord.y = mesh->mTextureCoords[0][i].y;
		}
		else
			vertex.texCoord = glm::vec2(0.0f, 0.0f);

		vertices.push_back(vertex);
	}


	for(unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace& face = mesh->mFaces[i];
		for(unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	return en_model::CMeshStatic(vertices, indices, mesh->mMaterialIndex);
}

void setVertexBoneData(en_model::SVertexBone& vertex, int boneID, float weight)
{
	for(int i = 0; i < MAX_BONE_INFLUENCE; ++i)
	{
		if(vertex.m_boneIDs[i] < 0)
		{
			vertex.m_weights[i] = weight;
			vertex.m_boneIDs[i] = boneID;
			return;
		}
	}

	int minIndex = 0;
	for(int i = 1; i < MAX_BONE_INFLUENCE; ++i)
		if(vertex.m_weights[i] < vertex.m_weights[minIndex])
			minIndex = i;

	if(vertex.m_weights[minIndex] < weight)
	{
		vertex.m_weights[minIndex] = weight;
		vertex.m_boneIDs[minIndex] = boneID;
	}

}

void extractBoneWeightForVertices(std::vector<en_model::SVertexBone>& vertices, aiMesh* mesh, int& boneCount, std::unordered_map<std::string, en_model::SBoneInfo>& boneInfoMap)
{
	//auto& boneInfoMap = model.m_boneInfoMap;
	//int& boneCount = model.m_boneCounter;

	for(int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
	{
		auto* pBone = mesh->mBones[boneIndex];
		int boneID = -1;
		std::string boneName = pBone->mName.C_Str();
		if(boneInfoMap.find(boneName) == boneInfoMap.end())
		{
			en_model::SBoneInfo newBoneInfo;
			newBoneInfo.id = boneCount;
			newBoneInfo.offset = AssimpGLMHelpers::convertMatrixToGLMFormat(pBone->mOffsetMatrix);
			boneInfoMap[boneName] = newBoneInfo;
			boneID = boneCount;
			boneCount++;
		}
		else
		{
			boneID = boneInfoMap[boneName].id;
		}
		assert(boneID != -1);
		auto weights = pBone->mWeights;
		int numWeights = pBone->mNumWeights;
		for(int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
		{
			int vertexId = weights[weightIndex].mVertexId;
			float weight = weights[weightIndex].mWeight;
			assert(vertexId < vertices.size());
			setVertexBoneData(vertices[vertexId], boneID, weight);
		}
	}
}


en_model::CAnimation createAnimation(const aiAnimation* animation, std::unordered_map<std::string, en_model::SBoneInfo>& boneInfoMap, int& boneCount, const aiMatrix4x4& globalTransformationIn, const aiNode* root)
{
	//Assimp::Importer importer;
	//const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
	//assert(scene && scene->mRootNode);

	en_model::CAnimation anim;

	if(animation)
	{
		std::string strAn{animation->mName.C_Str()};
		anim.m_animationName = strAn;
		
		anim.m_duration = animation->mDuration;
		anim.m_ticksPerSecond = animation->mTicksPerSecond;
		aiMatrix4x4 globalTransformation = globalTransformationIn;// ;
		globalTransformation = globalTransformation.Inverse();
		readHierarchyData(anim.m_rootNode, root, anim);
		readMissingBones(animation, boneInfoMap, boneCount, anim);
	}
	return anim;
}

void readMissingBones(const aiAnimation* animation, std::unordered_map<std::string, en_model::SBoneInfo>& boneInfoMap, int& boneCount, en_model::CAnimation& anim)
{
	int size = animation->mNumChannels;

	//reading channels(bones engaged in an animation and their keyframes)
	for(int i = 0; i < size; i++)
	{
		auto channel = animation->mChannels[i];
		std::string boneName = channel->mNodeName.data;

		if(boneInfoMap.find(boneName) == boneInfoMap.end())
		{
			boneInfoMap[boneName].id = boneCount;
			boneCount++;
		}
		auto b = createBone(channel->mNodeName.data, boneInfoMap[channel->mNodeName.data].id, channel);
		b.m_offset = boneInfoMap[channel->mNodeName.data].offset;
		anim.m_bones[b.getBoneName()] = b;
	}

	//m_BoneInfoMap = boneInfoMap;
}

void readHierarchyData(en_model::SAssimpNodeData& dest, const aiNode* src, en_model::CAnimation& anim)
{
	assert(src);

	dest.m_name = src->mName.data;
	dest.m_transformation = AssimpGLMHelpers::convertMatrixToGLMFormat(src->mTransformation);

	for(int i = 0; i < src->mNumChildren; i++)
	{
		en_model::SAssimpNodeData newData;
		readHierarchyData(newData, src->mChildren[i], anim);
		dest.m_children.push_back(newData);
	}
}

en_model::CBone createBone(const std::string& name, int ID, const aiNodeAnim* channel)
{
	en_model::CBone newBone;
	newBone.m_name = (name);
	newBone.m_id = (ID);
	newBone.m_localTransform = glm::mat4(1.0f);
	for(int positionIndex = 0; positionIndex < channel->mNumPositionKeys; ++positionIndex)
	{
		aiVector3D aiPosition = channel->mPositionKeys[positionIndex].mValue;
		float timeStamp = channel->mPositionKeys[positionIndex].mTime;
		en_model::SKeyPosition data;
		data.position = AssimpGLMHelpers::getGLMVec(aiPosition);
		data.timeStamp = timeStamp;
		newBone.m_positions.push_back(data);
	}

	for(int rotationIndex = 0; rotationIndex < channel->mNumRotationKeys; ++rotationIndex)
	{
		aiQuaternion aiOrientation = channel->mRotationKeys[rotationIndex].mValue;
		float timeStamp = channel->mRotationKeys[rotationIndex].mTime;
		en_model::SKeyRotation data;
		data.orientation = AssimpGLMHelpers::getGLMQuat(aiOrientation);
		data.timeStamp = timeStamp;
		newBone.m_rotations.push_back(data);
	}

	for(int keyIndex = 0; keyIndex < channel->mNumScalingKeys; ++keyIndex)
	{
		aiVector3D scale = channel->mScalingKeys[keyIndex].mValue;
		float timeStamp = channel->mScalingKeys[keyIndex].mTime;
		en_model::SKeyScale data;
		data.scale = AssimpGLMHelpers::getGLMVec(scale);
		data.timeStamp = timeStamp;
		newBone.m_scales.push_back(data);
	}
	return newBone;
}
}