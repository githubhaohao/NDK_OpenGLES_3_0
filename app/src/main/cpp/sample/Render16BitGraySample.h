/**
 *
 * Created by 公众号：字节流动 on 2022/3/12.
 * https://github.com/githubhaohao/NDK_OpenGLES_3_0
 * 最新文章首发于公众号：字节流动，有疑问或者技术交流可以添加微信 Byte-Flow ,领取视频教程, 拉你进技术交流群
 *
 * */

#ifndef NDK_OPENGLES_3_0_RENDER_16Bit_GRAY_SAMPLE_H
#define NDK_OPENGLES_3_0_RENDER_16Bit_GRAY_SAMPLE_H


#include "GLSampleBase.h"
#include "../util/ImageDef.h"

class Render16BitGraySample: public GLSampleBase
{
public:
	Render16BitGraySample()
	{
		m_yTextureId = GL_NONE;

		m_ySamplerLoc = GL_NONE;

	}

	virtual ~Render16BitGraySample()
	{
		NativeImageUtil::FreeNativeImage(&m_RenderImage);
	}

	virtual void LoadImage(NativeImage *pImage);

	virtual void Init();

	virtual void Draw(int screenW, int screenH);

	virtual void Destroy();

private:
	GLuint m_yTextureId;

	GLint m_ySamplerLoc;

	NativeImage m_RenderImage;

};


#endif //NDK_OPENGLES_3_0_RENDER_16Bit_GRAY_SAMPLE_H
