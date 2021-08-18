/**
 *
 * Created by 公众号：字节流动 on 2022/4/18.
 * https://github.com/githubhaohao/NDK_OpenGLES_3_0
 * 最新文章首发于公众号：字节流动，有疑问或者技术交流可以添加微信 Byte-Flow ,领取视频教程, 拉你进技术交流群
 *
 * */

#include <gtc/matrix_transform.hpp>
#include "ConveyorBeltExample.h"
#include "../util/GLUtils.h"

ConveyorBeltExample::ConveyorBeltExample()
{

	m_MVPMatLoc = GL_NONE;

    for (int i = 0; i < BF_TEXTURE_NUM; ++i) {
        m_TextureIds[i] = GL_NONE;
    }
	m_VaoId = GL_NONE;

	m_AngleX = 0;
	m_AngleY = 0;

	m_ScaleX = 1.0f;
	m_ScaleY = 1.0f;

    m_frameIndex = 0;
	m_loopCount = 0;
}

ConveyorBeltExample::~ConveyorBeltExample()
{
    NativeImageUtil::FreeNativeImage(&m_RenderImage);

    for (int i = 0; i < BF_TEXTURE_NUM; ++i) {
        NativeImageUtil::FreeNativeImage(&m_TexImages[i]);
    }
}

void ConveyorBeltExample::Init()
{
	if(m_ProgramObj)
		return;

    for (int i = 0; i < BF_TEXTURE_NUM; ++i) {
        glGenTextures(1, &m_TextureIds[i]);
        glBindTexture(GL_TEXTURE_2D, m_TextureIds[i]);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, GL_NONE);
    }

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
            "uniform sampler2D u_texture;\n"
            "uniform float u_offset;\n"
            "uniform vec2 u_texSize;\n"
            "\n"
            "void main()\n"
            "{\n"
            "    outColor = texture(u_texture, v_texCoord);\n"
            "}";

	m_ProgramObj = GLUtils::CreateProgram(vShaderStr, fShaderStr);
	if (m_ProgramObj)
	{
		m_MVPMatLoc = glGetUniformLocation(m_ProgramObj, "u_MVPMatrix");
	}
	else
	{
		LOGCATE("ConveyorBeltExample::Init create program fail");
	}
    vec2 pTmpCoords[BF_TEXTURE_NUM* 2 * 3];
	//第一块矩形
    pTmpCoords[0] = vec2(0,0);
    pTmpCoords[1] = vec2(0,0.5);
    pTmpCoords[2] = vec2(1,0.5);

    pTmpCoords[3] = vec2(0,0);
    pTmpCoords[4] = vec2(1,0.5);
    pTmpCoords[5] = vec2(1,0);

    m_pTexCoords[0] = vec2(0,0);
    m_pTexCoords[1] = vec2(0,1);
    m_pTexCoords[2] = vec2(1,1);

    m_pTexCoords[3] = vec2(0,0);
    m_pTexCoords[4] = vec2(1,1);
    m_pTexCoords[5] = vec2(1,0);

    float delta = 0.5f / BF_BANNER_NUM;
    //其他小矩形
    for (int i = 1; i <= BF_BANNER_NUM; ++i) {
        vec2 p0 = vec2(0, 0.5 + (i-1)*delta);
        vec2 p1 = vec2(0, 0.5 + i*delta);
        vec2 p2 = vec2(1, 0.5 + i*delta);
        vec2 p3 = vec2(1, 0.5 + (i-1)*delta);

        pTmpCoords[6 * i]     = p0;
        pTmpCoords[6 * i + 1] = p1;
        pTmpCoords[6 * i + 2] = p2;

        pTmpCoords[6 * i + 3] = p0;
        pTmpCoords[6 * i + 4] = p2;
        pTmpCoords[6 * i + 5] = p3;

        m_pTexCoords[6 * i]     = vec2(0,0);
        m_pTexCoords[6 * i + 1] = vec2(0,1);
        m_pTexCoords[6 * i + 2] = vec2(1,1);

        m_pTexCoords[6 * i + 3] = vec2(0,0);
        m_pTexCoords[6 * i + 4] = vec2(1,1);
        m_pTexCoords[6 * i + 5] = vec2(1,0);
    }

    for (int i = 0; i < BF_TEXTURE_NUM * 2 * 3; ++i) {
        m_pVtxCoords[i] = GLUtils::texCoordToVertexCoord(pTmpCoords[i]);
    }

	// Generate VBO Ids and load the VBOs with data
	glGenBuffers(2, m_VboIds);
	glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(m_pVtxCoords), m_pVtxCoords, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(m_pTexCoords), m_pTexCoords, GL_STATIC_DRAW);

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

	glBindVertexArray(GL_NONE);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	m_TexImages[0].width  = m_RenderImage.width;
	m_TexImages[0].height = m_RenderImage.height / 2;
	m_TexImages[0].format = m_RenderImage.format;
	NativeImageUtil::AllocNativeImage(&m_TexImages[0]);
	memcpy(m_TexImages[0].ppPlane[0], m_RenderImage.ppPlane[0], m_TexImages[0].width * m_TexImages[0].height * 4);

	uint8 *pBuf = m_RenderImage.ppPlane[0] + m_TexImages[0].width * m_TexImages[0].height * 4;
    for (int i = 1; i <= BF_BANNER_NUM; ++i) {
        m_TexImages[i].width  = m_RenderImage.width;
        m_TexImages[i].height = m_RenderImage.height / 2 / BF_BANNER_NUM;
        m_TexImages[i].format = m_RenderImage.format;
        NativeImageUtil::AllocNativeImage(&m_TexImages[i]);
        memcpy(m_TexImages[i].ppPlane[0], pBuf, m_TexImages[i].width * m_TexImages[i].height * 4);
        pBuf += m_TexImages[i].width * m_TexImages[i].height * 4;
    }

    for (int i = 0; i < BF_TEXTURE_NUM; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, m_TextureIds[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_TexImages[i].width, m_TexImages[i].height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_TexImages[i].ppPlane[0]);
        glBindTexture(GL_TEXTURE_2D, GL_NONE);
    }

}

