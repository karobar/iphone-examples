#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "SceneManager.h"
#include "MaterialManager.h"
#include "ResourcePath.h"
#include "SDL.h"

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
	std::vector<textureData> textures;
	aiColor4D col;
	aiMaterial* mat=scene->mMaterials[m->mMaterialIndex];
	aiGetMaterialColor(mat,AI_MATKEY_COLOR_DIFFUSE,&col);
	glm::vec3 defaultColor(col.r,col.g,col.b);
	aiString name;
	std::string matName;
	MaterialManager* pMaterialManager;
	int shader;


	for(int i=0;i<m->mNumVertices;i++)
	{
			vertexData tmp;
			glm::vec3 tmpVec;
            glm::vec2 tmpVec2D;

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

            //UV layer 1
            if(m->mTextureCoords[0])
            {
                tmpVec2D.x=m->mTextureCoords[0][i].x;
                tmpVec2D.y=m->mTextureCoords[0][i].y;
            }else
            {
                tmpVec2D.x=tmpVec2D.y=0.0;
            }
            tmp.UV1 = tmpVec2D;
            
            //UV layer 2
            if(m->mTextureCoords[1])
            {
                tmpVec2D.x=m->mTextureCoords[1][i].x;
                tmpVec2D.y=m->mTextureCoords[1][i].y;
            }else
            {
                tmpVec2D.x=tmpVec2D.y=0.0;
            }
            tmp.UV2 = tmpVec2D;
        
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


    for(int i=0;i<mat->GetTextureCount(aiTextureType_DIFFUSE);i++)
	{
		aiString str;
		mat->GetTexture(aiTextureType_DIFFUSE,i,&str);
		textureData tmp;
		tmp.id=loadTexture(str.C_Str());
		tmp.type=0;
		textures.push_back(tmp);
	}
    
	for(int i=0;i<mat->GetTextureCount(aiTextureType_NORMALS);i++)
	{
		aiString str;
		mat->GetTexture(aiTextureType_NORMALS,i,&str);
		textureData tmp;
		tmp.id=loadTexture(str.C_Str());
		tmp.type=0;
		textures.push_back(tmp);
	}
    
	for(int i=0;i<mat->GetTextureCount(aiTextureType_SPECULAR);i++)
	{
		aiString str;
		mat->GetTexture(aiTextureType_SPECULAR,i,&str);
		textureData tmp;
		tmp.id=loadTexture(str.C_Str());
		tmp.type=0;
		textures.push_back(tmp);
	}
    
	mat->Get(AI_MATKEY_NAME,name);
	matName = std::string(name.C_Str());
	pMaterialManager = MaterialManager::GetInstance();
	shader = pMaterialManager->GetShaderByMaterialName(matName);
    
	meshes.push_back(new mesh(&data,&indices,shader,&textures));
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
        
		lights.push_back(tmpLight);
	}
}

///
// esLoadTGA()
//
//    Loads a 24-bit or 32-bit TGA image from a file
//
char* SceneManager::loadTGA ( const char *fileName, int *width, int *height, int* numChannels )
{
	char *BufferOut = NULL;
    char *buffer = NULL;
    FILE *f;
    unsigned char tgaheader[12];
    unsigned char attributes[6];
    unsigned int imagesize;
	int x, y, i = 0;
    
    f = fopen(fileName, "rb");
    if(f == NULL) return NULL;
    
    if(fread(&tgaheader, sizeof(tgaheader), 1, f) == 0)
    {
        fclose(f);
        return NULL;
    }
    
    if(fread(attributes, sizeof(attributes), 1, f) == 0)
    {
        fclose(f);
        return NULL;
    }
    
    *width = attributes[1] * 256 + attributes[0];
    *height = attributes[3] * 256 + attributes[2];
	*numChannels = attributes[4] / 8;
    imagesize = *numChannels * *width * *height;
    buffer = (char*)malloc(imagesize);
    if (buffer == NULL)
    {
        fclose(f);
        return NULL;
    }
    
    if(fread(buffer, 1, imagesize, f) != imagesize)
    {
        free(buffer);
        return NULL;
    }
	else
	{
		if (*numChannels == 3)
		{
			BufferOut = (char*)malloc(imagesize);
            
			for ( y = 0; y < *height; y++ )
				for( x = 0; x < *width; x++ )
				{
                    int Index= 3 * (y * (*width) + x);
                    
                    if(!(attributes[5] & (1 << 5))) // INVERTED_BIT Check
                        Index= 3 * (((*height) - 1 - y) * (*width) + x);
                    
                    BufferOut[(i * 3)] =      buffer[Index + 2];
                    BufferOut[(i * 3) + 1] =  buffer[Index + 1];
                    BufferOut[(i * 3) + 2] =  buffer[Index + 0];
                    
                    i++;
				}
		}
		else if (*numChannels == 4)
		{
			BufferOut = (char*)malloc(imagesize);
            
			for ( y = 0; y < *height; y++ )
				for( x = 0; x < *width; x++ )
				{
                    int Index= 4 * (y * (*width) + x);
                    
                    if(!(attributes[5] & (1 << 5))) // INVERTED_BIT Check
                        Index= 4 * (((*height) - 1 - y) * (*width) + x);
                    
                    BufferOut[(i * 4)] =      buffer[Index + 2];
                    BufferOut[(i * 4) + 1] =  buffer[Index + 1];
                    BufferOut[(i * 4) + 2] =  buffer[Index + 0];
                    BufferOut[(i * 4) + 3] =  buffer[Index + 3];
                    
                    i++;
				}
		}
	}
    
    fclose(f);
    return BufferOut;
}


