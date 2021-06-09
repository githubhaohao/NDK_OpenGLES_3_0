//
// Created by ByteFlow on 2019/7/16.
//

#include <GLUtils.h>
#include <gtc/matrix_transform.hpp>
#include "FBOLegLengthenSample.h"

#define VERTEX_POS_INDX  0
#define TEXTURE_POS_INDX 1

const char vShaderStr[] =
		"#version 300 es                            \n"
		"layout(location = 0) in vec4 a_position;   \n"
		"layout(location = 1) in vec2 a_texCoord;   \n"
		"out vec2 v_texCoord;                       \n"
		"uniform mat4 u_MVPMatrix;                  \n"
		"void main()                                \n"
		"{                                          \n"
		"   gl_Position = u_MVPMatrix * a_position; \n"
		"   v_texCoord = a_texCoord;                \n"
		"}                                          \n";

const char fShaderStr[] =
		"#version 300 es\n"
		"precision mediump float;\n"
		"in vec2 v_texCoord;\n"
		"layout(location = 0) out vec4 outColor;\n"
		"uniform sampler2D s_TextureMap;\n"
		"void main()\n"
		"{\n"
		"    outColor = texture(s_TextureMap, v_texCoord);\n"
		"}";

const char vFboShaderStr[] =
		"#version 300 es                            \n"
		"layout(location = 0) in vec4 a_position;   \n"
		"layout(location = 1) in vec2 a_texCoord;   \n"
		"out vec2 v_texCoord;                       \n"
		"uniform mat4 u_MVPMatrix;\n"
		"void main()                                \n"
		"{                                          \n"
		"   gl_Position = u_MVPMatrix * a_position; \n"
		"   v_texCoord = a_texCoord;                \n"
		"}                                          \n";

const char fFboShaderStr[] =
		"#version 300 es\n"
		"precision mediump float;\n"
		"in vec2 v_texCoord;\n"
		"layout(location = 0) out vec4 outColor;\n"
		"uniform sampler2D s_TextureMap;\n"
		"void main()\n"
		"{\n"
		"    vec4 tempColor = texture(s_TextureMap, v_texCoord);\n"
		"    float luminance = tempColor.r * 0.299 + tempColor.g * 0.587 + tempColor.b * 0.114;\n"
		"    //outColor = vec4(vec3(luminance), tempColor.a);\n"
		"    outColor = tempColor;\n"
		"}"; // 输出灰度图

const GLushort V_EIGHT_POINT_INDICES[] = { 0, 1, 2,  0, 2, 3,  1, 4, 7,  1, 7, 2,  4, 5, 6,  4, 6, 7};
const GLushort V_SIX_POINT_INDICES[] = { 0, 1, 2,  0, 2, 3,  1, 4, 5,  1, 5, 2};

const GLushort H_EIGHT_POINT_INDICES[] = { 0, 1, 2,  0, 2, 3,  3, 2, 5,  3, 5, 4,  4, 5, 6,  4, 6, 7};
const GLushort H_SIX_POINT_INDICES[] = { 0, 1, 2,  0, 2, 3,  3, 2, 5,  3, 5, 4,};

const GLushort FOUR_POINT_INDICES[] = { 0, 1, 2,  0, 2, 3};

FBOLegLengthenSample::FBOLegLengthenSample()
{
	m_ImageTextureId = GL_NONE;
	m_FboTextureId = GL_NONE;
	m_SamplerLoc = GL_NONE;
	m_FboId = GL_NONE;
	m_FboProgramObj = GL_NONE;
	m_FboVertexShader = GL_NONE;
	m_FboFragmentShader = GL_NONE;
	m_FboSamplerLoc = GL_NONE;

	m_dt = 0.0;
	m_isgo = true;

	m_StretchMode = VERTICAL_STRETCH_8_POINTS;
}

FBOLegLengthenSample::~FBOLegLengthenSample()
{
	NativeImageUtil::FreeNativeImage(&m_RenderImage);
}

