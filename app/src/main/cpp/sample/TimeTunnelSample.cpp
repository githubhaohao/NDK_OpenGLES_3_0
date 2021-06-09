//
// Created by ByteFlow on 2019/12/26.
//

#include <GLUtils.h>
#include <gtc/matrix_transform.hpp>
#include <cstdlib>
#include <opencv2/opencv.hpp>
#include "TimeTunnelSample.h"

#define VERTEX_POS_INDX  0
#define TEXTURE_POS_INDX 1

TimeTunnelSample::TimeTunnelSample()
{
    m_ImageTextureId = GL_NONE;
	m_FboTextureId = GL_NONE;
	m_SamplerLoc = GL_NONE;
	m_FboId = GL_NONE;
	m_FboProgramObj = GL_NONE;
	m_FboVertexShader = GL_NONE;
	m_FboFragmentShader = GL_NONE;
	m_FboSamplerLoc = GL_NONE;
	m_MVPMatrixLoc = GL_NONE;
	m_ImgSizeLoc = GL_NONE;
	m_TimeLoc = GL_NONE;

    m_AngleX = 0;
    m_AngleY = 0;

    m_ScaleX = 1.0f;
    m_ScaleY = 1.0f;

    m_FrameIndex = 0;
}

TimeTunnelSample::~TimeTunnelSample()
{
	NativeImageUtil::FreeNativeImage(&m_RenderImage);
}

void TimeTunnelSample::LoadImage(NativeImage *pImage)
{
	LOGCATE("TimeTunnelSample::LoadImage pImage = %p", pImage->ppPlane[0]);
	if (pImage)
	{
		m_RenderImage.width = pImage->width;
		m_RenderImage.height = pImage->height;
		m_RenderImage.format = pImage->format;
		NativeImageUtil::CopyNativeImage(pImage, &m_RenderImage);
	}
}