unsigned int SceneManager::loadTexture(const char* filename)
{
    //SDL_Log("beginning loadTexture");
	unsigned int num;
	int width, height, numChannels;

	std::string path = getResourcePath() + std::string("Textures/") + std::string(filename);

	char* pixelData = loadTGA( path.c_str(), &width, &height, &numChannels );
	if(pixelData==NULL)
	{
		std::cout << "img was not loaded" << std::endl;
		return -1;
	}

    // B481-TODO:
    // Generate a new texture handle and store it in the function's return value, num:
    // glGenTextures(...);
    glGenTextures(1, &num);
    
    // Bind to the new texture handle, so that subsequent texture related calls will apply to it:
	// glBindTexture(GL_TEXTURE_2D,...);
    glBindTexture(GL_TEXTURE_2D, num);
    
    
    // B481-TODO:
    // Specify where from ane how to transfer the texture data to the GPU:
	if (numChannels == 3)
	{
        // RGB case:
		// glTexImage2D(GL_TEXTURE_2D, ...);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,width,height,0,GL_RGB, GL_UNSIGNED_BYTE, pixelData);
	}
	else if (numChannels == 4)
	{
		// RGBA case:
		// glTexImage2D(GL_TEXTURE_2D, ...);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,0,GL_RGBA, GL_UNSIGNED_BYTE, pixelData);
	}
    
    // B481-TODO:
    // Enable MipMapping. Call glGenerateMipmap to create the mip levels for the current texture,
    // and switch to a minification filter that interpolates linearly between mip-levels, too.
    glGenerateMipmap(GL_TEXTURE_2D);
    
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);

	glFlush();

	delete [] pixelData;
	return num;
}

SceneManager::SceneManager(const char* filename)
{
	m_Time = 0.0f;
    
    const aiScene* scene = aiImportFile(filename, aiProcess_GenSmoothNormals | aiProcess_Triangulate  | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
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

void SceneManager::draw(Camera* camera, float dt)
{
    glm::mat4x4 ProjMatrix = (*camera->getProjMatrix());
    glm::mat4x4 ViewMatrix = (*camera->getViewMatrix());
    glm::mat4x4 ModelMatrix = glm::mat4(1.0f);
    glm::mat4x4 NormalMatrix = glm::transpose(glm::inverse(ViewMatrix * ModelMatrix));
    
    for(int i=0;i<meshes.size();i++)
	{
		unsigned int programObject = meshes[i]->getShader();
		glUseProgram(programObject);
        
        GLint MM_Addr = glGetUniformLocation(programObject, "Model_Matrix");
        glUniformMatrix4fv(MM_Addr, 1, false, glm::value_ptr(ModelMatrix));
        
        GLint VM_Addr = glGetUniformLocation(programObject, "View_Matrix");
        glUniformMatrix4fv(VM_Addr, 1, false, glm::value_ptr(ViewMatrix));
        
        GLint NM_Addr = glGetUniformLocation(programObject, "Normal_Matrix");
        glUniformMatrix4fv(NM_Addr, 1, false, glm::value_ptr(NormalMatrix));
        
        GLint PM_Addr = glGetUniformLocation(programObject, "Proj_Matrix");
        glUniformMatrix4fv(PM_Addr, 1, false, glm::value_ptr(ProjMatrix));
        
        int addr;
        for(int i=0;i<lights.size();i++)
        {
            switch(lights[i].mType)
            {
                case E_LIGHT_SUN:
                    addr = glGetUniformLocation(programObject,(std::string("u_lightDir")+(char)('0'+i)).c_str());
                    glUniform3fv(addr, 1, glm::value_ptr(lights[i].mDirection));
                    addr = glGetUniformLocation(programObject,(std::string("u_lightColor")+(char)('0'+i)).c_str());
                    glUniform3fv(addr, 1, glm::value_ptr(lights[i].mColorDiffuse));
                    break;
                case E_LIGHT_POINT:
                    addr = glGetUniformLocation(programObject,(std::string("u_lightPos")+(char)('0'+i)).c_str());
                    glUniform3fv(addr, 1, glm::value_ptr(lights[i].mPosition));
                    addr = glGetUniformLocation(programObject,(std::string("u_lightColor")+(char)('0'+i)).c_str());
                    glUniform3fv(addr, 1, glm::value_ptr(lights[i].mColorDiffuse));
                    break;
                case E_LIGHT_SPOT:
                    addr = glGetUniformLocation(programObject,(std::string("u_lightPos")+(char)('0'+i)).c_str());
                    glUniform3fv(addr, 1, glm::value_ptr(lights[i].mPosition));
                    addr = glGetUniformLocation(programObject,(std::string("u_lightDir")+(char)('0'+i)).c_str());
                    glUniform3fv(addr, 1, glm::value_ptr(lights[i].mDirection));
                    addr = glGetUniformLocation(programObject,(std::string("u_lightCosCutoff")+(char)('0'+i)).c_str());
                    glUniform1f(addr, lights[i].mSpotCosCutoff);
                    addr = glGetUniformLocation(programObject,(std::string("u_lightColor")+(char)('0'+i)).c_str());
                    glUniform3fv(addr, 1, glm::value_ptr(lights[i].mColorDiffuse));
                    break;
                default:
                    std::cout << "Unhandled Light Type: " << lights[i].mType;
                    break;
            }
        }
        
        //B481-TODO:
        // Update m_Time with dt and use the result
        // to initialize the u_Time uniform, if it is present for the current shader:
        m_Time = (m_Time + dt);
        int timeAddr = glGetUniformLocation(programObject,"u_Time");
        glUniform1f(timeAddr,m_Time);
        meshes[i]->draw();
    }
}

std::vector<mesh*>& SceneManager::getMeshes()
{
	return meshes;
}
