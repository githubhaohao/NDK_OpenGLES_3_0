//
// Created by ByteFlow on 2019/7/16.
//

#ifndef NDK_OPENGLES_3_0_TRANS_FEEDBACK_SAMPLE_H
#define NDK_OPENGLES_3_0_TRANS_FEEDBACK_SAMPLE_H


#include "GLSampleBase.h"
#include "../util/ImageDef.h"

class TransformFeedbackSample : public GLSampleBase
{
public:
	TransformFeedbackSample();

	virtual ~TransformFeedbackSample();

	virtual void LoadImage(NativeImage *pImage);

	virtual void Init();
	virtual void Draw(int screenW, int screenH);

	virtual void Destroy();

private:
	GLuint m_ImageTextureId;
	GLuint m_VaoId;
	GLuint m_VboIds[2] = {GL_NONE};;
	GLint m_SamplerLoc;
	GLuint m_TransFeedbackObjId;
	GLuint m_TransFeedbackBufId;
	NativeImage m_RenderImage;

};


#endif //NDK_OPENGLES_3_0_TRANS_FEEDBACK_SAMPLE_H
