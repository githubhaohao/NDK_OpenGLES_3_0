/**
 *
 * Created by 公众号：字节流动 on 2020/4/18.
 * https://github.com/githubhaohao/NDK_OpenGLES_3_0
 * 最新文章首发于公众号：字节流动，有疑问或者技术交流可以添加微信 Byte-Flow ,领取视频教程, 拉你进技术交流群
 *
 * */

#include <gtc/matrix_transform.hpp>
#include "DepthTestingSample.h"
#include "../util/GLUtils.h"

glm::vec3 transPos[] = {
		glm::vec3( 0.0f,  0.0f,  0.0f),
		glm::vec3( 2.0f,  2.0f, -1.0f) * 1.6f,
		glm::vec3(-1.5f, -2.2f, -1.5f) * 1.6f,
		glm::vec3(-1.8f, -2.0f,  1.3f) * 1.6f,
		glm::vec3( 1.4f, -1.4f, -1.5f) * 1.6f,
		glm::vec3(-1.7f,  2.0f, -1.5f) * 1.6f,
		glm::vec3( 1.3f, -2.0f,  2.5f) * 1.6f,
		glm::vec3( 0.5f,  1.3f, -0.1f) * 1.6f,
		glm::vec3( 1.5f,  2.2f,  1.5f) * 1.6f,
		glm::vec3(-1.3f,  1.0f, -1.5f) * 1.6f,
		glm::vec3(-1.3f,  0.0f, -1.5f) * 1.6f,
		glm::vec3( 0.0f, -1.3f, -0.5f) * 1.6f,
		glm::vec3( 0.0f, -1.5f,  1.5f) * 1.6f,
};

DepthTestingSample::DepthTestingSample()
{

	m_SamplerLoc = GL_NONE;
	m_MVPMatLoc = GL_NONE;
	m_TextureId = GL_NONE;
	m_VaoId = GL_NONE;

	m_AngleX = 0;
	m_AngleY = 0;

	m_ModelMatrix = glm::mat4(0.0f);
}

DepthTestingSample::~DepthTestingSample()
{
	NativeImageUtil::FreeNativeImage(&m_RenderImage);

}

