//
// Created by chh7563 on 2019/7/30.
//

#ifndef NDK_OPENGLES_3_0_BASIC_LIGHTING_H
#define NDK_OPENGLES_3_0_BASIC_LIGHTING_H


#include <detail/type_mat.hpp>
#include <detail/type_mat4x4.hpp>
#include "GLSampleBase.h"

class BasicLightingSample : public GLSampleBase
{
public:
	BasicLightingSample();

	~BasicLightingSample();

	virtual void LoadImage(NativeImage *pImage);

	virtual void Init();
	virtual void Draw(int screenW, int screenH);

	virtual void Destroy();

	virtual void SetParamsInt(int paramType, int value0, int value1);

	void UpdateMVPMatrix(glm::mat4 &mvpMatrix, int angleX, int angleY, float ratio);

private:
	GLuint m_TextureId;
	GLint m_SamplerLoc;
	GLint m_MVPMatLoc;
	GLuint m_VaoId;
	GLuint m_VboIds[1];
	NativeImage m_RenderImage;
	glm::mat4 m_MVPMatrix;

	int m_AngleX;
	int m_AngleY;

};


#endif //NDK_OPENGLES_3_0_BASIC_LIGHTING_H
