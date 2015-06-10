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
    
    void draw(unsigned int programId, Camera* camera);
    std::vector<mesh*>& getMeshes();
    
private:
	std::vector<mesh*>      meshes;
    std::vector<Light>		lights;
    
	void recursiveProcess(aiNode* node,const aiScene* scene);
	void processMesh(aiMesh* mesh,const aiScene* scene);
    void processLights(const aiScene* scene);
};

#endif
