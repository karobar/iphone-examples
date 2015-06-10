#ifndef SCENELOADER_H
#define SCENELOADER_H

#include "Mesh.h"
#include "Camera.h"
#include "Light.h"

#include <vector>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class SceneManager{
    
public:
	SceneManager(const char* filename);
	~SceneManager();
    
    void draw(Camera* camera, float dt);
    std::vector<mesh*>& getMeshes();
    
private:
	std::vector<mesh*>      meshes;
    std::vector<Light>		lights;
    
    float                   m_Time;
    
	void recursiveProcess(aiNode* node,const aiScene* scene);
	void processMesh(aiMesh* mesh,const aiScene* scene);
    void processLights(const aiScene* scene);
    
    char* loadTGA ( const char *fileName, int *width, int *height, int* numChannels );
	unsigned int loadTexture(const char* filename);
};

#endif
