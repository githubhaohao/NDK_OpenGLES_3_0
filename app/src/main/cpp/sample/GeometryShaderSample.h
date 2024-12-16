/**
 *
 * Created by 公众号：字节流动 on 2024/12/02.
 * https://github.com/githubhaohao/NDK_OpenGLES_3_0
 * 最新文章首发于公众号：字节流动，有疑问或者技术交流可以添加微信 Byte-Flow ,领取视频教程, 拉你进技术交流群
 *
 * */
#ifndef OPENGLES_3_0_GEOMETRYSHADER_SAMPLE_H
#define OPENGLES_3_0_GEOMETRYSHADER_SAMPLE_H


#include "GLSampleBase.h"
#include "../util/ImageDef.h"

class GeometryShaderSample : public GLSampleBase
{
public:
	GeometryShaderSample();

	virtual ~GeometryShaderSample();

	void LoadImage(NativeImage *pImage);

	virtual void Init();

	virtual void Draw(int screenW, int screenH);

	virtual void Destroy();

private:
	GLuint m_TextureId;
    GLuint m_VaoId;
    GLuint m_VboId;
	NativeImage m_RenderImage;
    int m_FrameIndex;
};


#endif //OPENGLES_3_0_GEOMETRYSHADER_SAMPLE_H
