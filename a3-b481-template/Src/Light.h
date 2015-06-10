#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

enum eLightType
{
    E_LIGHT_UNDEFINED = 0,
	E_LIGHT_SUN,
    E_LIGHT_POINT,
    E_LIGHT_SPOT,
    NUM_LIGHT_TYPES
};

struct Light
{
    std::string	mName;
	eLightType	mType;
	
	glm::vec3	mPosition;
	glm::vec3	mDirection;
	glm::vec3	mColorDiffuse;
    
	float		mSpotCosCutoff;
};

#endif