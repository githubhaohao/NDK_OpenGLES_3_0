/**
 *
 * Created by 公众号：字节流动 on 2021/10/12.
 * https://github.com/githubhaohao/NDK_OpenGLES_3_0
 * 最新文章首发于公众号：字节流动，有疑问或者技术交流可以添加微信 Byte-Flow ,领取视频教程, 拉你进技术交流群
 *
 * */
#ifndef NDK_OPENGLES_3_0_FULLSCREENTRIANGLESAMPLE_H
#define NDK_OPENGLES_3_0_FULLSCREENTRIANGLESAMPLE_H


#include "GLSampleBase.h"
#include "../util/ImageDef.h"

class FullScreenTriangleSample : public GLSampleBase
{
public:
	FullScreenTriangleSample();

	virtual ~FullScreenTriangleSample();

	void LoadImage(NativeImage *pImage);

	virtual void Init();

	virtual void Draw(int screenW, int screenH);

	virtual void Destroy();

private:
	GLuint m_TextureId;
	GLint m_SamplerLoc;
	NativeImage m_RenderImage;
};


#endif //NDK_OPENGLES_3_0_FullScreenTriangleSample_H
