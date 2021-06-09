//
// Created by ByteFlow on 2019/7/17.
//

#include <LogUtil.h>
#include <GLUtils.h>
#include "EGLRender.h"

#define VERTEX_POS_LOC  0
#define TEXTURE_POS_LOC 1

EGLRender *EGLRender::m_Instance = nullptr;

#define PARAM_TYPE_SHADER_INDEX    200

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

const char fShaderStr0[] =
		"#version 300 es\n"
		"precision mediump float;\n"
		"in vec2 v_texCoord;\n"
		"layout(location = 0) out vec4 outColor;\n"
		"uniform sampler2D s_TextureMap;\n"
		"void main()\n"
		"{\n"
		"    outColor = texture(s_TextureMap, v_texCoord);\n"
		"}";
// 马赛克
const char fShaderStr1[] =
		"#version 300 es\n"
		"precision highp float;\n"
		"layout(location = 0) out vec4 outColor;\n"
		"in vec2 v_texCoord;\n"
		"uniform sampler2D s_TextureMap;\n"
		"uniform vec2 u_texSize;\n"
		"\n"
		"vec4 CrossStitching(vec2 uv) {\n"
		"    float stitchSize = u_texSize.x / 35.0;\n"
		"    int invert = 0;\n"
		"    vec4 color = vec4(0.0);\n"
		"    float size = stitchSize;\n"
		"    vec2 cPos = uv * u_texSize.xy;\n"
		"    vec2 tlPos = floor(cPos / vec2(size, size));\n"
		"    tlPos *= size;\n"
		"    int remX = int(mod(cPos.x, size));\n"
		"    int remY = int(mod(cPos.y, size));\n"
		"    if (remX == 0 && remY == 0)\n"
		"    tlPos = cPos;\n"
		"    vec2 blPos = tlPos;\n"
		"    blPos.y += (size - 1.0);\n"
		"    if ((remX == remY) || (((int(cPos.x) - int(blPos.x)) == (int(blPos.y) - int(cPos.y))))) {\n"
		"        if (invert == 1)\n"
		"        color = vec4(0.2, 0.15, 0.05, 1.0);\n"
		"        else\n"
		"        color = texture(s_TextureMap, tlPos * vec2(1.0 / u_texSize.x, 1.0 / u_texSize.y)) * 1.4;\n"
		"    } else {\n"
		"        if (invert == 1)\n"
		"        color = texture(s_TextureMap, tlPos * vec2(1.0 / u_texSize.x, 1.0 / u_texSize.y)) * 1.4;\n"
		"        else\n"
		"        color = vec4(0.0, 0.0, 0.0, 1.0);\n"
		"    }\n"
		"    return color;\n"
		"}\n"
		"void main() {\n"
		"    outColor = CrossStitching(v_texCoord);\n"
		"}";

// 网格
const char fShaderStr2[] =
		"#version 300 es\n"
		"precision highp float;\n"
		"layout(location = 0) out vec4 outColor;\n"
		"in vec2 v_texCoord;\n"
		"uniform sampler2D s_TextureMap;\n"
		"uniform vec2 u_texSize;\n"
		"void main() {\n"
		"    float size = u_texSize.x / 75.0;\n"
		"    float radius = size * 0.5;\n"
		"    vec2 fragCoord = v_texCoord * u_texSize.xy;\n"
		"    vec2 quadPos = floor(fragCoord.xy / size) * size;\n"
		"    vec2 quad = quadPos/u_texSize.xy;\n"
		"    vec2 quadCenter = (quadPos + size/2.0);\n"
		"    float dist = length(quadCenter - fragCoord.xy);\n"
		"\n"
		"    if (dist > radius) {\n"
		"        outColor = vec4(0.25);\n"
		"    } else {\n"
		"        outColor = texture(s_TextureMap, v_texCoord);\n"
		"    }\n"
		"}";

