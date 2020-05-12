//
// Created by ByteFlow on 2019/7/10.
//

#ifndef NDK_OPENGLES_3_0_TEXTUREMAPSAMPLE_H
#define NDK_OPENGLES_3_0_TEXTUREMAPSAMPLE_H


#include "GLSampleBase.h"
#include "../util/ImageDef.h"

class TextureMapSample : public GLSampleBase
{
public:
	TextureMapSample();

	virtual ~TextureMapSample();

	void LoadImage(NativeImage *pImage);

	virtual void Init();

	virtual void Draw(int screenW, int screenH);

	virtual void Destroy();

private:
	GLuint m_TextureId;
	GLint m_SamplerLoc;
	NativeImage m_RenderImage;
};


#endif //NDK_OPENGLES_3_0_TEXTUREMAPSAMPLE_H
