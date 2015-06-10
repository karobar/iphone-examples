#include "Mesh.h"

#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

#include "SDL.h"

mesh::mesh(std::vector<vertexData>* vd, std::vector<unsigned int>* id, unsigned int shader, std::vector<textureData>* td)
{
	data=*vd;
	indices=*id;
	if(td)
	{
		textures=*td;
	}
	programId = shader;
	
	glGenBuffers(1,&VBO);
	glBindBuffer(GL_ARRAY_BUFFER,VBO);
	glBufferData(GL_ARRAY_BUFFER,data.size()*sizeof(vertexData),
                 &data[0],GL_STATIC_DRAW);
	
	
	glGenBuffers(1,&IND);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,IND);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,indices.size()*sizeof(unsigned int),&indices[0],GL_STATIC_DRAW);
	
	glBindBuffer(GL_ARRAY_BUFFER,0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
	
}

mesh::~mesh()
{
	glDeleteBuffers(1,&VBO);
	glDeleteBuffers(1,&IND);
}

void mesh::draw()
{
	//attribute vec3 vertex
	int vertex=glGetAttribLocation(programId,"vertex"); //0
	int normal=glGetAttribLocation(programId,"normal"); //1
	int tangent=glGetAttribLocation(programId,"tangent"); //2
	int color=glGetAttribLocation(programId,"color"); //3
	int UV=glGetAttribLocation(programId,"UV"); //4
	int UV2=glGetAttribLocation(programId,"UV2"); //5
	
	//texture0
	//texture1...
        
	std::string str="texture";
	for(unsigned int i=0;i<textures.size();i++)
	{
        // B481-TODO:
        // For each texture:
        
        // Enable a unique texture unit for this texture, for this draw call.
        // The available texture units start from GL_TEXTURE0, and it is OK to assume
        // that the rest of the texture unit identifier macros are sequential numbers that
        // start from GL_TEXTURE0.
        glActiveTexture(GL_TEXTURE0+i);
        
        // Bind GL_TEXTURE_2D to the current texture's handle:
		glBindTexture(GL_TEXTURE_2D,textures[i].id);
        
        // Look for a uniform sampler that matches the current iteration
        // and initialize it with the index of the texture unit.
        // GL_TEXTURE0 is index 0.
        int addr = glGetUniformLocation(programId,(str+(char)('0'+i)).c_str());
		
        //glUniform...
        glUniform1i(addr,i);
	}
	
	glBindBuffer(GL_ARRAY_BUFFER,VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,IND);
	
	if (vertex != -1)
	{
		glEnableVertexAttribArray(vertex);
		glVertexAttribPointer(vertex,3,GL_FLOAT,GL_FALSE,sizeof(vertexData),0);
	}
	
	if (normal != -1)
	{
		glEnableVertexAttribArray(normal);
		glVertexAttribPointer(normal,3,GL_FLOAT,GL_FALSE,sizeof(vertexData),(void*)(3*sizeof(float)));
	}
	
	if (tangent != -1)
	{
		glEnableVertexAttribArray(tangent);
		glVertexAttribPointer(tangent,3,GL_FLOAT,GL_FALSE,sizeof(vertexData),(void*)(6*sizeof(float)));
	}
	
	if (color != -1)
	{
		glEnableVertexAttribArray(color);
		glVertexAttribPointer(color,3,GL_FLOAT,GL_FALSE,sizeof(vertexData),(void*)(9*sizeof(float)));
	}
	
	if (UV != -1)
	{
		glEnableVertexAttribArray(UV);
		glVertexAttribPointer(UV,2,GL_FLOAT,GL_FALSE,sizeof(vertexData),(void*)(12*sizeof(float)));
	}
    
	if (UV2 != -1)
	{
		glEnableVertexAttribArray(UV2);
		glVertexAttribPointer(UV2,2,GL_FLOAT,GL_FALSE,sizeof(vertexData),(void*)(14*sizeof(float)));
	}
	
	glDrawElements(GL_TRIANGLES,indices.size(),GL_UNSIGNED_INT,0);
	
	if (vertex!=-1) glDisableVertexAttribArray(vertex);
	if (normal!=-1) glDisableVertexAttribArray(normal);
	if (tangent!=-1) glDisableVertexAttribArray(tangent);
	if (color!=-1) glDisableVertexAttribArray(color);
	if (UV!=-1) glDisableVertexAttribArray(UV);
	if (UV2!=-1) glDisableVertexAttribArray(UV2);
	glBindBuffer(GL_ARRAY_BUFFER,0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
}
