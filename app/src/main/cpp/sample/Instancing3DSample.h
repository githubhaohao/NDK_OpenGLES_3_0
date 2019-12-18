//
// Created by ByteFlow on 2019/7/30.
//

#ifndef NDK_OPENGLES_3_0_INSTANCING3DSAMPLE_H
#define NDK_OPENGLES_3_0_INSTANCING3DSAMPLE_H


#include <detail/type_mat.hpp>
#include <detail/type_mat4x4.hpp>
#include "GLSampleBase.h"

class Instancing3DSample : public GLSampleBase
{
public:
	Instancing3DSample();

	virtual ~Instancing3DSample();

	virtual void LoadImage(NativeImage *pImage);

	virtual void Init();
	virtual void Draw(int screenW, int screenH);

	virtual void Destroy();

	virtual void UpdateTransformMatrix(float rotateX, float rotateY, float scaleX, float scaleY);

	void UpdateMVPMatrix(glm::mat4 &mvpMatrix, int angleX, int angleY, float ratio);

	void UpdateMatrix(glm::mat4 &mvpMatrix, glm::mat4 &modelMatrix, int angleXRotate, int angleYRotate, float scale, glm::vec3 transVec3, float ratio);

private:
	GLuint m_TextureId;
	GLint m_SamplerLoc;
	GLint m_MVPMatLoc;
	GLint m_ModelMatrixLoc;
	GLint m_ViewPosLoc;

	GLuint m_VaoId;
	GLuint m_VboIds[2];
	NativeImage m_RenderImage;
	glm::mat4 m_MVPMatrix;
	glm::mat4 m_ModelMatrix;

	int m_AngleX;
	int m_AngleY;

	float m_ScaleX;
	float m_ScaleY;
};


#endif //NDK_OPENGLES_3_0_INSTANCING3DSAMPLE_H
