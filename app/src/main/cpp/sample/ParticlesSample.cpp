//
// Created by ByteFlow on 2019/7/30.
//

#include <gtc/matrix_transform.hpp>
#include "ParticlesSample.h"
#include <ctime>
#include <cstdlib>
#include <algorithm>

#include "../util/GLUtils.h"

ParticlesSample::ParticlesSample()
{

	m_SamplerLoc = GL_NONE;
	m_MVPMatLoc = GL_NONE;

	m_TextureId = GL_NONE;
	m_VaoId = GL_NONE;

	m_AngleX = 0;
	m_AngleY = 0;

	m_ScaleX = 1.0f;
	m_ScaleY = 1.0f;

	m_LastUsedParticle = 0;
	m_pParticlesPosData = nullptr;
	m_pParticlesColorData = nullptr;

}

ParticlesSample::~ParticlesSample()
{
	NativeImageUtil::FreeNativeImage(&m_RenderImage);
	if (m_pParticlesColorData)
	{
		delete[] m_pParticlesColorData;
	}

	if (m_pParticlesPosData)
	{
		delete[] m_pParticlesPosData;
	}

}

void ParticlesSample::LoadImage(NativeImage *pImage)
{
	LOGCATE("ParticlesSample::LoadImage pImage = %p", pImage->ppPlane[0]);
	if (pImage)
	{
		m_RenderImage.width = pImage->width;
		m_RenderImage.height = pImage->height;
		m_RenderImage.format = pImage->format;
		NativeImageUtil::CopyNativeImage(pImage, &m_RenderImage);
	}

}

