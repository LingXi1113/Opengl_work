#ifndef LEARNOPENGL_MODEL_ANIMATION_H
#define LEARNOPENGL_MODEL_ANIMATION_H

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <learnopengl/mesh.h>
#include <learnopengl/shader.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <filesystem>
#include <learnopengl/assimp_glm_helpers.h>
#include <learnopengl/animdata.h>
#include <assimp/material.h>

using namespace std;

class SkinnedModel 
{
public:
    // model data 
    vector<Texture> textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
    vector<Mesh>    meshes;
    string directory;
    bool gammaCorrection;
	
	

	// constructor, expects a filepath to a 3D model.
	// preTransformVertices=true bakes node transforms into vertex positions (useful for multi-object scene exports).
	SkinnedModel(string const &path, bool gamma = false, bool preTransformVertices = false)
		: gammaCorrection(gamma), m_PreTransformVertices(preTransformVertices)
	{
		loadModel(path);
	}

	// draws the model, and thus all its meshes
	void Draw(Shader &shader, int textureUnitOffset = 0)
    {
        for(unsigned int i = 0; i < meshes.size(); i++)
			meshes[i].Draw(shader, textureUnitOffset);
    }
    
	auto& GetBoneInfoMap() { return m_BoneInfoMap; }
	int& GetBoneCount() { return m_BoneCounter; }
	

private:

	std::map<string, BoneInfo> m_BoneInfoMap;
	int m_BoneCounter = 0;
	bool m_PreTransformVertices = false;

    // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
    void loadModel(string const &path)
    {
		// read file via ASSIMP
		Assimp::Importer importer;
		unsigned int flags = 0;
		if (m_PreTransformVertices)
		{
			// Static scene export: keep post-process light to avoid very slow/hung imports on some FBX.
			flags = aiProcess_Triangulate |
				aiProcess_PreTransformVertices |
				aiProcess_GenNormals |
				aiProcess_JoinIdenticalVertices |
				aiProcess_SortByPType;
		}
		else
		{
			// Skinned/animated models: keep original quality flags.
			flags = aiProcess_Triangulate |
				aiProcess_GenSmoothNormals |
				aiProcess_CalcTangentSpace |
				aiProcess_LimitBoneWeights;
		}
		const aiScene* scene = importer.ReadFile(path, flags);
		// check for errors
		if(!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode)
		{
			cout << "ERROR::ASSIMP::MODEL load failed: " << importer.GetErrorString() << endl;
			cout << "  path='" << path << "'" << endl;
			return;
		}

		cout << "ASSIMP::MODEL loaded ok: meshes=" << scene->mNumMeshes
			 << " animations=" << scene->mNumAnimations << endl;
		// retrieve the directory path of the filepath (support Windows and POSIX separators)
		size_t lastSlash = path.find_last_of("/\\");
		if (lastSlash == std::string::npos)
			directory = ".";
		else
			directory = path.substr(0, lastSlash);

        // process ASSIMP's root node recursively
        processNode(scene->mRootNode, scene);
    }

    // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
    void processNode(aiNode *node, const aiScene *scene)
    {
        // process each mesh located at the current node
        for(unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            // the node object only contains indices to index the actual objects in the scene. 
            // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for(unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }

    }

	static unsigned int CreateSolidTextureRGBA(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255)
	{
		unsigned int textureID;
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		unsigned char pixel[4] = { r, g, b, a };
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
		glGenerateMipmap(GL_TEXTURE_2D);
		return textureID;
	}

	static unsigned int TextureFromMemory(const unsigned char* buffer, int length)
	{
		unsigned int textureID;
		glGenTextures(1, &textureID);

		int width = 0, height = 0, nrComponents = 0;
		unsigned char* data = stbi_load_from_memory(buffer, length, &width, &height, &nrComponents, 0);
		if (!data)
		{
			std::cout << "Texture failed to decode from memory: " << stbi_failure_reason() << std::endl;
			return textureID;
		}

		GLenum format = GL_RGBA;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
		return textureID;
	}

	void SetVertexBoneDataToDefault(Vertex& vertex)
	{
		for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
		{
			vertex.m_BoneIDs[i] = -1;
			vertex.m_Weights[i] = 0.0f;
		}
	}


	Mesh processMesh(aiMesh* mesh, const aiScene* scene)
	{
		vector<Vertex> vertices;
		vector<unsigned int> indices;
		vector<Texture> textures;

		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;
			SetVertexBoneDataToDefault(vertex);
			vertex.Position = AssimpGLMHelpers::GetGLMVec(mesh->mVertices[i]);
			if (mesh->HasNormals() && mesh->mNormals)
				vertex.Normal = AssimpGLMHelpers::GetGLMVec(mesh->mNormals[i]);
			else
				vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
			vertex.Tangent = glm::vec3(0.0f);
			vertex.Bitangent = glm::vec3(0.0f);
			
			if (mesh->mTextureCoords[0])
			{
				glm::vec2 vec;
				vec.x = mesh->mTextureCoords[0][i].x;
				vec.y = mesh->mTextureCoords[0][i].y;
				vertex.TexCoords = vec;
			}
			else
				vertex.TexCoords = glm::vec2(0.0f, 0.0f);

			vertices.push_back(vertex);
		}
		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

		vector<Texture> diffuseMaps = loadMaterialTextures(material, scene, aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		vector<Texture> specularMaps = loadMaterialTextures(material, scene, aiTextureType_SPECULAR, "texture_specular");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
		std::vector<Texture> normalMaps = loadMaterialTextures(material, scene, aiTextureType_HEIGHT, "texture_normal");
		textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
		std::vector<Texture> heightMaps = loadMaterialTextures(material, scene, aiTextureType_AMBIENT, "texture_height");
		textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

		// Fallback: if there is no diffuse texture, use material diffuse color as a 1x1 texture.
		if (diffuseMaps.empty())
		{
			aiColor3D diffuseColor(1.0f, 1.0f, 1.0f);
			if (material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor) == AI_SUCCESS)
			{
				Texture t;
				t.id = CreateSolidTextureRGBA(
					static_cast<unsigned char>(std::max(0.0f, std::min(1.0f, diffuseColor.r)) * 255.0f),
					static_cast<unsigned char>(std::max(0.0f, std::min(1.0f, diffuseColor.g)) * 255.0f),
					static_cast<unsigned char>(std::max(0.0f, std::min(1.0f, diffuseColor.b)) * 255.0f),
					255);
				t.type = "texture_diffuse";
				t.path = "__material_diffuse_color__";
				textures.push_back(t);
			}
		}

		ExtractBoneWeightForVertices(vertices,mesh,scene);

		return Mesh(vertices, indices, textures);
	}

	void SetVertexBoneData(Vertex& vertex, int boneID, float weight)
	{
		for (int i = 0; i < MAX_BONE_INFLUENCE; ++i)
		{
			if (vertex.m_BoneIDs[i] < 0)
			{
				vertex.m_Weights[i] = weight;
				vertex.m_BoneIDs[i] = boneID;
				break;
			}
		}
	}


