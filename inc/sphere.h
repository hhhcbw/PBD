#ifndef SPHERE_H
#define SPHERE_H

#include <glad/glad.h> // holds all OpenGL type declarations

#include <glm/glm.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "mesh.h"

#include <vector>

#define ROWS 100
#define COLS 100
#define PI   3.14159265358979323846

class Sphere {
public:
	// no default constructor
	Sphere() = default;

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

	// update the sphere according to the direction
	void update(glm::vec3 dir)
	{
		origin += dir;
		for (unsigned int i = 0; i < ROWS; i++)
			for (unsigned int j = 0; j < COLS; j++)
			{
				float theta = j * 2.0f * PI / COLS;
				float phi = i * 1.0f * PI / ROWS;
				vertices[i * COLS + j].Position += dir;
			}
		mesh.updateVertices(vertices);
	}

	glm::vec3 getOrigin()
	{
		return origin;
	}

	float getRadius()
	{
		return radius;
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
		for(unsigned int i = 0; i < ROWS; i++)
			for (unsigned int j = 0; j < COLS; j++)
			{
				Vertex v;
				float theta = j * 2.0f * PI / COLS;
				float phi = i * 1.0f * PI / ROWS;

				v.Position.x = radius * cos(theta) * sin(phi);
				v.Position.y = radius * sin(theta) * sin(phi);
				v.Position.z = radius * cos(phi);
				
				v.Position += origin;
				vertices.push_back(v);
			}
		vector<unsigned int> indices;
		for(unsigned int i = 0; i < ROWS - 1; i++)
			for (unsigned int j = 0; j < COLS; j++)
			{
				indices.push_back(i * COLS + j);
				indices.push_back(i * COLS + (j + 1) % COLS);
				indices.push_back((i + 1) * COLS + (j + 1) % COLS);

				indices.push_back(i * COLS + j);
				indices.push_back((i + 1) * COLS + j);
				indices.push_back((i + 1) * COLS + (j + 1) % COLS);
			}

		vector<Texture> textures; // now is empty;
		mesh = Mesh(vertices, indices, textures);
	}
};
#endif
