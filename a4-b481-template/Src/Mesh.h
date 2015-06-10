#ifndef MESH_H
#define MESH_H

#include "MeshData.h"

#include <iostream>
#include <vector>
#include <string>

class mesh
{
public:
    mesh(std::vector<vertexData>* vd, std::vector<unsigned int>* id, unsigned int shader, std::vector<textureData>* td=NULL);
    ~mesh();
    
    void draw();
    
    unsigned int getShader() {return programId;};
private:
	std::vector<vertexData> data;
	std::vector<textureData> textures;
	std::vector<unsigned int> indices;
	unsigned int programId;
	unsigned int VBO;
	unsigned int IND;
};
#endif