// 旋转
const char fShaderStr3[] =
		"#version 300 es\n"
		"precision highp float;\n"
		"layout(location = 0) out vec4 outColor;\n"
		"in vec2 v_texCoord;\n"
		"uniform lowp sampler2D s_TextureMap;\n"
		"uniform vec2 u_texSize;\n"
		"void main() {\n"
		"    float radius = 200.0;\n"
		"    float angle = 0.8;\n"
		"    vec2 center = vec2(u_texSize.x / 2.0, u_texSize.y / 2.0);\n"
		"    vec2 tc = v_texCoord * u_texSize;\n"
		"    tc -= center;\n"
		"    float dist = length(tc);\n"
		"    if (dist < radius) {\n"
		"        float percent = (radius - dist) / radius;\n"
		"        float theta = percent * percent * angle * 8.0;\n"
		"        float s = sin(theta);\n"
		"        float c = cos(theta);\n"
		"        tc = vec2(dot(tc, vec2(c, -s)), dot(tc, vec2(s, c)));\n"
		"    }\n"
		"    tc += center;\n"
		"    outColor = texture(s_TextureMap, tc / u_texSize);\n"
		"}";

// 边缘
const char fShaderStr4[] =
		"#version 300 es\n"
		"precision highp float;\n"
		"layout(location = 0) out vec4 outColor;\n"
		"in vec2 v_texCoord;\n"
		"uniform lowp sampler2D s_TextureMap;\n"
		"uniform vec2 u_texSize;\n"
		"void main() {\n"
		"    vec2 pos = v_texCoord.xy;\n"
		"    vec2 onePixel = vec2(1, 1) / u_texSize;\n"
		"    vec4 color = vec4(0);\n"
		"    mat3 edgeDetectionKernel = mat3(\n"
		"    -1, -1, -1,\n"
		"    -1, 8, -1,\n"
		"    -1, -1, -1\n"
		"    );\n"
		"    for(int i = 0; i < 3; i++) {\n"
		"        for(int j = 0; j < 3; j++) {\n"
		"            vec2 samplePos = pos + vec2(i - 1 , j - 1) * onePixel;\n"
		"            vec4 sampleColor = texture(s_TextureMap, samplePos);\n"
		"            sampleColor *= edgeDetectionKernel[i][j];\n"
		"            color += sampleColor;\n"
		"        }\n"
		"    }\n"
		"    outColor = vec4(color.rgb, 1.0);\n"
		"}";
// 放大
const char fShaderStr5[] =
		"#version 300 es\n"
		"precision highp float;\n"
		"layout(location = 0) out vec4 outColor;\n"
		"in vec2 v_texCoord;\n"
		"uniform sampler2D s_TextureMap;\n"
		"uniform vec2 u_texSize;\n"
		"\n"
		"vec2 warpPositionToUse(vec2 centerPostion, vec2 currentPosition, float radius, float scaleRatio, float aspectRatio)\n"
		"{\n"
		"    vec2 positionToUse = currentPosition;\n"
		"    vec2 currentPositionToUse = vec2(currentPosition.x, currentPosition.y * aspectRatio + 0.5 - 0.5 * aspectRatio);\n"
		"    vec2 centerPostionToUse = vec2(centerPostion.x, centerPostion.y * aspectRatio + 0.5 - 0.5 * aspectRatio);\n"
		"    //float r = distance(currentPositionToUse, centerPostionToUse);\n"
        "    float r = distance(currentPosition, centerPostion);\n"
		"    if(r < radius)\n"
		"    {\n"
		"        float alpha = 1.0 - scaleRatio * pow(r / radius - 1.0, 2.0);\n"
		"        positionToUse = centerPostion + alpha * (currentPosition - centerPostion);\n"
		"    }\n"
		"    return positionToUse;\n"
		"}\n"
		"\n"
		"void main() {\n"
		"    vec2 centerPostion = vec2(0.5, 0.5);\n"
		"    float radius = 0.34;\n"
		"    float scaleRatio = 1.0;\n"
		"    float aspectRatio = u_texSize.x / u_texSize.y;\n"
		"    outColor = texture(s_TextureMap, warpPositionToUse(centerPostion, v_texCoord, radius, scaleRatio, aspectRatio));\n"
		"}";