void ParticlesSample::Init()
{
	if (m_ProgramObj)
	{
		return;
	}

	//create RGBA texture
	glGenTextures(1, &m_TextureId);
	glBindTexture(GL_TEXTURE_2D, m_TextureId);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_RenderImage.width, m_RenderImage.height, 0, GL_RGBA,
				 GL_UNSIGNED_BYTE, m_RenderImage.ppPlane[0]);
	glBindTexture(GL_TEXTURE_2D, GL_NONE);

	char vShaderStr[] =
			"#version 300 es\n"
			"precision mediump float;\n"
			"layout(location = 0) in vec3 a_vertex;\n"
			"layout(location = 1) in vec2 a_texCoord;\n"
			"layout(location = 2) in vec3 a_offset;\n"
			"layout(location = 3) in vec4 a_particlesColor;\n"
			"uniform mat4 u_MVPMatrix;\n"
			"out vec2 v_texCoord;\n"
			"out vec4 v_color;\n"
			"void main()\n"
			"{\n"
			"    gl_Position = u_MVPMatrix * vec4(a_vertex - vec3(0.0, 0.95, 0.0) + a_offset, 1.0);\n"
			"    v_texCoord = a_texCoord;\n"
			"    v_color = a_particlesColor;\n"
			"}";

	char fShaderStr[] =
			"#version 300 es\n"
			"precision mediump float;\n"
			"in vec2 v_texCoord;\n"
			"in vec4 v_color;\n"
			"layout(location = 0) out vec4 outColor;\n"
			"uniform sampler2D s_TextureMap;\n"
			"void main()\n"
			"{\n"
			"    outColor = texture(s_TextureMap, v_texCoord) * v_color;\n"
			"}";

	m_ProgramObj = GLUtils::CreateProgram(vShaderStr, fShaderStr, m_VertexShader, m_FragmentShader);
	if (m_ProgramObj)
	{
		m_SamplerLoc = glGetUniformLocation(m_ProgramObj, "s_TextureMap");
		m_MVPMatLoc = glGetUniformLocation(m_ProgramObj, "u_MVPMatrix");
	}
	else
	{
		LOGCATE("ParticlesSample::Init create program fail");
		return;
	}

	m_pParticlesPosData = new GLfloat[MAX_PARTICLES * 3];
	m_pParticlesColorData = new GLubyte[MAX_PARTICLES * 4];

	for (int i = 0; i < MAX_PARTICLES; i++)
	{
		GenerateNewParticle(m_ParticlesContainer[i]);
	}

	static const GLfloat g_vertex_buffer_data[] = {
			//position            //texture coord
			-0.05f, -0.05f, -0.05f, 0.0f, 0.0f,
			0.05f, -0.05f, -0.05f, 1.0f, 0.0f,
			0.05f, 0.05f, -0.05f, 1.0f, 1.0f,
			0.05f, 0.05f, -0.05f, 1.0f, 1.0f,
			-0.05f, 0.05f, -0.05f, 0.0f, 1.0f,
			-0.05f, -0.05f, -0.05f, 0.0f, 0.0f,
			-0.05f, -0.05f, 0.05f, 0.0f, 0.0f,
			0.05f, -0.05f, 0.05f, 1.0f, 0.0f,
			0.05f, 0.05f, 0.05f, 1.0f, 1.0f,
			0.05f, 0.05f, 0.05f, 1.0f, 1.0f,
			-0.05f, 0.05f, 0.05f, 0.0f, 1.0f,
			-0.05f, -0.05f, 0.05f, 0.0f, 0.0f,
			-0.05f, 0.05f, 0.05f, 1.0f, 0.0f,
			-0.05f, 0.05f, -0.05f, 1.0f, 1.0f,
			-0.05f, -0.05f, -0.05f, 0.0f, 1.0f,
			-0.05f, -0.05f, -0.05f, 0.0f, 1.0f,
			-0.05f, -0.05f, 0.05f, 0.0f, 0.0f,
			-0.05f, 0.05f, 0.05f, 1.0f, 0.0f,
			0.05f, 0.05f, 0.05f, 1.0f, 0.0f,
			0.05f, 0.05f, -0.05f, 1.0f, 1.0f,
			0.05f, -0.05f, -0.05f, 0.0f, 1.0f,
			0.05f, -0.05f, -0.05f, 0.0f, 1.0f,
			0.05f, -0.05f, 0.05f, 0.0f, 0.0f,
			0.05f, 0.05f, 0.05f, 1.0f, 0.0f,
			-0.05f, -0.05f, -0.05f, 0.0f, 1.0f,
			0.05f, -0.05f, -0.05f, 1.0f, 1.0f,
			0.05f, -0.05f, 0.05f, 1.0f, 0.0f,
			0.05f, -0.05f, 0.05f, 1.0f, 0.0f,
			-0.05f, -0.05f, 0.05f, 0.0f, 0.0f,
			-0.05f, -0.05f, -0.05f, 0.0f, 1.0f,
			-0.05f, 0.05f, -0.05f, 0.0f, 1.0f,
			0.05f, 0.05f, -0.05f, 1.0f, 1.0f,
			0.05f, 0.05f, 0.05f, 1.0f, 0.0f,
			0.05f, 0.05f, 0.05f, 1.0f, 0.0f,
			-0.05f, 0.05f, 0.05f, 0.0f, 0.0f,
			-0.05f, 0.05f, -0.05f, 0.0f, 1.0f,
	};

	glGenBuffers(1, &m_ParticlesVertexVboId);
	glBindBuffer(GL_ARRAY_BUFFER, m_ParticlesVertexVboId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data,
				 GL_STATIC_DRAW);
	GO_CHECK_GL_ERROR();

	glGenBuffers(1, &m_ParticlesPosVboId);
	glBindBuffer(GL_ARRAY_BUFFER, m_ParticlesPosVboId);
	// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * 3 * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);
	GO_CHECK_GL_ERROR();

	glGenBuffers(1, &m_ParticlesColorVboId);
	glBindBuffer(GL_ARRAY_BUFFER, m_ParticlesColorVboId);
	// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * 4 * sizeof(GLubyte), NULL, GL_DYNAMIC_DRAW);
	GO_CHECK_GL_ERROR();


	// Generate VAO Id
	glGenVertexArrays(1, &m_VaoId);
	glBindVertexArray(m_VaoId);
	GO_CHECK_GL_ERROR();

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, m_ParticlesVertexVboId);
	glVertexAttribPointer(
			0,
			3,
			GL_FLOAT,
			GL_FALSE,
			5 * sizeof(GLfloat),
			(void *) 0
	);
	GO_CHECK_GL_ERROR();

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, m_ParticlesVertexVboId);
	glVertexAttribPointer(
			1,
			2,
			GL_FLOAT,
			GL_FALSE,
			5 * sizeof(GLfloat),
			(const void *) (3 * sizeof(GLfloat))
	);
	GO_CHECK_GL_ERROR();

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, m_ParticlesPosVboId);
	glVertexAttribPointer(
			2,
			3,
			GL_FLOAT,
			GL_FALSE,
			0,
			(void *) 0
	);
	GO_CHECK_GL_ERROR();

	glEnableVertexAttribArray(3);
	glBindBuffer(GL_ARRAY_BUFFER, m_ParticlesColorVboId);
	glVertexAttribPointer(
			3,
			4,
			GL_UNSIGNED_BYTE,
			GL_TRUE,
			0,
			(void *) 0
	);
	GO_CHECK_GL_ERROR();

	glVertexAttribDivisor(0, 0);
	glVertexAttribDivisor(1, 0);
	glVertexAttribDivisor(2, 1);
	glVertexAttribDivisor(3, 1);
	GO_CHECK_GL_ERROR();
	glBindVertexArray(GL_NONE);

	//m_TextureId = GLUtils::loadDDS("/sdcard/particle.DDS");
	GO_CHECK_GL_ERROR();

}

