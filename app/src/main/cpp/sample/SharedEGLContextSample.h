/**
 *
 * Created by 公众号：字节流动 on 2021/3/12.
 * https://github.com/githubhaohao/NDK_OpenGLES_3_0
 * 最新文章首发于公众号：字节流动，有疑问或者技术交流可以添加微信 Byte-Flow ,领取视频教程, 拉你进技术交流群
 *
 * */

#ifndef NDK_OPENGLES_3_0_SHAREDEGLCTXSAMPLE_H
#define NDK_OPENGLES_3_0_SHAREDEGLCTXSAMPLE_H

#include "GLSampleBase.h"
#include "../util/ImageDef.h"
#include <thread>
#include <GLRenderLooper.h>

using namespace std;

class SharedEGLContextSample : public GLSampleBase
{
public:
	SharedEGLContextSample();

	virtual ~SharedEGLContextSample();

	virtual void LoadImage(NativeImage *pImage);

	virtual void Init();
	virtual void Draw(int screenW, int screenH);

	virtual void Destroy();

	static void OnAsyncRenderDone(void* callback, int fboTexId);

private:
	GLuint m_ImageTextureId;
	GLuint m_FboTextureId;
	GLuint m_VaoId;
	GLuint m_VboIds[4];
	NativeImage m_RenderImage;
	GLuint m_FboProgramObj;

	mutex m_Mutex;
	condition_variable m_Cond;
	GLEnv m_GLEnv;
};


#endif //NDK_OPENGLES_3_0_SHAREDEGLCTXSAMPLE_H