const char fShaderStr6[] =
		"#version 300 es\n"
		"precision highp float;\n"
		"layout(location = 0) out vec4 outColor;\n"
		"in vec2 v_texCoord;\n"
		"uniform sampler2D s_TextureMap;\n"
		"uniform vec2 u_texSize;\n"
		"\n"
		"vec2 reshape(vec2 src, vec2 dst, vec2 curPos, float radius)\n"
		"{\n"
		"    vec2 pos = curPos;\n"
		"\n"
		"    float r = distance(curPos, src);\n"
		"\n"
		"    if (r < radius)\n"
		"    {\n"
		"        float alpha = 1.0 -  r / radius;\n"
		"        vec2 displacementVec = (dst - src) * pow(alpha, 2.0);\n"
		"        pos = curPos - displacementVec;\n"
		"\n"
		"    }\n"
		"    return pos;\n"
		"}\n"
		"\n"
		"void main() {\n"
		"    vec2 srcPos = vec2(0.5, 0.5);\n"
		"    vec2 dstPos = vec2(0.6, 0.5);\n"
		"    float radius = 0.18;\n"
		"    float scaleRatio = 1.0;\n"
		"    float aspectRatio = u_texSize.x / u_texSize.y;\n"
		"    \n"
		"    if(radius <= distance(v_texCoord, srcPos) && distance(v_texCoord, srcPos) <= radius + 0.008)\n"
		"    {\n"
		"        outColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
		"    } \n"
		"    else\n"
		"    {\n"
		"        outColor = texture(s_TextureMap, reshape(srcPos, dstPos, v_texCoord, radius));\n"
		"    }\n"
		"}";

// 形变
const char fShaderStr7[] =
		"#version 300 es\n"
		"precision highp float;\n"
		"layout(location = 0) out vec4 outColor;\n"
		"in vec2 v_texCoord;\n"
		"uniform sampler2D s_TextureMap;\n"
		"uniform vec2 u_texSize;\n"
		"\n"
		"float distanceTex(vec2 pos0, vec2 pos1, float aspectRatio)\n"
		"{\n"
		"    vec2 newPos0 = vec2(pos0.x, pos0.y * aspectRatio + 0.5 - 0.5 * aspectRatio);\n"
		"    vec2 newPos1 = vec2(pos1.x, pos1.y * aspectRatio + 0.5 - 0.5 * aspectRatio);\n"
		"    return distance(newPos0, newPos1);\n"
		"}\n"
		"\n"
		"vec2 reshape(vec2 src, vec2 dst, vec2 curPos, float radius, float aspectRatio)\n"
		"{\n"
		"    vec2 pos = curPos;\n"
		"\n"
		"    vec2 newSrc = vec2(src.x, src.y * aspectRatio + 0.5 - 0.5 * aspectRatio);\n"
		"    vec2 newDst = vec2(dst.x, dst.y * aspectRatio + 0.5 - 0.5 * aspectRatio);\n"
		"    vec2 newCur = vec2(curPos.x, curPos.y * aspectRatio + 0.5 - 0.5 * aspectRatio);\n"
		"\n"
		"\n"
		"    float r = distance(newSrc, newCur);\n"
		"\n"
		"    if (r < radius)\n"
		"    {\n"
		"        float alpha = 1.0 -  r / radius;\n"
		"        vec2 displacementVec = (dst - src) * pow(alpha, 1.7);\n"
		"        pos = curPos - displacementVec;\n"
		"\n"
		"    }\n"
		"    return pos;\n"
		"}\n"
		"\n"
		"void main() {\n"
		"    vec2 srcPos = vec2(0.5, 0.5);\n"
		"    vec2 dstPos = vec2(0.55, 0.55);\n"
		"    float radius = 0.30;\n"
		"    float scaleRatio = 1.0;\n"
		"    float aspectRatio = u_texSize.y/u_texSize.x;\n"
		"\n"
		"    if(radius <= distanceTex(v_texCoord, srcPos, aspectRatio) && distanceTex(v_texCoord, srcPos, aspectRatio) <= radius + 0.008)\n"
		"    {\n"
		"        outColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
		"    }\n"
		"    else\n"
		"    {\n"
		"        outColor = texture(s_TextureMap, reshape(srcPos, dstPos, v_texCoord, radius, aspectRatio));\n"
		"    }\n"
		"}";

