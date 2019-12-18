//
// Created by ByteFlow on 2019/7/9.
//

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