void FBOLegLengthenSample::LoadImage(NativeImage *pImage)
{
	LOGCATE("FBOLegLengthenSample::LoadImage pImage = %p", pImage->ppPlane[0]);
	if (pImage)
	{
		m_RenderImage.width = pImage->width;
		m_RenderImage.height = pImage->height;
		m_RenderImage.format = pImage->format;
		NativeImageUtil::CopyNativeImage(pImage, &m_RenderImage);
	}
}

void FBOLegLengthenSample::Init()
{
	m_bIsVerticalMode = true;

	RectF inRectF;
	inRectF.left = 0.0f ;
	inRectF.right = m_RenderImage.width;
	inRectF.top = m_RenderImage.height * 0.5f;
	inRectF.bottom = m_RenderImage.height;

	m_StretchRect.left = inRectF.left / m_RenderImage.width;
	m_StretchRect.right = inRectF.right / m_RenderImage.width;
	m_StretchRect.top = inRectF.top / m_RenderImage.height;
	m_StretchRect.bottom = inRectF.bottom / m_RenderImage.height;

	if (m_bIsVerticalMode)
	{
		if (m_StretchRect.top == 0 && m_StretchRect.bottom == 1.0f)
		{
			m_StretchMode = VERTICAL_STRETCH_4_POINTS;
		}
		else if (m_StretchRect.top == 0.0f)
		{
			m_StretchMode = VERTICAL_STRETCH_TOP_6_POINTS;
		}
		else if (m_StretchRect.bottom == 1.0f)
		{
			m_StretchMode = VERTICAL_STRETCH_BOTTOM_6_POINTS;
		}
		else
		{
			m_StretchMode = VERTICAL_STRETCH_8_POINTS;
		}
	}
	else
	{
		if (m_StretchRect.left == 0 && m_StretchRect.right == 1.0f)
		{
			m_StretchMode = HORIZONTAL_STRETCH_4_POINTS;
		}
		else if (m_StretchRect.left == 0.0f)
		{
			m_StretchMode = HORIZONTAL_STRETCH_LEFT_6_POINTS;
		}
		else if (m_StretchRect.right == 1.0f)
		{
			m_StretchMode = HORIZONTAL_STRETCH_RIGHT_6_POINTS;
		}
		else
		{
			m_StretchMode = HORIZONTAL_STRETCH_8_POINTS;
		}
	}

	if (m_dt <= -0.2)
	{
		m_isgo = true;
	}

	if (m_dt >= 0.2)
	{
		m_isgo = false;
	}

	if (m_isgo)
	{
		m_dt += 0.01;
	}
	else
	{
		m_dt -= 0.01;
	}

	float y1 = 1 - 2 * m_StretchRect.top;
	float y2 = 1 - 2 * m_StretchRect.bottom;
	float x1 = 2 * m_StretchRect.left - 1;
	float x2 = 2 * m_StretchRect.right - 1;

	float wbl = m_RenderImage.width*1.0f / m_RenderImage.height;
	float hbl = 1 / wbl;
	if (wbl > hbl)
	{
		wbl = 1.0f;
	}
	else
	{
		hbl = 1.0f;
	}

	/**vertical 顶点坐标*/
	GLfloat vVertices[] = {
			-0.8f * wbl,   0.8f* hbl + m_dt*0.8f, 0.0f,
			-0.8f * wbl,  -0.8f* hbl - m_dt*0.8f, 0.0f,
			 0.8f * wbl,  -0.8f* hbl - m_dt*0.8f, 0.0f,
			 0.8f * wbl,   0.8f* hbl + m_dt*0.8f, 0.0f,
	};

	/**horizontal 顶点坐标*/
	GLfloat vHVertices[] = {
			-0.8f * wbl  - m_dt*0.8f,   0.8f* hbl, 0.0f,
			-0.8f * wbl  - m_dt*0.8f,  -0.8f* hbl, 0.0f,
			 0.8f * wbl  + m_dt*0.8f,  -0.8f* hbl, 0.0f,
			 0.8f * wbl  + m_dt*0.8f,   0.8f* hbl, 0.0f,
	};
//	GLfloat vHVertices[] = {
//			-0.8f   - m_dt*0.8f,   0.8f, 0.0f,
//			-0.8f   - m_dt*0.8f,  -0.8f, 0.0f,
//			0.8f  + m_dt*0.8f,  -0.8f, 0.0f,
//			0.8f  + m_dt*0.8f,   0.8f, 0.0f,
//	};

	//正常纹理坐标
	GLfloat vTexCoors[] = {
			0.0f, 0.0f,
			0.0f, 1.0f,
			1.0f, 1.0f,
			1.0f, 0.0f,
	};


	/** 8 points vertical*/
	GLfloat vEightPointsFboVertices[] = {
			-1.0f,  1.0f, 0.0f,
			-1.0f,  y1 + m_dt, 0.0f,
			 1.0f,  y1 + m_dt, 0.0f,
			 1.0f,  1.0f, 0.0f,

			-1.0f,  y2 - m_dt, 0.0f,
			-1.0f, -1.0f , 0.0f,
			 1.0f, -1.0f, 0.0f,
			 1.0f,  y2 - m_dt, 0.0f,
	};

	//fbo 纹理坐标
	GLfloat vEightPointsFboTexCoors[] = {
			0.0f, 0.0f,
			0.0f, m_StretchRect.top,
			1.0f, m_StretchRect.top,
			1.0f, 0.0f,

			0.0f, m_StretchRect.bottom,
			0.0f, 1.0f,
			1.0f, 1.0f,
			1.0f, m_StretchRect.bottom,
	};

	/** 8 points horizontal*/
	GLfloat vHEightPointsFboVertices[] = {
			-1.0f,       1.0f, 0.0f,
			-1.0f,      -1.0f, 0.0f,
			x1 - m_dt,  -1.0f, 0.0f,
			x1 - m_dt,   1.0f, 0.0f,

			x2 + m_dt,   1.0f, 0.0f,
			x2 + m_dt,  -1.0f, 0.0f,
			 1.0f,      -1.0f, 0.0f,
			 1.0f,       1.0f, 0.0f,
	};

	//fbo 纹理坐标
	GLfloat vHEightPointsFboTexCoors[] = {
			0.0f,               0.0f,
			0.0f,               1.0f,
			m_StretchRect.left, 1.0f,
			m_StretchRect.left, 0.0f,

			m_StretchRect.right, 0.0f,
			m_StretchRect.right, 1.0f,
			1.0f, 1.0f,
			1.0f, 0.0f,
	};


	/** 6 points vertical top == 0 **/
	GLfloat vSixPointsTopFboVertices[] = {
			-1.0f,  1.0f, 0.0f,
			-1.0f,  y2 - 2*m_dt, 0.0f,
			1.0f,   y2 - 2*m_dt, 0.0f,
			1.0f,  1.0f, 0.0f,

			-1.0f, -1.0f , 0.0f,
			1.0f, -1.0f, 0.0f,

	};

	//fbo 纹理坐标
	GLfloat vSixPointsTopFboTexCoors[] = {
			0.0f, 0.0f,
			0.0f, m_StretchRect.bottom,
			1.0f, m_StretchRect.bottom,
			1.0f, 0.0f,

			0.0f, 1.0f,
			1.0f, 1.0f,
	};

	/** 6 points horizontal left == 0 **/
	GLfloat vHSixPointsTopFboVertices[] = {
			-1.0f,          1.0f, 0.0f,
			-1.0f,         -1.0f, 0.0f,
			 x2 + 2*m_dt,  -1.0f, 0.0f,
			 x2 + 2*m_dt,   1.0f, 0.0f,

			1.0f,           1.0f, 0.0f,
			1.0f,          -1.0f, 0.0f,

	};

	//fbo 纹理坐标
	GLfloat vHSixPointsTopFboTexCoors[] = {
			0.0f,                0.0f,
			0.0f,                1.0f,
			m_StretchRect.right, 1.0f,
			m_StretchRect.right, 0.0f,

			1.0f, 0.0f,
			1.0f, 1.0f,
	};

	/** 6 points vertical bottom == height*/
	GLfloat vSixPointsBottomFboVertices[] = {
			-1.0f,  1.0f, 0.0f,
			-1.0f,  y1 + 2*m_dt, 0.0f,
			1.0f,   y1 + 2*m_dt, 0.0f,
			1.0f,   1.0f, 0.0f,

			-1.0f, -1.0f , 0.0f,
			1.0f, -1.0f, 0.0f,
	};

	//fbo 纹理坐标
	GLfloat vSixPointsBottomFboTexCoors[] = {
			0.0f, 0.0f,
			0.0f, m_StretchRect.top,
			1.0f, m_StretchRect.top,
			1.0f, 0.0f,

			0.0f, 1.0f,
			1.0f, 1.0f,
	};

	/** 6 points horizontal right == width*/
	GLfloat vHSixPointsBottomFboVertices[] = {
			-1.0f,  1.0f, 0.0f,
			-1.0f, -1.0f, 0.0f,
			 x1 - 2*m_dt,   -1.0f, 0.0f,
			 x1 - 2*m_dt,    1.0f, 0.0f,

			 1.0f, 1.0f , 0.0f,
			 1.0f, -1.0f, 0.0f,
	};

	//fbo 纹理坐标
	GLfloat vHSixPointsBottomFboTexCoors[] = {
			0.0f, 0.0f,
			0.0f, 1.0f,
			m_StretchRect.left, 1.0f,
			m_StretchRect.left, 0.0f,

			1.0f, 0.0f,
			1.0f, 1.0f,
	};

	/**4 points vertical top == 0 && bottom == height, horizontal left == 0 && right == width*/
	GLfloat vFourPointsFboVertices[] = {
			-1.0f,  1.0f, 0.0f,
			-1.0f,  -1.0f, 0.0f,
			 1.0f,  -1.0f, 0.0f,
			 1.0f,  1.0f, 0.0f,

	};

	//fbo 纹理坐标
	GLfloat vFourPointsFboTexCoors[] = {
			0.0f, 0.0f,
			0.0f, 1.0f,
			1.0f, 1.0f,
			1.0f, 0.0f,
	};


	if(m_FboProgramObj) {
		if (m_bIsVerticalMode)
		{
			glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[0]);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vVertices), vVertices);
		}
		else
		{
			glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[0]);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vHVertices), vHVertices);
		}


		switch (m_StretchMode)
		{
			case VERTICAL_STRETCH_8_POINTS:
				glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[4]);
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vEightPointsFboVertices), vEightPointsFboVertices);

				glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[2]);
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vEightPointsFboTexCoors), vEightPointsFboTexCoors);
				break;
			case VERTICAL_STRETCH_TOP_6_POINTS:
				glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[4]);
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vSixPointsTopFboVertices), vSixPointsTopFboVertices);

				glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[2]);
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vSixPointsTopFboTexCoors), vSixPointsTopFboTexCoors);
				break;
			case VERTICAL_STRETCH_BOTTOM_6_POINTS:
				glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[4]);
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vSixPointsBottomFboVertices), vSixPointsBottomFboVertices);

				glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[2]);
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vSixPointsBottomFboTexCoors), vSixPointsBottomFboTexCoors);
				break;
			case VERTICAL_STRETCH_4_POINTS:
			case HORIZONTAL_STRETCH_4_POINTS:
				glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[4]);
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vFourPointsFboVertices), vFourPointsFboVertices);

				glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[2]);
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vFourPointsFboTexCoors), vFourPointsFboTexCoors);
				break;
			case HORIZONTAL_STRETCH_8_POINTS:
				glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[4]);
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vHEightPointsFboVertices), vHEightPointsFboVertices);

				glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[2]);
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vHEightPointsFboTexCoors), vHEightPointsFboTexCoors);
				break;
			case HORIZONTAL_STRETCH_LEFT_6_POINTS:
				glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[4]);
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vHSixPointsTopFboVertices), vHSixPointsTopFboVertices);

				glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[2]);
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vHSixPointsTopFboTexCoors), vHSixPointsTopFboTexCoors);
				break;
			case HORIZONTAL_STRETCH_RIGHT_6_POINTS:
				glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[4]);
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vHSixPointsBottomFboVertices), vHSixPointsBottomFboVertices);

				glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[2]);
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vHSixPointsBottomFboTexCoors), vHSixPointsBottomFboTexCoors);
				break;
			default:
				LOGCATE("FBOLegLengthenSample::Init don't support this mode");
				return;
		}


		if (m_FboTextureId)
		{
			glDeleteTextures(1, &m_FboTextureId);
		}

		glGenTextures(1, &m_FboTextureId);
		glBindTexture(GL_TEXTURE_2D, m_FboTextureId);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, GL_NONE);

		glBindFramebuffer(GL_FRAMEBUFFER, m_FboId);
		glBindTexture(GL_TEXTURE_2D, m_FboTextureId);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_FboTextureId, 0);
		if (m_bIsVerticalMode)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_RenderImage.width,
						 static_cast<GLsizei>(m_RenderImage.height * (1 + 2*m_dt)), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		}
		else
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
						 static_cast<GLsizei>(m_RenderImage.width * (1 + 2 * m_dt)),
						 m_RenderImage.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		}

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER)!= GL_FRAMEBUFFER_COMPLETE) {
			LOGCATE("FBOLegLengthenSample::Init glCheckFramebufferStatus status != GL_FRAMEBUFFER_COMPLETE");
		}
		glBindTexture(GL_TEXTURE_2D, GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
		return;
	}

	// Model matrix vertical mirror
	glm::mat4 Model = glm::mat4(1.0f);
	Model = glm::scale(Model, glm::vec3(1.0f, -1.0f, 1.0f));
	//Model = glm::rotate(Model, radiansX, glm::vec3(1.0f, 0.0f, 0.0f));
	Model = glm::rotate(Model, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	Model = glm::translate(Model, glm::vec3(0.0f, 0.0f, 0.0f));

	m_FboMVPMatrix = Model;

	float ratio = 1080 / 1950.f;
	glm::mat4 Projection = glm::ortho(-ratio, ratio, -1.0f, 1.0f, 0.0f, 1.0f);
	//glm::mat4 Projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f);
	//glm::mat4 Projection = glm::frustum(-ratio, ratio, -1.0f, 1.0f, 0.1f, 100.0f);
	//glm::mat4 Projection = glm::perspective(45.0f,ratio, 0.1f,100.f);

	// View matrix
	glm::mat4 View = glm::lookAt(
			glm::vec3(0, 0, 1), // Camera is at (0,0,1), in World Space
			glm::vec3(0, 0, 0), // and looks at the origin
			glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
	);

	// Model matrix
	Model = glm::mat4(1.0f);
	Model = glm::scale(Model, glm::vec3(1.0f, 1.0f, 1.0f));
	Model = glm::translate(Model, glm::vec3(0.0f, 0.0f, 0.0f));

	m_MVPMatrix = Projection * View * Model;


	m_ProgramObj = GLUtils::CreateProgram(vShaderStr, fShaderStr, m_VertexShader, m_FragmentShader);

	m_FboProgramObj = GLUtils::CreateProgram(vFboShaderStr, fFboShaderStr, m_FboVertexShader, m_FboFragmentShader);

	if (m_ProgramObj == GL_NONE || m_FboProgramObj == GL_NONE)
	{
		LOGCATE("FBOLegLengthenSample::Init m_ProgramObj == GL_NONE");
		return;
	}
	m_SamplerLoc = glGetUniformLocation(m_ProgramObj, "s_TextureMap");
	m_FboSamplerLoc = glGetUniformLocation(m_FboProgramObj, "s_TextureMap");

	m_MVPMatLoc = glGetUniformLocation(m_ProgramObj, "u_MVPMatrix");
	m_FboMVPMatLoc = glGetUniformLocation(m_FboProgramObj, "u_MVPMatrix");

	// Generate VBO Ids and load the VBOs with data
	if (m_bIsVerticalMode)
	{
		glGenBuffers(6, m_VboIds);
		glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vVertices), vVertices, GL_DYNAMIC_DRAW);
	}
	else
	{
		glGenBuffers(6, m_VboIds);
		glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vHVertices), vHVertices, GL_DYNAMIC_DRAW);
	}


	glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vTexCoors), vTexCoors, GL_STATIC_DRAW);

	switch (m_StretchMode)
	{
		case VERTICAL_STRETCH_8_POINTS:
			glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[2]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vEightPointsFboTexCoors), vEightPointsFboTexCoors, GL_DYNAMIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VboIds[3]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(V_EIGHT_POINT_INDICES), V_EIGHT_POINT_INDICES, GL_STATIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[4]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vEightPointsFboVertices), vEightPointsFboVertices, GL_DYNAMIC_DRAW);

			break;
		case VERTICAL_STRETCH_TOP_6_POINTS:
			glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[2]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vSixPointsTopFboTexCoors), vSixPointsTopFboTexCoors, GL_DYNAMIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VboIds[3]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(V_SIX_POINT_INDICES), V_SIX_POINT_INDICES, GL_STATIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[4]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vSixPointsTopFboVertices), vSixPointsTopFboVertices, GL_DYNAMIC_DRAW);

			break;
		case VERTICAL_STRETCH_BOTTOM_6_POINTS:
			glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[2]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vSixPointsBottomFboTexCoors), vSixPointsBottomFboTexCoors, GL_DYNAMIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VboIds[3]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(V_SIX_POINT_INDICES), V_SIX_POINT_INDICES, GL_STATIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[4]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vSixPointsBottomFboVertices), vSixPointsBottomFboVertices, GL_DYNAMIC_DRAW);
			break;
		case VERTICAL_STRETCH_4_POINTS:
		case HORIZONTAL_STRETCH_4_POINTS:
			glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[2]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vFourPointsFboTexCoors), vFourPointsFboTexCoors, GL_DYNAMIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VboIds[3]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(FOUR_POINT_INDICES), FOUR_POINT_INDICES, GL_STATIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[4]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vFourPointsFboVertices), vFourPointsFboVertices, GL_DYNAMIC_DRAW);
			break;
		case HORIZONTAL_STRETCH_8_POINTS:
			glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[2]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vHEightPointsFboTexCoors), vHEightPointsFboTexCoors, GL_DYNAMIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VboIds[3]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(H_EIGHT_POINT_INDICES), H_EIGHT_POINT_INDICES, GL_STATIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[4]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vHEightPointsFboVertices), vHEightPointsFboVertices, GL_DYNAMIC_DRAW);

			break;
		case HORIZONTAL_STRETCH_LEFT_6_POINTS:
			glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[2]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vHSixPointsTopFboTexCoors), vHSixPointsTopFboTexCoors, GL_DYNAMIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VboIds[3]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(H_SIX_POINT_INDICES), H_SIX_POINT_INDICES, GL_STATIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[4]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vHSixPointsTopFboVertices), vHSixPointsTopFboVertices, GL_DYNAMIC_DRAW);

			break;
		case HORIZONTAL_STRETCH_RIGHT_6_POINTS:
			glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[2]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vHSixPointsBottomFboTexCoors), vHSixPointsBottomFboTexCoors, GL_DYNAMIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VboIds[3]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(H_SIX_POINT_INDICES), H_SIX_POINT_INDICES, GL_STATIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[4]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vHSixPointsBottomFboVertices), vHSixPointsBottomFboVertices, GL_DYNAMIC_DRAW);
			break;

		default:
			LOGCATE("FBOLegLengthenSample::Init don't support this mode");
			return;
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VboIds[5]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(FOUR_POINT_INDICES), FOUR_POINT_INDICES, GL_STATIC_DRAW);

	GO_CHECK_GL_ERROR();

	// Generate VAO Ids
	glGenVertexArrays(2, m_VaoIds);

	// Normal rendering VAO
	glBindVertexArray(m_VaoIds[0]);

	glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[0]);
	glEnableVertexAttribArray(VERTEX_POS_INDX);
	glVertexAttribPointer(VERTEX_POS_INDX, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (const void *)0);
	glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);

	glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[1]);
	glEnableVertexAttribArray(TEXTURE_POS_INDX);
	glVertexAttribPointer(TEXTURE_POS_INDX, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (const void *)0);
	glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VboIds[5]);
	GO_CHECK_GL_ERROR();
	glBindVertexArray(GL_NONE);


	// FBO off screen rendering VAO
	glBindVertexArray(m_VaoIds[1]);

	glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[4]);
	glEnableVertexAttribArray(VERTEX_POS_INDX);
	glVertexAttribPointer(VERTEX_POS_INDX, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (const void *)0);
	glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);

	glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[2]);
	glEnableVertexAttribArray(TEXTURE_POS_INDX);
	glVertexAttribPointer(TEXTURE_POS_INDX, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (const void *)0);
	glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VboIds[3]);
	GO_CHECK_GL_ERROR();
	glBindVertexArray(GL_NONE);

	glGenTextures(1, &m_ImageTextureId);
	glBindTexture(GL_TEXTURE_2D, m_ImageTextureId);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_RenderImage.width, m_RenderImage.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_RenderImage.ppPlane[0]);
	glBindTexture(GL_TEXTURE_2D, GL_NONE);
	GO_CHECK_GL_ERROR();

	if (!CreateFrameBufferObj())
	{
		LOGCATE("FBOLegLengthenSample::Init CreateFrameBufferObj fail");
		return;
	}
}

