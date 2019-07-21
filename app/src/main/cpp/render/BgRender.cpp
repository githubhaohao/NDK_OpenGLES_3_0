//
// Created by chh7563 on 2019/7/17.
//

#include <LogUtil.h>
#include <GLUtils.h>
#include "BgRender.h"
BgRender *BgRender::m_Instance = nullptr;

#define VERTEX_POS_LOC  0
#define TEXTURE_POS_LOC 1

const char vShaderStr[] =
		"#version 300 es                            \n"
		"layout(location = 0) in vec4 a_position;   \n"
		"layout(location = 1) in vec2 a_texCoord;   \n"
		"out vec2 v_texCoord;                       \n"
		"void main()                                \n"
		"{                                          \n"
		"   gl_Position = a_position;               \n"
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

//顶点坐标
const GLfloat vVertices[] = {
		-1.0f, -1.0f, 0.0f, // bottom left
		1.0f, -1.0f, 0.0f, // bottom right
		-1.0f,  1.0f, 0.0f, // top left
		1.0f,  1.0f, 0.0f, // top right
};

////正常纹理坐标
//const GLfloat vTexCoors[] = {
//		0.0f, 1.0f, // bottom left
//		1.0f, 1.0f, // bottom right
//		0.0f, 0.0f, // top left
//		1.0f, 0.0f, // top right
//};

//fbo 纹理坐标与正常纹理方向不同
const GLfloat vFboTexCoors[] = {
		0.0f, 0.0f,  // bottom left
		1.0f, 0.0f,  // bottom right
		0.0f, 1.0f,  // top left
		1.0f, 1.0f,  // top right
};

const GLushort indices[] = { 0, 1, 2, 1, 3, 2 };

BgRender::BgRender()
{
	m_VaoIds[2] = {GL_NONE};
	m_VboIds[3] = {GL_NONE};
	m_ImageTextureId = GL_NONE;
	m_FboTextureId = GL_NONE;
	m_SamplerLoc = GL_NONE;
	m_FboId = GL_NONE;
	m_ProgramObj = GL_NONE;
	m_VertexShader = GL_NONE;
	m_FragmentShader = GL_NONE;

	m_IsGLContextReady = false;
}

BgRender::~BgRender()
{
	if (m_RenderImage.ppPlane[0])
	{
		NativeImageUtil::FreeNativeImage(&m_RenderImage);
		m_RenderImage.ppPlane[0] = nullptr;
	}

}

void BgRender::Init()
{
	LOGCATE("BgRender::Init");
	if (CreateGlesEnv() == 0)
	{
		m_IsGLContextReady = true;
	}

	if(!m_IsGLContextReady) return;

	glGenTextures(1, &m_ImageTextureId);
	glBindTexture(GL_TEXTURE_2D, m_ImageTextureId);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, GL_NONE);

	glGenTextures(1, &m_FboTextureId);
	glBindTexture(GL_TEXTURE_2D, m_FboTextureId);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, GL_NONE);


	m_ProgramObj = GLUtils::CreateProgram(vShaderStr, fShaderStr, m_VertexShader,
									 m_FragmentShader);
	if (!m_ProgramObj)
	{
		GLUtils::CheckGLError("Create Program");
		LOGCATE("BgRender::Init Could not create program.");
		return;
	}

	m_SamplerLoc = glGetUniformLocation(m_ProgramObj, "s_TextureMap");

	// Generate VBO Ids and load the VBOs with data
	glGenBuffers(3, m_VboIds);
	glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vVertices), vVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vFboTexCoors), vFboTexCoors, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VboIds[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	GO_CHECK_GL_ERROR();

	// Generate VAO Ids
	glGenVertexArrays(1, m_VaoIds);

	// FBO off screen rendering VAO
	glBindVertexArray(m_VaoIds[0]);

	glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[0]);
	glEnableVertexAttribArray(VERTEX_POS_LOC);
	glVertexAttribPointer(VERTEX_POS_LOC, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (const void *)0);
	glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);

	glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[1]);
	glEnableVertexAttribArray(TEXTURE_POS_LOC);
	glVertexAttribPointer(TEXTURE_POS_LOC, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (const void *)0);
	glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VboIds[2]);
	GO_CHECK_GL_ERROR();
	glBindVertexArray(GL_NONE);
}

