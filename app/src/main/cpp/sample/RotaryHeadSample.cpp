//
// Created by ByteFlow on 2020/2/30.
//

#include <gtc/matrix_transform.hpp>
#include "RotaryHeadSample.h"
#include "../util/GLUtils.h"
#include "CommonDef.h"

static float KEY_POINTS[KEY_POINTS_COUNT * 2] =
        {
           146, 101,//0
           174, 36, //1
           232, 6,  //2
           307, 38, //3
           335, 105,//4
           305, 218,//5
           250, 267,//6
           191, 218,//7
           237, 160,//8
        };

static float DotProduct(vec2 a, vec2 b)
{
	return a.x * b.x + a.y * b.y;
}

RotaryHeadSample::RotaryHeadSample()
{

	m_SamplerLoc = GL_NONE;
	m_MVPMatLoc = GL_NONE;

	m_TextureId = GL_NONE;

	m_AngleX = 0;
	m_AngleY = 0;

	m_ScaleX = 1.0f;
	m_ScaleY = 1.0f;

	m_FrameIndex = 0;
}

RotaryHeadSample::~RotaryHeadSample()
{
	NativeImageUtil::FreeNativeImage(&m_RenderImage);

}

void RotaryHeadSample::Init()
{
	if(m_ProgramObj)
		return;

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
			"    gl_PointSize = 8.0;\n"
			"}";

	char fShaderStr[] =
			"#version 300 es\n"
            "precision highp float;\n"
            "layout(location = 0) out vec4 outColor;\n"
            "in vec2 v_texCoord;\n"
            "uniform sampler2D s_TextureMap;\n"
			"uniform float u_type;\n"
            "void main() { \n"
			"    if(u_type == 0.0)\n"
            "    {\n"
			"        outColor = texture(s_TextureMap, v_texCoord);\n"
            "     }\n"
			"    else if(u_type == 1.0)\n"
            "    {\n"
			"        outColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
            "    }\n"
            "}";

	m_ProgramObj = GLUtils::CreateProgram(vShaderStr, fShaderStr, m_VertexShader, m_FragmentShader);
	if (m_ProgramObj)
	{
		m_SamplerLoc = glGetUniformLocation(m_ProgramObj, "s_TextureMap");
		m_MVPMatLoc = glGetUniformLocation(m_ProgramObj, "u_MVPMatrix");
	}
	else
	{
		LOGCATE("RotaryHeadSample::Init create program fail");
	}

	CalculateMesh(0);

	// Generate VBO Ids and load the VBOs with data
	glGenBuffers(2, m_VboIds);
	glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(m_Vertices), m_Vertices, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(m_TexCoords), m_TexCoords, GL_STATIC_DRAW);

	// Generate VAO Id
	glGenVertexArrays(1, &m_VaoId);
	glBindVertexArray(m_VaoId);

	glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[0]);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (const void *)0);
	glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);
	GO_CHECK_GL_ERROR();

	glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[1]);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (const void *)0);
	glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);
	GO_CHECK_GL_ERROR();

	glBindVertexArray(GL_NONE);
}

void RotaryHeadSample::LoadImage(NativeImage *pImage)
{
	LOGCATE("RotaryHeadSample::LoadImage pImage = %p", pImage->ppPlane[0]);
	if (pImage)
	{
        ScopedSyncLock lock(&m_Lock);
		m_RenderImage.width = pImage->width;
		m_RenderImage.height = pImage->height;
		m_RenderImage.format = pImage->format;
		NativeImageUtil::CopyNativeImage(pImage, &m_RenderImage);
	}

}

void RotaryHeadSample::Draw(int screenW, int screenH)
{
	LOGCATE("RotaryHeadSample::Draw() [w,h]=[%d,%d]", screenW, screenH);

	if(m_ProgramObj == GL_NONE) return;

	if(m_TextureId == GL_NONE)
    {
        ScopedSyncLock lock(&m_Lock);
        if(m_RenderImage.ppPlane[0] != nullptr)
        {
            glGenTextures(1, &m_TextureId);
            glBindTexture(GL_TEXTURE_2D, m_TextureId);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_RenderImage.width, m_RenderImage.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_RenderImage.ppPlane[0]);
            glBindTexture(GL_TEXTURE_2D, GL_NONE);
            GO_CHECK_GL_ERROR();
        }
        return;
    }
	glViewport(0, 0, screenW, screenH);

	m_FrameIndex ++;
	UpdateMVPMatrix(m_MVPMatrix, m_AngleX, m_AngleY, (float)screenW / screenH);


	float ratio = (m_FrameIndex % 100) * 1.0f / 100;
	//ratio = (m_FrameIndex / 100) % 2 == 1 ? (1 - ratio) : ratio;

	CalculateMesh(static_cast<float>(ratio * 2 * MATH_PI));
	glUseProgram (m_ProgramObj);
	glBindVertexArray(m_VaoId);
	glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[0]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(m_Vertices), m_Vertices);
	glUniformMatrix4fv(m_MVPMatLoc, 1, GL_FALSE, &m_MVPMatrix[0][0]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_TextureId);
	glUniform1i(m_SamplerLoc, 0);

	GLUtils::setFloat(m_ProgramObj, "u_type", 0);
	glDrawArrays(GL_TRIANGLES, 0, TRIANGLE_COUNT * 3);

	GLUtils::setFloat(m_ProgramObj, "u_type", 1);
	//glDrawArrays(GL_LINE_STRIP, 0, TRIANGLE_COUNT * 3);

}

