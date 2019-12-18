//
// Created by ByteFlow on 2019/7/30.
//

#ifndef NDK_OPENGLES_3_0_NOISE_3D_SAMPLE_H
#define NDK_OPENGLES_3_0_NOISE_3D_SAMPLE_H


#include <detail/type_mat.hpp>
#include <detail/type_mat4x4.hpp>
#include "GLSampleBase.h"

typedef struct _tag_VERTEX_DATA
{
	GLfloat *pVertices;
	GLfloat *pTexCoords;
	GLuint  *pIndices;
	int      indicesNum;
	_tag_VERTEX_DATA()
	{
		pVertices = nullptr;
		pTexCoords = nullptr;
		pIndices = nullptr;
		indicesNum = 0;
	}

} VERTEX_DATA;

class Noise3DSample : public GLSampleBase
{
public:
	Noise3DSample();

	virtual ~Noise3DSample();

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
	GLint m_FogMinDistLoc;
	GLint m_FogMaxDistLoc;
	GLint m_TimeLoc;
	GLint m_FogColorLoc;

	NativeImage m_RenderImage;
	glm::mat4 m_MVPMatrix;
	glm::mat4 m_ModelMatrix;

	int m_AngleX;
	int m_AngleY;

	float m_ScaleX;
	float m_ScaleY;

	float m_CurTime;

	VERTEX_DATA m_VertexData;
};


#endif //NDK_OPENGLES_3_0_NOISE_3D_SAMPLE_H
