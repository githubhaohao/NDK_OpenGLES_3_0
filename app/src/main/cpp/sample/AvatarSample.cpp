/**
 *
 * Created by 公众号：字节流动 on 2020/4/18.
 * https://github.com/githubhaohao/NDK_OpenGLES_3_0
 * 最新文章首发于公众号：字节流动，有疑问或者技术交流可以添加微信 Byte-Flow ,领取视频教程, 拉你进技术交流群
 *
 * */

#include <gtc/matrix_transform.hpp>
#include "AvatarSample.h"
#include "../util/GLUtils.h"

float clamp(float x, float lowerlimit, float upperlimit) {
	if (x < lowerlimit)
		x = lowerlimit;
	if (x > upperlimit)
		x = upperlimit;
	return x;
}

float smoothstep(float edge0, float edge1, float x) {
	// Scale, bias and saturate x to 0..1 range
	x = clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0);
	// Evaluate polynomial
	return x * x * (3 - 2 * x);
}

AvatarSample::AvatarSample()
{

	m_SamplerLoc = GL_NONE;
	m_MVPMatLoc = GL_NONE;

	m_VaoId = GL_NONE;

	m_AngleX = 0;
	m_AngleY = 0;

	m_ScaleX = 1.0f;
	m_ScaleY = 1.0f;

	m_TransX = 0.0f;
	m_TransY = 0.0f;

	memset(m_TextureIds, 0, sizeof(m_TextureIds));
    memset(m_RenderImages, 0, sizeof(m_RenderImages));

    m_FrameIndex = 0;
}

AvatarSample::~AvatarSample()
{
    for (int i = 0; i < RENDER_IMG_NUM; ++i) {
        NativeImageUtil::FreeNativeImage(&m_RenderImages[i]);
    }

}

