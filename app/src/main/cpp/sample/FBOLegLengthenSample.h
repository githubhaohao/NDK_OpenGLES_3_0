//
// Created by ByteFlow on 2019/7/16.
//

#ifndef NDK_OPENGLES_3_0_FBOLEGSAMPLE_H
#define NDK_OPENGLES_3_0_FBOLEGSAMPLE_H


#include "GLSampleBase.h"
#include "../util/ImageDef.h"

class FBOLegLengthenSample : public GLSampleBase
{
public:
	FBOLegLengthenSample();

	~FBOLegLengthenSample();

	virtual void LoadImage(NativeImage *pImage);

	virtual void Init();
	virtual void Draw(int screenW, int screenH);

	virtual void Destroy();

	bool CreateFrameBufferObj();

private:
	GLuint m_ImageTextureId;
	GLuint m_FboTextureId;
	GLuint m_FboId;
	GLuint m_VaoIds[2];
	GLuint m_VboIds[4];
	GLint m_SamplerLoc;
	NativeImage m_RenderImage;
	GLuint m_FboProgramObj;
	GLuint m_FboVertexShader;
	GLuint m_FboFragmentShader;
	GLint m_FboSamplerLoc;

	GLfloat m_dt;
	bool    m_isgo;

};


#endif //NDK_OPENGLES_3_0_FBOLEGSAMPLE_H
