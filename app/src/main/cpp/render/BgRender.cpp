//
// Created by chh7563 on 2019/7/17.
//

#include <LogUtil.h>
#include "BgRender.h"
BgRender *BgRender::m_Instance = nullptr;

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
}

BgRender::~BgRender()
{

}

void BgRender::Init()
{
	LOGCATE("BgRender::Init");
	glGenTextures(1, &m_ImageTextureId);
	glBindTexture(GL_TEXTURE_2D, m_ImageTextureId);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, static_cast<GLsizei>(imgW),static_cast<GLsizei>(imgH), 0, GL_RGBA, GL_UNSIGNED_BYTE, rgbaBuf);
	GO_CHECK_GL_ERROR();
	glBindTexture(GL_TEXTURE_2D, 0);
	GO_CHECK_GL_ERROR();

	glGenTextures(1, &m_FboTextureId);
	glBindTexture(GL_TEXTURE_2D, m_FboTextureId);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	GO_CHECK_GL_ERROR();


	glGenFramebuffers(1, &fboId);
	glBindTexture(GL_TEXTURE_2D, m_FboTextureId);
	glBindFramebuffer(GL_FRAMEBUFFER, fboId);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_FboTextureId, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imgW, imgH, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER)!= GL_FRAMEBUFFER_COMPLETE) {
		GO_CHECK_GL_ERROR();
		LOGCATE("glFramebufferTexture2D error");
	}
	//7. 解绑纹理和FBO
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);



	GLuint m_VertexShader, m_FragShader;

	program = GLUtils::CreateProgram(VertexShader, FragmentShader, m_VertexShader,
									 m_FragShader);
	if (!program)
	{
		GLUtils::CheckGLError("Create Program");
		LOGCATE("GLByteFlowRender::CreateProgram Could not create program.");
		return;
	}

	GO_CHECK_GL_ERROR();
	textureHandle = glGetUniformLocation(program, "s_texture");
	vertexCoorHandle = (GLuint) glGetAttribLocation(program, "position");
	textureCoorHandle = (GLuint) glGetAttribLocation(program, "texcoord");
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
		m_eglDisp = eglGetDisplay(EGL_DEFAULT_DISPLAY);
		if(m_eglDisp == EGL_NO_DISPLAY)
		{
			//Unable to open connection to local windowing system
			LOGCATE("BgRender::CreateGlesEnv Unable to open connection to local windowing system");
			resultCode = -1;
			break;
		}

		if(!eglInitialize(m_eglDisp, &eglMajVers, &eglMinVers))
		{
			// Unable to initialize EGL. Handle and recover
			LOGCATE("BgRender::CreateGlesEnv Unable to initialize EGL");
			resultCode = -1;
			break;
		}

		LOGCATE("BgRender::CreateGlesEnv EGL init with version %d.%d", eglMajVers, eglMinVers);

		// choose the first config, i.e. best config
		if(!eglChooseConfig(m_eglDisp, confAttr, &m_eglConf, 1, &numConfigs))
		{
			LOGCATE("BgRender::CreateGlesEnv some config is wrong");
			resultCode = -1;
			break;
		}

		// create a pixelbuffer surface
		m_eglSurface = eglCreatePbufferSurface(m_eglDisp, m_eglConf, surfaceAttr);
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

		m_eglCtx = eglCreateContext(m_eglDisp, m_eglConf, EGL_NO_CONTEXT, ctxAttr);
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

		if(!eglMakeCurrent(m_eglDisp, m_eglSurface, m_eglSurface, m_eglCtx))
		{
			LOGCATE("BgRender::CreateGlesEnv MakeCurrent failed");
			resultCode = -1;
			break;
		}
		LOGCATE("BgRender::CreateGlesEnv initialize success!");
	}
	while (false);

	return resultCode;
}

void BgRender::SetImageData(uint8_t *pData, int width, int height)
{
	LOGCATE("BgRender::SetImageData pData = %p, [w,h] = [%d, %d]", pData, width, height);

}

void BgRender::SetIntParams(int paramType, int param)
{
	LOGCATE("BgRender::SetIntParams paramType = %d, param = %d", paramType, param);

}

void BgRender::Draw()
{
	LOGCATE("BgRender::Draw");

}

void BgRender::UnInit()
{
	LOGCATE("BgRender::UnInit");

}

void BgRender::DestroyGlesEnv()
{
	if (m_eglDisp != EGL_NO_DISPLAY) {
		eglMakeCurrent(m_eglDisp, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		eglDestroyContext(m_eglDisp, m_eglCtx);
		eglDestroySurface(m_eglDisp, m_eglSurface);
		eglReleaseThread();
		eglTerminate(m_eglDisp);
	}

	m_eglDisp = EGL_NO_DISPLAY;
	m_eglSurface = EGL_NO_SURFACE;
	m_eglCtx = EGL_NO_CONTEXT;

}


