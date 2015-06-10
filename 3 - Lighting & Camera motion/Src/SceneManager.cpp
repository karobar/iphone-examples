#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "SceneManager.h"

#include "ResourcePath.h"

#include <string>

void SceneManager::recursiveProcess(aiNode* node,const aiScene* scene)
{
	//process
    if (node->mNumMeshes > 0)
    {
        for(int i=0;i<node->mNumMeshes;i++)
        {
            aiMesh* mesh=scene->mMeshes[node->mMeshes[i]];
            processMesh(mesh,scene);
        }
    }
    else
    {
		for (int i=0; i<lights.size(); i++)
		{
			if (lights[i].mName == std::string(node->mName.C_Str()))
			{
				glm::mat4 nodeTransform(*(glm::mat4*)&node->mTransformation.a1);
                nodeTransform = glm::transpose(nodeTransform);
				lights[i].mPosition = glm::vec3((nodeTransform * glm::vec4(lights[i].mPosition, 1.0)));
				lights[i].mDirection = glm::vec3((nodeTransform * glm::vec4(lights[i].mDirection, 0.0)));
			}
		}
    }

	//recursion
	for(int i=0;i<node->mNumChildren;i++)
	{
		recursiveProcess(node->mChildren[i],scene);
	}
}

void SceneManager::processMesh(aiMesh* m,const aiScene* scene)
{
	std::vector<vertexData> data;
	std::vector<unsigned int> indices;
	aiColor4D col;
	aiMaterial* mat=scene->mMaterials[m->mMaterialIndex];
	aiGetMaterialColor(mat,AI_MATKEY_COLOR_DIFFUSE,&col);
	glm::vec3 defaultColor(col.r,col.g,col.b);


	for(int i=0;i<m->mNumVertices;i++)
	{
			vertexData tmp;
			glm::vec3 tmpVec;

			//position
			tmpVec.x=m->mVertices[i].x;
			tmpVec.y=m->mVertices[i].y;
			tmpVec.z=m->mVertices[i].z;
			tmp.position=tmpVec;

			//normals
			tmpVec.x=m->mNormals[i].x;
			tmpVec.y=m->mNormals[i].y;
			tmpVec.z=m->mNormals[i].z;
			tmp.normal=tmpVec;


			//tangent
			if(m->mTangents)
			{
				tmpVec.x=m->mTangents[i].x;
				tmpVec.y=m->mTangents[i].y;
				tmpVec.z=m->mTangents[i].z;
			}else{
				tmpVec.x=1.0;
				tmpVec.y=tmpVec.z=0;
			}
			tmp.tangent=tmpVec;


			//colors
			if(m->mColors[0])
			{
				//!= material color
				tmpVec.x=m->mColors[0][i].r;
				tmpVec.y=m->mColors[0][i].g;
				tmpVec.z=m->mColors[0][i].b;
			}else{
				tmpVec=defaultColor;
			}
			tmp.color=tmpVec;
            
			data.push_back(tmp);
	}

	for(int i=0;i<m->mNumFaces;i++)
	{
		aiFace face=m->mFaces[i];
		for(int j=0;j<face.mNumIndices;j++) //0..2
		{
			indices.push_back(face.mIndices[j]);
		}
	}
    
	meshes.push_back(new mesh(&data,&indices));
}

void SceneManager::processLights(const aiScene* scene)
{
	struct Light tmpLight;
    
    std::cout << "Loading Lights:\n";
    
	for(int i=0;i<scene->mNumLights;i++)
	{
        tmpLight.mName = std::string(scene->mLights[i]->mName.C_Str());
		tmpLight.mType          = (eLightType)scene->mLights[i]->mType;
		tmpLight.mPosition      = glm::vec3( scene->mLights[i]->mPosition.x, scene->mLights[i]->mPosition.y, scene->mLights[i]->mPosition.z);
		tmpLight.mDirection     = glm::vec3( scene->mLights[i]->mDirection.x, scene->mLights[i]->mDirection.y, scene->mLights[i]->mDirection.z);
		tmpLight.mColorDiffuse	= glm::vec3( scene->mLights[i]->mColorDiffuse.r, scene->mLights[i]->mColorDiffuse.g, scene->mLights[i]->mColorDiffuse.b);
		tmpLight.mSpotCosCutoff = glm::cos(scene->mLights[i]->mAngleInnerCone);
        
        std::cout << scene->mLights[i]->mType << std::endl;
        std::cout << scene->mLights[i]->mPosition.x << scene->mLights[i]->mPosition.y << scene->mLights[i]->mPosition.z << std::endl;
        
		lights.push_back(tmpLight);
	}
}

