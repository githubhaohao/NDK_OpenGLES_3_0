/**
 *
 * Created by 公众号：字节流动 on 2022/4/18.
 * https://github.com/githubhaohao/NDK_OpenGLES_3_0
 * 最新文章首发于公众号：字节流动，有疑问或者技术交流可以添加微信 Byte-Flow ,领取视频教程, 拉你进技术交流群
 *
 * */

#include <gtc/matrix_transform.hpp>
#include "PortraitStayColorExample.h"
#include "../util/GLUtils.h"

PortraitStayColorExample::PortraitStayColorExample()
{

	m_SamplerLoc = GL_NONE;
	m_MVPMatLoc = GL_NONE;

	m_TextureId = GL_NONE;
	m_VaoId = GL_NONE;

	m_AngleX = 0;
	m_AngleY = 0;

	m_ScaleX = 1.0f;
	m_ScaleY = 1.0f;

    m_frameIndex = 0;
}

PortraitStayColorExample::~PortraitStayColorExample()
{
	NativeImageUtil::FreeNativeImage(&m_RenderImage);
	NativeImageUtil::FreeNativeImage(&m_GrayImage);
	NativeImageUtil::FreeNativeImage(&m_MappingImage);
}

void PortraitStayColorExample::Init()
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

	glGenTextures(1, &m_GrayTexId);
	glBindTexture(GL_TEXTURE_2D, m_GrayTexId);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, GL_NONE);

	glGenTextures(1, &m_MappingTexId);
	glBindTexture(GL_TEXTURE_2D, m_MappingTexId);
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
			"precision mediump float;\n"
			"in vec2 v_texCoord;\n"
			"layout(location = 0) out vec4 outColor;\n"
			"uniform sampler2D u_texture0;//rgba\n"
			"uniform sampler2D u_texture1;//gray\n"
			"uniform sampler2D u_texture2;//mapping\n"
			"uniform float u_offset;\n"
			"uniform vec2 u_texSize;\n"
			"uniform float u_time;\n"
			"void main()\n"
			"{\n"
			"    float gray = texture(u_texture1, v_texCoord).r;\n"
			"    vec4 rgba  = texture(u_texture0, v_texCoord);\n"
			"    if(gray > 0.01) {\n"
			"        outColor = rgba;\n"
			"    }\n"
			"    else\n"
			"    {\n"
			"        vec2 fragCoord = gl_FragCoord.xy;\n"
			"        vec2 p = (-u_texSize.xy + 2.0*fragCoord)/u_texSize.y;\n"
			"        float a = atan(p.y,p.x);\n"
			"        float r = pow( pow(p.x*p.x,4.0) + pow(p.y*p.y,4.0), 1.0/8.0 );\n"
			"        vec2 uv = vec2( 1.0/r + 0.2*u_time, a );\n"
			"        float f = cos(12.0*uv.x)*cos(6.0*uv.y);\n"
			"        vec3 col = 0.5 + 0.5*sin( 3.1416*f + vec3(0.0,0.5,1.0) );\n"
			"        col = col*r;\n"
			"        vec4 tunnelColor = vec4(col, 1.0);\n"
			"\n"
			"        float Y = 0.299 * rgba.r + 0.587 * rgba.g + 0.114 * rgba.b;\n"
			"        vec4 grayColor = vec4(vec3(Y), 1.0);\n"
			"        outColor = mix(grayColor, tunnelColor, u_offset);\n"
			"    }\n"
			"}";

	m_ProgramObj = GLUtils::CreateProgram(vShaderStr, fShaderStr);
	if (m_ProgramObj)
	{
		m_SamplerLoc = glGetUniformLocation(m_ProgramObj, "u_texture0");
		m_MVPMatLoc = glGetUniformLocation(m_ProgramObj, "u_MVPMatrix");
	}
	else
	{
		LOGCATE("PortraitStayColorExample::Init create program fail");
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
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_TextureId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_RenderImage.width, m_RenderImage.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_RenderImage.ppPlane[0]);
	glBindTexture(GL_TEXTURE_2D, GL_NONE);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_GrayTexId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, m_GrayImage.width, m_GrayImage.height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, m_GrayImage.ppPlane[0]);
	glBindTexture(GL_TEXTURE_2D, GL_NONE);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_MappingTexId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_MappingImage.width, m_MappingImage.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_MappingImage.ppPlane[0]);
	glBindTexture(GL_TEXTURE_2D, GL_NONE);

}

