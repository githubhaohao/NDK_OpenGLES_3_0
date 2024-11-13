/**
 *
 * Created by 公众号：字节流动 on 2022/4/18.
 * https://github.com/githubhaohao/NDK_OpenGLES_3_0
 * 最新文章首发于公众号：字节流动，有疑问或者技术交流可以添加微信 Byte-Flow ,领取视频教程, 拉你进技术交流群
 *
 * */

#ifndef NDK_OPENGLES_3_0_INSTANCINGSAMPLE_H
#define NDK_OPENGLES_3_0_INSTANCINGSAMPLE_H


#include <detail/type_mat.hpp>
#include "GLSampleBase.h"
#include "../util/ImageDef.h"

class InstancingSample : public GLSampleBase
{
public:
	InstancingSample();
	virtual ~InstancingSample();

	virtual void Init();

	virtual void Draw(int screenW, int screenH);

	virtual void Destroy();

private:
	GLuint m_InstanceVbo;
	GLuint m_VerticesVbo;
	GLuint m_Vao;
};


#endif //NDK_OPENGLES_3_0_INSTANCINGSAMPLE_H
