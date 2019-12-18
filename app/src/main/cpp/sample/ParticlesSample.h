//
// Created by ByteFlow on 2019/10/10.
//

#ifndef NDK_OPENGLES_3_0_PARTICLES_H
#define NDK_OPENGLES_3_0_PARTICLES_H

#include <detail/type_mat.hpp>
#include <detail/type_mat4x4.hpp>
#include "GLSampleBase.h"

#define MAX_PARTICLES 500

struct Particle {
	GLfloat dx,dy,dz;//offset
	GLfloat dxSpeed,dySpeed,dzSpeed;//offset
	GLubyte r,g,b,a; //r,g,b,a
	GLfloat life;
	GLfloat cameraDistance; // no use
	Particle()
	{
		dx = 0.0f;
		dy = 0.0f;
		dz = 0.0f;

		r = static_cast<GLubyte>(1.0f);
		g = static_cast<GLubyte>(1.0f);
		b = static_cast<GLubyte>(1.0f);
		a = static_cast<GLubyte>(1.0f);

		dxSpeed = 1.0f;
		dySpeed = 1.0f;
		dzSpeed = 1.0f;

		life = 5.0f;
	}

	bool operator<(const Particle& that) const {
		// Sort in reverse order : far particles drawn first.
		return this->cameraDistance > that.cameraDistance;
	}
};

class ParticlesSample : public GLSampleBase
{
public:

	ParticlesSample();

	virtual ~ParticlesSample();
	virtual void LoadImage(NativeImage *pImage);
	virtual void Init();
	virtual void Draw(int screenW, int screenH);

	virtual void Destroy();

	virtual void UpdateTransformMatrix(float rotateX, float rotateY, float scaleX, float scaleY);

	void UpdateMVPMatrix(glm::mat4 &mvpMatrix, int angleX, int angleY, float ratio);

	int FindUnusedParticle();

	void SortParticles();

	int UpdateParticles();

	void GenerateNewParticle(Particle &particle);

private:
	GLuint m_TextureId;
	GLint m_SamplerLoc;
	GLint m_MVPMatLoc;

	GLuint m_VaoId;
	GLuint m_ParticlesVertexVboId;
	GLuint m_ParticlesPosVboId;
	GLuint m_ParticlesColorVboId;

	NativeImage m_RenderImage;
	glm::mat4 m_MVPMatrix;

	// particles relation
	Particle m_ParticlesContainer[MAX_PARTICLES];
	GLfloat* m_pParticlesPosData;
	GLubyte* m_pParticlesColorData;
	int m_LastUsedParticle;

	int m_AngleX;
	int m_AngleY;

	float m_ScaleX;
	float m_ScaleY;
};


#endif //NDK_OPENGLES_3_0_PARTICLES_H
