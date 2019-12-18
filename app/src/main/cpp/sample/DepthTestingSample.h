//
// Created by ByteFlow on 2019/7/30.
//

#ifndef NDK_OPENGLES_3_0_DEPTH_TESTING_H
#define NDK_OPENGLES_3_0_DEPTH_TESTING_H


#include <detail/type_mat.hpp>
#include <detail/type_mat4x4.hpp>
#include "GLSampleBase.h"

class DepthTestingSample : public GLSampleBase
{
public:
	DepthTestingSample();

	virtual ~DepthTestingSample();

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

	GLuint m_VaoId;
	GLuint m_VboIds[1];
	NativeImage m_RenderImage;
	glm::mat4 m_MVPMatrix;
	glm::mat4 m_ModelMatrix;

	int m_AngleX;
	int m_AngleY;
};


#endif //NDK_OPENGLES_3_0_DEPTH_TESTING_H
