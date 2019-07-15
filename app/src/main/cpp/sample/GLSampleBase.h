//
// Created by ByteFlow on 2019/7/9.
//

#ifndef NDK_OPENGLES_3_0_GLSAMPLEBASE_H
#define NDK_OPENGLES_3_0_GLSAMPLEBASE_H


#include "stdint.h"
#include <GLES3/gl3.h>
#include <ImageDef.h>

class GLSampleBase
{
public:
	GLSampleBase()
	{
		m_ProgramObj = 0;
		m_VertexShader = 0;
		m_FragmentShader = 0;
	}

	virtual ~GLSampleBase()
	{

	}

	virtual void LoadImage(NativeImage *pImage) = 0;

	virtual void Init() = 0;
	virtual void Draw() = 0;


protected:
	GLuint m_VertexShader;
	GLuint m_FragmentShader;
	GLuint m_ProgramObj;
};


#endif //NDK_OPENGLES_3_0_GLSAMPLEBASE_H