	void ExtractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene)
	{
		auto& boneInfoMap = m_BoneInfoMap;
		int& boneCount = m_BoneCounter;

		for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
		{
			int boneID = -1;
			std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
			if (boneInfoMap.find(boneName) == boneInfoMap.end())
			{
				BoneInfo newBoneInfo;
				newBoneInfo.id = boneCount;
				newBoneInfo.offset = AssimpGLMHelpers::ConvertMatrixToGLMFormat(mesh->mBones[boneIndex]->mOffsetMatrix);
				boneInfoMap[boneName] = newBoneInfo;
				boneID = boneCount;
				boneCount++;
			}
			else
			{
				boneID = boneInfoMap[boneName].id;
			}
			assert(boneID != -1);
			auto weights = mesh->mBones[boneIndex]->mWeights;
			int numWeights = mesh->mBones[boneIndex]->mNumWeights;

			for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
			{
				int vertexId = weights[weightIndex].mVertexId;
				float weight = weights[weightIndex].mWeight;
				assert(static_cast<size_t>(vertexId) < vertices.size());
				SetVertexBoneData(vertices[vertexId], boneID, weight);
			}
		}
	}

	static std::string JoinPath(const std::string& baseDir, const std::string& rel)
	{
		if (rel.empty()) return baseDir;
		// absolute Windows path like C:\\...
		if (rel.size() > 1 && rel[1] == ':') return rel;
		// absolute POSIX or root-relative
		if (!rel.empty() && (rel[0] == '/' || rel[0] == '\\')) return rel;
		if (baseDir.empty() || baseDir == ".") return rel;

		char sep = (baseDir.find('\\') != std::string::npos) ? '\\' : '/';
		if (baseDir.back() == '/' || baseDir.back() == '\\')
			return baseDir + rel;
		return baseDir + sep + rel;
	}

	static std::string ResolveTexturePath(const std::string& directory, const std::string& requested)
	{
		namespace fs = std::filesystem;
		// First try the original join behavior.
		std::string candidate = JoinPath(directory, requested);
		if (!candidate.empty() && fs::exists(fs::path(candidate)))
			return candidate;

		// If it's an absolute path from the export machine, fall back to basename in the model directory.
		fs::path reqPath(requested);
		fs::path baseDir(directory);
		if (!reqPath.filename().empty())
		{
			const fs::path filenameOnly = reqPath.filename();
			const fs::path direct = baseDir / filenameOnly;
			if (fs::exists(direct))
				return direct.string();

			// Common texture folder names from asset packs.
			const fs::path t1 = baseDir / "textures" / filenameOnly;
			if (fs::exists(t1))
				return t1.string();
			const fs::path t2 = baseDir / "textures_2k" / filenameOnly;
			if (fs::exists(t2))
				return t2.string();
			const fs::path t3 = baseDir / "Textures" / filenameOnly;
			if (fs::exists(t3))
				return t3.string();

			// Last resort: recursive search within the model directory (depth-limited).
			try
			{
				if (fs::exists(baseDir) && fs::is_directory(baseDir))
				{
					for (fs::recursive_directory_iterator it(baseDir), end; it != end; ++it)
					{
						if (it.depth() > 4)
						{
							it.disable_recursion_pending();
							continue;
						}
						if (!it->is_regular_file())
							continue;
						if (it->path().filename() == filenameOnly)
							return it->path().string();
					}
				}
			}
			catch (...) {}
		}

		return candidate;
	}


	unsigned int TextureFromFile(const char* path, const string& directory, bool gamma = false)
	{
		string filename = ResolveTexturePath(directory, string(path));

		unsigned int textureID;
		glGenTextures(1, &textureID);

		int width, height, nrComponents;
		unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
		if (data)
		{
			GLenum format = GL_RGBA;
			if (nrComponents == 1)
				format = GL_RED;
			else if (nrComponents == 2)
				format = GL_RG;
			else if (nrComponents == 3)
				format = GL_RGB;
			else if (nrComponents == 4)
				format = GL_RGBA;

			glBindTexture(GL_TEXTURE_2D, textureID);
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			stbi_image_free(data);
		}
		else
		{
			std::cout << "Texture failed to load. requested='" << path << "' resolved='" << filename << "' baseDir='" << directory << "'" << std::endl;
			stbi_image_free(data);
			glDeleteTextures(1, &textureID);
			return CreateSolidTextureRGBA(255, 255, 255, 255);
		}

		return textureID;
	}
    
    // checks all material textures of a given type and loads the textures if they're not loaded yet.
    // the required info is returned as a Texture struct.
    vector<Texture> loadMaterialTextures(aiMaterial *mat, const aiScene* scene, aiTextureType type, string typeName)
    {
        vector<Texture> textures;
        for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);

			// Embedded textures in FBX are referenced as "*0", "*1", ...
			if (scene && str.length > 0 && str.C_Str()[0] == '*')
			{
				int embeddedIndex = atoi(str.C_Str() + 1);
				if (embeddedIndex >= 0 && embeddedIndex < static_cast<int>(scene->mNumTextures))
				{
					const aiTexture* embeddedTex = scene->mTextures[embeddedIndex];
					Texture texture;
					if (embeddedTex && embeddedTex->mHeight == 0)
					{
						// Compressed image data (PNG/JPG/etc.)
						texture.id = TextureFromMemory(reinterpret_cast<const unsigned char*>(embeddedTex->pcData), static_cast<int>(embeddedTex->mWidth));
					}
					else if (embeddedTex && embeddedTex->mWidth > 0 && embeddedTex->mHeight > 0)
					{
						// Uncompressed raw BGRA8888 (aiTexel)
						unsigned int textureID;
						glGenTextures(1, &textureID);
						glBindTexture(GL_TEXTURE_2D, textureID);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

						int w = static_cast<int>(embeddedTex->mWidth);
						int h = static_cast<int>(embeddedTex->mHeight);
						std::vector<unsigned char> rgba(static_cast<size_t>(w) * static_cast<size_t>(h) * 4);
						for (int p = 0; p < w * h; ++p)
						{
							const aiTexel& c = embeddedTex->pcData[p];
							rgba[p * 4 + 0] = c.r;
							rgba[p * 4 + 1] = c.g;
							rgba[p * 4 + 2] = c.b;
							rgba[p * 4 + 3] = c.a;
						}
						glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba.data());
						glGenerateMipmap(GL_TEXTURE_2D);
						texture.id = textureID;
					}
					else
					{
						texture.id = CreateSolidTextureRGBA(255, 255, 255, 255);
					}

					texture.type = typeName;
					texture.path = str.C_Str();
					textures.push_back(texture);
					textures_loaded.push_back(texture);
					continue;
				}
			}

            // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
            bool skip = false;
            for(unsigned int j = 0; j < textures_loaded.size(); j++)
            {
                if(std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
                {
                    textures.push_back(textures_loaded[j]);
                    skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
                    break;
                }
            }
            if(!skip)
            {   // if texture hasn't been loaded already, load it
                Texture texture;
                texture.id = TextureFromFile(str.C_Str(), this->directory);
                texture.type = typeName;
                texture.path = str.C_Str();
                textures.push_back(texture);
                textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecessary load duplicate textures.
            }
        }
        return textures;
    }
};



#endif
