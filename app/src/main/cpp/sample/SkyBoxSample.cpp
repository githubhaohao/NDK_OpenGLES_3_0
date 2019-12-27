//
// Created by ByteFlow on 2019/7/30.
//

#include <gtc/matrix_transform.hpp>
#include "SkyBoxSample.h"
#include "../util/GLUtils.h"

SkyBoxSample::SkyBoxSample():
m_CubeProgramObj(GL_NONE)
{

	m_SamplerLoc = GL_NONE;
	m_MVPMatLoc = GL_NONE;

	m_TextureId = GL_NONE;
	m_SkyBoxVaoId = GL_NONE;

	m_AngleX = 0;
	m_AngleY = 0;

	m_ScaleX = 1.0f;
	m_ScaleY = 1.0f;

	m_ModelMatrix = glm::mat4(0.0f);

	memset(m_pSkyBoxRenderImg, 0, sizeof(NativeImage) * 6);
}

SkyBoxSample::~SkyBoxSample()
{
	NativeImageUtil::FreeNativeImage(&m_RenderImage);

	for(NativeImage nativeImage : m_pSkyBoxRenderImg)
	{
		NativeImageUtil::FreeNativeImage(&nativeImage);
	}

}

void SkyBoxSample::Init()
{
	if (m_ProgramObj)
	{
		return;
	}

	char vSkyBoxShaderStr[] =
			"#version 300 es\n"
			"precision mediump float;\n"
			"layout(location = 0) in vec3 a_position;\n"
			"uniform mat4 u_MVPMatrix;\n"
			"out vec3 v_texCoord;\n"
			"void main()\n"
			"{\n"
			"    gl_Position = u_MVPMatrix * vec4(a_position, 1.0);\n"
			"    v_texCoord = a_position;\n"
			"}";

	char fSkyBoxShaderStr[] =
			"#version 300 es\n"
			"precision mediump float;\n"
			"in vec3 v_texCoord;\n"
			"layout(location = 0) out vec4 outColor;\n"
			"uniform samplerCube s_SkyBox;\n"
			"void main()\n"
			"{\n"
			"    outColor = texture(s_SkyBox, v_texCoord);\n"
			"}";

	char vCubeShaderStr[] =
			"#version 300 es\n"
			"precision mediump float;\n"
			"layout(location = 0) in vec3 a_position;\n"
			"layout(location = 1) in vec3 a_normal;\n"
			"uniform mat4 u_MVPMatrix;\n"
			"uniform mat4 u_ModelMatrix;\n"
			"out vec3 v_texCoord;\n"
			"out vec3 v_normal;\n"
			"void main()\n"
			"{\n"
			"    gl_Position = u_MVPMatrix * vec4(a_position, 1.0);\n"
            "    v_normal = mat3(transpose(inverse(u_ModelMatrix))) * a_normal;\n"
			"    v_texCoord = vec3(u_ModelMatrix * vec4(a_position, 1.0));\n"
			"}";

	char fCubeShaderStr[] =
			"#version 300 es\n"
			"precision mediump float;\n"
			"in vec3 v_texCoord;\n"
			"in vec3 v_normal;\n"
			"layout(location = 0) out vec4 outColor;\n"
			"uniform samplerCube s_SkyBox;\n"
            "uniform vec3 u_cameraPos;"
			"void main()\n"
			"{\n"
            "    float ratio = 1.00 / 1.52;"
            "    vec3 I = normalize(v_texCoord - u_cameraPos);\n"
			"    //反射  \n"
			"    vec3 R = reflect(I, normalize(v_normal));\n"
            "    //折射\n"
            "    //vec3 R = refract(I, normalize(v_normal), ratio);\n"
			"    outColor = texture(s_SkyBox, R);\n"
			"}";

	m_ProgramObj = GLUtils::CreateProgram(vSkyBoxShaderStr, fSkyBoxShaderStr, m_VertexShader, m_FragmentShader);
	if (m_ProgramObj)
	{
		m_SamplerLoc = glGetUniformLocation(m_ProgramObj, "s_SkyBox");
		m_MVPMatLoc = glGetUniformLocation(m_ProgramObj, "u_MVPMatrix");
	}
	else
	{
		LOGCATE("SkyBoxSample::Init create program fail");
		return;
	}

	m_CubeProgramObj = GLUtils::CreateProgram(vCubeShaderStr, fCubeShaderStr, m_VertexShader, m_FragmentShader);
	if (m_CubeProgramObj)
	{
		m_CubeSamplerLoc = glGetUniformLocation(m_CubeProgramObj, "s_SkyBox");
		m_CubeMVPMatLoc = glGetUniformLocation(m_CubeProgramObj, "u_MVPMatrix");
		m_CubeModelMatLoc = glGetUniformLocation(m_CubeProgramObj, "u_ModelMatrix");
		m_ViewPosLoc = glGetUniformLocation(m_CubeProgramObj, "u_cameraPos");
	}
	else
	{
		LOGCATE("SkyBoxSample::Init create program fail");
		return;
	}

	GLfloat cubeVertices[] = {
			 //position           //normal
			-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
			 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
			 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
			 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
			-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
			-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

			-0.5f, -0.5f, 0.5f,   0.0f,  0.0f,  1.0f,
			 0.5f, -0.5f, 0.5f,   0.0f,  0.0f,  1.0f,
			 0.5f,  0.5f, 0.5f,   0.0f,  0.0f,  1.0f,
			 0.5f,  0.5f, 0.5f,   0.0f,  0.0f,  1.0f,
			-0.5f,  0.5f, 0.5f,   0.0f,  0.0f,  1.0f,
			-0.5f, -0.5f, 0.5f,   0.0f,  0.0f,  1.0f,

			-0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
			-0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
			-0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
			-0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
			-0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
			-0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

			 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
			 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
			 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
			 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
			 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
			 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

			-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
			 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
			 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
			 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
			-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
			-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

			-0.5f, 0.5f, -0.5f,   0.0f,  1.0f,  0.0f,
			 0.5f, 0.5f, -0.5f,   0.0f,  1.0f,  0.0f,
			 0.5f, 0.5f,  0.5f,   0.0f,  1.0f,  0.0f,
			 0.5f, 0.5f,  0.5f,   0.0f,  1.0f,  0.0f,
			-0.5f, 0.5f,  0.5f,   0.0f,  1.0f,  0.0f,
			-0.5f, 0.5f, -0.5f,   0.0f,  1.0f,  0.0f,
	};

	GLfloat skyboxVertices[] = {
			// Positions
			 -2.0f,  2.0f, -2.0f,
			 -2.0f, -2.0f, -2.0f,
			2.0f, -2.0f, -2.0f,
			2.0f, -2.0f, -2.0f,
			2.0f,  2.0f, -2.0f,
			-2.0f,  2.0f, -2.0f,

			-2.0f, -2.0f,  2.0f,
			-2.0f, -2.0f, -2.0f,
			-2.0f,  2.0f, -2.0f,
			-2.0f,  2.0f, -2.0f,
			-2.0f,  2.0f,  2.0f,
			-2.0f, -2.0f,  2.0f,

			2.0f, -2.0f, -2.0f,
			2.0f, -2.0f,  2.0f,
			2.0f,  2.0f,  2.0f,
			2.0f,  2.0f,  2.0f,
			2.0f,  2.0f, -2.0f,
			2.0f, -2.0f, -2.0f,

			-2.0f, -2.0f,  2.0f,
			-2.0f,  2.0f,  2.0f,
			2.0f,  2.0f,  2.0f,
			2.0f,  2.0f,  2.0f,
			2.0f, -2.0f,  2.0f,
			-2.0f, -2.0f,  2.0f,

			-2.0f,  2.0f, -2.0f,
			2.0f,  2.0f, -2.0f,
			2.0f,  2.0f,  2.0f,
			2.0f,  2.0f,  2.0f,
			-2.0f,  2.0f,  2.0f,
			-2.0f,  2.0f, -2.0f,

			-2.0f, -2.0f, -2.0f,
			-2.0f, -2.0f,  2.0f,
			2.0f, -2.0f, -2.0f,
			2.0f, -2.0f, -2.0f,
			-2.0f, -2.0f,  2.0f,
			2.0f, -2.0f,  2.0f
	};

	// Generate SkyBox VBO Ids and load the VBOs with data
	glGenBuffers(1, &m_SkyBoxVboId);
	glBindBuffer(GL_ARRAY_BUFFER, m_SkyBoxVboId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);

	// Generate SkyBox VAO Id
	glGenVertexArrays(1, &m_SkyBoxVaoId);
	glBindVertexArray(m_SkyBoxVaoId);
	glBindBuffer(GL_ARRAY_BUFFER, m_SkyBoxVboId);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (const void *) 0);
	glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);
	glBindVertexArray(GL_NONE);

	// Generate Cube VBO Ids and load the VBOs with data
	glGenBuffers(1, &m_CubeVboId);
	glBindBuffer(GL_ARRAY_BUFFER, m_CubeVboId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

	// Generate Cube VAO Id
	glGenVertexArrays(1, &m_CubeVaoId);
	glBindVertexArray(m_CubeVaoId);
	glBindBuffer(GL_ARRAY_BUFFER, m_CubeVboId);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (const void *) 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);
	glBindVertexArray(GL_NONE);

}