void FBOLegLengthenSample::Draw(int screenW, int screenH)
{
	LOGCATE("FBOLegLengthenSample::Draw [screenW, screenH] = [%d, %d]", screenW, screenH);
	//纹理就是一个“可以被采样的复杂的数据集合” 纹理作为 GPU 图像数据结构
	//glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	if (m_bIsVerticalMode)
	{
		glViewport(0, 0, static_cast<GLsizei>(m_RenderImage.width),
				   static_cast<GLsizei>(m_RenderImage.height*(1+2*m_dt)));
	}
	else
	{
		glViewport(0, 0, static_cast<GLsizei>(m_RenderImage.width*(1+2*m_dt)),
				   static_cast<GLsizei>(m_RenderImage.height));
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Do FBO off screen rendering
	glUseProgram(m_FboProgramObj);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FboId);

	glBindVertexArray(m_VaoIds[1]);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_ImageTextureId);
	glUniform1i(m_FboSamplerLoc, 0);
	glUniformMatrix4fv(m_FboMVPMatLoc, 1, GL_FALSE, &m_FboMVPMatrix[0][0]);
	GO_CHECK_GL_ERROR();
	GLsizei indicesNum;
	switch (m_StretchMode)
	{
		case VERTICAL_STRETCH_8_POINTS:
		case HORIZONTAL_STRETCH_8_POINTS:
			indicesNum = sizeof(V_EIGHT_POINT_INDICES) / sizeof(GLushort);
			break;
		case VERTICAL_STRETCH_TOP_6_POINTS:
		case VERTICAL_STRETCH_BOTTOM_6_POINTS:
		case HORIZONTAL_STRETCH_LEFT_6_POINTS:
		case HORIZONTAL_STRETCH_RIGHT_6_POINTS:
			indicesNum = sizeof(V_SIX_POINT_INDICES) / sizeof(GLushort);
			break;
		case VERTICAL_STRETCH_4_POINTS:
		case HORIZONTAL_STRETCH_4_POINTS:
			indicesNum = sizeof(FOUR_POINT_INDICES) / sizeof(GLushort);
			break;
		default:
			break;
	}
	glDrawElements(GL_TRIANGLES, indicesNum, GL_UNSIGNED_SHORT, (const void *)0);
	GO_CHECK_GL_ERROR();
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, screenW, screenH);
	// Do normal rendering
	glUseProgram(m_ProgramObj);
	GO_CHECK_GL_ERROR();
	glBindVertexArray(m_VaoIds[0]);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_FboTextureId);
	glUniform1i(m_SamplerLoc, 0);
	glUniformMatrix4fv(m_MVPMatLoc, 1, GL_FALSE, &m_MVPMatrix[0][0]);
	GO_CHECK_GL_ERROR();
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (const void *)0);
	GO_CHECK_GL_ERROR();
	glBindTexture(GL_TEXTURE_2D, GL_NONE);
	glBindVertexArray(GL_NONE);

