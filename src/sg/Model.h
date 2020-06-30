#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <imgui.h>
#include <string>
#include <vector>

#include "assimp/Importer.hpp"
#include "assimp/scene.h"

struct aiMesh;

struct Texture
{
	GLuint id;
	aiString path; // path, used to identify -- TODO hash?
	int width;
	int height;
	unsigned char* data;
};

struct Material
{
	Material() 
		: diffuseTexture(-1)
		, normalTexture(-1)
		, specularTexture(-1)
	{
	}

	int diffuseTexture;
	int normalTexture;
	int specularTexture;
};

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 tangent;
	glm::vec3 bitangent; // TODO -- compare performance of precomputing vs. computing in vertex shader?
	glm::vec2 texcoord;
};

struct Mesh
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

	GLuint vao;
	GLuint vertexBuffer;
	GLuint elementBuffer;

	int material;

	void Draw();
};

class Model
{
public:

	Model(const char *filename);

	void Draw();
	void Load();

	int LoadTexture(aiMaterial* pMaterial, aiTextureType type);
	void LoadMesh(const aiMesh *pMesh, float fScale);

	void Bind();
	void BindMesh(Mesh& mesh);
	void BindTexture(Texture& t);

	bool IsLoaded() { return m_loaded; }
	bool IsBound() { return m_bound; }

	const char* GetFilename() { return m_filename.c_str(); }

	glm::mat4 GetTransform() { return transform; }
	void SetTransform(glm::mat4 t) { transform = t; }

	void Widgets() 
	{
		ImGui::DragFloat3("Position", (float *)&transform[3], 0.01, -100, 100);
	}

private:

	std::string m_filename;

	std::vector<Mesh> m_meshes;
	std::vector<Texture> m_textures;
	std::vector<Material> m_materials;

	// TODO -- separate resource + instance data
	glm::mat4 transform;

	bool m_loaded: 1;
	bool m_bound: 1;

};