void DepthTestingSample::Init()
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
	glBindTexture(GL_TEXTURE_2D, GL_NONE);

	char vShaderStr[] =
			"#version 300 es\n"
			"precision mediump float;\n"
			"layout(location = 0) in vec4 a_position;\n"
			"layout(location = 1) in vec2 a_texCoord;\n"
			"layout(location = 2) in vec3 a_normal;\n"
			"\n"
			"out vec2 v_texCoord;\n"
			"uniform mat4 u_MVPMatrix;\n"
			"void main()\n"
			"{\n"
			"    gl_Position = u_MVPMatrix * a_position;\n"
			"    v_texCoord = a_texCoord;\n"
			"}";

	char fShaderStr[] =
			"#version 300 es\n"
			"precision mediump float;\n"
			"in vec2 v_texCoord;\n"
			"layout(location = 0) out vec4 outColor;\n"
			"uniform sampler2D s_TextureMap;\n"
			"void main()\n"
			"{\n"
			"    vec4 objectColor = vec4(vec3(gl_FragCoord.z), 1.0f);\n"
			"    outColor = objectColor;\n"
			"}";

	m_ProgramObj = GLUtils::CreateProgram(vShaderStr, fShaderStr, m_VertexShader, m_FragmentShader);
	if (m_ProgramObj)
	{
		m_SamplerLoc = glGetUniformLocation(m_ProgramObj, "s_TextureMap");
		GO_CHECK_GL_ERROR();
		m_MVPMatLoc = glGetUniformLocation(m_ProgramObj, "u_MVPMatrix");
		GO_CHECK_GL_ERROR();
	}
	else
	{
		LOGCATE("DepthTestingSample::Init create program fail");
		return;
	}

	GLfloat vertices[] = {
			 //position            //texture coord  //normal
			-0.5f, -0.5f, -0.5f,   0.0f, 0.0f,      0.0f,  0.0f, -1.0f,
			 0.5f, -0.5f, -0.5f,   1.0f, 0.0f,      0.0f,  0.0f, -1.0f,
			 0.5f,  0.5f, -0.5f,   1.0f, 1.0f,      0.0f,  0.0f, -1.0f,
			 0.5f,  0.5f, -0.5f,   1.0f, 1.0f,      0.0f,  0.0f, -1.0f,
			-0.5f,  0.5f, -0.5f,   0.0f, 1.0f,      0.0f,  0.0f, -1.0f,
			-0.5f, -0.5f, -0.5f,   0.0f, 0.0f,      0.0f,  0.0f, -1.0f,

			-0.5f, -0.5f, 0.5f,    0.0f, 0.0f,      0.0f,  0.0f,  1.0f,
			 0.5f, -0.5f, 0.5f,    1.0f, 0.0f,      0.0f,  0.0f,  1.0f,
			 0.5f,  0.5f, 0.5f,    1.0f, 1.0f,      0.0f,  0.0f,  1.0f,
			 0.5f,  0.5f, 0.5f,    1.0f, 1.0f,      0.0f,  0.0f,  1.0f,
			-0.5f,  0.5f, 0.5f,    0.0f, 1.0f,      0.0f,  0.0f,  1.0f,
			-0.5f, -0.5f, 0.5f,    0.0f, 0.0f,      0.0f,  0.0f,  1.0f,

			-0.5f,  0.5f,  0.5f,   1.0f, 0.0f,     -1.0f,  0.0f,  0.0f,
			-0.5f,  0.5f, -0.5f,   1.0f, 1.0f,     -1.0f,  0.0f,  0.0f,
			-0.5f, -0.5f, -0.5f,   0.0f, 1.0f,     -1.0f,  0.0f,  0.0f,
			-0.5f, -0.5f, -0.5f,   0.0f, 1.0f,     -1.0f,  0.0f,  0.0f,
			-0.5f, -0.5f,  0.5f,   0.0f, 0.0f,     -1.0f,  0.0f,  0.0f,
			-0.5f,  0.5f,  0.5f,   1.0f, 0.0f,     -1.0f,  0.0f,  0.0f,

			 0.5f,  0.5f,  0.5f,   1.0f, 0.0f,      1.0f,  0.0f,  0.0f,
			 0.5f,  0.5f, -0.5f,   1.0f, 1.0f,      1.0f,  0.0f,  0.0f,
			 0.5f, -0.5f, -0.5f,   0.0f, 1.0f,      1.0f,  0.0f,  0.0f,
			 0.5f, -0.5f, -0.5f,   0.0f, 1.0f,      1.0f,  0.0f,  0.0f,
			 0.5f, -0.5f,  0.5f,   0.0f, 0.0f,      1.0f,  0.0f,  0.0f,
			 0.5f,  0.5f,  0.5f,   1.0f, 0.0f,      1.0f,  0.0f,  0.0f,

			-0.5f, -0.5f, -0.5f,   0.0f, 1.0f,      0.0f, -1.0f,  0.0f,
			 0.5f, -0.5f, -0.5f,   1.0f, 1.0f,      0.0f, -1.0f,  0.0f,
			 0.5f, -0.5f,  0.5f,   1.0f, 0.0f,      0.0f, -1.0f,  0.0f,
			 0.5f, -0.5f,  0.5f,   1.0f, 0.0f,      0.0f, -1.0f,  0.0f,
			-0.5f, -0.5f,  0.5f,   0.0f, 0.0f,      0.0f, -1.0f,  0.0f,
			-0.5f, -0.5f, -0.5f,   0.0f, 1.0f,      0.0f, -1.0f,  0.0f,

			-0.5f, 0.5f, -0.5f,    0.0f, 1.0f,      0.0f,  1.0f,  0.0f,
			 0.5f, 0.5f, -0.5f,    1.0f, 1.0f,      0.0f,  1.0f,  0.0f,
			 0.5f, 0.5f,  0.5f,    1.0f, 0.0f,      0.0f,  1.0f,  0.0f,
			 0.5f, 0.5f,  0.5f,    1.0f, 0.0f,      0.0f,  1.0f,  0.0f,
			-0.5f, 0.5f,  0.5f,    0.0f, 0.0f,      0.0f,  1.0f,  0.0f,
			-0.5f, 0.5f, -0.5f,    0.0f, 1.0f,      0.0f,  1.0f,  0.0f,
	};

	// Generate VBO Ids and load the VBOs with data
	glGenBuffers(1, m_VboIds);
	glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Generate VAO Id
	glGenVertexArrays(1, &m_VaoId);
	glBindVertexArray(m_VaoId);

	glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[0]);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (const void *) 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (const void *) (3* sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (const void *) (5* sizeof(GLfloat)));
	glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);

	glBindVertexArray(GL_NONE);

	//upload RGBA image data
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_TextureId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_RenderImage.width, m_RenderImage.height, 0, GL_RGBA,
				 GL_UNSIGNED_BYTE, m_RenderImage.ppPlane[0]);
	glBindTexture(GL_TEXTURE_2D, GL_NONE);

}

