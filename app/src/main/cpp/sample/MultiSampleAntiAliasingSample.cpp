/**
 *
 * Created by 公众号：字节流动 on 2020/4/18.
 * https://github.com/githubhaohao/NDK_OpenGLES_3_0
 * 最新文章首发于公众号：字节流动，有疑问或者技术交流可以添加微信 Byte-Flow ,领取视频教程, 拉你进技术交流群
 *
 * */

#include <gtc/matrix_transform.hpp>
#include "MultiSampleAntiAliasingSample.h"
#include "../util/GLUtils.h"

MultiSampleAntiAliasingSample::MultiSampleAntiAliasingSample()
{
	m_ProgramObj = GL_NONE;
	m_SamplerLoc = GL_NONE;
	m_MVPMatLoc = GL_NONE;

	m_TextureId = GL_NONE;
	m_VaoId = GL_NONE;

	m_AngleX = 0;
	m_AngleY = 0;

	m_ScaleX = 1.0f;
	m_ScaleY = 1.0f;

	m_FboId = GL_NONE;
	m_RboId = GL_NONE;
	m_FboId2 = GL_NONE;
}

MultiSampleAntiAliasingSample::~MultiSampleAntiAliasingSample()
{
	NativeImageUtil::FreeNativeImage(&m_RenderImage);

}

void MultiSampleAntiAliasingSample::Init()
{
	if(m_ProgramObj)
		return;
	
	//create RGBA texture
	glGenTextures(1, &m_TextureId);
	glBindTexture(GL_TEXTURE_2D, m_TextureId);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, GL_NONE);

	glGenTextures(1, &m_TextureId2);
	glBindTexture(GL_TEXTURE_2D, m_TextureId2);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, GL_NONE);

	char vColShaderStr[] =
            "#version 300 es\n"
            "layout(location = 0) in vec4 a_position;\n"
            "layout(location = 1) in vec2 a_texCoord;\n"
            "uniform mat4 u_MVPMatrix;\n"
            "out vec2 v_texCoord;\n"
            "void main()\n"
            "{\n"
            "    gl_Position = u_MVPMatrix * a_position;\n"
            "    v_texCoord = a_texCoord;\n"
            "}";

	char fColShaderStr[] =
			"#version 300 es                                     \n"
			"precision mediump float;                            \n"
			"in vec2 v_texCoord;                                 \n"
			"layout(location = 0) out vec4 outColor;             \n"
			"uniform sampler2D s_Texture;                        \n"
			"void main()                                         \n"
			"{                                                   \n"
			"  outColor = texture(s_Texture, v_texCoord);        \n"
			"}                                                   \n";

	char vShaderStr[] =
			"#version 300 es\n"
			"layout(location = 0) in vec4 a_position;\n"
			"layout(location = 1) in vec2 a_texCoord;\n"
			"out vec2 v_texCoord;\n"
			"void main()\n"
			"{\n"
			"    gl_Position = a_position;\n"
			"    v_texCoord = a_texCoord;\n"
			"}";

	char fShaderStr[] =
			"#version 300 es                                     \n"
			"precision mediump float;                            \n"
			"in vec2 v_texCoord;                                 \n"
			"layout(location = 0) out vec4 outColor;             \n"
			"uniform sampler2D s_Texture;                        \n"
			"void main()                                         \n"
			"{                                                   \n"
			"  outColor = texture(s_Texture, v_texCoord);        \n"
			"}                                                   \n";

	m_ProgramObj = GLUtils::CreateProgram(vColShaderStr, fColShaderStr);
	m_ProgramObj2 = GLUtils::CreateProgram(vShaderStr, fShaderStr);

	GLfloat verticesCoords[] = {
			-1.0f,  1.0f, 0.0f,  // Position 0
			-1.0f, -1.0f, 0.0f,  // Position 1
			1.0f,  -1.0f, 0.0f,  // Position 2
			1.0f,   1.0f, 0.0f, // Position 3
	};

	GLfloat textureCoords[] = {
			0.0f,  0.0f,        // TexCoord 0
			0.0f,  1.0f,        // TexCoord 1
			1.0f,  1.0f,        // TexCoord 2
			1.0f,  0.0f         // TexCoord 3
	};

	GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

	// Generate VBO Ids and load the VBOs with data
	glGenBuffers(3, m_VboIds);
	glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesCoords), verticesCoords, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(textureCoords), textureCoords, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VboIds[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Generate VAO Id
	glGenVertexArrays(1, &m_VaoId);
	glBindVertexArray(m_VaoId);

	glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[0]);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (const void *)0);
	glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);

	glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[1]);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (const void *)0);
	glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VboIds[2]);
	glBindVertexArray(GL_NONE);

	glBindTexture(GL_TEXTURE_2D, m_TextureId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_RenderImage.width, m_RenderImage.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_RenderImage.ppPlane[0]);
	glBindTexture(GL_TEXTURE_2D, GL_NONE);
}

void MultiSampleAntiAliasingSample::LoadImage(NativeImage *pImage)
{
	LOGCATE("MultiSampleAntiAliasingSample::LoadImage pImage = %p", pImage->ppPlane[0]);
	if (pImage)
	{
		m_RenderImage.width = pImage->width;
		m_RenderImage.height = pImage->height;
		m_RenderImage.format = pImage->format;
		NativeImageUtil::CopyNativeImage(pImage, &m_RenderImage);
	}

}

