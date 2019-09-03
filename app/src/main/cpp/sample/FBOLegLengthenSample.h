//
// Created by ByteFlow on 2019/7/16.
//

#ifndef NDK_OPENGLES_3_0_FBOLEGSAMPLE_H
#define NDK_OPENGLES_3_0_FBOLEGSAMPLE_H

#include <detail/type_mat.hpp>
#include <detail/type_mat4x4.hpp>
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
	GLuint m_VboIds[6];
	GLint m_SamplerLoc;
	NativeImage m_RenderImage;
	GLuint m_FboProgramObj;
	GLuint m_FboVertexShader;
	GLuint m_FboFragmentShader;
	GLint m_FboSamplerLoc;

	glm::mat4 m_MVPMatrix;
	GLint m_MVPMatLoc;

	GLfloat m_dt;
	bool    m_isgo;

	RectF m_StretchRect;

};


#endif //NDK_OPENGLES_3_0_FBOLEGSAMPLE_H