void TimeTunnelSample::Init()
{
    if(m_ProgramObj)
        return;

	//顶点坐标
	GLfloat vVertices[] = {
			-1.0f, -1.0f, 0.0f,
			 1.0f, -1.0f, 0.0f,
			-1.0f,  1.0f, 0.0f,
			 1.0f,  1.0f, 0.0f,
	};

	//正常纹理坐标
	GLfloat vTexCoors[] = {
            0.0f, 1.0f,
            1.0f, 1.0f,
            0.0f, 0.0f,
            1.0f, 0.0f,
    };

	//fbo 纹理坐标与正常纹理方向不同，原点位于左下角
	GLfloat vFboTexCoors[] = {
			0.0f, 0.0f,
			1.0f, 0.0f,
			0.0f, 1.0f,
			1.0f, 1.0f,
	};

	GLushort indices[] = { 0, 1, 2, 1, 3, 2 };

	char vShaderStr[] =
			"#version 300 es                            \n"
			"layout(location = 0) in vec4 a_position;   \n"
			"layout(location = 1) in vec2 a_texCoord;   \n"
            "uniform mat4 u_MVPMatrix;                  \n"
            "out vec2 v_texCoord;                       \n"
			"void main()                                \n"
			"{                                          \n"
			"   gl_Position = u_MVPMatrix * a_position; \n"
			"   v_texCoord = a_texCoord;                \n"
			"}                                          \n";

	// 用于普通渲染的片段着色器脚本，简单纹理映射
	char fShaderStr[] =
			"#version 300 es\n"
			"precision mediump float;\n"
			"in vec2 v_texCoord;\n"
			"layout(location = 0) out vec4 outColor;\n"
			"uniform sampler2D s_TextureMap;\n"
			"void main()\n"
			"{\n"
			"    outColor = texture(s_TextureMap, v_texCoord);\n"
			"}";

	// 用于离屏渲染的顶点着色器脚本，不使用变换矩阵
    char vFboShaderStr[] =
            "#version 300 es                            \n"
            "layout(location = 0) in vec4 a_position;   \n"
            "layout(location = 1) in vec2 a_texCoord;   \n"
            "out vec2 v_texCoord;                       \n"
            "void main()                                \n"
            "{                                          \n"
            "   gl_Position = a_position;               \n"
            "   v_texCoord = a_texCoord;                \n"
            "}                                          \n";

	// 用于离屏渲染的片段着色器脚本，取每个像素的灰度值
	char fFboShaderStr[] =
			"#version 300 es\n"
            "precision highp float;\n"
            "layout(location = 0) out vec4 outColor;\n"
            "uniform float u_time;\n"
            "uniform vec2 u_imgSize;\n"
            "void main()\n"
            "{\n"
            "    vec2 fragCoord = gl_FragCoord.xy;\n"
            "\n"
            "    // input: pixel coordinates\n"
            "    vec2 p = (-u_imgSize.xy + 2.0*fragCoord)/u_imgSize.y;\n"
            "\n"
            "    // angle of each pixel to the center of the screen\n"
            "    float a = atan(p.y,p.x);\n"
            "\n"
            "    // modified distance metric\n"
            "    float r = pow( pow(p.x*p.x,4.0) + pow(p.y*p.y,4.0), 1.0/8.0 );\n"
            "\n"
            "    // index texture by (animated inverse) radius and angle\n"
            "    vec2 uv = vec2( 1.0/r + 0.2*u_time, a );\n"
            "\n"
            "    // pattern: cosines\n"
            "    float f = cos(12.0*uv.x)*cos(6.0*uv.y);\n"
            "\n"
            "    // color fetch: palette\n"
            "    vec3 col = 0.5 + 0.5*sin( 3.1416*f + vec3(0.0,0.5,1.0) );\n"
            "\n"
            "    // lighting: darken at the center    \n"
            "    col = col*r;\n"
            "\n"
            "    // output: pixel color\n"
            "    outColor = vec4( col, 1.0 );\n"
            "}";

	// 编译链接用于普通渲染的着色器程序
	m_ProgramObj = GLUtils::CreateProgram(vShaderStr, fShaderStr, m_VertexShader, m_FragmentShader);

	// 编译链接用于离屏渲染的着色器程序
	m_FboProgramObj = GLUtils::CreateProgram(vFboShaderStr, fFboShaderStr, m_FboVertexShader, m_FboFragmentShader);

	if (m_ProgramObj == GL_NONE || m_FboProgramObj == GL_NONE)
	{
		LOGCATE("TimeTunnelSample::Init m_ProgramObj == GL_NONE");
		return;
	}
	m_SamplerLoc = glGetUniformLocation(m_ProgramObj, "s_TextureMap");
	m_MVPMatrixLoc = glGetUniformLocation(m_ProgramObj, "u_MVPMatrix");
	m_FboSamplerLoc = glGetUniformLocation(m_FboProgramObj, "s_TextureMap");
	m_ImgSizeLoc = glGetUniformLocation(m_FboProgramObj, "u_imgSize");
	m_TimeLoc = glGetUniformLocation(m_FboProgramObj, "u_time");

	// 生成 VBO ，加载顶点数据和索引数据
	// Generate VBO Ids and load the VBOs with data
	glGenBuffers(4, m_VboIds);
	glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vVertices), vVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vTexCoors), vTexCoors, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vFboTexCoors), vFboTexCoors, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VboIds[3]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	GO_CHECK_GL_ERROR();

	// 生成 2 个 VAO，一个用于普通渲染，另一个用于离屏渲染
	// Generate VAO Ids
	glGenVertexArrays(2, m_VaoIds);
    // 初始化用于普通渲染的 VAO
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

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VboIds[3]);
	GO_CHECK_GL_ERROR();
	glBindVertexArray(GL_NONE);


	// 初始化用于离屏渲染的 VAO
	// FBO off screen rendering VAO
	glBindVertexArray(m_VaoIds[1]);

	glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[0]);
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

	// 创建并初始化图像纹理
	glGenTextures(1, &m_ImageTextureId);
	glBindTexture(GL_TEXTURE_2D, m_ImageTextureId);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_RenderImage.width, m_RenderImage.height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 m_RenderImage.ppPlane[0]);
	glBindTexture(GL_TEXTURE_2D, GL_NONE);
	GO_CHECK_GL_ERROR();

	if (!CreateFrameBufferObj())
	{
		LOGCATE("TimeTunnelSample::Init CreateFrameBufferObj fail");
		return;
	}

}