void SkyBoxSample::LoadImage(NativeImage *pImage)
{
	LOGCATE("SkyBoxSample::LoadImage pImage = %p", pImage->ppPlane[0]);
	if (pImage)
	{
		m_RenderImage.width = pImage->width;
		m_RenderImage.height = pImage->height;
		m_RenderImage.format = pImage->format;
		NativeImageUtil::CopyNativeImage(pImage, &m_RenderImage);
	}

}

void SkyBoxSample::LoadMultiImageWithIndex(int index, NativeImage *pImage)
{
	LOGCATE("SkyBoxSample::LoadMultiImageWithIndex pImage = %p, index=%d", pImage->ppPlane[0], index);
	if (pImage)
	{
		if(m_pSkyBoxRenderImg[index].ppPlane[0])
		{
			NativeImageUtil::FreeNativeImage(&m_pSkyBoxRenderImg[index]);
		}

		m_pSkyBoxRenderImg[index].width = pImage->width;
		m_pSkyBoxRenderImg[index].height = pImage->height;
		m_pSkyBoxRenderImg[index].format = pImage->format;
		NativeImageUtil::CopyNativeImage(pImage, &m_pSkyBoxRenderImg[index]);
	}
}

void SkyBoxSample::Draw(int screenW, int screenH)
{
	LOGCATE("SkyBoxSample::Draw()");

	if (m_ProgramObj == GL_NONE) return;
	glEnable(GL_DEPTH_TEST);

	UpdateMVPMatrix(m_MVPMatrix, m_AngleX, m_AngleY, 1.0, (float) screenW / screenH);

	if (!m_TextureId)
	{
		//create RGBA texture
		glGenTextures(1, &m_TextureId);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_TextureId);

		for (int i = 0; i < sizeof(m_pSkyBoxRenderImg) / sizeof(NativeImage); ++i)
		{
			glTexImage2D(
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
					GL_RGBA, m_pSkyBoxRenderImg[i].width, m_pSkyBoxRenderImg[i].height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_pSkyBoxRenderImg[i].ppPlane[0]
			);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	}

	// draw SkyBox
	glUseProgram(m_ProgramObj);
	glBindVertexArray(m_SkyBoxVaoId);
	glUniformMatrix4fv(m_MVPMatLoc, 1, GL_FALSE, &m_MVPMatrix[0][0]);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_TextureId);
	glUniform1i(m_SamplerLoc, 0);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	GO_CHECK_GL_ERROR();

	UpdateMVPMatrix(m_MVPMatrix, m_AngleX, m_AngleY, 0.4f, (float) screenW / screenH);

	// draw Cube
	glUseProgram(m_CubeProgramObj);
	glBindVertexArray(m_CubeVaoId);
	glUniformMatrix4fv(m_CubeMVPMatLoc, 1, GL_FALSE, &m_MVPMatrix[0][0]);
	glUniformMatrix4fv(m_CubeModelMatLoc, 1, GL_FALSE, &m_ModelMatrix[0][0]);
	glUniform3f(m_ViewPosLoc,  0.0f, 0.0f, 1.8f);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_TextureId);
	glUniform1i(m_CubeSamplerLoc, 0);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	GO_CHECK_GL_ERROR();
}

