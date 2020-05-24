//
// Created by ByteFlow on 2020/5/19.
//

#ifndef NDK_OPENGLES_3_0_SCRATCHCARDSAMPLE_H
#define NDK_OPENGLES_3_0_SCRATCHCARDSAMPLE_H


#include <detail/type_mat.hpp>
#include <detail/type_mat4x4.hpp>
#include <vector>
#include "GLSampleBase.h"

using namespace glm;

#define TRIANGLE_NUM  43
#define EFFECT_RADIUS 0.03

class ScratchCardSample : public GLSampleBase
{
public:
	ScratchCardSample();

	virtual ~ScratchCardSample();

	virtual void LoadImage(NativeImage *pImage);

	virtual void Init();
	virtual void Draw(int screenW, int screenH);

	virtual void Destroy();

	virtual void UpdateTransformMatrix(float rotateX, float rotateY, float scaleX, float scaleY);

	virtual void SetTouchLocation(float x, float y);

	void UpdateMVPMatrix(mat4 &mvpMatrix, int angleX, int angleY, float ratio);

	void CalculateMesh(vec2 pre, vec2 cur);

private:
	GLuint m_TextureId;
	GLint m_SamplerLoc;
	GLint m_MVPMatLoc;
	GLuint m_VaoId;
	GLuint m_VboIds[2];
	NativeImage m_RenderImage;
	mat4 m_MVPMatrix;

	int m_AngleX;
	int m_AngleY;
	float m_ScaleX;
	float m_ScaleY;
	vec3 m_pVtxCoords[TRIANGLE_NUM * 3];
	vec2 m_pTexCoords[TRIANGLE_NUM * 3];

	std::vector<vec4> m_PointVector;

	vec2 m_CurTouchPoint;
	vec2 m_PreTouchPoint;
	bool m_bReset;

};


#endif //NDK_OPENGLES_3_0_SCRATCHCARDSAMPLE_H
