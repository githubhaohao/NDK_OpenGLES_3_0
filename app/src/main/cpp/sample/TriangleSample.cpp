//
// Created by ByteFlow on 2019/7/9.
//

#include "TriangleSample.h"
#include "../util/GLUtils.h"
#include "../util/LogUtil.h"

TriangleSample::TriangleSample()
{

}

TriangleSample::~TriangleSample()
{
}


void TriangleSample::LoadImage(NativeImage *pImage)
{
	//null implement

}

void TriangleSample::Init()
{
	if(m_ProgramObj != 0)
		return;
	char vShaderStr[] =
			"#version 300 es                          \n"
			"layout(location = 0) in vec4 vPosition;  \n"
			"void main()                              \n"
			"{                                        \n"
			"   gl_Position = vPosition;              \n"
			"}                                        \n";

	char fShaderStr[] =
			"#version 300 es                              \n"
			"precision mediump float;                     \n"
			"out vec4 fragColor;                          \n"
			"void main()                                  \n"
			"{                                            \n"
			"   fragColor = vec4 ( 1.0, 0.0, 0.0, 1.0 );  \n"
			"}                                            \n";

	m_ProgramObj = GLUtils::CreateProgram(vShaderStr, fShaderStr, m_VertexShader, m_FragmentShader);

}

void TriangleSample::Draw(int screenW, int screenH)
{
	LOGCATE("TriangleSample::Draw");
	GLfloat vVertices[] = {
			0.0f,  0.5f, 0.0f,
			-0.5f, -0.5f, 0.0f,
			0.5f, -0.5f, 0.0f,
	};

	if(m_ProgramObj == 0)
		return;

	glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(1.0, 1.0, 1.0, 1.0);

	// Use the program object
	glUseProgram (m_ProgramObj);

	// Load the vertex data
	glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, vVertices );
	glEnableVertexAttribArray (0);

	glDrawArrays (GL_TRIANGLES, 0, 3);
	glUseProgram (GL_NONE);

}

void TriangleSample::Destroy()
{
	if (m_ProgramObj)
	{
		glDeleteProgram(m_ProgramObj);
		m_ProgramObj = GL_NONE;
	}

}