void ParticlesSample::Draw(int screenW, int screenH)
{
	LOGCATE("ParticlesSample::Draw()");
	if (m_ProgramObj == GL_NONE || m_TextureId == GL_NONE) return;
	glEnable(GL_DEPTH_TEST);
	glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glDisable(GL_BLEND);

	UpdateMVPMatrix(m_MVPMatrix, m_AngleX, m_AngleY, (float) screenW / screenH);

	int particleCount = UpdateParticles();

	// Use the program object
	glUseProgram(m_ProgramObj);

	glBindVertexArray(m_VaoId);
	glUniformMatrix4fv(m_MVPMatLoc, 1, GL_FALSE, &m_MVPMatrix[0][0]);

	// Bind the RGBA map
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_TextureId);
	glUniform1i(m_SamplerLoc, 0);
	
	glDrawArraysInstanced(GL_TRIANGLES, 0, 36, particleCount);
}

void ParticlesSample::Destroy()
{
	if (m_ProgramObj)
	{
		glDeleteProgram(m_ProgramObj);
		glDeleteVertexArrays(1, &m_VaoId);
		glDeleteTextures(1, &m_TextureId);
		glDeleteBuffers(1, &m_ParticlesPosVboId);
		glDeleteBuffers(1, &m_ParticlesVertexVboId);
		glDeleteBuffers(1, &m_ParticlesPosVboId);
		m_ProgramObj = GL_NONE;
	}
}


/**
 * @param angleX 绕X轴旋转度数
 * @param angleY 绕Y轴旋转度数
 * @param ratio 宽高比
 * */
void ParticlesSample::UpdateMVPMatrix(glm::mat4 &mvpMatrix, int angleX, int angleY, float ratio)
{
	LOGCATE("ParticlesSample::UpdateMVPMatrix angleX = %d, angleY = %d, ratio = %f", angleX,
			angleY, ratio);
	angleX = angleX % 360;
	angleY = angleY % 360;

	//转化为弧度角
	float radiansX = static_cast<float>(MATH_PI / 180.0f * angleX);
	float radiansY = static_cast<float>(MATH_PI / 180.0f * angleY);


	// Projection matrix
	//glm::mat4 Projection = glm::ortho(-ratio, ratio, -1.0f, 1.0f, 0.0f, 100.0f);
	//glm::mat4 Projection = glm::frustum(-ratio, ratio, -1.0f, 1.0f, 4.0f, 100.0f);
	glm::mat4 Projection = glm::perspective(45.0f, ratio, 0.1f, 100.f);

	// View matrix
	glm::mat4 View = glm::lookAt(
			glm::vec3(0, 6, 0), // Camera is at (0,0,1), in World Space
			glm::vec3(0, 0, 0), // and looks at the origin
			glm::vec3(0, 0, 1)  // Head is up (set to 0,-1,0 to look upside-down)
	);

	// Model matrix
	glm::mat4 Model = glm::mat4(1.0f);
	Model = glm::scale(Model, glm::vec3(m_ScaleX, m_ScaleX, m_ScaleX));
	Model = glm::rotate(Model, radiansX, glm::vec3(1.0f, 0.0f, 0.0f));
	Model = glm::rotate(Model, radiansY, glm::vec3(0.0f, 1.0f, 0.0f));
	Model = glm::translate(Model, glm::vec3(0.0f, 0.0f, 0.0f));

	mvpMatrix = Projection * View * Model;

}

void ParticlesSample::UpdateTransformMatrix(float rotateX, float rotateY, float scaleX, float scaleY)
{
	GLSampleBase::UpdateTransformMatrix(rotateX, rotateY, scaleX, scaleY);
	m_AngleX = static_cast<int>(rotateX);
	m_AngleY = static_cast<int>(rotateY);
	m_ScaleX = scaleX;
	m_ScaleY = scaleY;
}