void MultiSampleAntiAliasingSample::Draw(int screenW, int screenH)
{
	LOGCATE("MultiSampleAntiAliasingSample::Draw() [w,h]=[%d,%d]", screenW, screenH);

	if(m_ProgramObj == GL_NONE || m_TextureId == GL_NONE) return;

	if(m_FboId == GL_NONE) {
		GLint defaultFrameBuffer = GL_NONE;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &defaultFrameBuffer);

		glGenFramebuffers(1, &m_FboId);
		glBindFramebuffer(GL_FRAMEBUFFER, m_FboId);

//		glGenRenderbuffers(1, &m_RboId);
//		glBindRenderbuffer(GL_RENDERBUFFER, m_RboId);
//		glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGBA8, screenW, screenH);
//		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_RboId);

		glGenTextures(1, &m_MultiSampleTexId);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_MultiSampleTexId);
		glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, screenW,
								  screenH, GL_TRUE);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
		glFramebufferTexture2D(
				GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_MultiSampleTexId, 0
		);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
			LOGCATE("MultiSampleAntiAliasingSample:: Framebuffer is not complete!");
		}

		glGenFramebuffers(1, &m_FboId2);
		glBindFramebuffer(GL_FRAMEBUFFER, m_FboId2);
		glBindTexture(GL_TEXTURE_2D, m_TextureId2);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_TextureId2, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screenW, screenH, 0, GL_RGBA, GL_UNSIGNED_BYTE,
					 nullptr);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER)!= GL_FRAMEBUFFER_COMPLETE) {
			LOGCATE("MultiSampleAntiAliasingSample::CreateFrameBufferObj glCheckFramebufferStatus status != GL_FRAMEBUFFER_COMPLETE");
		}

		glBindFramebuffer(GL_FRAMEBUFFER, defaultFrameBuffer);
	}

	GLint defaultFrameBuffer = GL_NONE;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &defaultFrameBuffer);

	glBindFramebuffer(GL_FRAMEBUFFER, m_FboId);
	glViewport(0, 0, screenW, screenH);
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	UpdateMVPMatrix(m_MVPMatrix, m_AngleX, m_AngleY, (float)screenW / screenH);
	glUseProgram (m_ProgramObj);
	glBindVertexArray(m_VaoId);
	GLUtils::setMat4(m_ProgramObj, "u_MVPMatrix", m_MVPMatrix);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_TextureId);
	GLUtils::setInt(m_ProgramObj, "s_Texture", 1);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (const void *)0);

	//多重采样缓冲区无法直接上屏（渲染），先搞到另外一个缓冲区，然后再上屏
	glBindFramebuffer(GL_FRAMEBUFFER, m_FboId2);
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_FboId);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glBlitFramebuffer(0, 0, screenW, screenH,
					  0, 0, screenW, screenH,
					  GL_COLOR_BUFFER_BIT, GL_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, defaultFrameBuffer);
	glViewport(0, 0, screenW, screenH);
	glUseProgram (m_ProgramObj2);
	glBindVertexArray(m_VaoId);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_TextureId2);
	GLUtils::setInt(m_ProgramObj2, "s_Texture", 0);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (const void *)0);
}

void MultiSampleAntiAliasingSample::Destroy()
{
	if (m_ProgramObj)
	{
		glDeleteProgram(m_ProgramObj);
		glDeleteBuffers(3, m_VboIds);
		glDeleteVertexArrays(1, &m_VaoId);
		glDeleteTextures(1, &m_TextureId);
		glDeleteFramebuffers(1, &m_FboId);
		glDeleteRenderbuffers(1, &m_RboId);
		glDeleteTextures(1, &m_TextureId2);
		glDeleteFramebuffers(1, &m_FboId2);
		glDeleteTextures(1, &m_MultiSampleTexId);
	}
}

void MultiSampleAntiAliasingSample::UpdateMVPMatrix(glm::mat4 &mvpMatrix, int angleX, int angleY, float ratio)
{
	LOGCATE("MultiSampleAntiAliasingSample::UpdateMVPMatrix angleX = %d, angleY = %d, ratio = %f", angleX, angleY, ratio);
	angleX = angleX % 360;
	angleY = angleY % 360;

	//转化为弧度角
	float radiansX = static_cast<float>(MATH_PI / 180.0f * angleX);
	float radiansY = static_cast<float>(MATH_PI / 180.0f * angleY);


	// Projection matrix
	//glm::mat4 Projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 100.0f);
	//glm::mat4 Projection = glm::frustum(-ratio, ratio, -1.0f, 1.0f, 4.0f, 100.0f);
	glm::mat4 Projection = glm::perspective(45.0f,ratio, 0.1f,100.f);

	// View matrix
	glm::mat4 View = glm::lookAt(
			glm::vec3(0, 0, 4), // Camera is at (0,0,1), in World Space
			glm::vec3(0, 0, 0), // and looks at the origin
			glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
	);

	// Model matrix
	glm::mat4 Model = glm::mat4(1.0f);
	Model = glm::scale(Model, glm::vec3(m_ScaleX, m_ScaleY, 1.0f));
	Model = glm::rotate(Model, radiansX, glm::vec3(1.0f, 0.0f, 0.0f));
	Model = glm::rotate(Model, radiansY, glm::vec3(0.0f, 1.0f, 0.0f));
	Model = glm::translate(Model, glm::vec3(0.0f, 0.0f, 0.0f));

	mvpMatrix = Projection * View * Model;

}

void MultiSampleAntiAliasingSample::UpdateTransformMatrix(float rotateX, float rotateY, float scaleX, float scaleY)
{
	GLSampleBase::UpdateTransformMatrix(rotateX, rotateY, scaleX, scaleY);
	m_AngleX = static_cast<int>(rotateX);
	m_AngleY = static_cast<int>(rotateY);
	m_ScaleX = scaleX;
	m_ScaleY = scaleY;
}