void SkyBoxSample::Destroy()
{
	if (m_ProgramObj)
	{
		glDeleteProgram(m_ProgramObj);
		glDeleteVertexArrays(1, &m_SkyBoxVaoId);
		glDeleteBuffers(1, &m_SkyBoxVboId);
		glDeleteTextures(1, &m_TextureId);
		m_ProgramObj = GL_NONE;
	}

	if (m_CubeProgramObj)
	{
		glDeleteProgram(m_CubeProgramObj);
		glDeleteVertexArrays(1, &m_CubeVaoId);
		glDeleteBuffers(1, &m_CubeVboId);
		m_CubeProgramObj = GL_NONE;
	}
}


/**
 * @param angleX 绕X轴旋转度数
 * @param angleY 绕Y轴旋转度数
 * @param ratio 宽高比
 * */
void SkyBoxSample::UpdateMVPMatrix(glm::mat4 &mvpMatrix, int angleX, int angleY, float scale, float ratio)
{
	LOGCATE("SkyBoxSample::UpdateMVPMatrix angleX = %d, angleY = %d, ratio = %f", angleX,
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
			glm::vec3(0, 0, 1.8), // Camera is at (0,0,1), in World Space
			glm::vec3(0, 0, -1), // and looks at the origin
			glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
	);

	// Model matrix
	glm::mat4 Model = glm::mat4(1.0f);
	Model = glm::scale(Model, glm::vec3(scale, scale, scale));
	Model = glm::rotate(Model, radiansX, glm::vec3(1.0f, 0.0f, 0.0f));
	Model = glm::rotate(Model, radiansY, glm::vec3(0.0f, 1.0f, 0.0f));
	Model = glm::translate(Model, glm::vec3(0.0f, 0.0f, 0.0f));

	m_ModelMatrix = Model;

	mvpMatrix = Projection * View * Model;

}

void SkyBoxSample::UpdateTransformMatrix(float rotateX, float rotateY, float scaleX, float scaleY)
{
	GLSampleBase::UpdateTransformMatrix(rotateX, rotateY, scaleX, scaleY);
	m_AngleX = static_cast<int>(rotateX);
	m_AngleY = static_cast<int>(rotateY);
	m_ScaleX = scaleX;
	m_ScaleY = scaleY;
}

