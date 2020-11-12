//
// Created by ByteFlow on 2021/7/30.
//

#ifndef NDK_OPENGLES_3_0_TEXTUREBUFFERSAMPLE_H
#define NDK_OPENGLES_3_0_TEXTUREBUFFERSAMPLE_H


#include <detail/type_mat.hpp>
#include <detail/type_mat4x4.hpp>
#include <GLES3/gl32.h>
#include "GLSampleBase.h"

class TextureBufferSample : public GLSampleBase
{
public:
	TextureBufferSample();

	virtual ~TextureBufferSample();

	virtual void LoadImage(NativeImage *pImage);

	virtual void Init();
	virtual void Draw(int screenW, int screenH);

	virtual void Destroy();

	virtual void UpdateTransformMatrix(float rotateX, float rotateY, float scaleX, float scaleY);

	void UpdateMVPMatrix(glm::mat4 &mvpMatrix, int angleX, int angleY, float ratio);

private:
	GLuint m_TextureId;
	GLint m_SamplerLoc;
	GLint m_MVPMatLoc;
	GLuint m_VaoId;
	GLuint m_VboIds[3];
	GLuint m_TboId;
	GLuint m_TboTexId;
	NativeImage m_RenderImage;
	glm::mat4 m_MVPMatrix;

	int m_AngleX;
	int m_AngleY;
	float m_ScaleX;
	float m_ScaleY;

};


#endif //NDK_OPENGLES_3_0_TEXTUREBUFFERSAMPLE_H
