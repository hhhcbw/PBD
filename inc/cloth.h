#ifndef CLOTH_H
#define CLOTH_H

#include <glad/glad.h> // holds all OpenGL type declarations

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "mesh.h"

#include <vector>

#define g glm::vec3(0.0f, -9.8f, 0.0f)

typedef glm::vec3 Normal;
typedef glm::vec3 Velocity;
typedef glm::vec3 Acceleration;
typedef glm::vec3 Force;

class Cloth{
public:
	// no default constructor
	Cloth() = delete;
	
	// constructior
	Cloth(unsigned int rows, unsigned int cols)
	{
		for(unsigned int i = 0; i < rows; i++)
			for(unsigned int j = 0; j < cols; j++)
			{
				Vertex v;
				v.Position.x = (float)i / (float)rows - 0.5f;
				v.Position.y = 0.0f;
				v.Position.z = (float)j / (float)cols - 0.5f;
				vertices.push_back(v);
				vels.push_back({ 0.0f, 0.0f, 0.0f });
				//forces.push_back({ 0.0f, 0.0f, 0.0f });
			}
		vector<unsigned int> indices;
		for(unsigned int i = 0; i < rows - 1; i++)
			for (unsigned int j = 0; j < cols - 1; j++)
			{
				// triangle1
				indices.push_back(i * cols + j);
				indices.push_back(i * cols + j + 1);
				indices.push_back((i + 1) * cols + j + 1);
				normals.push_back(Normal(0.0f, 1.0f, 0.0f));
				// triangle2
				indices.push_back(i * cols + j);
				indices.push_back((i + 1) * cols + j);
				indices.push_back((i + 1) * cols + j + 1);
				normals.push_back(Normal(0.0f, 1.0f, 0.0f));
			}

		vector<Texture> textures; // now is empty
		mesh = Mesh(vertices, indices, textures);
	}

	// render the cloth
	void Draw(Shader& shader)
	{
		mesh.Draw(shader);
	}

	// update the cloth
	void update(float deltaTime)
	{
		float dt = deltaTime * 0.1f;
		for (unsigned int i = 0; i < vertices.size(); i++)
		{
			vels[i] = vels[i] + g * dt;
			vertices[i].Position = vertices[i].Position + vels[i] * dt;
		}
		// update mesh
		mesh.updateVertices(vertices);
	}

	~Cloth()
	{
		mesh.Delete();
	}

private:
	// cloth data
	vector<Vertex> vertices;
	vector<Velocity> vels;
	//vector<Force> forces;
	vector<Normal> normals;
	Mesh mesh;
};
#endif