void ConveyorBeltExample::LoadImage(NativeImage *pImage)
{
    LOGCATE("ConveyorBeltExample::LoadImage pImage = %p,[w=%d,h=%d,f=%d]", pImage->ppPlane[0], pImage->width, pImage->height, pImage->format);
    if(pImage) {
        m_RenderImage.width = pImage->width;
        m_RenderImage.height = pImage->height;
        m_RenderImage.format = pImage->format;
        NativeImageUtil::CopyNativeImage(pImage, &m_RenderImage);
	}
}

void ConveyorBeltExample::Draw(int screenW, int screenH)
{
	LOGCATE("ConveyorBeltExample::Draw()");

	if(m_ProgramObj == GL_NONE || m_TextureIds[0] == GL_NONE) return;

    m_frameIndex ++;

	UpdateMVPMatrix(m_MVPMatrix, m_AngleX, m_AngleY, (float)screenW / screenH);

	// Use the program object
	glUseProgram (m_ProgramObj);

	glBindVertexArray(m_VaoId);

	glUniformMatrix4fv(m_MVPMatLoc, 1, GL_FALSE, &m_MVPMatrix[0][0]);

	float offset = (m_frameIndex % BF_LOOP_COUNT) * 1.0f / BF_LOOP_COUNT;

	if(m_frameIndex % BF_LOOP_COUNT == 0)
		m_loopCount ++;

    uint8 *pBuf = m_RenderImage.ppPlane[0] + m_TexImages[0].width * m_TexImages[0].height * 4 - m_TexImages[1].width * m_TexImages[1].height * 4;
    for (int i = BF_BANNER_NUM; i >= 2; --i) {
        memcpy(m_TexImages[i].ppPlane[0], m_TexImages[i-1].ppPlane[0], m_TexImages[i].width * m_TexImages[i].height * 4);
    }
    memcpy(m_TexImages[1].ppPlane[0], pBuf, m_TexImages[1].width * m_TexImages[1].height * 4);

    for (int i = 1; i < BF_TEXTURE_NUM; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, m_TextureIds[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_TexImages[i].width, m_TexImages[i].height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_TexImages[i].ppPlane[0]);
        glBindTexture(GL_TEXTURE_2D, GL_NONE);
    }

	GLUtils::setFloat(m_ProgramObj, "u_offset", offset);

    for (int i = 0; i < BF_TEXTURE_NUM; ++i) {
        GLUtils::setVec2(m_ProgramObj, "u_texSize", m_TexImages[i].width,  m_TexImages[i].height);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_TextureIds[i]);
        GLUtils::setInt(m_ProgramObj, "u_texture", 0);
        glDrawArrays(GL_TRIANGLES, i * 6, 6);
    }

}

void ConveyorBeltExample::Destroy()
{
	if (m_ProgramObj)
	{
		glDeleteProgram(m_ProgramObj);
		glDeleteBuffers(2, m_VboIds);
		glDeleteVertexArrays(1, &m_VaoId);
		glDeleteTextures(BF_TEXTURE_NUM, m_TextureIds);
	}
}

void ConveyorBeltExample::UpdateMVPMatrix(glm::mat4 &mvpMatrix, int angleX, int angleY, float ratio)
{
	LOGCATE("ConveyorBeltExample::UpdateMVPMatrix angleX = %d, angleY = %d, ratio = %f", angleX, angleY, ratio);
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

void ConveyorBeltExample::UpdateTransformMatrix(float rotateX, float rotateY, float scaleX, float scaleY)
{
	GLSampleBase::UpdateTransformMatrix(rotateX, rotateY, scaleX, scaleY);
	m_AngleX = static_cast<int>(rotateX);
	m_AngleY = static_cast<int>(rotateY);
	m_ScaleX = scaleX;
	m_ScaleY = scaleY;
}

void ConveyorBeltExample::LoadMultiImageWithIndex(int index, NativeImage *pImage) {
	LOGCATE("ConveyorBeltExample::LoadMultiImageWithIndex pImage = %p,[w=%d,h=%d,f=%d]", pImage->ppPlane[0], pImage->width, pImage->height, pImage->format);
}
