//
// Created by ByteFlow on 2019/7/16.
//

#ifndef NDK_OPENGLES_3_0_FBOLEGSAMPLE_H
#define NDK_OPENGLES_3_0_FBOLEGSAMPLE_H

#include <detail/type_mat.hpp>
#include <detail/type_mat4x4.hpp>
#include "GLSampleBase.h"
#include "../util/ImageDef.h"

#define VERTICAL_STRETCH_8_POINTS          0x10
#define VERTICAL_STRETCH_TOP_6_POINTS      0x11
#define VERTICAL_STRETCH_BOTTOM_6_POINTS   0x12
#define VERTICAL_STRETCH_4_POINTS          0x13

#define HORIZONTAL_STRETCH_8_POINTS        0x20
#define HORIZONTAL_STRETCH_LEFT_6_POINTS   0x21
#define HORIZONTAL_STRETCH_RIGHT_6_POINTS  0x22
#define HORIZONTAL_STRETCH_4_POINTS        0x23

class FBOLegLengthenSample : public GLSampleBase
{
public:
	FBOLegLengthenSample();

	virtual ~FBOLegLengthenSample();

	virtual void LoadImage(NativeImage *pImage);

	virtual void Init();
	virtual void Draw(int screenW, int screenH);

	virtual void Destroy();

	bool CreateFrameBufferObj();

private:
	GLuint m_ImageTextureId;
	GLuint m_FboTextureId;
	GLuint m_FboId;
	GLuint m_VaoIds[2] = {GL_NONE};;
	GLuint m_VboIds[6] = {GL_NONE};;
	GLint m_SamplerLoc;
	NativeImage m_RenderImage;
	GLuint m_FboProgramObj;
	GLuint m_FboVertexShader;
	GLuint m_FboFragmentShader;
	GLint m_FboSamplerLoc;

	glm::mat4 m_FboMVPMatrix;
	GLint m_FboMVPMatLoc;

	glm::mat4 m_MVPMatrix;
	GLint m_MVPMatLoc;

	GLfloat m_dt;
	bool    m_isgo;

	RectF m_StretchRect;
	int   m_StretchMode;

	bool  m_bIsVerticalMode;
};


#endif //NDK_OPENGLES_3_0_FBOLEGSAMPLE_H