//顶点坐标
const GLfloat vVertices[] = {
		-1.0f, -1.0f, 0.0f, // bottom left
		1.0f, -1.0f, 0.0f, // bottom right
		-1.0f,  1.0f, 0.0f, // top left
		1.0f,  1.0f, 0.0f, // top right
};

//正常纹理坐标
const GLfloat vTexCoors[] = {
		0.0f, 1.0f, // bottom left
		1.0f, 1.0f, // bottom right
		0.0f, 0.0f, // top left
		1.0f, 0.0f, // top right
};

//fbo 纹理坐标与正常纹理方向不同(上下镜像)
const GLfloat vFboTexCoors[] = {
		0.0f, 0.0f,  // bottom left
		1.0f, 0.0f,  // bottom right
		0.0f, 1.0f,  // top left
		1.0f, 1.0f,  // top right
};

const GLushort indices[] = { 0, 1, 2, 1, 3, 2 };

EGLRender::EGLRender()
{
	m_ImageTextureId = GL_NONE;
	m_FboTextureId = GL_NONE;
	m_SamplerLoc = GL_NONE;
	m_TexSizeLoc = GL_NONE;
	m_FboId = GL_NONE;
	m_ProgramObj = GL_NONE;
	m_VertexShader = GL_NONE;
	m_FragmentShader = GL_NONE;

	m_IsGLContextReady = false;
	m_ShaderIndex = 0;
}

EGLRender::~EGLRender()
{
	if (m_RenderImage.ppPlane[0])
	{
		NativeImageUtil::FreeNativeImage(&m_RenderImage);
		m_RenderImage.ppPlane[0] = nullptr;
	}

}

void EGLRender::Init()
{
	LOGCATE("EGLRender::Init");
	if (CreateGlesEnv() == 0)
	{
		m_IsGLContextReady = true;
	}

	if(!m_IsGLContextReady) return;
	m_fShaderStrs[0] = fShaderStr0;
	m_fShaderStrs[1] = fShaderStr1;
	m_fShaderStrs[2] = fShaderStr2;
	m_fShaderStrs[3] = fShaderStr3;
	m_fShaderStrs[4] = fShaderStr4;
	m_fShaderStrs[5] = fShaderStr5;
	m_fShaderStrs[6] = fShaderStr7;


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


	m_ProgramObj = GLUtils::CreateProgram(vShaderStr, m_fShaderStrs[m_ShaderIndex], m_VertexShader,
									 m_FragmentShader);
	if (!m_ProgramObj)
	{
		GLUtils::CheckGLError("Create Program");
		LOGCATE("EGLRender::Init Could not create program.");
		return;
	}

	m_SamplerLoc = glGetUniformLocation(m_ProgramObj, "s_TextureMap");
	m_TexSizeLoc = glGetUniformLocation(m_ProgramObj, "u_texSize");

	// Generate VBO Ids and load the VBOs with data
	glGenBuffers(3, m_VboIds);
	glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vVertices), vVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vFboTexCoors), vTexCoors, GL_STATIC_DRAW);

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

