/**
 *
 * Created by 公众号：字节流动 on 2023/3/12.
 * https://github.com/githubhaohao/NDK_OpenGLES_3_0
 * 最新文章首发于公众号：字节流动，有疑问或者技术交流可以添加微信 Byte-Flow ,领取视频教程, 拉你进技术交流群
 *
 * */

#ifndef NDK_OPENGLES_3_0_YUVTEXTUREMAPSAMPLE_H
#define NDK_OPENGLES_3_0_YUVTEXTUREMAPSAMPLE_H


#include "GLSampleBase.h"
#include "../util/ImageDef.h"

class NV21TextureMapSample: public GLSampleBase
{
public:
	NV21TextureMapSample()
	{
		m_yTextureId = GL_NONE;
		m_uvTextureId = GL_NONE;

		m_ySamplerLoc = GL_NONE;
		m_uvSamplerLoc = GL_NONE;

	}

	virtual ~NV21TextureMapSample()
	{
		NativeImageUtil::FreeNativeImage(&m_RenderImage);
	}

	virtual void LoadImage(NativeImage *pImage);

	virtual void Init();

	virtual void Draw(int screenW, int screenH);

	virtual void Destroy();

private:
	GLuint m_yTextureId;
	GLuint m_uvTextureId;

	GLint m_ySamplerLoc;
	GLint m_uvSamplerLoc;

	NativeImage m_RenderImage;

};


#endif //NDK_OPENGLES_3_0_YUVTEXTUREMAPSAMPLE_H
