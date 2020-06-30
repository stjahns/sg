#include <glad/glad.h>

// Standard Headers
#include <iostream>
#include "stb_image.h"

#include <glm/gtc/matrix_transform.hpp>
#include "Model.h"

#include "assimp/postprocess.h"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

Model::Model(const char* filename)
	: m_loaded(false)
	, m_bound(false)
{
	m_filename = filename;
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
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(offsetof(Vertex, position))); 
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(offsetof(Vertex, normal))); 
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(offsetof(Vertex, texcoord))); 
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(offsetof(Vertex, tangent))); 
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(offsetof(Vertex, bitangent))); 
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
			int width, height, components;
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
	const aiScene* pScene = importer.ReadFile(m_filename, aiProcess_CalcTangentSpace | aiProcess_Triangulate);

	if (!pScene)
	{
		std::cout << "Error loading file:" << m_filename << std::endl;
		return;
	}

	float fScale = 1.0f;

	if (aiNode* pNode = pScene->mRootNode)
	{
		fScale = pNode->mTransformation.a1;
	}

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

void Model::Draw()
{
	if (!IsBound())
	{
		return;
	}

	// Shared?

	for (Mesh &mesh : m_meshes)
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
		
		glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
	}

	glBindVertexArray(0);
}