int EGLRender::CreateGlesEnv()
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
            EGL_DEPTH_SIZE, 16,// if you need the depth buffer
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
		//1. 获取 EGLDisplay 对象，建立与本地窗口系统的连接
		m_eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
		if(m_eglDisplay == EGL_NO_DISPLAY)
		{
			//Unable to open connection to local windowing system
			LOGCATE("EGLRender::CreateGlesEnv Unable to open connection to local windowing system");
			resultCode = -1;
			break;
		}

		//2. 初始化 EGL 方法
		if(!eglInitialize(m_eglDisplay, &eglMajVers, &eglMinVers))
		{
			// Unable to initialize EGL. Handle and recover
			LOGCATE("EGLRender::CreateGlesEnv Unable to initialize EGL");
			resultCode = -1;
			break;
		}

		LOGCATE("EGLRender::CreateGlesEnv EGL init with version %d.%d", eglMajVers, eglMinVers);

		//3. 获取 EGLConfig 对象，确定渲染表面的配置信息
		if(!eglChooseConfig(m_eglDisplay, confAttr, &m_eglConf, 1, &numConfigs))
		{
			LOGCATE("EGLRender::CreateGlesEnv some config is wrong");
			resultCode = -1;
			break;
		}

		//4. 创建渲染表面 EGLSurface, 使用 eglCreatePbufferSurface 创建屏幕外渲染区域
		m_eglSurface = eglCreatePbufferSurface(m_eglDisplay, m_eglConf, surfaceAttr);
		if(m_eglSurface == EGL_NO_SURFACE)
		{
			switch(eglGetError())
			{
				case EGL_BAD_ALLOC:
					// Not enough resources available. Handle and recover
					LOGCATE("EGLRender::CreateGlesEnv Not enough resources available");
					break;
				case EGL_BAD_CONFIG:
					// Verify that provided EGLConfig is valid
					LOGCATE("EGLRender::CreateGlesEnv provided EGLConfig is invalid");
					break;
				case EGL_BAD_PARAMETER:
					// Verify that the EGL_WIDTH and EGL_HEIGHT are
					// non-negative values
					LOGCATE("EGLRender::CreateGlesEnv provided EGL_WIDTH and EGL_HEIGHT is invalid");
					break;
				case EGL_BAD_MATCH:
					// Check window and EGLConfig attributes to determine
					// compatibility and pbuffer-texture parameters
					LOGCATE("EGLRender::CreateGlesEnv Check window and EGLConfig attributes");
					break;
			}
		}

		//5. 创建渲染上下文 EGLContext
		m_eglCtx = eglCreateContext(m_eglDisplay, m_eglConf, EGL_NO_CONTEXT, ctxAttr);
		if(m_eglCtx == EGL_NO_CONTEXT)
		{
			EGLint error = eglGetError();
			if(error == EGL_BAD_CONFIG)
			{
				// Handle error and recover
				LOGCATE("EGLRender::CreateGlesEnv EGL_BAD_CONFIG");
				resultCode = -1;
				break;
			}
		}

		//6. 绑定上下文
		if(!eglMakeCurrent(m_eglDisplay, m_eglSurface, m_eglSurface, m_eglCtx))
		{
			LOGCATE("EGLRender::CreateGlesEnv MakeCurrent failed");
			resultCode = -1;
			break;
		}
		LOGCATE("EGLRender::CreateGlesEnv initialize success!");
	}
	while (false);

	if (resultCode != 0)
	{
		LOGCATE("EGLRender::CreateGlesEnv fail");
	}

	return resultCode;
}

void EGLRender::SetImageData(uint8_t *pData, int width, int height)
{
	LOGCATE("EGLRender::SetImageData pData = %p, [w,h] = [%d, %d]", pData, width, height);

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
		NativeImageUtil::AllocNativeImage(&m_RenderImage);
		memcpy(m_RenderImage.ppPlane[0], pData, static_cast<size_t>(width * height * 4));

		glBindTexture(GL_TEXTURE_2D, m_ImageTextureId);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_RenderImage.width, m_RenderImage.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_RenderImage.ppPlane[0]);
		glBindTexture(GL_TEXTURE_2D, GL_NONE);

		if (m_FboId == GL_NONE)
		{
			// Create FBO
			glGenFramebuffers(1, &m_FboId);
			glBindFramebuffer(GL_FRAMEBUFFER, m_FboId);
			glBindTexture(GL_TEXTURE_2D, m_FboTextureId);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_FboTextureId, 0);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_RenderImage.width, m_RenderImage.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER)!= GL_FRAMEBUFFER_COMPLETE) {
				LOGCATE("EGLRender::SetImageData glCheckFramebufferStatus status != GL_FRAMEBUFFER_COMPLETE");
			}
			glBindTexture(GL_TEXTURE_2D, GL_NONE);
			glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
		}

	}

}

