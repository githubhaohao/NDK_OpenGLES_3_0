/**
 *
 * Created by 公众号：字节流动 on 2021/10/12.
 * https://github.com/githubhaohao/NDK_OpenGLES_3_0
 * 最新文章首发于公众号：字节流动，有疑问或者技术交流可以添加微信 Byte-Flow ,领取视频教程, 拉你进技术交流群
 *
 * */

#include <gtc/matrix_transform.hpp>
#include "MRTSample.h"
#include "../util/GLUtils.h"

const GLenum attachments[ATTACHMENT_NUM] = {
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1,
		GL_COLOR_ATTACHMENT2,
		GL_COLOR_ATTACHMENT3
};

MRTSample::MRTSample()
{

	m_SamplerLoc = GL_NONE;
	m_MVPMatLoc = GL_NONE;

	m_TextureId = GL_NONE;
	m_VaoId = GL_NONE;

	m_AngleX = 0;
	m_AngleY = 0;

	m_ScaleX = 1.0f;
	m_ScaleY = 1.0f;

    m_MRTProgramObj = GL_NONE;
}

MRTSample::~MRTSample()
{
	NativeImageUtil::FreeNativeImage(&m_RenderImage);

}

void MRTSample::Init()
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

	char vShaderStr[] =
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

	char fMRTShaderStr[] =
			"#version 300 es\n"
            "precision mediump float;\n"
            "in vec2 v_texCoord;\n"
            "layout(location = 0) out vec4 outColor0;\n"
            "layout(location = 1) out vec4 outColor1;\n"
            "layout(location = 2) out vec4 outColor2;\n"
            "layout(location = 3) out vec4 outColor3;\n"
            "uniform sampler2D s_Texture;\n"
            "void main()\n"
            "{\n"
            "    vec4 outputColor = texture(s_Texture, v_texCoord);\n"
            "    outColor0 = outputColor;\n"
            "    outColor1 = vec4(outputColor.r, 0.0, 0.0, 1.0);\n"
            "    outColor2 = vec4(0.0, outputColor.g, 0.0, 1.0);\n"
            "    outColor3 = vec4(0.0, 0.0, outputColor.b, 1.0);\n"
            "}";

	char fShaderStr[] =
	        "#version 300 es\n"
            "precision mediump float;\n"
            "in vec2 v_texCoord;\n"
            "layout(location = 0) out vec4 outColor;\n"
            "uniform sampler2D s_Texture0;\n"
            "uniform sampler2D s_Texture1;\n"
            "uniform sampler2D s_Texture2;\n"
            "uniform sampler2D s_Texture3;\n"
            "void main()\n"
            "{\n"
            "    if(v_texCoord.x < 0.5 && v_texCoord.y < 0.5)\n"
            "    {\n"
            "        outColor = texture(s_Texture0, v_texCoord);\n"
            "    }\n"
            "    else if(v_texCoord.x > 0.5 && v_texCoord.y < 0.5)\n"
            "    {\n"
            "        outColor = texture(s_Texture1, v_texCoord);\n"
            "    }\n"
            "    else if(v_texCoord.x < 0.5 && v_texCoord.y > 0.5)\n"
            "    {\n"
            "        outColor = texture(s_Texture2, v_texCoord);\n"
            "    }\n"
            "    else\n"
            "    {\n"
            "        outColor = texture(s_Texture3, v_texCoord);\n"
            "    }\n"
            "}";

	m_MRTProgramObj = GLUtils::CreateProgram(vShaderStr, fMRTShaderStr);
    m_ProgramObj = GLUtils::CreateProgram(vShaderStr, fShaderStr);
	if (m_MRTProgramObj)
	{
		m_SamplerLoc = glGetUniformLocation(m_MRTProgramObj, "s_Texture");
		m_MVPMatLoc = glGetUniformLocation(m_MRTProgramObj, "u_MVPMatrix");
	}
	else
	{
		LOGCATE("MRTSample::Init create program fail");
	}

	GLfloat verticesCoords[] = {
			-1.0f,  1.0f, 0.0f,  // Position 0
			-1.0f, -1.0f, 0.0f,  // Position 1
			1.0f,  -1.0f, 0.0f,  // Position 2
			1.0f,   1.0f, 0.0f,  // Position 3
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

	//upload RGBA image data
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_TextureId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_RenderImage.width, m_RenderImage.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_RenderImage.ppPlane[0]);
	glBindTexture(GL_TEXTURE_2D, GL_NONE);

	LOGCATE("MRTSample::Init InitFBO = %d", InitFBO());

}

