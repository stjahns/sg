#include <glad/glad.h>

// Standard Headers
#include <map>
#include <iostream>
#include "stb_image.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Model.h"
#include "Asset.h"

#include <assimp/postprocess.h>
#include <tiny_gltf.h>
#include "ShaderProgram.h"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

Model::Model(const char* filename)
	: m_loaded(false)
	, m_bound(false)
	, transform(1.0f)
{
	m_filename = filename;
	m_filename = GetAssetPath(filename);
}

template <typename T>
void CopyBuffer(const tinygltf::Model& gltfSource, int accessorIndex, std::vector<T>& output)
{
    const tinygltf::Accessor& accessor = gltfSource.accessors[accessorIndex];
    const tinygltf::BufferView& bufferView = gltfSource.bufferViews[accessor.bufferView];
    const tinygltf::Buffer& buffer = gltfSource.buffers[bufferView.buffer];

    output.resize(accessor.count);
    memcpy(output.data(), &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(T));
}

void Model::LoadSkin(const tinygltf::Model& gltfSource)
{
	if (gltfSource.skins.empty())
	{
		return;
	}

	tinygltf::Skin skin = gltfSource.skins.front();

	for (int i = 0; i < gltfSource.meshes.size(); ++i)
	{
        std::vector<Vertex>& vertices = m_meshes[i].vertices;

        const int jointsAccessorIndex = gltfSource.meshes[i].primitives[0].attributes.at("JOINTS_0");
        const int weightsAccessorIndex = gltfSource.meshes[i].primitives[0].attributes.at("WEIGHTS_0");

        std::vector<glm::i16vec4> skinIndices;
        CopyBuffer(gltfSource, jointsAccessorIndex, skinIndices);

        std::vector<glm::vec4> skinWeights;
        CopyBuffer(gltfSource, weightsAccessorIndex, skinWeights);

        for (int vertexIndex = 0; vertexIndex < vertices.size(); ++vertexIndex)
        {
            Vertex& vertex = vertices[vertexIndex];
			vertex.skinIndex = skinIndices[vertexIndex];
            vertex.skinWeight = skinWeights[vertexIndex];
        }
	}
}

void LoadBoneWeights(const aiMesh* pMesh, std::vector<Vertex>& vertices)
{
    if (pMesh->HasBones())
    {
        for (int boneIndex = 0; boneIndex < pMesh->mNumBones; ++boneIndex)
        {
            const aiBone& bone = *pMesh->mBones[boneIndex];

            for (int weightIndex = 0; weightIndex < bone.mNumWeights; ++weightIndex)
            {
				const aiVertexWeight& weight = bone.mWeights[weightIndex];
				if (weight.mWeight > 0 && weight.mVertexId < vertices.size())
				{
					Vertex& vertex = vertices[weight.mVertexId];
					for (int i = 0; i < 4; ++i)
					{
						if (vertex.skinWeight[i] == 0)
						{
							vertex.skinIndex[i] = boneIndex;
							vertex.skinWeight[i] = weight.mWeight;
							break;
						}
					}
				}
            }
		}
    }
}

void Model::LoadMesh(const aiMesh* pMesh, float fScale)
{
	Mesh mesh;

	for (int i = 0; i < pMesh->mNumVertices; ++i)
	{
		Vertex v;

		if (pMesh->HasPositions())
		{
			v.position.x = pMesh->mVertices[i].x * fScale;
			v.position.y = pMesh->mVertices[i].y * fScale;
			v.position.z = pMesh->mVertices[i].z * fScale;
		}

		if (pMesh->HasNormals())
		{
			v.normal.x = pMesh->mNormals[i].x;
			v.normal.y = pMesh->mNormals[i].y;
			v.normal.z = pMesh->mNormals[i].z;
		}

		if (pMesh->HasTangentsAndBitangents())
		{
			v.tangent.x = pMesh->mTangents[i].x;
			v.tangent.y = pMesh->mTangents[i].y;
			v.tangent.z = pMesh->mTangents[i].z;
			v.bitangent.x = pMesh->mBitangents[i].x;
			v.bitangent.y = pMesh->mBitangents[i].y;
			v.bitangent.z = pMesh->mBitangents[i].z;
		}

		// TODO -- support multiple texcoord sets
		if (pMesh->HasTextureCoords(0))
		{
			v.texcoord.x = pMesh->mTextureCoords[0][i].x;
			v.texcoord.y = pMesh->mTextureCoords[0][i].y;
		}

		mesh.vertices.push_back(v);
	}

	LoadBoneWeights(pMesh, mesh.vertices);

	for (int i = 0; i < pMesh->mNumFaces; ++i)
	{
		const aiFace& face = pMesh->mFaces[i];

		// Faces should have been converted to triangles
		assert(face.mNumIndices == 3); 

		// TODO can we just memcpy?

		for (int j = 0; j < face.mNumIndices; ++j)
		{
			mesh.indices.push_back(face.mIndices[j]);
		}
	}

	mesh.material = pMesh->mMaterialIndex;

	m_meshes.push_back(mesh);
}