void TimeTunnelSample::Draw(int screenW, int screenH)
{
	// 离屏渲染
	//glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glViewport(0, 0, m_RenderImage.width, m_RenderImage.height);

	glBindTexture(GL_TEXTURE_2D, m_ImageTextureId);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_RenderImage.width, m_RenderImage.height, GL_RGBA, GL_UNSIGNED_BYTE, m_RenderImage.ppPlane[0]);
    GO_CHECK_GL_ERROR();

	// Do FBO off screen rendering
	glBindFramebuffer(GL_FRAMEBUFFER, m_FboId);
	glUseProgram(m_FboProgramObj);
	glBindVertexArray(m_VaoIds[1]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_ImageTextureId);
	glUniform1i(m_FboSamplerLoc, 0);

	glUniform2f(m_ImgSizeLoc, m_RenderImage.width, m_RenderImage.height);
	glUniform1f(m_TimeLoc, m_FrameIndex * 0.04f);
	GO_CHECK_GL_ERROR();
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (const void *)0);
	GO_CHECK_GL_ERROR();
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// 普通渲染
	// Do normal rendering
	glViewport(0, 0, screenW, screenH);
    UpdateMVPMatrix(m_MVPMatrix, m_AngleX, m_AngleY, (float)screenW / screenH);
    glUseProgram(m_ProgramObj);
	GO_CHECK_GL_ERROR();
	glBindVertexArray(m_VaoIds[0]);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_FboTextureId);
    glUniformMatrix4fv(m_MVPMatrixLoc, 1, GL_FALSE, &m_MVPMatrix[0][0]);
	glUniform1i(m_SamplerLoc, 0);
	GO_CHECK_GL_ERROR();
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (const void *)0);
	GO_CHECK_GL_ERROR();
	glBindTexture(GL_TEXTURE_2D, GL_NONE);
	glBindVertexArray(GL_NONE);
	m_FrameIndex++;

}

void TimeTunnelSample::Destroy()
{
	if (m_ProgramObj)
	{
		glDeleteProgram(m_ProgramObj);
		m_ProgramObj = GL_NONE;
	}

	if (m_FboProgramObj)
	{
		glDeleteProgram(m_FboProgramObj);
		m_FboProgramObj = GL_NONE;
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
		glDeleteBuffers(4, m_VboIds);
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

bool TimeTunnelSample::CreateFrameBufferObj()
{
	// 创建并初始化 FBO 纹理
	glGenTextures(1, &m_FboTextureId);
	glBindTexture(GL_TEXTURE_2D, m_FboTextureId);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, GL_NONE);

	// 创建并初始化 FBO
	glGenFramebuffers(1, &m_FboId);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FboId);
	glBindTexture(GL_TEXTURE_2D, m_FboTextureId);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_FboTextureId, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_RenderImage.width, m_RenderImage.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER)!= GL_FRAMEBUFFER_COMPLETE) {
		LOGCATE("TimeTunnelSample::CreateFrameBufferObj glCheckFramebufferStatus status != GL_FRAMEBUFFER_COMPLETE");
		return false;
	}
	glBindTexture(GL_TEXTURE_2D, GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
	return true;

}

void TimeTunnelSample::UpdateMVPMatrix(glm::mat4 &mvpMatrix, int angleX, int angleY, float ratio)
{
    LOGCATE("TimeTunnelSample::UpdateMVPMatrix angleX = %d, angleY = %d, ratio = %f", angleX, angleY, ratio);
    angleX = angleX % 360;
    angleY = angleY % 360;

    float radiansX = static_cast<float>(MATH_PI / 180.0f * angleX);
    float radiansY = static_cast<float>(MATH_PI / 180.0f * angleY);

    // Projection matrix
    glm::mat4 Projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f);
    //glm::mat4 Projection = glm::frustum(-ratio, ratio, -1.0f, 1.0f, 1.0f, 100);
    //glm::mat4 Projection = glm::perspective(45.0f,ratio, 0.1f,100.f);

    // View matrix
    glm::mat4 View = glm::lookAt(
            glm::vec3(0, 0, 1), // Camera is at (0,0,1), in World Space
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