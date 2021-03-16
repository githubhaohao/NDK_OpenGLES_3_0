/**
 *
 * Created by 公众号：字节流动 on 2020/4/18.
 * https://github.com/githubhaohao/NDK_OpenGLES_3_0
 * 最新文章首发于公众号：字节流动，有疑问或者技术交流可以添加微信 Byte-Flow ,领取视频教程, 拉你进技术交流群
 *
 * */

#ifndef NDK_OPENGLES_3_0_BASIC_LIGHTING_H
#define NDK_OPENGLES_3_0_BASIC_LIGHTING_H


#include <detail/type_mat.hpp>
#include <detail/type_mat4x4.hpp>
#include "GLSampleBase.h"

class BasicLightingSample : public GLSampleBase
{
public:
	BasicLightingSample();

	virtual ~BasicLightingSample();

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
	GLint m_ModelMatrixLoc;
	GLint m_LightPosLoc;
	GLint m_LightColorLoc;
	GLint m_ViewPosLoc;

	GLuint m_VaoId;
	GLuint m_VboIds[1];
	GLuint m_TfoId;
	GLuint m_TfoBufId;
	NativeImage m_RenderImage;
	glm::mat4 m_MVPMatrix;
	glm::mat4 m_ModelMatrix;

	int m_AngleX;
	int m_AngleY;

	float m_ScaleX;
	float m_ScaleY;
};


#endif //NDK_OPENGLES_3_0_BASIC_LIGHTING_H
