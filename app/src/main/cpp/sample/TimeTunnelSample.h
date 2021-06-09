//
// Created by ByteFlow on 2020/01/08.
//

#ifndef NDK_OPENGLES_3_0_TIMETUNNEL_SAMPLE_H
#define NDK_OPENGLES_3_0_TIMETUNNEL_SAMPLE_H


#include <detail/type_mat.hpp>
#include "GLSampleBase.h"
#include "../util/ImageDef.h"

class TimeTunnelSample : public GLSampleBase
{
public:
	TimeTunnelSample();

	virtual ~TimeTunnelSample();

	virtual void LoadImage(NativeImage *pImage);

	virtual void Init();
	virtual void Draw(int screenW, int screenH);

	virtual void Destroy();

    void UpdateMVPMatrix(glm::mat4 &mvpMatrix, int angleX, int angleY, float ratio);

	bool CreateFrameBufferObj();

private:
	GLuint m_ImageTextureId;
	GLuint m_FboTextureId;
	GLuint m_FboId;
	GLuint m_VaoIds[2] = {GL_NONE};;
	GLuint m_VboIds[4] = {GL_NONE};;
	GLint m_SamplerLoc;
	GLint m_ImgSizeLoc;
	GLint m_TimeLoc;
	NativeImage m_RenderImage;
	GLuint m_FboProgramObj;
	GLuint m_FboVertexShader;
	GLuint m_FboFragmentShader;
	GLint m_FboSamplerLoc;
	GLint m_MVPMatrixLoc;

    int m_AngleX;
    int m_AngleY;
    float m_ScaleX;
    float m_ScaleY;
    glm::mat4 m_MVPMatrix;

    int m_FrameIndex;

};


#endif //NDK_OPENGLES_3_0_TIMETUNNEL_SAMPLE_H