//	glBindFramebuffer(GL_FRAMEBUFFER, m_FboId);
//	NativeImage nativeImage = m_RenderImage;
//	nativeImage.format = IMAGE_FORMAT_RGBA;
//	nativeImage.height = static_cast<int>(m_RenderImage.height * (1 + 2 * m_dt));
//	uint8_t *pBuffer = new uint8_t[nativeImage.width * nativeImage.height * 4];
//
//	nativeImage.ppPlane[0] = pBuffer;
//	FUN_BEGIN_TIME("FBO glReadPixels")
//		glReadPixels(0, 0, nativeImage.width, nativeImage.height, GL_RGBA, GL_UNSIGNED_BYTE, pBuffer);
//	FUN_END_TIME("FBO cost glReadPixels")
//
//	NativeImageUtil::DumpNativeImage(&nativeImage, "/sdcard/DCIM", "NDK");
//	delete []pBuffer;
//	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void FBOLegLengthenSample::Destroy()
{
	if (m_ProgramObj)
	{
		glDeleteProgram(m_ProgramObj);
	}

	if (m_FboProgramObj)
	{
		glDeleteProgram(m_FboProgramObj);
	}

	if (m_ImageTextureId)
	{
		glDeleteTextures(1, &m_ImageTextureId);
	}

	if (m_FboTextureId)
	{
		glDeleteTextures(1, &m_FboTextureId);
	}

	if (m_VboIds[0])
	{
		glDeleteBuffers(6, m_VboIds);
	}

	if (m_VaoIds[0])
	{
		glDeleteVertexArrays(2, m_VaoIds);
	}

	if (m_FboId)
	{
		glDeleteFramebuffers(1, &m_FboId);
	}

}

bool FBOLegLengthenSample::CreateFrameBufferObj()
{
	glGenTextures(1, &m_FboTextureId);
	glBindTexture(GL_TEXTURE_2D, m_FboTextureId);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, GL_NONE);

	glGenFramebuffers(1, &m_FboId);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FboId);
	glBindTexture(GL_TEXTURE_2D, m_FboTextureId);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_FboTextureId, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_RenderImage.width, m_RenderImage.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER)!= GL_FRAMEBUFFER_COMPLETE) {
		LOGCATE("FBOLegLengthenSample::CreateFrameBufferObj glCheckFramebufferStatus status != GL_FRAMEBUFFER_COMPLETE");
		return false;
	}
	glBindTexture(GL_TEXTURE_2D, GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
	return true;

}
