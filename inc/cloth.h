#ifndef CLOTH_H
#define CLOTH_H

#include <glad/glad.h> // holds all OpenGL type declarations

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "mesh.h"

#include <vector>

#define g glm::vec3(0.0f, -9.8f, 0.0f)
#define damping 0.99f
#define iteration 32

typedef glm::vec3 Normal;
typedef glm::vec3 Velocity;
typedef glm::vec3 Acceleration;
typedef glm::vec3 Force;

struct Edge
{
	unsigned int indice_x, indice_y;

	bool operator!=(Edge& otherEdge)
	{
		if (this->indice_x == otherEdge.indice_x && this->indice_y == otherEdge.indice_y)
			return false;
		else
			return true;
	}

	bool operator>=(Edge& otherEdge)
	{
		if (this->indice_x > otherEdge.indice_x)
			return true;
		else
		{
			if (this->indice_x == otherEdge.indice_x && this->indice_y >= otherEdge.indice_y)
				return true;
			else
				return false;
		}
	}
};

class Cloth{
public:
	// no default constructor
	Cloth() = delete;
	
	// constructior
	Cloth(unsigned int rows, unsigned int cols)
	{
		this->rows = rows;
		this->cols = cols;
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
		vector<Edge> duplicate_edges;
		for(unsigned int i = 0; i < rows - 1; i++)
			for (unsigned int j = 0; j < cols - 1; j++)
			{
				// triangle1
				indices.push_back(i * cols + j);
				indices.push_back(i * cols + j + 1);
				indices.push_back((i + 1) * cols + j + 1);
				normals.push_back(Normal(0.0f, 1.0f, 0.0f));
				duplicate_edges.push_back({ i * cols + j, i * cols + j + 1 });
				duplicate_edges.push_back({ i * cols + j, (i + 1) * cols + j + 1 });
				duplicate_edges.push_back({ i * cols + j + 1, (i + 1) * cols + j + 1 });

				// triangle2
				indices.push_back(i * cols + j);
				indices.push_back((i + 1) * cols + j);
				indices.push_back((i + 1) * cols + j + 1);
				normals.push_back(Normal(0.0f, 1.0f, 0.0f));
				duplicate_edges.push_back({ i * cols + j, (i + 1) * cols + j });
				duplicate_edges.push_back({ i * cols + j, (i + 1) * cols + j + 1 });
				duplicate_edges.push_back({ (i + 1) * cols + j, (i + 1) * cols + j + 1 });
			}

		vector<Texture> textures; // now is empty
		mesh = Mesh(vertices, indices, textures);

		// remove duplicated edge
		edgeDuplicateRemoval(duplicate_edges);
	}

	// render the cloth
	void Draw(Shader& shader)
	{
		mesh.Draw(shader);
	}

	// update the cloth
	void update(float deltaTime)
	{
		dt = deltaTime;
		for (unsigned int i = 0; i < vertices.size(); i++)
		{
			if (i == 0 || i == (rows - 1) * cols)
				continue;
			vels[i] = vels[i] + g * dt;
			vels[i] = vels[i] * damping;
			vertices[i].Position = vertices[i].Position + vels[i] * dt;
		}
		for (unsigned int i = 0; i < iteration; i++)
			pbdConstraint();
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
	vector<Edge> edges;
	vector<float> lengths;
	//vector<Force> forces;
	vector<Normal> normals;
	Mesh mesh;
	unsigned int rows, cols;
	float dt;

	void edgeDuplicateRemoval(vector<Edge>& duplicate_edges)
	{
		quickSort(duplicate_edges, 0, duplicate_edges.size() - 1);
		for (unsigned int i = 0; i < duplicate_edges.size(); i++)
		{
			if (i == 0 || duplicate_edges[i] != duplicate_edges[i - 1])
				edges.push_back(duplicate_edges[i]);
		}
		for (unsigned int i = 0; i < edges.size(); i++)
		{
			Vertex vx = vertices[edges[i].indice_x];
			Vertex vy = vertices[edges[i].indice_y];
			lengths.push_back(glm::length(vx.Position - vy.Position));
		}
	}

	void quickSort(vector<Edge>& duplicate_edges, int l, int r)
	{
		if (l >= r)
			return;

		int st = 0;
		int ed = r - l;
		int mid = (r + l) / 2;
		swap(duplicate_edges[l], duplicate_edges[mid]);

		{
			vector<Edge> temp(r - l + 1);
			//cout << l << " " << duplicate_edges.size() << endl;
			Edge val = duplicate_edges[l];
			for (unsigned int i = l + 1; i <= r; i++)
			{
				//cout << ed << " " << st << " " << i << " " << temp.size() << endl;
				if (duplicate_edges[i] >= val)
					temp[ed--] = duplicate_edges[i];
				else
					temp[st++] = duplicate_edges[i];
			}
			//cout << st << " " << temp.size() << endl;
			temp[st] = val;
			for (unsigned int i = 0; i < r - l + 1; i++)
			{
				//cout << i + l << " " << i << " " << temp.size() << endl;
				duplicate_edges[i + l] = temp[i];
			}
		}
		
		quickSort(duplicate_edges, l, l + st - 1);
		quickSort(duplicate_edges, l + st + 1, r);
	}

	void pbdConstraint()
	{
		vector<glm::vec3> pos_sum(rows * cols);
		vector<unsigned int> cnt(rows * cols, 0);

		for (unsigned int i = 0; i < edges.size(); i++)
		{
			Edge edge = edges[i];
			float len = lengths[i];

			Vertex vx = vertices[edge.indice_x];
			Vertex vy = vertices[edge.indice_y];

			pos_sum[edge.indice_x] += vx.Position - 0.5f * (glm::length(vx.Position - vy.Position) - len) * glm::normalize(vx.Position - vy.Position);
			pos_sum[edge.indice_y] += vy.Position + 0.5f * (glm::length(vx.Position - vy.Position) - len) * glm::normalize(vx.Position - vy.Position);
		
			cnt[edge.indice_x] ++;
			cnt[edge.indice_y] ++;
		}

		for (unsigned int i = 0; i < vertices.size(); i++)
		{
			if (i == 0 || i == (rows - 1) * cols)
				continue;
			vels[i] = vels[i] + (1.0f / dt) * ((0.2f * vertices[i].Position + pos_sum[i]) / (0.2f + (float)cnt[i]) - vertices[i].Position);
			vertices[i].Position = (0.2f * vertices[i].Position + pos_sum[i]) / (0.2f + (float)cnt[i]);
		}
	}
};
#endif