int BgRender::CreateGlesEnv()
{
	// EGL config attributes
    const EGLint confAttr[] =
    {
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
            EGL_SURFACE_TYPE,EGL_PBUFFER_BIT,//EGL_WINDOW_BIT EGL_PBUFFER_BIT we will create a pixelbuffer surface
            EGL_RED_SIZE,   8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE,  8,
            EGL_ALPHA_SIZE, 8,// if you need the alpha channel
            EGL_DEPTH_SIZE, 8,// if you need the depth buffer
            EGL_STENCIL_SIZE,8,
            EGL_NONE
    };

	// EGL context attributes
    const EGLint ctxAttr[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2,
            EGL_NONE
    };

	// surface attributes
	// the surface size is set to the input frame size
	const EGLint surfaceAttr[] = {
			EGL_WIDTH, 1,
			EGL_HEIGHT,1,
			EGL_NONE
	};
	EGLint eglMajVers, eglMinVers;
	EGLint numConfigs;

	int resultCode = 0;
	do
	{
		m_eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
		if(m_eglDisplay == EGL_NO_DISPLAY)
		{
			//Unable to open connection to local windowing system
			LOGCATE("BgRender::CreateGlesEnv Unable to open connection to local windowing system");
			resultCode = -1;
			break;
		}

		if(!eglInitialize(m_eglDisplay, &eglMajVers, &eglMinVers))
		{
			// Unable to initialize EGL. Handle and recover
			LOGCATE("BgRender::CreateGlesEnv Unable to initialize EGL");
			resultCode = -1;
			break;
		}

		LOGCATE("BgRender::CreateGlesEnv EGL init with version %d.%d", eglMajVers, eglMinVers);

		// choose the first config, i.e. best config
		if(!eglChooseConfig(m_eglDisplay, confAttr, &m_eglConf, 1, &numConfigs))
		{
			LOGCATE("BgRender::CreateGlesEnv some config is wrong");
			resultCode = -1;
			break;
		}

		// create a pixelbuffer surface
		m_eglSurface = eglCreatePbufferSurface(m_eglDisplay, m_eglConf, surfaceAttr);
		if(m_eglSurface == EGL_NO_SURFACE)
		{
			switch(eglGetError())
			{
				case EGL_BAD_ALLOC:
					// Not enough resources available. Handle and recover
					LOGCATE("BgRender::CreateGlesEnv Not enough resources available");
					break;
				case EGL_BAD_CONFIG:
					// Verify that provided EGLConfig is valid
					LOGCATE("BgRender::CreateGlesEnv provided EGLConfig is invalid");
					break;
				case EGL_BAD_PARAMETER:
					// Verify that the EGL_WIDTH and EGL_HEIGHT are
					// non-negative values
					LOGCATE("BgRender::CreateGlesEnv provided EGL_WIDTH and EGL_HEIGHT is invalid");
					break;
				case EGL_BAD_MATCH:
					// Check window and EGLConfig attributes to determine
					// compatibility and pbuffer-texture parameters
					LOGCATE("BgRender::CreateGlesEnv Check window and EGLConfig attributes");
					break;
			}
		}

		m_eglCtx = eglCreateContext(m_eglDisplay, m_eglConf, EGL_NO_CONTEXT, ctxAttr);
		if(m_eglCtx == EGL_NO_CONTEXT)
		{
			EGLint error = eglGetError();
			if(error == EGL_BAD_CONFIG)
			{
				// Handle error and recover
				LOGCATE("BgRender::CreateGlesEnv EGL_BAD_CONFIG");
				resultCode = -1;
				break;
			}
		}

		if(!eglMakeCurrent(m_eglDisplay, m_eglSurface, m_eglSurface, m_eglCtx))
		{
			LOGCATE("BgRender::CreateGlesEnv MakeCurrent failed");
			resultCode = -1;
			break;
		}
		LOGCATE("BgRender::CreateGlesEnv initialize success!");
	}
	while (false);

	if (resultCode != 0)
	{
		LOGCATE("BgRender::CreateGlesEnv fail");
	}

	return resultCode;
}

