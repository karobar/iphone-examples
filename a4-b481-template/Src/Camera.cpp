
#include "Camera.h"

#include <glm/gtc/constants.hpp>

#include <math.h>

Camera::Camera()
{
	resetCameraFrame();
    
    orientationMatrix = glm::mat4(1.0);
    aspectRatio = 16.0f/9.0f;
}

void Camera::resetCameraFrame()
{
	m_yaw = 0.0;
	m_pitch = 0.0;
	m_position = glm::vec3(0.0f);

	//Local View Direction:
	frameMatrix[0][0] = 0.0f;
	frameMatrix[0][1] = 0.0f;
	frameMatrix[0][2] = -1.0f;
	frameMatrix[0][3] = 0.0f;

	//Local Up Direction:
	frameMatrix[1][0] = 0.0f;
	frameMatrix[1][1] = 1.0f;
	frameMatrix[1][2] = 0.0f;
	frameMatrix[1][3] = 0.0f;

	//Local Right Direction:
	frameMatrix[2][0] = 1.0f;
	frameMatrix[2][1] = 0.0f;
	frameMatrix[2][2] = 0.0f;
	frameMatrix[2][3] = 0.0f;

	//Unused:
	frameMatrix[3][0] = 0.0f;
	frameMatrix[3][1] = 0.0f;
	frameMatrix[3][2] = 0.0f;
	frameMatrix[3][3] = 1.0f;

	viewMatrix = glm::mat4x4(1.0f); //Load Identity
}

const glm::mat4x4* Camera::getViewMatrix()
{
	glm::mat4x4 rotMat(1.0f); //Load Identity
	glm::mat4x4 rotMatX = glm::rotate(glm::mat4x4(1.0f), m_pitch, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4x4 rotMatY = glm::rotate(glm::mat4x4(1.0f), m_yaw, glm::vec3(0.0f, 1.0f, 0.0f));
	rotMat = rotMatY * rotMatX;
	
	glm::mat4x4 cameraMatrix = rotMat * frameMatrix;
	
	viewMatrix = glm::lookAt(m_position,
		glm::vec3(m_position.x + cameraMatrix[0][0], m_position.y + cameraMatrix[0][1], m_position.z + cameraMatrix[0][2]),         
		glm::vec3(cameraMatrix[1][0], cameraMatrix[1][1], cameraMatrix[1][2]));
	
	return &viewMatrix;
}

const glm::mat4x4* Camera::getProjMatrix()
{
	projMatrix = orientationMatrix * glm::perspective(60.0f, aspectRatio, 0.1f, 100.f);
	
	return &projMatrix;
}

glm::vec3 Camera::getPosition()
{
	return m_position;
}

void Camera::translateCamera(float dx, float dy, float dz)
{
	glm::vec4 dPos(dx, dy, dz, 0.0f);
	dPos = glm::rotate(glm::mat4x4(1.0f), m_yaw, glm::vec3(0.0f, 1.0f, 0.0f)) * dPos;
	m_position += glm::vec3(dPos);
}

void Camera::addPitchRotation(float deg)
{
	m_pitch =  fmod( m_pitch + deg, 360.0f );
}

void Camera::addYawRotation(float deg)
{
	m_yaw =  fmod( m_yaw + deg, 360.0f );
}

void Camera::setOrientationTransform(glm::mat4 & newOrientationMat)
{
    
    orientationMatrix = newOrientationMat;
}

void Camera::setAspectRatio(float aRatio)
{
    aspectRatio = aRatio;
}
