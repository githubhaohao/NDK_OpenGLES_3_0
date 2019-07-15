//
// Created by ByteFlow on 2019/7/9.
//

#ifndef NDK_OPENGLES_3_0_TRIANGLESAMPLE_H
#define NDK_OPENGLES_3_0_TRIANGLESAMPLE_H


#include "GLSampleBase.h"

class TriangleSample : public GLSampleBase
{
public:
	TriangleSample();
	~TriangleSample();

	virtual void LoadImage(NativeImage *pImage);

	virtual void Init();

	virtual void Draw();
};


#endif //NDK_OPENGLES_3_0_TRIANGLESAMPLE_H