int ParticlesSample::FindUnusedParticle()
{

	for (int i = m_LastUsedParticle; i < MAX_PARTICLES; i++)
	{
		if (m_ParticlesContainer[i].life <= 0)
		{
			m_LastUsedParticle = i;
			return i;
		}
	}

	for (int i = 0; i < m_LastUsedParticle; i++)
	{
		if (m_ParticlesContainer[i].life <= 0)
		{
			m_LastUsedParticle = i;
			return i;
		}
	}

	return -1;
}

void ParticlesSample::SortParticles()
{
	std::sort(&m_ParticlesContainer[0], &m_ParticlesContainer[MAX_PARTICLES]);
}

int ParticlesSample::UpdateParticles()
{
	LOGCATE("ParticlesSample::UpdateParticles");

	int newParticles = 300;
	for (int i = 0; i < newParticles; i++)
	{
		int particleIndex = FindUnusedParticle();
		if (particleIndex >= 0)
		{
			GenerateNewParticle(m_ParticlesContainer[particleIndex]);
		}
	}



	// Simulate all particles
	int particlesCount = 0;
	for (int i = 0; i < MAX_PARTICLES; i++)
	{

		Particle &p = m_ParticlesContainer[i]; // shortcut

		if (p.life > 0.0f)
		{
			float delta = 0.1f;
			// Decrease life
			glm::vec3 speed = glm::vec3(p.dxSpeed, p.dySpeed, p.dzSpeed), pos = glm::vec3(p.dx,
																						  p.dy,
																						  p.dz);
			p.life -= delta;
			if (p.life > 0.0f)
			{

				// Simulate simple physics : gravity only, no collisions
				speed += glm::vec3(0.0f, 0.081f, 0.0f) * delta * 0.5f;
				pos += speed * delta;

				p.dxSpeed = speed.x;
				p.dySpeed = speed.y;
				p.dzSpeed = speed.z;

				p.dx = pos.x;
				p.dy = pos.y;
				p.dz = pos.z;

				m_pParticlesPosData[3 * particlesCount + 0] = p.dx;
				m_pParticlesPosData[3 * particlesCount + 1] = p.dy;
				m_pParticlesPosData[3 * particlesCount + 2] = p.dz;

				m_pParticlesColorData[4 * particlesCount + 0] = p.r;
				m_pParticlesColorData[4 * particlesCount + 1] = p.g;
				m_pParticlesColorData[4 * particlesCount + 2] = p.b;
				m_pParticlesColorData[4 * particlesCount + 3] = p.a;

			}

			particlesCount++;

		}
	}

	//SortParticles();

	glBindBuffer(GL_ARRAY_BUFFER, m_ParticlesPosVboId);
	glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * 3 * sizeof(GLfloat), NULL,
				 GL_DYNAMIC_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
	GO_CHECK_GL_ERROR();
	glBufferSubData(GL_ARRAY_BUFFER, 0, particlesCount * sizeof(GLfloat) * 3, m_pParticlesPosData);
	GO_CHECK_GL_ERROR();

	glBindBuffer(GL_ARRAY_BUFFER, m_ParticlesColorVboId);
	glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * 4 * sizeof(GLubyte), NULL,
				 GL_DYNAMIC_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
	glBufferSubData(GL_ARRAY_BUFFER, 0, particlesCount * sizeof(GLubyte) * 4,
					m_pParticlesColorData);
	GO_CHECK_GL_ERROR();

	return particlesCount;

}

void ParticlesSample::GenerateNewParticle(Particle &particle)
{
	particle.life = 5.0f;
	particle.cameraDistance = -1.0f;
	particle.dx = (rand() % 2000 - 1000.0f) / 3000.0f;
	particle.dy = (rand() % 2000 - 1000.0f) / 3000.0f;
	particle.dz = (rand() % 2000 - 1000.0f) / 3000.0f;

	float spread = 1.5f;

	glm::vec3 maindir = glm::vec3(0.0f, 2.0f, 0.0f);
	glm::vec3 randomdir = glm::vec3(
			(rand() % 2000 - 1000.0f) / 1000.0f,
			(rand() % 2000 - 1000.0f) / 1000.0f,
			(rand() % 2000 - 1000.0f) / 1000.0f
	);

	glm::vec3 speed = maindir + randomdir * spread;
	particle.dxSpeed = speed.x;
	particle.dySpeed = speed.y;
	particle.dzSpeed = speed.z;

	particle.r = static_cast<unsigned char>(rand() % 256);
	particle.g = static_cast<unsigned char>(rand() % 256);
	particle.b = static_cast<unsigned char>(rand() % 256);
	particle.a = static_cast<unsigned char>((rand() % 256) / 3);

}