void DepthTestingSample::LoadImage(NativeImage *pImage)
{
	LOGCATE("DepthTestingSample::LoadImage pImage = %p", pImage->ppPlane[0]);
	if (pImage)
	{
		m_RenderImage.width = pImage->width;
		m_RenderImage.height = pImage->height;
		m_RenderImage.format = pImage->format;
		NativeImageUtil::CopyNativeImage(pImage, &m_RenderImage);
	}

}

void DepthTestingSample::Draw(int screenW, int screenH)
{
	LOGCATE("DepthTestingSample::Draw()");

	if (m_ProgramObj == GL_NONE || m_TextureId == GL_NONE) return;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);

	float ratio = (float)screenW / screenH;

	// Use the program object
	glUseProgram(m_ProgramObj);

	glBindVertexArray(m_VaoId);

	// Bind the RGBA map
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_TextureId);
	glUniform1i(m_SamplerLoc, 0);

	// 绘制多个立方体，不同的位移和旋转角度
	for(int i = 0; i < sizeof(transPos)/ sizeof(transPos[0]); i++)
	{
		UpdateMatrix(m_MVPMatrix, m_ModelMatrix, m_AngleX + 10, m_AngleY + 10, 0.4, transPos[i], ratio);
		glUniformMatrix4fv(m_MVPMatLoc, 1, GL_FALSE, &m_MVPMatrix[0][0]);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}

}

void DepthTestingSample::Destroy()
{
	if (m_ProgramObj)
	{
		glDeleteProgram(m_ProgramObj);
		glDeleteBuffers(1, m_VboIds);
		glDeleteVertexArrays(1, &m_VaoId);
		glDeleteTextures(1, &m_TextureId);
		m_ProgramObj = GL_NONE;
		m_VaoId = GL_NONE;
		m_TextureId = GL_NONE;
	}

}

void DepthTestingSample::UpdateMVPMatrix(glm::mat4 &mvpMatrix, int angleX, int angleY, float ratio)
{
	//No implement
}

void DepthTestingSample::UpdateMatrix(glm::mat4 &mvpMatrix, glm::mat4 &modelMatrix, int angleXRotate, int angleYRotate, float scale, glm::vec3 transVec3, float ratio)
{
	LOGCATE("DepthTestingSample::UpdateMatrix angleX = %d, angleY = %d, ratio = %f", angleXRotate,
			angleYRotate, ratio);
	angleXRotate = angleXRotate % 360;
	angleYRotate = angleYRotate % 360;

	//转化为弧度角
	float radiansX = static_cast<float>(MATH_PI / 180.0f * angleXRotate);
	float radiansY = static_cast<float>(MATH_PI / 180.0f * angleYRotate);


	// Projection matrix
	//glm::mat4 Projection = glm::ortho(-ratio, ratio, -1.0f, 1.0f, 0.0f, 100.0f);
	//glm::mat4 Projection = glm::frustum(-ratio, ratio, -1.0f, 1.0f, 4.0f, 100.0f);
	glm::mat4 Projection = glm::perspective(45.0f, ratio, 0.1f, 100.f);

	// View matrix
	glm::mat4 View = glm::lookAt(
			glm::vec3(0, 0, 3), // Camera is at (0,0,1), in World Space
			glm::vec3(0, 0, 0), // and looks at the origin
			glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
	);

	// Model matrix
	glm::mat4 Model = glm::mat4(1.0f);
	Model = glm::scale(Model, glm::vec3(scale, scale, scale));
	Model = glm::rotate(Model, radiansX, glm::vec3(1.0f, 0.0f, 0.0f));
	Model = glm::rotate(Model, radiansY, glm::vec3(0.0f, 1.0f, 0.0f));
	Model = glm::translate(Model, transVec3);

	modelMatrix = Model;

	mvpMatrix = Projection * View * Model;
}

void DepthTestingSample::UpdateTransformMatrix(float rotateX, float rotateY, float scaleX, float scaleY)
{
	GLSampleBase::UpdateTransformMatrix(rotateX, rotateY, scaleX, scaleY);
	m_AngleX = static_cast<int>(rotateX);
	m_AngleY = static_cast<int>(rotateY);
}
