#ifndef MESHDATA_H
#define MESHDATA_H

#include <glm/glm.hpp>

struct vertexData{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 tangent;
	glm::vec3 color;
	glm::vec2 UV1;
	glm::vec2 UV2;
};

struct textureData{
	unsigned int id;
	unsigned int type;
};

#endif
