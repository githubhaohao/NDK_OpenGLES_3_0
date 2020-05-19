//
// Created by ByteFlow on 2019/7/30.
//

#ifndef NDK_OPENGLES_3_0_VISUALIZEAUDIOSAMPLE_H
#define NDK_OPENGLES_3_0_VISUALIZEAUDIOSAMPLE_H

#define MAX_AUDIO_LEVEL 2000
#define RESAMPLE_LEVEL  112

#include <detail/type_mat.hpp>
#include <detail/type_mat4x4.hpp>
#include <mutex>
#include "GLSampleBase.h"

using namespace glm;

class VisualizeAudioSample : public GLSampleBase
{
public:
	VisualizeAudioSample();

	virtual ~VisualizeAudioSample();

	virtual void LoadImage(NativeImage *pImage);

    virtual void LoadShortArrData(short *const pShortArr, int arrSize);

    virtual void Init();
	virtual void Draw(int screenW, int screenH);

	virtual void Destroy();

	virtual void UpdateTransformMatrix(float rotateX, float rotateY, float scaleX, float scaleY);

	void UpdateMVPMatrix(glm::mat4 &mvpMatrix, int angleX, int angleY, float ratio);

	void UpdateMesh();

private:
	GLint m_SamplerLoc;
	GLint m_MVPMatLoc;
	GLuint m_VaoId;
	GLuint m_VboIds[2];
	glm::mat4 m_MVPMatrix;

	int m_AngleX;
	int m_AngleY;
	float m_ScaleX;
	float m_ScaleY;

	short *m_pCurAudioData;
	short *m_pAudioBuffer;
	int m_AudioDataSize;
	std::mutex m_Mutex;
	std::condition_variable m_Cond;

	vec3 *m_pVerticesCoords;
	vec2 *m_pTextureCoords;
	int m_RenderDataSize;

	int m_FrameIndex;
	volatile bool m_bAudioDataReady;

};


#endif //NDK_OPENGLES_3_0_COORDSYSTEMSAMPLE_H