void EGLRender::SetIntParams(int paramType, int param)
{
	LOGCATE("EGLRender::SetIntParams paramType = %d, param = %d", paramType, param);
	switch (paramType)
	{
		case PARAM_TYPE_SHADER_INDEX:
		{
			if (param >= 0)
			{
				m_ShaderIndex = param % EGL_FEATURE_NUM;

				if (m_ProgramObj)
				{
					glDeleteProgram(m_ProgramObj);
					m_ProgramObj = GL_NONE;
				}

				m_ProgramObj = GLUtils::CreateProgram(vShaderStr, m_fShaderStrs[m_ShaderIndex], m_VertexShader,
													  m_FragmentShader);
				if (!m_ProgramObj)
				{
					GLUtils::CheckGLError("Create Program");
					LOGCATE("EGLRender::SetIntParams Could not create program.");
					return;
				}

				m_SamplerLoc = glGetUniformLocation(m_ProgramObj, "s_TextureMap");
				GO_CHECK_GL_ERROR();
				m_TexSizeLoc = glGetUniformLocation(m_ProgramObj, "u_texSize");
				GO_CHECK_GL_ERROR();
			}

		}
			break;
		default:
			break;
	}
}

void EGLRender::Draw()
{
	LOGCATE("EGLRender::Draw");
	if (m_ProgramObj == GL_NONE) return;
	glViewport(0, 0, m_RenderImage.width, m_RenderImage.height);

	// Do FBO off screen rendering
	glUseProgram(m_ProgramObj);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FboId);

	glBindVertexArray(m_VaoIds[0]);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_ImageTextureId);
	glUniform1i(m_SamplerLoc, 0);

	if (m_TexSizeLoc > -1) {
		GLfloat size[2];
		size[0] = m_RenderImage.width;
		size[1] = m_RenderImage.height;
		glUniform2f(m_TexSizeLoc, size[0], size[1]);
	}


	//7. 渲染
	GO_CHECK_GL_ERROR();
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (const void *)0);
	GO_CHECK_GL_ERROR();
	glBindVertexArray(GL_NONE);
	glBindTexture(GL_TEXTURE_2D, GL_NONE);

	//一旦解绑 FBO 后面就不能调用 readPixels
	//glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);

}

void EGLRender::UnInit()
{
	LOGCATE("EGLRender::UnInit");
	if (m_ProgramObj)
	{
		glDeleteProgram(m_ProgramObj);
		m_ProgramObj = GL_NONE;
	}

	if (m_ImageTextureId)
	{
		glDeleteTextures(1, &m_ImageTextureId);
		m_ImageTextureId = GL_NONE;
	}

	if (m_FboTextureId)
	{
		glDeleteTextures(1, &m_FboTextureId);
		m_FboTextureId = GL_NONE;
	}

	if (m_VboIds[0])
	{
		glDeleteBuffers(3, m_VboIds);
		m_VboIds[0] = GL_NONE;
		m_VboIds[1] = GL_NONE;
		m_VboIds[2] = GL_NONE;

	}

	if (m_VaoIds[0])
	{
		glDeleteVertexArrays(1, m_VaoIds);
		m_VaoIds[0] = GL_NONE;
	}

	if (m_FboId)
	{
		glDeleteFramebuffers(1, &m_FboId);
		m_FboId = GL_NONE;
	}


	if (m_IsGLContextReady)
	{
		DestroyGlesEnv();
		m_IsGLContextReady = false;
	}

}

void EGLRender::DestroyGlesEnv()
{
	//8. 释放 EGL 环境
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


