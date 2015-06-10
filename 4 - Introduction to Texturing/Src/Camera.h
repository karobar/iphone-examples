#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera
{
public:
	Camera();

	const glm::mat4x4*  getViewMatrix();
    const glm::mat4x4*	getProjMatrix();
	glm::vec3           getPosition();

	void		resetCameraFrame();
	void		translateCamera(float dx, float dy, float dz);
	void		addPitchRotation(float deg);
	void		addYawRotation(float deg);
    
    void        setOrientationTransform(glm::mat4 & newOrientationMat);
    void        setAspectRatio(float aRatio);

private:
	// yaw and pitch angles
	float		m_yaw;
	float		m_pitch;
	
	glm::vec3	m_position;

	glm::mat4x4	frameMatrix; // Stores as colums |D U R 0| the direction, up, and right vectors.
    
    glm::mat4x4 orientationMatrix;
    float       aspectRatio;

	glm::mat4x4	viewMatrix;
    glm::mat4x4	projMatrix;
};

#endif