void AvatarSample::Init()
{
	if(m_ProgramObj)
		return;
	//create RGBA texture
	glGenTextures(RENDER_IMG_NUM, m_TextureIds);
    for (int i = 0; i < RENDER_IMG_NUM; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, m_TextureIds[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_RenderImages[i].width, m_RenderImages[i].height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_RenderImages[i].ppPlane[0]);
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
			"precision highp float;\n"
			"in vec2 v_texCoord;\n"
			"layout(location = 0) out vec4 outColor;\n"
			"uniform sampler2D s_TextureMap;\n"
			"uniform vec2 u_texSize;\n"
			"uniform float u_needRotate;\n"
			"uniform float u_rotateAngle;\n"
			"\n"
			"vec2 rotate(float radius, float angle, vec2 texSize, vec2 texCoord)\n"
			"{\n"
			"    vec2 newTexCoord = texCoord;\n"
			"    vec2 center = vec2(texSize.x / 2.0, texSize.y / 2.0);\n"
			"    vec2 tc = texCoord * texSize;\n"
			"    tc -= center;\n"
			"    float dist = length(tc);\n"
			"    if (dist < radius) {\n"
			"        float percent = (radius - dist) / radius;\n"
			"        float theta = percent * percent * angle * 8.0;\n"
			"        float s = sin(theta);\n"
			"        float c = cos(theta);\n"
			"        tc = vec2(dot(tc, vec2(c, -s)), dot(tc, vec2(s, c)));\n"
			"        tc += center;\n"
			"\n"
			"        newTexCoord = tc / texSize;\n"
			"    }\n"
			"    return newTexCoord;\n"
			"}\n"
			"void main()\n"
			"{\n"
			"    vec2 texCoord = v_texCoord;\n"
			"\n"
			"    if(u_needRotate > 0.0)\n"
			"    {\n"
			"        texCoord = rotate(0.5 * u_texSize.x, u_rotateAngle, u_texSize, v_texCoord);\n"
			"    }\n"
			"\n"
			"    outColor = texture(s_TextureMap, texCoord);\n"
			"    if (outColor.a < 0.6) discard;\n"
			"}";

	char fBlurShaderStr[] =
			"#version 300 es\n"
			"precision highp float;\n"
			"layout(location = 0) out vec4 outColor;\n"
			"in vec2 v_texCoord;\n"
			"uniform sampler2D s_TextureMap;\n"
			"void main() {\n"
			"    vec4 sample0, sample1, sample2, sample3;\n"
			"    float blurStep = 0.16;\n"
			"    float step = blurStep / 100.0;\n"
			"    sample0 = texture(s_TextureMap, vec2(v_texCoord.x - step, v_texCoord.y - step));\n"
			"    sample1 = texture(s_TextureMap, vec2(v_texCoord.x + step, v_texCoord.y + step));\n"
			"    sample2 = texture(s_TextureMap, vec2(v_texCoord.x + step, v_texCoord.y - step));\n"
			"    sample3 = texture(s_TextureMap, vec2(v_texCoord.x - step, v_texCoord.y + step));\n"
			"    outColor = (sample0 + sample1 + sample2 + sample3) / 4.0;\n"
			"    if(outColor.a > 0.05) \n"
			"    {\n"
			"        outColor += vec4(0.2, 0.2, 0.2, 0.0);\n"
			"    } \n"
			"    else\n"
			"    {\n"
			"        discard;\n"
			"    }\n"
			"}";


	m_ProgramObj = GLUtils::CreateProgram(vShaderStr, fShaderStr, m_VertexShader, m_FragmentShader);
	m_BlurProgramObj = GLUtils::CreateProgram(vShaderStr, fBlurShaderStr);
	if (m_ProgramObj)
	{
		m_SamplerLoc = glGetUniformLocation(m_ProgramObj, "s_TextureMap");
		m_MVPMatLoc = glGetUniformLocation(m_ProgramObj, "u_MVPMatrix");
	}
	else
	{
		LOGCATE("AvatarSample::Init create program fail");
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

}

void AvatarSample::LoadImage(NativeImage *pImage)
{
	LOGCATE("AvatarSample::LoadImage pImage = %p", pImage->ppPlane[0]);

}

void AvatarSample::LoadMultiImageWithIndex(int index, NativeImage *pImage) {
    LOGCATE("AvatarSample::LoadMultiImageWithIndex pImage = %p, index = %d", pImage, index);
    if (pImage->ppPlane[0])
    {
        m_RenderImages[index].width = pImage->width;
        m_RenderImages[index].height = pImage->height;
        m_RenderImages[index].format = pImage->format;
        NativeImageUtil::CopyNativeImage(pImage, &m_RenderImages[index]);
    }
}

void AvatarSample::Draw(int screenW, int screenH)
{
	LOGCATE("AvatarSample::Draw()");

	if(m_ProgramObj == GL_NONE) return;
	float dScaleLevel = m_FrameIndex % 200 * 1.0f / 1000 + 0.0001f;
	float scaleLevel = 1.0;

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(m_ProgramObj);
	glBindVertexArray(m_VaoId);

    //1. 背景层的绘制
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_TextureIds[0]);
	glUniform1i(m_SamplerLoc, 0);
	scaleLevel = static_cast<float>(1.0f + dScaleLevel * pow(-1, m_FrameIndex / 200));
	scaleLevel = scaleLevel < 1.0 ? scaleLevel + 0.2f : scaleLevel;
	m_ScaleY = m_ScaleX = scaleLevel + 0.4f;
	GLUtils::setVec2(m_ProgramObj, "u_texSize", glm::vec2(m_RenderImages[0].width, m_RenderImages[0].height));
	GLUtils::setFloat(m_ProgramObj, "u_needRotate", 1.0f); // u_needRotate == 1 开启形变
	GLUtils::setFloat(m_ProgramObj, "u_rotateAngle", m_TransX * 1.5f);
	UpdateMVPMatrix(m_MVPMatrix, m_AngleX, m_AngleY, m_TransX / 2, m_TransY / 2, (float)screenW / screenH);
	glUniformMatrix4fv(m_MVPMatLoc, 1, GL_FALSE, &m_MVPMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (const void *)0);

    //2. 人像层的绘制
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_TextureIds[1]);
	glUniform1i(m_SamplerLoc, 1);
	scaleLevel = static_cast<float>(1.0f + dScaleLevel * pow(-1, m_FrameIndex / 200 + 1));
	scaleLevel = scaleLevel < 1.0 ? scaleLevel + 0.2f : scaleLevel;
	m_ScaleY = m_ScaleX = scaleLevel + 0.4f;
	LOGCATE("AvatarSample::Draw() scaleLevel=%f", scaleLevel);
	UpdateMVPMatrix(m_MVPMatrix, m_AngleX, m_AngleY, m_TransX * 1.2f, m_TransY * 1.2f, (float)screenW / screenH);
	GLUtils::setVec2(m_ProgramObj, "u_texSize", glm::vec2(m_RenderImages[0].width, m_RenderImages[0].height));
	GLUtils::setFloat(m_ProgramObj, "u_needRotate", 0.0f);// u_needRotate == 0 关闭形变
	GLUtils::setFloat(m_ProgramObj, "u_rotateAngle", m_TransX / 20);
	glUniformMatrix4fv(m_MVPMatLoc, 1, GL_FALSE, &m_MVPMatrix[0][0]);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (const void *)0);

    //3. 外层的绘制
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_ALPHA);
	glUseProgram(m_BlurProgramObj);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_TextureIds[2]);
	GLUtils::setFloat(m_BlurProgramObj, "s_TextureMap", 0);
	scaleLevel = static_cast<float>(1.0f + dScaleLevel * pow(-1, m_FrameIndex / 200));
	scaleLevel = scaleLevel < 1.0 ? scaleLevel + 0.2f : scaleLevel;
	m_ScaleY = m_ScaleX = scaleLevel + 0.8f;
	UpdateMVPMatrix(m_MVPMatrix, m_AngleX, m_AngleY, m_TransX * 2.5f, m_TransY * 2.5f, (float)screenW / screenH);
	GLUtils::setMat4(m_BlurProgramObj, "u_MVPMatrix", m_MVPMatrix);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (const void *)0);

	glDisable(GL_BLEND);
	m_FrameIndex ++;

}