SceneManager::SceneManager(const char* filename)
{
	const aiScene* scene = aiImportFile(filename, aiProcess_GenSmoothNormals | aiProcess_Triangulate  | aiProcess_FlipUVs);
	if(scene == NULL)
	{
		std::cout << "The file wasn't successfuly opened " << filename << " " << aiGetErrorString() << std::endl;
		return;
	}

    processLights(scene);
	recursiveProcess(scene->mRootNode,scene);
}

SceneManager::~SceneManager()
{
	for(int i=0;i<meshes.size();i++)
		delete meshes[i];
}

void SceneManager::draw(unsigned int programId, Camera* camera)
{
    glm::mat4x4 ProjMatrix = (*camera->getProjMatrix());
    glm::mat4x4 ViewMatrix = (*camera->getViewMatrix());
    glm::mat4x4 ModelMatrix = glm::mat4(1.0f);
    
    // Use the same shader for all meshes.
    glUseProgram ( programId );
    
    GLint MM_Addr = glGetUniformLocation(programId, "Model_Matrix");
    glUniformMatrix4fv(MM_Addr, 1, false, glm::value_ptr(ModelMatrix));
    
    GLint VM_Addr = glGetUniformLocation(programId, "View_Matrix");
    glUniformMatrix4fv(VM_Addr, 1, false, glm::value_ptr(ViewMatrix));
    
    GLint PM_Addr = glGetUniformLocation(programId, "Proj_Matrix");
    glUniformMatrix4fv(PM_Addr, 1, false, glm::value_ptr(ProjMatrix));
    
    
    int addr;
    for(int i=0;i<lights.size();i++)
    {
        switch(lights[i].mType)
        {
            /* B481-TODO: you may have to implement light properties here. */
            
            /* B481-TODO:
               for each light type, a subset of properties are going to be relevant.
               Light properties need to be defined as uniform variables in the shader programs.
               Those uniform variables can then be initialized here.
               You can use a naming convention based on the index of each light,
               e.g. for "u_lightDir0" you may use:
                    (std::string("u_lightDir")+(char)('0'+i)).c_str()
             */
            
            case E_LIGHT_SUN: {
                /* B481-TODO: you may have to implement the "sun" light here. */
                GLint SUN_Dir = glGetUniformLocation(programId, "u_lightDir0");
                GLint SUN_Diffuse = glGetUniformLocation(programId, "u_lightDiffuse0");
                glUniform3fv(SUN_Dir, 1, glm::value_ptr(lights[i].mDirection));
                glUniform3fv(SUN_Diffuse, 1, glm::value_ptr(lights[i].mColorDiffuse));
            }break;
            case E_LIGHT_POINT: {
                /* B481-TODO: you may have to implement the positional "point-source" light here. */
                GLint POINT_Pos = glGetUniformLocation(programId, "u_pointPos");
                GLint POINT_Diffuse = glGetUniformLocation(programId, "u_pointDiffuse");
                glUniform3fv(POINT_Pos, 1, glm::value_ptr(lights[i].mPosition));
                glUniform3fv(POINT_Diffuse, 1, glm::value_ptr(lights[i].mColorDiffuse));
            }   break;
            case E_LIGHT_SPOT:
                /* B481-TODO: you can implement the spotlight light here. */

                break;
            default:
                std::cout << "Unhandled Light Type: " << lights[i].mType;
                break;
        }
    }
    
	for(int i=0;i<meshes.size();i++)
    {
        meshes[i]->draw(programId);
    }
}

std::vector<mesh*>& SceneManager::getMeshes()
{
	return meshes;
}
