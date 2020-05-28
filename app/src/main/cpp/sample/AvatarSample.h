//
// Created by ByteFlow on 2020/7/30.
//

#ifndef NDK_OPENGLES_3_0_AVATARSAMPLE_H
#define NDK_OPENGLES_3_0_AVATARSAMPLE_H


#include <detail/type_mat.hpp>
#include <detail/type_mat4x4.hpp>
#include "GLSampleBase.h"

#define RENDER_IMG_NUM 3

class AvatarSample : public GLSampleBase
{
public:
	AvatarSample();

	virtual ~AvatarSample();

	virtual void LoadImage(NativeImage *pImage);

    virtual void LoadMultiImageWithIndex(int index, NativeImage *pImage);

	virtual void Init();
	virtual void Draw(int screenW, int screenH);

	virtual void Destroy();

	virtual void UpdateTransformMatrix(float rotateX, float rotateY, float scaleX, float scaleY);

	virtual void SetGravityXY(float x, float y);

	void UpdateMVPMatrix(glm::mat4 &mvpMatrix, int angleX, int angleY, float transX, float transY, float ratio);


private:
	GLuint m_TextureIds[RENDER_IMG_NUM];
	GLint m_SamplerLoc;
	GLint m_MVPMatLoc;
	GLuint m_VaoId;
	GLuint m_VboIds[3];
	NativeImage m_RenderImages[RENDER_IMG_NUM];
	glm::mat4 m_MVPMatrix;

	int m_AngleX;
	int m_AngleY;
	float m_ScaleX;
	float m_ScaleY;
	float m_TransX;
	float m_TransY;

	int m_FrameIndex;

	GLuint m_BlurProgramObj;

};


#endif //NDK_OPENGLES_3_0_AVATARSAMPLE_H