void AvatarSample::Destroy()
{
	if (m_ProgramObj)
	{
		glDeleteProgram(m_ProgramObj);
		glDeleteBuffers(3, m_VboIds);
		glDeleteVertexArrays(1, &m_VaoId);
		glDeleteTextures(RENDER_IMG_NUM, m_TextureIds);
	}
}

/**
 *
 * @param mvpMatrix
 * @param angleX 绕X轴旋转度数
 * @param angleY 绕Y轴旋转度数
 * @param transX 沿X轴位移大小
 * @param transY 沿Y轴位移大小
 * @param ratio 宽高比
 */
void AvatarSample::UpdateMVPMatrix(glm::mat4 &mvpMatrix, int angleX, int angleY, float transX, float transY, float ratio)
{
	LOGCATE("AvatarSample::UpdateMVPMatrix angleX = %d, angleY = %d, ratio = %f", angleX, angleY, ratio);
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
	Model = glm::translate(Model, glm::vec3(transX, transY, 0.0f));

	mvpMatrix = Projection * View * Model;

}

void AvatarSample::UpdateTransformMatrix(float rotateX, float rotateY, float scaleX, float scaleY)
{
	GLSampleBase::UpdateTransformMatrix(rotateX, rotateY, scaleX, scaleY);
	m_AngleX = static_cast<int>(rotateX);
	m_AngleY = static_cast<int>(rotateY);
	m_ScaleX = scaleX;
	m_ScaleY = scaleY;
}

void AvatarSample::SetGravityXY(float x, float y) {
	//GLSampleBase::SetGravityXY(x, y);
	m_TransX = - x / 60;
	m_TransY = - y / 60;
}