void RotaryHeadSample::Destroy()
{
	if (m_ProgramObj)
	{
		glDeleteProgram(m_ProgramObj);
		glDeleteTextures(1, &m_TextureId);
		glDeleteBuffers(2, m_VboIds);
		glDeleteVertexArrays(1, &m_VaoId);
		m_ProgramObj = GL_NONE;
	}
}


/**
 * @param angleX 绕X轴旋转度数
 * @param angleY 绕Y轴旋转度数
 * @param ratio 宽高比
 * */
void RotaryHeadSample::UpdateMVPMatrix(glm::mat4 &mvpMatrix, int angleX, int angleY, float ratio)
{
	LOGCATE("RotaryHeadSample::UpdateMVPMatrix angleX = %d, angleY = %d, ratio = %f", angleX, angleY, ratio);
	angleX = angleX % 360;
	angleY = angleY % 360;

	//转化为弧度角
	float radiansX = static_cast<float>(MATH_PI / 180.0f * angleX);
	float radiansY = static_cast<float>(MATH_PI / 180.0f * angleY);


	// Projection matrix
	glm::mat4 Projection = glm::ortho( -1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 100.0f);
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

void RotaryHeadSample::UpdateTransformMatrix(float rotateX, float rotateY, float scaleX, float scaleY)
{
	GLSampleBase::UpdateTransformMatrix(rotateX, rotateY, scaleX, scaleY);
	m_AngleX = static_cast<int>(rotateX);
	m_AngleY = static_cast<int>(rotateY);
	m_ScaleX = scaleX;
	m_ScaleY = scaleY;
}

void RotaryHeadSample::CalculateMesh(float rotaryAngle) {

	vec2 centerPoint(KEY_POINTS[16] / m_RenderImage.width, KEY_POINTS[17] / m_RenderImage.height);
	//centerPoint = RotaryKeyPoint(centerPoint, rotaryAngle);
	m_KeyPointsInts[8] = centerPoint;
	m_KeyPoints[8] = RotaryKeyPoint(centerPoint, rotaryAngle);
	for (int i = 0; i < KEY_POINTS_COUNT - 1; ++i) {
		vec2 inputPoint(KEY_POINTS[i * 2] / m_RenderImage.width, KEY_POINTS[i * 2 + 1] / m_RenderImage.height);
		//inputPoint = RotaryKeyPoint(inputPoint, rotaryAngle);
		m_KeyPoints[i] = RotaryKeyPoint(inputPoint, rotaryAngle);;
		m_KeyPointsInts[i] = CalculateIntersection(inputPoint, centerPoint);
		LOGCATE("RotaryHeadSample::CalculateMesh index=%d, input[x,y]=[%f, %f], interscet[x, y]=[%f, %f]", i,
				m_KeyPoints[i].x, m_KeyPoints[i].y, m_KeyPointsInts[i].x, m_KeyPointsInts[i].y);
	}

    for (int i = 0; i < KEY_POINTS_COUNT - 1; ++i) {
        m_MeshPoints[i * 2] = m_KeyPointsInts[i];
        m_MeshPoints[i * 2 + 1] = m_KeyPoints[i];
    }
    m_MeshPoints[16] = m_KeyPointsInts[0];
    m_MeshPoints[17] = m_KeyPoints[0];

    //周围
	for (int i = 2; i < KEY_POINTS_COUNT * 2; ++i) {
		m_TexCoords[(i-2) * 3] = m_MeshPoints[i-2];
		m_TexCoords[(i-2) * 3 + 1] = m_MeshPoints[i-1];
		m_TexCoords[(i-2) * 3 + 2] = m_MeshPoints[i];
	}

	//中心
	for (int i = 0; i < KEY_POINTS_COUNT - 2; ++i) {
		m_TexCoords[(i + 16) * 3] = m_KeyPoints[i];
		m_TexCoords[(i + 16) * 3 + 1] = m_KeyPoints[i+1];
		m_TexCoords[(i + 16) * 3 + 2] = m_KeyPoints[8];
	}
	m_TexCoords[23 * 3 + 2] = m_KeyPoints[7];
	m_TexCoords[23 * 3 + 1] = m_KeyPoints[8];
	m_TexCoords[23 * 3 + 0] = m_KeyPoints[0];

	//处理四个角
    m_TexCoords[24 * 3]     = vec2(0,0);
    m_TexCoords[24 * 3 + 1] = m_KeyPointsInts[0];
    m_TexCoords[24 * 3 + 2] = m_KeyPointsInts[1];

    m_TexCoords[25 * 3]     = vec2(0,0);
    m_TexCoords[25 * 3 + 1] = m_KeyPointsInts[1];
    m_TexCoords[25 * 3 + 2] = m_KeyPointsInts[2];

    m_TexCoords[26 * 3]     = vec2(1,0);
    m_TexCoords[26 * 3 + 1] = m_KeyPointsInts[2];
    m_TexCoords[26 * 3 + 2] = m_KeyPointsInts[3];

    m_TexCoords[27 * 3]     = vec2(1,0);
    m_TexCoords[27 * 3 + 1] = m_KeyPointsInts[3];
    m_TexCoords[27 * 3 + 2] = m_KeyPointsInts[4];

    m_TexCoords[28 * 3]     = vec2(1,1);
    m_TexCoords[28 * 3 + 1] = m_KeyPointsInts[4];
    m_TexCoords[28 * 3 + 2] = m_KeyPointsInts[5];

    m_TexCoords[29 * 3]     = vec2(1,1);
    m_TexCoords[29 * 3 + 1] = m_KeyPointsInts[5];
    m_TexCoords[29 * 3 + 2] = m_KeyPointsInts[6];

    m_TexCoords[30 * 3]     = vec2(0,1);
    m_TexCoords[30 * 3 + 1] = m_KeyPointsInts[6];
    m_TexCoords[30 * 3 + 2] = m_KeyPointsInts[7];

    m_TexCoords[31 * 3]     = vec2(0,1);
    m_TexCoords[31 * 3 + 1] = m_KeyPointsInts[7];
    m_TexCoords[31 * 3 + 2] = m_KeyPointsInts[0];

	for (int i = 0; i < TRIANGLE_COUNT * 3; ++i) {
		//LOGCATE("m_TexCoords[%d]=(%f, %f)", i, m_TexCoords[i].x, m_TexCoords[i].y);
		m_Vertices[i] = vec3( m_TexCoords[i].x * 2 - 1, 1 - 2 * m_TexCoords[i].y, 0);
	}
}

vec2 RotaryHeadSample::CalculateIntersection(vec2 inputPoint, vec2 centerPoint) {
	vec2 outputPoint;
	if(inputPoint.x == centerPoint.x) //直线与 y 轴平行
	{
		vec2 pointA(inputPoint.x, 0);
		vec2 pointB(inputPoint.x, 1);

		float dA = distance(inputPoint, pointA);
		float dB = distance(inputPoint, pointB);
		outputPoint = dA > dB ? pointB : pointA;
		return outputPoint;
	}

	if(inputPoint.y == centerPoint.y) //直线与 x 轴平行
	{
		vec2 pointA(0, inputPoint.y);
		vec2 pointB(1, inputPoint.y);

		float dA = distance(inputPoint, pointA);
		float dB = distance(inputPoint, pointB);
		outputPoint = dA > dB ? pointB : pointA;
		return outputPoint;
	}

	// y = a*x + c
	float a=0, c=0;

	a = (inputPoint.y - centerPoint.y) / (inputPoint.x - centerPoint.x);

	c = inputPoint.y - a * inputPoint.x;

	//x=0, x=1, y=0, y=1 四条线交点

	//x=0
	vec2 point_0(0, c);
	float d0 = DotProduct((centerPoint - inputPoint),(centerPoint - point_0));

	if(c >= 0 && c <= 1 && d0 > 0)
		outputPoint = point_0;

	//x=1
	vec2 point_1(1, a + c);
	float d1 = DotProduct((centerPoint - inputPoint),(centerPoint - point_1));

	if((a + c) >= 0 && (a + c) <= 1 && d1 > 0)
		outputPoint = point_1;

	//y=0
	vec2 point_2(-c / a, 0);
	float d2 = DotProduct((centerPoint - inputPoint),(centerPoint - point_2));

	if((-c / a) >= 0 && (-c / a) <= 1 && d2 > 0)
		outputPoint = point_2;

	//y=1
	vec2 point_3((1-c) / a, 1);
	float d3 = DotProduct((centerPoint - inputPoint),(centerPoint - point_3));

	if(((1-c) / a) >= 0 && ((1-c) / a) <= 1 && d3 > 0)
		outputPoint = point_3;

	return outputPoint;
}

vec2 RotaryHeadSample::RotaryKeyPoint(vec2 input, float rotaryAngle) {
	return input + vec2(cos(rotaryAngle), sin(rotaryAngle)) * 0.02f;
}

//vec2 RotaryHeadSample::WarpKeyPoint(vec2 input, vec2 centerPoint, float level) {
//	vec2 output;
//	vec2 direct_vec = centerPoint - input;
//	output = input + level * direct_vec * 0.24f;
//	return output;
//}
