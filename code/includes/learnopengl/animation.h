#pragma once

#include <vector>
#include <map>
#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <learnopengl/bone.h>
#include <functional>
#include <learnopengl/animdata.h>
#include <learnopengl/model_animation.h>

struct AssimpNodeData
{
	glm::mat4 transformation;
	std::string name;
	int childrenCount;
	std::vector<AssimpNodeData> children;
};

class Animation
{
public:
	Animation() = default;

	Animation(const std::string& animationPath, SkinnedModel* model)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
		if (!scene || !scene->mRootNode)
		{
			std::cout << "ERROR::ASSIMP::Animation load failed: " << importer.GetErrorString() << std::endl;
			std::cout << "  path='" << animationPath << "'" << std::endl;
			m_Duration = 0.0f;
			m_TicksPerSecond = 0.0f;
			m_BoneInfoMap = model ? model->GetBoneInfoMap() : std::map<std::string, BoneInfo>{};
			m_RootNode = AssimpNodeData{};
			m_RootNode.transformation = glm::mat4(1.0f);
			m_RootNode.name = "";
			m_RootNode.childrenCount = 0;
			return;
		}
		if (scene->mNumAnimations == 0)
		{
			m_Duration = 0.0f;
			m_TicksPerSecond = 0.0f;
			ReadHierarchyData(m_RootNode, scene->mRootNode);
			m_BoneInfoMap = model ? model->GetBoneInfoMap() : std::map<std::string, BoneInfo>{};
			return;
		}
		auto animation = scene->mAnimations[0];
		if (!animation)
		{
			std::cout << "ERROR::ASSIMP::Animation is null for path='" << animationPath << "'" << std::endl;
			m_Duration = 0.0f;
			m_TicksPerSecond = 0.0f;
			ReadHierarchyData(m_RootNode, scene->mRootNode);
			m_BoneInfoMap = model ? model->GetBoneInfoMap() : std::map<std::string, BoneInfo>{};
			return;
		}
		m_TicksPerSecond = (animation->mTicksPerSecond != 0.0) ? static_cast<float>(animation->mTicksPerSecond) : 25.0f;
		m_Duration = static_cast<float>(animation->mDuration);
		if (m_Duration <= 0.0f)
		{
			// Some FBX files report 0 duration; approximate it using the max key timestamp.
			double maxTime = 0.0;
			for (unsigned int ch = 0; ch < animation->mNumChannels; ++ch)
			{
				const aiNodeAnim* channel = animation->mChannels[ch];
				if (!channel) continue;
				if (channel->mNumPositionKeys > 0)
					maxTime = std::max(maxTime, channel->mPositionKeys[channel->mNumPositionKeys - 1].mTime);
				if (channel->mNumRotationKeys > 0)
					maxTime = std::max(maxTime, channel->mRotationKeys[channel->mNumRotationKeys - 1].mTime);
				if (channel->mNumScalingKeys > 0)
					maxTime = std::max(maxTime, channel->mScalingKeys[channel->mNumScalingKeys - 1].mTime);
			}
			m_Duration = static_cast<float>(maxTime);
		}
		aiMatrix4x4 globalTransformation = scene->mRootNode->mTransformation;
		globalTransformation = globalTransformation.Inverse();
		ReadHierarchyData(m_RootNode, scene->mRootNode);
		ReadMissingBones(animation, *model);
	}

	~Animation()
	{
	}

	Bone* FindBone(const std::string& name)
	{
		auto iter = std::find_if(m_Bones.begin(), m_Bones.end(),
			[&](const Bone& Bone)
			{
				return Bone.GetBoneName() == name;
			}
		);
		if (iter == m_Bones.end()) return nullptr;
		else return &(*iter);
	}

	
	inline float GetTicksPerSecond() { return m_TicksPerSecond; }
	inline float GetDuration() { return m_Duration;}
	inline const AssimpNodeData& GetRootNode() { return m_RootNode; }
	inline const std::map<std::string,BoneInfo>& GetBoneIDMap() 
	{ 
		return m_BoneInfoMap;
	}

private:
	void ReadMissingBones(const aiAnimation* animation, SkinnedModel& model)
	{
		if (!animation)
			return;
		int size = animation->mNumChannels;

		auto& boneInfoMap = model.GetBoneInfoMap();//getting m_BoneInfoMap from Model class
		int& boneCount = model.GetBoneCount(); //getting the m_BoneCounter from Model class

		//reading channels(bones engaged in an animation and their keyframes)
		for (int i = 0; i < size; i++)
		{
			auto channel = animation->mChannels[i];
			if (!channel)
				continue;
			std::string boneName = channel->mNodeName.data;
			if (boneName.empty())
				continue;

			if (boneInfoMap.find(boneName) == boneInfoMap.end())
			{
				boneInfoMap[boneName].id = boneCount;
				boneCount++;
			}
			m_Bones.push_back(Bone(channel->mNodeName.data,
				boneInfoMap[channel->mNodeName.data].id, channel));
		}

		m_BoneInfoMap = boneInfoMap;
	}

	void ReadHierarchyData(AssimpNodeData& dest, const aiNode* src)
	{
		assert(src);

		dest.name = src->mName.data;
		dest.transformation = AssimpGLMHelpers::ConvertMatrixToGLMFormat(src->mTransformation);
		dest.childrenCount = src->mNumChildren;

		for (int i = 0; i < src->mNumChildren; i++)
		{
			AssimpNodeData newData;
			ReadHierarchyData(newData, src->mChildren[i]);
			dest.children.push_back(newData);
		}
	}
	float m_Duration;
	float m_TicksPerSecond;
	std::vector<Bone> m_Bones;
	AssimpNodeData m_RootNode;
	std::map<std::string, BoneInfo> m_BoneInfoMap;
};

