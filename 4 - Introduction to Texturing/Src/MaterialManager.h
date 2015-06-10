
#ifndef MATERIAL_MANAGER_H
#define MATERIAL_MANAGER_H

#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

#include <string>
#include <map>

class MaterialManager
{
public:
	static MaterialManager* GetInstance();

	int LoadShaders(std::string& path);

	int GetShaderByName(std::string& shaderName);
	int GetShaderByMaterialName(std::string& materialName);

private:
	MaterialManager();
	~MaterialManager();
	MaterialManager(MaterialManager const&){};
	MaterialManager& operator=(MaterialManager const&){};

	void InitShaderDefs();
	void DeleteShaders();
	int CreateShader ( std::string& shaderPath );
	GLuint LoadShader ( GLenum type, const char *shaderSrc );

	std::map<std::string, std::string> m_materialMap;
	std::map<std::string, int> m_shaderMap;

	static MaterialManager*	m_pInstance;
};

#endif