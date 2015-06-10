#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

#include "Mesh.h"

mesh::mesh(std::vector<vertexData>* vd,std::vector<unsigned int>* id)
{
	data=*vd;
	indices=*id;
	
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

void mesh::draw(unsigned int programId)
{
	//attribute vec3 vertex
	int vertex=glGetAttribLocation(programId,"vertex"); //0
	int normal=glGetAttribLocation(programId,"normal"); //1
	int tangent=glGetAttribLocation(programId,"tangent"); //2
	int color=glGetAttribLocation(programId,"color"); //3
	
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
	
	
	glDrawElements(GL_TRIANGLES,indices.size(),GL_UNSIGNED_INT,0);
	
	if (vertex!=-1) glDisableVertexAttribArray(vertex);
	if (normal!=-1) glDisableVertexAttribArray(normal);
	if (tangent!=-1) glDisableVertexAttribArray(tangent);
	if (color!=-1) glDisableVertexAttribArray(color);
	glBindBuffer(GL_ARRAY_BUFFER,0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);	
}
