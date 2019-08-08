//
// Created by ByteFlow on 2019/7/30.
//

#include <gtc/matrix_transform.hpp>
#include "BasicLightingSample.h"
#include "../util/GLUtils.h"

BasicLightingSample::BasicLightingSample()
{

	m_SamplerLoc = GL_NONE;
	m_MVPMatLoc = GL_NONE;
	m_ModelMatrixLoc = GL_NONE;
	m_LightPosLoc = GL_NONE;

	m_TextureId = GL_NONE;
	m_VaoId = GL_NONE;

	m_AngleX = 0;
	m_AngleY = 0;

	m_ModelMatrix = glm::mat4(0.0f);
}

BasicLightingSample::~BasicLightingSample()
{
	NativeImageUtil::FreeNativeImage(&m_RenderImage);

}

void BasicLightingSample::Init()
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
			"uniform mat4 u_MVPMatrix;\n"
			"uniform mat4 u_ModelMatrix;\n"
			"uniform vec3 lightPos;\n"
			"uniform vec3 lightColor;\n"
			"uniform vec3 viewPos;\n"
			"out vec2 v_texCoord;\n"
			"out vec3 ambient;\n"
			"out vec3 diffuse;\n"
			"out vec3 specular;\n"
			"out vec3 outVec0;\n"
			"out vec3 outVec1;\n"
			"void main()\n"
			"{\n"
			"    gl_Position = u_MVPMatrix * a_position;\n"
			"    vec3 fragPos = vec3(u_ModelMatrix * a_position);\n"
			"\n"
			"    // Ambient\n"
			"    float ambientStrength = 0.2;\n"
			"    ambient = ambientStrength * lightColor;\n"
			"\n"
			"    // Diffuse\n"
			"    vec3 unitNormal = normalize(vec3(u_ModelMatrix * vec4(a_normal, 1.0)));\n"
			"    vec3 lightDir = normalize(lightPos - fragPos);\n"
			"    float diff = max(dot(unitNormal, lightDir), 0.0);\n"
			"    diffuse = diff * lightColor;\n"
			"\n"
			"    // Specular\n"
			"    float specularStrength = 0.9;\n"
			"    vec3 viewDir = normalize(viewPos - fragPos);\n"
			"    vec3 reflectDir = reflect(-lightDir, unitNormal);\n"
			"    float spec = pow(max(dot(unitNormal, reflectDir), 0.0), 16.0);\n"
			"    specular = specularStrength * spec * lightColor;\n"
			"    outVec0 = unitNormal;outVec1 = specular;\n"
			"    v_texCoord = a_texCoord;\n"
			"}";

	char fShaderStr[] =
			"#version 300 es\n"
			"precision mediump float;\n"
			"in vec2 v_texCoord;\n"
			"in vec3 ambient;\n"
			"in vec3 diffuse;\n"
			"in vec3 specular;\n"
			"layout(location = 0) out vec4 outColor;\n"
			"uniform sampler2D s_TextureMap;\n"
			"void main()\n"
			"{\n"
			"    vec4 objectColor = texture(s_TextureMap, v_texCoord);\n"
			"    vec3 finalColor = (ambient + diffuse + specular) * vec3(objectColor);\n"
			"    outColor = vec4(finalColor, 1.0);\n"
			"}";
	GLchar const * varyings[] = {"outVec0", "outVec1"};
	m_ProgramObj = GLUtils::CreateProgramWithFeedback(vShaderStr, fShaderStr, m_VertexShader, m_FragmentShader, varyings,
													  sizeof(varyings)/ sizeof(varyings[0]));
	if (m_ProgramObj)
	{
		m_SamplerLoc = glGetUniformLocation(m_ProgramObj, "s_TextureMap");
		m_MVPMatLoc = glGetUniformLocation(m_ProgramObj, "u_MVPMatrix");
		m_ModelMatrixLoc = glGetUniformLocation(m_ProgramObj, "u_ModelMatrix");
		m_LightPosLoc = glGetUniformLocation(m_ProgramObj, "lightPos");
		m_LightColorLoc = glGetUniformLocation(m_ProgramObj, "lightColor");
		m_ViewPosLoc = glGetUniformLocation(m_ProgramObj, "viewPos");
	}
	else
	{
		LOGCATE("BasicLightingSample::Init create program fail");
		return;
	}


	//glTransformFeedbackVaryings(m_ProgramObj, 1, Strings, GL_INTERLEAVED_ATTRIBS);

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


	glGenBuffers(1, &m_TfoBufId);
	glBindBuffer(GL_ARRAY_BUFFER, m_TfoBufId);
	glBufferData(GL_ARRAY_BUFFER, 6 * 36 * sizeof(GLfloat), NULL, GL_STATIC_READ);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenTransformFeedbacks(1, &m_TfoId);
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, m_TfoId);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, m_TfoBufId); // Specify the index of the binding point within the array specified by target.
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
	glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0);


}