void MRTSample::LoadImage(NativeImage *pImage)
{
	LOGCATE("MRTSample::LoadImage pImage = %p", pImage->ppPlane[0]);
	if (pImage)
	{
		m_RenderImage.width = pImage->width;
		m_RenderImage.height = pImage->height;
		m_RenderImage.format = pImage->format;
		NativeImageUtil::CopyNativeImage(pImage, &m_RenderImage);
	}
}

void MRTSample::Draw(int screenW, int screenH)
{
	LOGCATE("MRTSample::Draw()");
	m_SurfaceWidth = screenW;
	m_SurfaceHeight = screenH;

	if(m_MRTProgramObj == GL_NONE || m_TextureId == GL_NONE) return;

	GLint defaultFrameBuffer = GL_NONE;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &defaultFrameBuffer);

	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
	glViewport ( 0, 0, m_RenderImage.width, m_RenderImage.height);
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawBuffers(ATTACHMENT_NUM, attachments);

	glUseProgram (m_MRTProgramObj);

	glBindVertexArray(m_VaoId);
	UpdateMVPMatrix(m_MVPMatrix, 180, m_AngleY, (float)screenW / screenH);
	glUniformMatrix4fv(m_MVPMatLoc, 1, GL_FALSE, &m_MVPMatrix[0][0]);

	// Bind the RGBA map
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_TextureId);
	glUniform1i(m_SamplerLoc, 0);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (const void *)0);

	//绘制到屏幕上
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, defaultFrameBuffer);
	glViewport ( 0, 0, m_SurfaceWidth, m_SurfaceHeight);
	glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram (m_ProgramObj);
	UpdateMVPMatrix(m_MVPMatrix, 0, m_AngleY, (float)screenW / screenH);
	glUniformMatrix4fv(m_MVPMatLoc, 1, GL_FALSE, &m_MVPMatrix[0][0]);

    for (int i = 0; i < ATTACHMENT_NUM; ++i)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, m_AttachTexIds[i]);
        char samplerName[64] = {0};
        sprintf(samplerName, "s_Texture%d", i);
        GLUtils::setInt(m_ProgramObj, samplerName, i);
    }

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (const void *)0);
}

void MRTSample::Destroy()
{
	if (m_ProgramObj)
	{
		glDeleteProgram(m_ProgramObj);
        glDeleteProgram(m_MRTProgramObj);
		glDeleteBuffers(3, m_VboIds);
		glDeleteVertexArrays(1, &m_VaoId);
		glDeleteTextures(1, &m_TextureId);
		glDeleteTextures(ATTACHMENT_NUM, m_AttachTexIds);
	}
}

/**
 * @param angleX 绕X轴旋转度数
 * @param angleY 绕Y轴旋转度数
 * @param ratio 宽高比
 * */
void MRTSample::UpdateMVPMatrix(glm::mat4 &mvpMatrix, int angleX, int angleY, float ratio)
{
	LOGCATE("MRTSample::UpdateMVPMatrix angleX = %d, angleY = %d, ratio = %f", angleX, angleY, ratio);
	angleX = angleX % 360;
	angleY = angleY % 360;

	//转化为弧度角
	float radiansX = static_cast<float>(MATH_PI / 180.0f * angleX);
	float radiansY = static_cast<float>(MATH_PI / 180.0f * angleY);


	// Projection matrix
	glm::mat4 Projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 100.0f);
	//glm::mat4 Projection = glm::frustum(-ratio, ratio, -1.0f, 1.0f, 4.0f, 100.0f);
	//glm::mat4 Projection = glm::perspective(45.0f,ratio, 0.1f,100.f);

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

void MRTSample::UpdateTransformMatrix(float rotateX, float rotateY, float scaleX, float scaleY)
{
	GLSampleBase::UpdateTransformMatrix(rotateX, rotateY, scaleX, scaleY);
	m_AngleX = static_cast<int>(rotateX);
	m_AngleY = static_cast<int>(rotateY);
	m_ScaleX = scaleX;
	m_ScaleY = scaleY;
}

bool MRTSample::InitFBO() {
	GLint defaultFrameBuffer = GL_NONE;

	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &defaultFrameBuffer);

	glGenFramebuffers(1, &m_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

	glGenTextures(ATTACHMENT_NUM, m_AttachTexIds);
	for (int i = 0; i < ATTACHMENT_NUM; ++i) {
		glBindTexture(GL_TEXTURE_2D, m_AttachTexIds[i]);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_RenderImage.width, m_RenderImage.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, attachments[i], GL_TEXTURE_2D, m_AttachTexIds[i], 0);
	}

	glDrawBuffers(ATTACHMENT_NUM, attachments);

	if (GL_FRAMEBUFFER_COMPLETE != glCheckFramebufferStatus(GL_FRAMEBUFFER))
	{
		return false;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, defaultFrameBuffer);
    return true;
}