void PortraitStayColorExample::LoadImage(NativeImage *pImage)
{
	LOGCATE("PortraitStayColorExample::LoadImage pImage = %p", pImage->ppPlane[0]);
	if (pImage)
	{
		m_RenderImage.width = pImage->width;
		m_RenderImage.height = pImage->height;
		m_RenderImage.format = pImage->format;
		NativeImageUtil::CopyNativeImage(pImage, &m_RenderImage);
	}

}

void PortraitStayColorExample::Draw(int screenW, int screenH)
{
	LOGCATE("PortraitStayColorExample::Draw()");

	if(m_ProgramObj == GL_NONE || m_TextureId == GL_NONE) return;

    m_frameIndex ++;

	UpdateMVPMatrix(m_MVPMatrix, m_AngleX, m_AngleY, (float)screenW / screenH);

	// Use the program object
	glUseProgram (m_ProgramObj);

	glBindVertexArray(m_VaoId);

	glUniformMatrix4fv(m_MVPMatLoc, 1, GL_FALSE, &m_MVPMatrix[0][0]);

	// Bind the RGBA map
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_TextureId);
	GLUtils::setInt(m_ProgramObj, "u_texture0", 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_GrayTexId);
	GLUtils::setInt(m_ProgramObj, "u_texture1", 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_MappingTexId);
	GLUtils::setInt(m_ProgramObj, "u_texture2", 2);

	GLUtils::setVec2(m_ProgramObj, "u_texSize", m_RenderImage.width, m_RenderImage.height);
	//GLUtils::setVec2(m_ProgramObj, "u_mappingTexSize", m_MappingImage.width, m_MappingImage.height);

	float offset = (sin(m_frameIndex * MATH_PI / 160) + 1.0f) / 2.0f;
	GLUtils::setFloat(m_ProgramObj, "u_offset", offset);
    GLUtils::setFloat(m_ProgramObj, "u_time", m_frameIndex * 0.04f);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (const void *)0);

}

void PortraitStayColorExample::Destroy()
{
	if (m_ProgramObj)
	{
		glDeleteProgram(m_ProgramObj);
		glDeleteBuffers(3, m_VboIds);
		glDeleteVertexArrays(1, &m_VaoId);
		glDeleteTextures(1, &m_TextureId);
		glDeleteTextures(1, &m_GrayTexId);
		glDeleteTextures(1, &m_MappingTexId);
	}
}


/**
 * @param angleX 绕X轴旋转度数
 * @param angleY 绕Y轴旋转度数
 * @param ratio 宽高比
 * */
void PortraitStayColorExample::UpdateMVPMatrix(glm::mat4 &mvpMatrix, int angleX, int angleY, float ratio)
{
	LOGCATE("PortraitStayColorExample::UpdateMVPMatrix angleX = %d, angleY = %d, ratio = %f", angleX, angleY, ratio);
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

void PortraitStayColorExample::UpdateTransformMatrix(float rotateX, float rotateY, float scaleX, float scaleY)
{
	GLSampleBase::UpdateTransformMatrix(rotateX, rotateY, scaleX, scaleY);
	m_AngleX = static_cast<int>(rotateX);
	m_AngleY = static_cast<int>(rotateY);
	m_ScaleX = scaleX;
	m_ScaleY = scaleY;
}

void PortraitStayColorExample::LoadMultiImageWithIndex(int index, NativeImage *pImage) {
	LOGCATE("PortraitStayColorExample::LoadMultiImageWithIndex pImage = %p,[w=%d,h=%d,f=%d]", pImage->ppPlane[0], pImage->width, pImage->height, pImage->format);
	if (pImage)
	{
        switch (index) {
            case 0:
                m_GrayImage.width = pImage->width;
                m_GrayImage.height = pImage->height;
                m_GrayImage.format = pImage->format;
                NativeImageUtil::CopyNativeImage(pImage, &m_GrayImage);
                //NativeImageUtil::DumpNativeImage(&m_GrayImage, "/sdcard/DCIM", "PortraitStayColorExample");
                break;
            case 1:
				m_MappingImage.width = pImage->width;
				m_MappingImage.height = pImage->height;
				m_MappingImage.format = pImage->format;
				NativeImageUtil::CopyNativeImage(pImage, &m_MappingImage);
                break;
        }

    }
}
