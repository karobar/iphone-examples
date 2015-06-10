#include "SDL.h"

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
    /* B481-TODO: you may have to create your View Matrix in this function. */
    glm::mat4x4 transMatrix(1.0f);

	glm::mat4x4 rotMat(1.0f); //Load Identity
    glm::mat4x4 pitchMat(1.0f); //Load Identity
    glm::mat4x4 yawMat(1.0f); //Load Identity
    
    /*  B481-TODO:
       Reorient the camera
       by transforming its original frameMatrix orientation frame
       with a rotMat matrix that you build using "pitch" and "yaw" values.
    */
    pitchMat = glm::rotate(pitchMat, m_pitch,(glm::vec3)frameMatrix[1]);
    yawMat   = glm::rotate(yawMat, m_yaw,  (glm::vec3)frameMatrix[2]);
    rotMat =  pitchMat * yawMat;
    
    transMatrix = glm::translate(transMatrix, m_position);
    glm::mat4x4 cameraMatrix = transMatrix * rotMat * frameMatrix;
	
    /*  B481-TODO:
        once you have the new camera matrix,
        you may call glm::lookAt (...) to build the View Matrix.
    */
    glm::vec3 center = m_position + (glm::vec3)cameraMatrix[0];
    glm::vec3 up     = (glm::vec3)cameraMatrix[1];
    viewMatrix = glm::lookAt(m_position, center, up);
    
    cameraMatrix = transMatrix * rotMat;
    viewMatrix = glm::inverse(cameraMatrix);
    
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
    /* B481-TODO: you may have to update the camera position in this function. */
	glm::vec4 dPos(dx, dy, dz, 0.0);
    

    /* B481-TODO:
       to translate the camera position,
       you may have to transform dPos to determine the direction of motion of your camera.
     */
    glm::mat4x4 rotMat(1.0f); 
    glm::mat4x4 rotMatX(1.0f);
    glm::mat4x4 rotMatY(1.0f);
    rotMatX = glm::rotate(rotMatX, m_yaw,   glm::vec3(frameMatrix[2]));
    rotMatY = glm::rotate(rotMatY, m_pitch, glm::vec3(frameMatrix[1]));
    rotMat = rotMatY * rotMatX;
    dPos = rotMat * dPos;
    
    SDL_Log("m_position = (%f, %f, %f)", m_position.x, m_position.y, m_position.z);
    m_position += (glm::vec3)dPos;
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
