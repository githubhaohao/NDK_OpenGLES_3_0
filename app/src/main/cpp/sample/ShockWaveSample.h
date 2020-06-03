//
// Created by ByteFlow on 2019/7/30.
//

#ifndef NDK_OPENGLES_3_0_SHOCKWAVESAMPLE_H
#define NDK_OPENGLES_3_0_SHOCKWAVESAMPLE_H


#include <detail/type_mat.hpp>
#include <detail/type_mat4x4.hpp>
#include "GLSampleBase.h"

using namespace glm;

class ShockWaveSample : public GLSampleBase
{
public:
	ShockWaveSample();

	virtual ~ShockWaveSample();

	virtual void LoadImage(NativeImage *pImage);

	virtual void Init();
	virtual void Draw(int screenW, int screenH);

	virtual void Destroy();

	virtual void UpdateTransformMatrix(float rotateX, float rotateY, float scaleX, float scaleY);

	void UpdateMVPMatrix(glm::mat4 &mvpMatrix, int angleX, int angleY, float ratio);

	virtual void SetTouchLocation(float x, float y);


private:
	GLuint m_TextureId;
	GLuint m_VaoId;
	GLuint m_VboIds[3];
	NativeImage m_RenderImage;
	glm::mat4 m_MVPMatrix;

	int m_AngleX;
	int m_AngleY;
	float m_ScaleX;
	float m_ScaleY;
	int m_FrameIndex;

	vec2 m_touchXY;

};


#endif //NDK_OPENGLES_3_0_SHOCKWAVESAMPLE_H
