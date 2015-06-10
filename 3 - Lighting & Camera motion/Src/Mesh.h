#ifndef MESH_H
#define MESH_H

#include "MeshData.h"

#include <iostream>
#include <vector>
#include <string>

class mesh{
	std::vector<vertexData> data;
	std::vector<unsigned int> indices;
	unsigned int VBO;
	unsigned int IND;
	public:
		mesh(std::vector<vertexData>* vd,std::vector<unsigned int>* id);
		~mesh();
		void draw(unsigned int programId);
};
#endif
