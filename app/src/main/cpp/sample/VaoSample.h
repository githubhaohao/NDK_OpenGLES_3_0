//
// Created by ByteFlow on 2019/7/15.
//

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