void BgRender::SetImageData(uint8_t *pData, int width, int height)
{
	LOGCATE("BgRender::SetImageData pData = %p, [w,h] = [%d, %d]", pData, width, height);

	if (pData && m_IsGLContextReady)
	{
		if (m_RenderImage.ppPlane[0])
		{
			NativeImageUtil::FreeNativeImage(&m_RenderImage);
			m_RenderImage.ppPlane[0] = nullptr;
		}

		m_RenderImage.width = width;
		m_RenderImage.height = height;
		m_RenderImage.format = IMAGE_FORMAT_RGBA;
		m_RenderImage.ppPlane[0] = pData;

		glBindTexture(GL_TEXTURE_2D, m_ImageTextureId);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_RenderImage.width, m_RenderImage.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_RenderImage.ppPlane[0]);
		glBindTexture(GL_TEXTURE_2D, GL_NONE);

		// Create FBO
		glGenFramebuffers(1, &m_FboId);
		glBindFramebuffer(GL_FRAMEBUFFER, m_FboId);
		glBindTexture(GL_TEXTURE_2D, m_FboTextureId);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_FboTextureId, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_RenderImage.width, m_RenderImage.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER)!= GL_FRAMEBUFFER_COMPLETE) {
			LOGCATE("BgRender::SetImageData glCheckFramebufferStatus status != GL_FRAMEBUFFER_COMPLETE");
		}
		glBindTexture(GL_TEXTURE_2D, GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
	}

}

void BgRender::SetIntParams(int paramType, int param)
{
	LOGCATE("BgRender::SetIntParams paramType = %d, param = %d", paramType, param);


}

void BgRender::Draw()
{
	LOGCATE("BgRender::Draw");
	if (m_ProgramObj == GL_NONE) return;
	glViewport(0, 0, m_RenderImage.width, m_RenderImage.height);

	// Do FBO off screen rendering
	glUseProgram(m_ProgramObj);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FboId);

	glBindVertexArray(m_VaoIds[0]);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_ImageTextureId);
	glUniform1i(m_SamplerLoc, 0);
	GO_CHECK_GL_ERROR();
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (const void *)0);
	GO_CHECK_GL_ERROR();
	glBindVertexArray(GL_NONE);
	glBindTexture(GL_TEXTURE_2D, GL_NONE);

	//一旦解绑 FBO 后面就不能调用 readPixels
	//glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);

}

void BgRender::UnInit()
{
	LOGCATE("BgRender::UnInit");
	if (m_ProgramObj)
	{
		glDeleteProgram(m_ProgramObj);
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
		glDeleteBuffers(3, m_VboIds);
	}

	if (m_VaoIds[0])
	{
		glDeleteVertexArrays(1, m_VaoIds);
	}

	if (m_FboId)
	{
		glDeleteFramebuffers(1, &m_FboId);
	}


	if (m_IsGLContextReady)
	{
		DestroyGlesEnv();
		m_IsGLContextReady = false;
	}

}

void BgRender::DestroyGlesEnv()
{
	if (m_eglDisplay != EGL_NO_DISPLAY) {
		eglMakeCurrent(m_eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		eglDestroyContext(m_eglDisplay, m_eglCtx);
		eglDestroySurface(m_eglDisplay, m_eglSurface);
		eglReleaseThread();
		eglTerminate(m_eglDisplay);
	}

	m_eglDisplay = EGL_NO_DISPLAY;
	m_eglSurface = EGL_NO_SURFACE;
	m_eglCtx = EGL_NO_CONTEXT;

}