void Model::BindMesh(Mesh& mesh)
{
	glGenVertexArrays(1, &mesh.vao);
	glBindVertexArray(mesh.vao);

	glGenBuffers(1, &mesh.vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * mesh.vertices.size(), &mesh.vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &mesh.elementBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.elementBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * mesh.indices.size(), &mesh.indices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);
	glEnableVertexAttribArray(5);
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(offsetof(Vertex, position))); 
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(offsetof(Vertex, normal))); 
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(offsetof(Vertex, texcoord))); 
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(offsetof(Vertex, tangent))); 
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(offsetof(Vertex, bitangent))); 
	glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), BUFFER_OFFSET(offsetof(Vertex, skinIndex))); 
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(offsetof(Vertex, skinWeight))); 
}

void Model::BindTexture(Texture& t)
{
	glGenTextures(1, &t.id);

	glBindTexture(GL_TEXTURE_2D, t.id);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, t.width, t.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, t.data);

	// can free this now, not sure if we would ever need to unload from graphics memory
	// and swap back in again without reading from disk
	stbi_image_free(t.data);

	t.data = nullptr;
}

int Model::LoadTexture(aiMaterial *pMaterial, aiTextureType type)
{
	if (pMaterial->GetTextureCount(type) > 0)
	{
		// TODO -- check if texture is already loaded by another material?
		aiString path;
		if (pMaterial->GetTexture(type, 0, &path) == aiReturn_SUCCESS)
		{
			// image paths are relative to the model file's directory
			if (m_filename.rfind('/') > 0)
			{
				std::string directory = m_filename.substr(0, m_filename.rfind('/') + 1);
				path.Set(directory.append(path.C_Str()).c_str());
			}

			Texture t;
			int components;
			t.data = stbi_load(path.C_Str(), &t.width, &t.height, &components, 4);

			if (t.data)
			{
				m_textures.push_back(t);
				return m_textures.size() - 1;
			}
		}
	}

	// No texture of this type
	return -1;
}
void Model::Load()
{
	if (IsLoaded())
	{
		return;
	}

	Assimp::Importer importer;
	const aiScene* pScene = importer.ReadFile(m_filename, aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_GenNormals);

	if (!pScene)
	{
		std::cout << "Error loading file:" << m_filename << std::endl;
		return;
	}

	float fScale = 1.0f;

	for (int i = 0; i < pScene->mNumMeshes; ++i)
	{
		LoadMesh(pScene->mMeshes[i], fScale);
	}

	for (int i = 0; i < pScene->mNumMaterials; ++i)
	{
		if (aiMaterial* pMaterial = pScene->mMaterials[i])
		{
			Material material;

			material.diffuseTexture = LoadTexture(pMaterial, aiTextureType_DIFFUSE);

			material.normalTexture = LoadTexture(pMaterial, aiTextureType_NORMALS);

			material.specularTexture = LoadTexture(pMaterial, aiTextureType_SPECULAR);

			aiColor3D color(1.0f, 1.0f, 1.0f);
			pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color);

			material.colorFactor.r = color.r;
			material.colorFactor.g = color.g;
			material.colorFactor.b = color.b;

			m_materials.push_back(material);
		}
	}

	// TODO - clean up texture data
	m_loaded = true;
}

void Model::Bind()
{
	for (Texture& t : m_textures)
	{
		BindTexture(t);
	}

	for (Mesh& mesh : m_meshes)
	{
		BindMesh(mesh);
	}

	m_bound = true;
}

void Mesh::Draw()
{
}

void Model::Draw(ShaderProgram* shader) const
{
	if (!IsBound())
	{
		return;
	}

	// Shared?

	for (const Mesh &mesh : m_meshes)
	{
		glBindVertexArray(mesh.vao);

		glActiveTexture(GL_TEXTURE0);
		const Material &mat = m_materials[mesh.material];
		if (mat.diffuseTexture != -1)
		{
			const Texture &tex = m_textures[mat.diffuseTexture];
			glBindTexture(GL_TEXTURE_2D, tex.id);
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		glActiveTexture(GL_TEXTURE1);
		if (mat.specularTexture != -1)
		{
			const Texture &tex = m_textures[mat.specularTexture];
			glBindTexture(GL_TEXTURE_2D, tex.id);
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		glActiveTexture(GL_TEXTURE2);
		if (mat.normalTexture != -1)
		{
			const Texture &tex = m_textures[mat.normalTexture];
			glBindTexture(GL_TEXTURE_2D, tex.id);
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		if (shader) shader->SetUniform("colorFactor", mat.colorFactor);
		
		glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
	}

	glBindVertexArray(0);
}