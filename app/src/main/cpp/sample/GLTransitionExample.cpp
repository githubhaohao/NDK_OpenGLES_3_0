/**
 *
 * Created by 公众号：字节流动 on 2022/4/18.
 * https://github.com/githubhaohao/NDK_OpenGLES_3_0
 * 最新文章首发于公众号：字节流动，有疑问或者技术交流可以添加微信 Byte-Flow ,领取视频教程, 拉你进技术交流群
 *
 * */

#include <gtc/matrix_transform.hpp>
#include "GLTransitionExample.h"
#include "../util/GLUtils.h"

GLTransitionExample::GLTransitionExample()
{

	m_MVPMatLoc = GL_NONE;

    for (int i = 0; i < BF_IMG_NUM; ++i) {
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

GLTransitionExample::~GLTransitionExample()
{
    for (int i = 0; i < BF_IMG_NUM; ++i) {
        NativeImageUtil::FreeNativeImage(&m_RenderImages[i]);
    }
}

void GLTransitionExample::Init()
{
	if(m_ProgramObj)
		return;
    for (int i = 0; i < BF_IMG_NUM; ++i) {
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
			"uniform sampler2D u_texture0;\n"
			"uniform sampler2D u_texture1;\n"
			"uniform float u_offset;\n"
			"uniform vec2 u_texSize;\n"
			"\n"
			"const float MIN_AMOUNT = -0.16;\n"
			"const float MAX_AMOUNT = 1.5;\n"
			"\n"
			"const float PI = 3.141592653589793;\n"
			"\n"
			"const float scale = 512.0;\n"
			"const float sharpness = 3.0;\n"
			"\n"
			"const float cylinderRadius = 1.0 / PI / 2.0;\n"
			"\n"
			"float amount = 0.0;\n"
			"float cylinderCenter = 0.0;\n"
			"float cylinderAngle = 0.0;\n"
			"\n"
			"vec3 hitPoint(float hitAngle, float yc, vec3 point, mat3 rrotation)\n"
			"{\n"
			"    float hitPoint = hitAngle / (2.0 * PI);\n"
			"    point.y = hitPoint;\n"
			"    return rrotation * point;\n"
			"}\n"
			"\n"
			"vec4 antiAlias(vec4 color1, vec4 color2, float distanc)\n"
			"{\n"
			"    distanc *= scale;\n"
			"    if (distanc < 0.0) return color2;\n"
			"    if (distanc > 2.0) return color1;\n"
			"    float dd = pow(1.0 - distanc / 2.0, sharpness);\n"
			"    return ((color2 - color1) * dd) + color1;\n"
			"}\n"
			"\n"
			"float distanceToEdge(vec3 point)\n"
			"{\n"
			"    float dx = abs(point.x > 0.5 ? 1.0 - point.x : point.x);\n"
			"    float dy = abs(point.y > 0.5 ? 1.0 - point.y : point.y);\n"
			"    if (point.x < 0.0) dx = -point.x;\n"
			"    if (point.x > 1.0) dx = point.x - 1.0;\n"
			"    if (point.y < 0.0) dy = -point.y;\n"
			"    if (point.y > 1.0) dy = point.y - 1.0;\n"
			"    if ((point.x < 0.0 || point.x > 1.0) && (point.y < 0.0 || point.y > 1.0)) return sqrt(dx * dx + dy * dy);\n"
			"    return min(dx, dy);\n"
			"}\n"
			"\n"
			"vec4 seeThrough(float yc, vec2 p, mat3 rotation, mat3 rrotation)\n"
			"{\n"
			"    float hitAngle = PI - (acos(yc / cylinderRadius) - cylinderAngle);\n"
			"    vec3 point = hitPoint(hitAngle, yc, rotation * vec3(p, 1.0), rrotation);\n"
			"    if (yc <= 0.0 && (point.x < 0.0 || point.y < 0.0 || point.x > 1.0 || point.y > 1.0))\n"
			"    {\n"
			"        return texture(u_texture1, p);\n"
			"    }\n"
			"\n"
			"    if (yc > 0.0) return texture(u_texture0, p);\n"
			"\n"
			"    vec4 color = texture(u_texture0, point.xy);\n"
			"    vec4 tcolor = vec4(0.0);\n"
			"\n"
			"    return antiAlias(color, tcolor, distanceToEdge(point));\n"
			"}\n"
			"\n"
			"vec4 seeThroughWithShadow(float yc, vec2 p, vec3 point, mat3 rotation, mat3 rrotation)\n"
			"{\n"
			"    float shadow = distanceToEdge(point) * 30.0;\n"
			"    shadow = (1.0 - shadow) / 3.0;\n"
			"\n"
			"    if (shadow < 0.0) shadow = 0.0; else shadow *= amount;\n"
			"\n"
			"    vec4 shadowColor = seeThrough(yc, p, rotation, rrotation);\n"
			"    shadowColor.r -= shadow;\n"
			"    shadowColor.g -= shadow;\n"
			"    shadowColor.b -= shadow;\n"
			"\n"
			"    return shadowColor;\n"
			"}\n"
			"\n"
			"vec4 backside(float yc, vec3 point)\n"
			"{\n"
			"    vec4 color = texture(u_texture0, point.xy);\n"
			"    float gray = (color.r + color.b + color.g) / 15.0;\n"
			"    gray += (8.0 / 10.0) * (pow(1.0 - abs(yc / cylinderRadius), 2.0 / 10.0) / 2.0 + (5.0 / 10.0));\n"
			"    color.rgb = vec3(gray);\n"
			"    return color;\n"
			"}\n"
			"\n"
			"vec4 behindSurface(vec2 p, float yc, vec3 point, mat3 rrotation)\n"
			"{\n"
			"    float shado = (1.0 - ((-cylinderRadius - yc) / amount * 7.0)) / 6.0;\n"
			"    shado *= 1.0 - abs(point.x - 0.5);\n"
			"\n"
			"    yc = (-cylinderRadius - cylinderRadius - yc);\n"
			"\n"
			"    float hitAngle = (acos(yc / cylinderRadius) + cylinderAngle) - PI;\n"
			"    point = hitPoint(hitAngle, yc, point, rrotation);\n"
			"\n"
			"    if (yc < 0.0 && point.x >= 0.0 && point.y >= 0.0 && point.x <= 1.0 && point.y <= 1.0 && (hitAngle < PI || amount > 0.5))\n"
			"    {\n"
			"        shado = 1.0 - (sqrt(pow(point.x - 0.5, 2.0) + pow(point.y - 0.5, 2.0)) / (71.0 / 100.0));\n"
			"        shado *= pow(-yc / cylinderRadius, 3.0);\n"
			"        shado *= 0.5;\n"
			"    }\n"
			"    else\n"
			"    {\n"
			"        shado = 0.0;\n"
			"    }\n"
			"    return vec4(texture(u_texture1, p).rgb - shado, 1.0);\n"
			"}\n"
			"\n"
			"vec4 transition(vec2 p) {\n"
			"\n"
			"    const float angle = 100.0 * PI / 180.0;\n"
			"    float c = cos(-angle);\n"
			"    float s = sin(-angle);\n"
			"\n"
			"    mat3 rotation = mat3( c, s, 0,\n"
			"    -s, c, 0,\n"
			"    -0.801, 0.8900, 1\n"
			"    );\n"
			"    c = cos(angle);\n"
			"    s = sin(angle);\n"
			"\n"
			"    mat3 rrotation = mat3(\tc, s, 0,\n"
			"    -s, c, 0,\n"
			"    0.98500, 0.985, 1\n"
			"    );\n"
			"\n"
			"    vec3 point = rotation * vec3(p, 1.0);\n"
			"\n"
			"    float yc = point.y - cylinderCenter;\n"
			"\n"
			"    if (yc < -cylinderRadius)\n"
			"    {\n"
			"        // Behind surface\n"
			"        return behindSurface(p,yc, point, rrotation);\n"
			"    }\n"
			"\n"
			"    if (yc > cylinderRadius)\n"
			"    {\n"
			"        // Flat surface\n"
			"        return texture(u_texture0, p);\n"
			"    }\n"
			"\n"
			"    float hitAngle = (acos(yc / cylinderRadius) + cylinderAngle) - PI;\n"
			"\n"
			"    float hitAngleMod = mod(hitAngle, 2.0 * PI);\n"
			"    if ((hitAngleMod > PI && amount < 0.5) || (hitAngleMod > PI/2.0 && amount < 0.0))\n"
			"    {\n"
			"        return seeThrough(yc, p, rotation, rrotation);\n"
			"    }\n"
			"\n"
			"    point = hitPoint(hitAngle, yc, point, rrotation);\n"
			"\n"
			"    if (point.x < 0.0 || point.y < 0.0 || point.x > 1.0 || point.y > 1.0)\n"
			"    {\n"
			"        return seeThroughWithShadow(yc, p, point, rotation, rrotation);\n"
			"    }\n"
			"\n"
			"    vec4 color = backside(yc, point);\n"
			"\n"
			"    vec4 otherColor;\n"
			"    if (yc < 0.0)\n"
			"    {\n"
			"        float shado = 1.0 - (sqrt(pow(point.x - 0.5, 2.0) + pow(point.y - 0.5, 2.0)) / 0.71);\n"
			"        shado *= pow(-yc / cylinderRadius, 3.0);\n"
			"        shado *= 0.5;\n"
			"        otherColor = vec4(0.0, 0.0, 0.0, shado);\n"
			"    }\n"
			"    else\n"
			"    {\n"
			"        otherColor = texture(u_texture0, p);\n"
			"    }\n"
			"\n"
			"    color = antiAlias(color, otherColor, cylinderRadius - abs(yc));\n"
			"\n"
			"    vec4 cl = seeThroughWithShadow(yc, p, point, rotation, rrotation);\n"
			"    float dist = distanceToEdge(point);\n"
			"\n"
			"    return antiAlias(color, cl, dist);\n"
			"}\n"
			"\n"
			"void main()\n"
			"{\n"
			"    amount = u_offset * (MAX_AMOUNT - MIN_AMOUNT) + MIN_AMOUNT;\n"
			"    cylinderCenter = amount;\n"
			"    cylinderAngle = 2.0 * PI * amount;\n"
			"\n"
			"    outColor = transition(v_texCoord);\n"
			"}";

	m_ProgramObj = GLUtils::CreateProgram(vShaderStr, fShaderStr);
	if (m_ProgramObj)
	{
		m_MVPMatLoc = glGetUniformLocation(m_ProgramObj, "u_MVPMatrix");
	}
	else
	{
		LOGCATE("GLTransitionExample::Init create program fail");
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

    for (int i = 0; i < BF_IMG_NUM; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, m_TextureIds[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_RenderImages[i].width, m_RenderImages[i].height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_RenderImages[i].ppPlane[0]);
        glBindTexture(GL_TEXTURE_2D, GL_NONE);
    }
}

void GLTransitionExample::LoadImage(NativeImage *pImage)
{
	LOGCATE("GLTransitionExample::LoadImage pImage = %p", pImage->ppPlane[0]);
}

void GLTransitionExample::Draw(int screenW, int screenH)
{
	LOGCATE("GLTransitionExample::Draw()");

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

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_TextureIds[m_loopCount % BF_IMG_NUM]);
	GLUtils::setInt(m_ProgramObj, "u_texture0", 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_TextureIds[(m_loopCount + 1) % BF_IMG_NUM]);
	GLUtils::setInt(m_ProgramObj, "u_texture1", 1);

	GLUtils::setVec2(m_ProgramObj, "u_texSize", m_RenderImages[0].width, m_RenderImages[0].height);
	GLUtils::setFloat(m_ProgramObj, "u_offset", offset);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (const void *)0);

}

void GLTransitionExample::Destroy()
{
	if (m_ProgramObj)
	{
		glDeleteProgram(m_ProgramObj);
		glDeleteBuffers(3, m_VboIds);
		glDeleteVertexArrays(1, &m_VaoId);
		glDeleteTextures(BF_IMG_NUM, m_TextureIds);
	}
}

void GLTransitionExample::UpdateMVPMatrix(glm::mat4 &mvpMatrix, int angleX, int angleY, float ratio)
{
	LOGCATE("GLTransitionExample::UpdateMVPMatrix angleX = %d, angleY = %d, ratio = %f", angleX, angleY, ratio);
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

void GLTransitionExample::UpdateTransformMatrix(float rotateX, float rotateY, float scaleX, float scaleY)
{
	GLSampleBase::UpdateTransformMatrix(rotateX, rotateY, scaleX, scaleY);
	m_AngleX = static_cast<int>(rotateX);
	m_AngleY = static_cast<int>(rotateY);
	m_ScaleX = scaleX;
	m_ScaleY = scaleY;
}

void GLTransitionExample::LoadMultiImageWithIndex(int index, NativeImage *pImage) {
	LOGCATE("GLTransitionExample::LoadMultiImageWithIndex pImage = %p,[w=%d,h=%d,f=%d]", pImage->ppPlane[0], pImage->width, pImage->height, pImage->format);
	if (pImage && index >=0 && index < BF_IMG_NUM)
	{
		m_RenderImages[index].width = pImage->width;
		m_RenderImages[index].height = pImage->height;
		m_RenderImages[index].format = pImage->format;
		NativeImageUtil::CopyNativeImage(pImage, &m_RenderImages[index]);
		//NativeImageUtil::DumpNativeImage(&m_GrayImage, "/sdcard/DCIM", "GLTransitionExample");
    }
}
