#ifndef SPHERE_H
#define SPHERE_H

#include <glad/glad.h> // holds all OpenGL type declarations

#include <glm/glm.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "mesh.h"

#include <vector>

#define rows 100
#define cols 100
#define PI   3.14159265358979323846

class Sphere {
public:
	// no default constructor
	Sphere() = delete;

	// constructor
	Sphere(float radius) : radius(radius), origin(glm::vec3()) 
	{
		generateMesh();
	}
	
	Sphere(float radius, glm::vec3 origin) : radius(radius), origin(origin) 
	{
		generateMesh();
	}

	// render the sphere
	void Draw(Shader& shader)
	{
		mesh.Draw(shader);
	}

	~Sphere()
	{
		mesh.Delete();
	}

private:
	vector<Vertex> vertices;

	float radius;
	glm::vec3 origin;
	Mesh mesh;

	void generateMesh()
	{
		for(unsigned int i = 0; i < rows; i++)
			for (unsigned int j = 0; j < cols; j++)
			{
				Vertex v;
				float theta = j * 2.0f * PI / cols;
				float phi = i * 1.0f * PI / rows;

				v.Position.x = radius * cos(theta) * sin(phi);
				v.Position.y = radius * sin(theta) * sin(phi);
				v.Position.z = radius * cos(phi);
				
				vertices.push_back(v);
			}
		vector<unsigned int> indices;
		for(unsigned int i = 0; i < rows - 1; i++)
			for (unsigned int j = 0; j < cols; j++)
			{
				indices.push_back(i * cols + j);
				indices.push_back(i * cols + (j + 1) % cols);
				indices.push_back((i + 1) * cols + (j + 1) % cols);

				indices.push_back(i * cols + j);
				indices.push_back((i + 1) * cols + j);
				indices.push_back((i + 1) * cols + (j + 1) % cols);
			}

		vector<Texture> textures; // now is empty;
		mesh = Mesh(vertices, indices, textures);
	}
};
#endif