void BasicLightingSample::LoadImage(NativeImage *pImage)
{
	LOGCATE("BasicLightingSample::LoadImage pImage = %p", pImage->ppPlane[0]);
	if (pImage)
	{
		m_RenderImage.width = pImage->width;
		m_RenderImage.height = pImage->height;
		m_RenderImage.format = pImage->format;
		NativeImageUtil::CopyNativeImage(pImage, &m_RenderImage);
	}

}

void BasicLightingSample::Draw(int screenW, int screenH)
{
	LOGCATE("BasicLightingSample::Draw()");

	if (m_ProgramObj == GL_NONE || m_TextureId == GL_NONE) return;
	glEnable(GL_DEPTH_TEST);

	UpdateMVPMatrix(m_MVPMatrix, m_AngleX, m_AngleY, (float) screenW / screenH);

	//upload RGBA image data
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_TextureId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_RenderImage.width, m_RenderImage.height, 0, GL_RGBA,
				 GL_UNSIGNED_BYTE, m_RenderImage.ppPlane[0]);
	glBindTexture(GL_TEXTURE_2D, GL_NONE);

	// Use the program object
	glUseProgram(m_ProgramObj);

	glBindVertexArray(m_VaoId);

	glUniformMatrix4fv(m_MVPMatLoc, 1, GL_FALSE, &m_MVPMatrix[0][0]);
	glUniformMatrix4fv(m_ModelMatrixLoc, 1, GL_FALSE, &m_ModelMatrix[0][0]);

	glUniform3f(m_LightColorLoc,  1.0f, 1.0f, 1.0f);
	glUniform3f(m_LightPosLoc,    -2.0f, 0.0f, 2.0f);
	glUniform3f(m_ViewPosLoc,     -2.0f, 0.0f, 2.0f);

	// Bind the RGBA map
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_TextureId);
	glUniform1i(m_SamplerLoc, 0);

	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, m_TfoId);
	glBeginTransformFeedback(GL_TRIANGLES);
	GO_CHECK_GL_ERROR();

	glDrawArrays(GL_TRIANGLES, 0, 36);
	GO_CHECK_GL_ERROR();

	glEndTransformFeedback();
	GO_CHECK_GL_ERROR();
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
	GO_CHECK_GL_ERROR();

	glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, m_TfoBufId);
	GO_CHECK_GL_ERROR();
	void* rawData = glMapBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0,  6 * 36 * sizeof(GLfloat), GL_MAP_READ_BIT);
	GO_CHECK_GL_ERROR();

	float *p = (float*)rawData;
	for(int i=0; i< 36; i++)
	{
		LOGCATE("BasicLightingSample::Draw() outVec0[%d] = [%f, %f, %f], outVec1[%d] = [%f, %f, %f]", i, p[i * 6], p[i * 6 + 1], p[i * 6 + 2], i, p[i * 6 + 3], p[i * 6 + 4], p[i * 6 + 5]);
	}


	glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);
	glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0);
}

void BasicLightingSample::Destroy()
{
//	if (m_ProgramObj)
//	{
//		glDeleteProgram(m_ProgramObj);
//		glDeleteBuffers(3, m_VboIds);
//		glDeleteVertexArrays(1, &m_VaoId);
//	}

}


/**
 * @param angleX 绕X轴旋转度数
 * @param angleY 绕Y轴旋转度数
 * @param ratio 宽高比
 * */
void BasicLightingSample::UpdateMVPMatrix(glm::mat4 &mvpMatrix, int angleX, int angleY, float ratio)
{
	LOGCATE("BasicLightingSample::UpdateMVPMatrix angleX = %d, angleY = %d, ratio = %f", angleX,
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
			glm::vec3(-2, 0, 2), // Camera is at (0,0,1), in World Space
			glm::vec3(0, 0, 0), // and looks at the origin
			glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
	);

	// Model matrix
	glm::mat4 Model = glm::mat4(1.0f);
	Model = glm::scale(Model, glm::vec3(1.0f, 1.0f, 1.0f));
	Model = glm::rotate(Model, radiansX, glm::vec3(1.0f, 0.0f, 0.0f));
	Model = glm::rotate(Model, radiansY, glm::vec3(0.0f, 1.0f, 0.0f));
	Model = glm::translate(Model, glm::vec3(0.0f, 0.0f, 0.0f));

	m_ModelMatrix = Model;

	mvpMatrix = Projection * View * Model;

}

void BasicLightingSample::SetParamsInt(int paramType, int value0, int value1)
{
	LOGCATE("BasicLightingSample::SetParamsInt paramType = %d, value0 = %d", paramType, value0);
	GLSampleBase::SetParamsInt(paramType, value0, value1);
	if (paramType == ROTATE_ANGLE_PARAM_TYPE)
	{
		m_AngleX = value0;
		m_AngleY = value1;
	}


}
