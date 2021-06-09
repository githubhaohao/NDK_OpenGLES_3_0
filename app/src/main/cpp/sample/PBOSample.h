//
// Created by ByteFlow on 2019/12/26.
//

#ifndef NDK_OPENGLES_3_0_PBOSAMPLE_H
#define NDK_OPENGLES_3_0_PBOSAMPLE_H


#include <detail/type_mat.hpp>
#include "GLSampleBase.h"
#include "../util/ImageDef.h"

class PBOSample : public GLSampleBase
{
public:
	PBOSample();

	virtual ~PBOSample();

	virtual void LoadImage(NativeImage *pImage);

	virtual void Init();
	virtual void Draw(int screenW, int screenH);

	virtual void Destroy();

    void UpdateMVPMatrix(glm::mat4 &mvpMatrix, int angleX, int angleY, float ratio);

	bool CreateFrameBufferObj();

	void UploadPixels();

	void DownloadPixels();

private:
	GLuint m_ImageTextureId;
	GLuint m_FboTextureId;
	GLuint m_FboId;
	GLuint m_VaoIds[2] = {GL_NONE};
	GLuint m_VboIds[4] = {GL_NONE};;
	GLint m_SamplerLoc;
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

    GLuint m_UploadPboIds[2] = {GL_NONE};;
    GLuint m_DownloadPboIds[2] = {GL_NONE};;
    int m_FrameIndex;

};


#endif //NDK_OPENGLES_3_0_PBOSAMPLE_H
