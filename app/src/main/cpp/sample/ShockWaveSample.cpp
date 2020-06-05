//
// Created by ByteFlow on 2020/7/30.
//

#include <gtc/matrix_transform.hpp>
#include "ShockWaveSample.h"
#include "../util/GLUtils.h"

ShockWaveSample::ShockWaveSample()
{
	m_TextureId = GL_NONE;
	m_VaoId = GL_NONE;

	m_AngleX = 0;
	m_AngleY = 0;

	m_ScaleX = 1.0f;
	m_ScaleY = 1.0f;

	m_FrameIndex = 0;

	m_touchXY = vec2(0.5f, 0.5f);
}

ShockWaveSample::~ShockWaveSample()
{
	NativeImageUtil::FreeNativeImage(&m_RenderImage);

}

void ShockWaveSample::Init()
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

	char fShaderStr[] =
			"#version 300 es\n"
			"precision highp float;\n"
			"in vec2 v_texCoord;\n"
			"layout(location = 0) out vec4 outColor;\n"
			"uniform sampler2D s_TextureMap;\n"
			"uniform vec2 u_TouchXY;\n"
			"uniform vec2 u_TexSize;\n"
			"uniform float u_Time;\n"
			"uniform float u_Boundary;//0.1\n"
			"void main()\n"
			"{\n"
			"    float ratio = u_TexSize.y / u_TexSize.x;\n"
			"    vec2 texCoord = v_texCoord * vec2(1.0, ratio);\n"
			"    vec2 touchXY = u_TouchXY * vec2(1.0, ratio);\n"
			"    float distance = distance(texCoord, touchXY);\n"
			"    if ((u_Time - u_Boundary) > 0.0\n"
			"    && (distance <= (u_Time + u_Boundary))\n"
			"    && (distance >= (u_Time - u_Boundary))) {\n"
			"        float diff = (distance - u_Time);\n"
			"        float moveDis = 20.0*diff*(diff - u_Boundary)*(diff + u_Boundary);//采样坐标移动距离\n"
			"        vec2 unitDirectionVec = normalize(texCoord - touchXY);//单位方向向量\n"
			"        texCoord = texCoord + (unitDirectionVec * moveDis);\n"
			"    }\n"
			"\n"
			"    texCoord = texCoord / vec2(1.0, ratio);\n"
			"    outColor = texture(s_TextureMap, texCoord);\n"
			"}";

	m_ProgramObj = GLUtils::CreateProgram(vShaderStr, fShaderStr, m_VertexShader, m_FragmentShader);
	if (m_ProgramObj)
	{
		LOGCATE("ShockWaveSample::Init create program fail");
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

}

void ShockWaveSample::LoadImage(NativeImage *pImage)
{
	LOGCATE("ShockWaveSample::LoadImage pImage = %p", pImage->ppPlane[0]);
	if (pImage)
	{
		m_RenderImage.width = pImage->width;
		m_RenderImage.height = pImage->height;
		m_RenderImage.format = pImage->format;
		NativeImageUtil::CopyNativeImage(pImage, &m_RenderImage);
	}

}

void ShockWaveSample::Draw(int screenW, int screenH)
{
	LOGCATE("ShockWaveSample::Draw()");
    m_SurfaceWidth = screenW;
    m_SurfaceHeight = screenH;
	if(m_ProgramObj == GL_NONE || m_TextureId == GL_NONE) return;

	m_FrameIndex ++;

	UpdateMVPMatrix(m_MVPMatrix, m_AngleX, m_AngleY, (float)screenW / screenH);

	glUseProgram (m_ProgramObj);
	glBindVertexArray(m_VaoId);

	GLUtils::setMat4(m_ProgramObj, "u_MVPMatrix", m_MVPMatrix);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_TextureId);
	GLUtils::setFloat(m_ProgramObj, "s_TextureMap", 0);

	//float time = static_cast<float>(fmod(GetSysCurrentTime(), 2000) / 2000);
    float time = static_cast<float>(fmod(m_FrameIndex, 150) / 120);
	GLUtils::setFloat(m_ProgramObj, "u_Time", time);

	GLUtils::setVec2(m_ProgramObj, "u_TouchXY", m_touchXY);
    GLUtils::setVec2(m_ProgramObj, "u_TexSize", vec2(m_RenderImage.width, m_RenderImage.height));
	GLUtils::setFloat(m_ProgramObj, "u_Boundary", 0.1f);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (const void *)0);

}

void ShockWaveSample::Destroy()
{
	if (m_ProgramObj)
	{
		glDeleteProgram(m_ProgramObj);
		glDeleteBuffers(3, m_VboIds);
		glDeleteVertexArrays(1, &m_VaoId);
		glDeleteTextures(1, &m_TextureId);
	}
}


/**
 * @param angleX 绕X轴旋转度数
 * @param angleY 绕Y轴旋转度数
 * @param ratio 宽高比
 * */
void ShockWaveSample::UpdateMVPMatrix(glm::mat4 &mvpMatrix, int angleX, int angleY, float ratio)
{
	LOGCATE("ShockWaveSample::UpdateMVPMatrix angleX = %d, angleY = %d, ratio = %f", angleX, angleY, ratio);
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

void ShockWaveSample::UpdateTransformMatrix(float rotateX, float rotateY, float scaleX, float scaleY)
{
	GLSampleBase::UpdateTransformMatrix(rotateX, rotateY, scaleX, scaleY);
	m_AngleX = static_cast<int>(rotateX);
	m_AngleY = static_cast<int>(rotateY);
	m_ScaleX = scaleX;
	m_ScaleY = scaleY;
}

void ShockWaveSample::SetTouchLocation(float x, float y)
{
    m_touchXY = vec2(x / m_SurfaceWidth, y / m_SurfaceHeight);
    m_FrameIndex = 0;
}
