/**
 *
 * Created by 公众号：字节流动 on 2022/4/18.
 * https://github.com/githubhaohao/NDK_OpenGLES_3_0
 * 最新文章首发于公众号：字节流动，有疑问或者技术交流可以添加微信 Byte-Flow ,领取视频教程, 拉你进技术交流群
 *
 * */

#ifndef NDK_OPENGLES_3_0_VAOSAMPLE_H
#define NDK_OPENGLES_3_0_VAOSAMPLE_H


#include "GLSampleBase.h"

class VaoSample : public GLSampleBase
{
public:
	VaoSample();

	virtual ~VaoSample();

	virtual void LoadImage(NativeImage *pImage);

	virtual void Init();
	virtual void Draw(int screenW, int screenH);
	virtual void Destroy();

private:
	GLuint  m_VaoId;
	GLuint  m_VboIds[2];
};


#endif //NDK_OPENGLES_3_0_VAOSAMPLE_H
