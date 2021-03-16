/**
 *
 * Created by 公众号：字节流动 on 2020/4/18.
 * https://github.com/githubhaohao/NDK_OpenGLES_3_0
 * 最新文章首发于公众号：字节流动，有疑问或者技术交流可以添加微信 Byte-Flow ,领取视频教程, 拉你进技术交流群
 *
 * */

#ifndef NDK_OPENGLES_3_0_BIGHEAD_SAMPLE_H
#define NDK_OPENGLES_3_0_BIGHEAD_SAMPLE_H


#include <detail/type_mat.hpp>
#include <detail/type_mat4x4.hpp>
#include <ByteFlowLock.h>
#include <CommonDef.h>
#include "GLSampleBase.h"

using namespace glm;

#define TRIANGLE_COUNT           32
#define KEY_POINTS_COUNT         9

class BigHeadSample : public GLSampleBase
{
public:
	BigHeadSample();

	virtual ~BigHeadSample();

	virtual void LoadImage(NativeImage *pImage);

	virtual void Init();
	virtual void Draw(int screenW, int screenH);

	virtual void Destroy();

	virtual void UpdateTransformMatrix(float rotateX, float rotateY, float scaleX, float scaleY);

	void UpdateMVPMatrix(glm::mat4 &mvpMatrix, int angleX, int angleY, float ratio);

	void CalculateMesh(float warpLevel);

	vec2 CalculateIntersection(vec2 inputPoint, vec2 centerPoint);

	vec2 WarpKeyPoint(vec2 input, vec2 centerPoint, float level);

private:
	GLuint m_TextureId;
	GLint m_SamplerLoc;
	GLint m_MVPMatLoc;
	NativeImage m_RenderImage;
	mat4 m_MVPMatrix;

	int m_AngleX;
	int m_AngleY;
	float m_ScaleX;
	float m_ScaleY;

	int m_FrameIndex;

	vec3 m_Vertices[TRIANGLE_COUNT * 3]; //28个三角形
    vec2 m_TexCoords[TRIANGLE_COUNT * 3]; //28个三角形
	vec2 m_MeshPoints[24];
	vec2 m_KeyPointsInts[KEY_POINTS_COUNT]; //关键点对应的交点
    vec2 m_KeyPoints[KEY_POINTS_COUNT]; //关键点归一化
    GLuint m_VaoId;
    GLuint m_VboIds[2];

};


#endif //NDK_OPENGLES_3_0_BIGHEAD_SAMPLE_H
