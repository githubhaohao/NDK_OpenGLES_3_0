//
// Created by ByteFlow on 2019/12/26.
//

#include <GLUtils.h>
#include <gtc/matrix_transform.hpp>
#include <cstdlib>
#include <opencv2/opencv.hpp>
#include "PBOSample.h"

//#define PBO_UPLOAD
#define PBO_DOWNLOAD

#define VERTEX_POS_INDX  0
#define TEXTURE_POS_INDX 1

PBOSample::PBOSample()
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

    m_AngleX = 0;
    m_AngleY = 0;

    m_ScaleX = 1.0f;
    m_ScaleY = 1.0f;

    m_FrameIndex = 0;
}

PBOSample::~PBOSample()
{
	NativeImageUtil::FreeNativeImage(&m_RenderImage);
}

void PBOSample::LoadImage(NativeImage *pImage)
{
	LOGCATE("PBOSample::LoadImage pImage = %p", pImage->ppPlane[0]);
	if (pImage)
	{
		m_RenderImage.width = pImage->width;
		m_RenderImage.height = pImage->height;
		m_RenderImage.format = pImage->format;
		NativeImageUtil::CopyNativeImage(pImage, &m_RenderImage);
	}
}

void PBOSample::Init()
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
			"precision mediump float;\n"
			"in vec2 v_texCoord;\n"
			"layout(location = 0) out vec4 outColor;\n"
			"uniform sampler2D s_TextureMap;\n"
			"void main()\n"
			"{\n"
			"    vec4 tempColor = texture(s_TextureMap, v_texCoord);\n"
			"    float luminance = tempColor.r * 0.299 + tempColor.g * 0.587 + tempColor.b * 0.114;\n"
			"    outColor = vec4(vec3(luminance), tempColor.a);\n"
			"}"; // 输出灰度图

	// 编译链接用于普通渲染的着色器程序
	m_ProgramObj = GLUtils::CreateProgram(vShaderStr, fShaderStr, m_VertexShader, m_FragmentShader);

	// 编译链接用于离屏渲染的着色器程序
	m_FboProgramObj = GLUtils::CreateProgram(vFboShaderStr, fFboShaderStr, m_FboVertexShader, m_FboFragmentShader);

	if (m_ProgramObj == GL_NONE || m_FboProgramObj == GL_NONE)
	{
		LOGCATE("PBOSample::Init m_ProgramObj == GL_NONE");
		return;
	}
	m_SamplerLoc = glGetUniformLocation(m_ProgramObj, "s_TextureMap");
	m_MVPMatrixLoc = glGetUniformLocation(m_ProgramObj, "u_MVPMatrix");
	m_FboSamplerLoc = glGetUniformLocation(m_FboProgramObj, "s_TextureMap");

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

	//初始化 FBO
    glGenBuffers(2, m_UploadPboIds);
    int imgByteSize = m_RenderImage.width * m_RenderImage.height * 4;//RGBA
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_UploadPboIds[0]);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, imgByteSize, 0, GL_STREAM_DRAW);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_UploadPboIds[1]);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, imgByteSize, 0, GL_STREAM_DRAW);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    glGenBuffers(2, m_DownloadPboIds);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, m_DownloadPboIds[0]);
    glBufferData(GL_PIXEL_PACK_BUFFER, imgByteSize, 0, GL_STREAM_READ);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, m_DownloadPboIds[1]);
    glBufferData(GL_PIXEL_PACK_BUFFER, imgByteSize, 0, GL_STREAM_READ);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

	if (!CreateFrameBufferObj())
	{
		LOGCATE("PBOSample::Init CreateFrameBufferObj fail");
		return;
	}

}

void PBOSample::Draw(int screenW, int screenH)
{
	// 离屏渲染
	//glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	glViewport(0, 0, m_RenderImage.width, m_RenderImage.height);

	//Upload
	UploadPixels();
    GO_CHECK_GL_ERROR();

	// Do FBO off screen rendering
	glBindFramebuffer(GL_FRAMEBUFFER, m_FboId);
	glUseProgram(m_FboProgramObj);
	glBindVertexArray(m_VaoIds[1]);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_ImageTextureId);
	glUniform1i(m_FboSamplerLoc, 0);
	GO_CHECK_GL_ERROR();
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (const void *)0);
	GO_CHECK_GL_ERROR();
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);

	//Download
	DownloadPixels();

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

void PBOSample::Destroy()
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

    if (m_DownloadPboIds[0]) {
        glDeleteBuffers(2, m_DownloadPboIds);
    }

    if (m_UploadPboIds[0]) {
        glDeleteBuffers(2, m_UploadPboIds);
    }

}

bool PBOSample::CreateFrameBufferObj()
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
		LOGCATE("PBOSample::CreateFrameBufferObj glCheckFramebufferStatus status != GL_FRAMEBUFFER_COMPLETE");
		return false;
	}
	glBindTexture(GL_TEXTURE_2D, GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
	return true;

}

void PBOSample::UpdateMVPMatrix(glm::mat4 &mvpMatrix, int angleX, int angleY, float ratio)
{
    LOGCATE("PBOSample::UpdateMVPMatrix angleX = %d, angleY = %d, ratio = %f", angleX, angleY, ratio);
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

void PBOSample::UploadPixels() {
    LOGCATE("PBOSample::UploadPixels");
	int dataSize = m_RenderImage.width * m_RenderImage.height * 4;

#ifdef PBO_UPLOAD
	int index = m_FrameIndex % 2;
	int nextIndex = (index + 1) % 2;
	BEGIN_TIME("PBOSample::UploadPixels Copy Pixels from PBO to Textrure Obj")
		glBindTexture(GL_TEXTURE_2D, m_ImageTextureId);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_UploadPboIds[index]);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_RenderImage.width, m_RenderImage.height, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	END_TIME("PBOSample::UploadPixels Copy Pixels from PBO to Textrure Obj")
#else
	BEGIN_TIME("PBOSample::UploadPixels Copy Pixels from System Mem to Textrure Obj")
    glBindTexture(GL_TEXTURE_2D, m_ImageTextureId);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_RenderImage.width, m_RenderImage.height, GL_RGBA, GL_UNSIGNED_BYTE, m_RenderImage.ppPlane[0]);
    END_TIME("PBOSample::UploadPixels Copy Pixels from System Mem to Textrure Obj")
#endif

#ifdef PBO_UPLOAD
	BEGIN_TIME("PBOSample::UploadPixels Update Image data")
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_UploadPboIds[nextIndex]);
	glBufferData(GL_PIXEL_UNPACK_BUFFER, dataSize, nullptr, GL_STREAM_DRAW);
	GLubyte *bufPtr = (GLubyte *) glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0,
												   dataSize,
												   GL_MAP_WRITE_BIT |
												   GL_MAP_INVALIDATE_BUFFER_BIT);
	GO_CHECK_GL_ERROR();
	LOGCATE("PBOSample::UploadPixels bufPtr=%p",bufPtr);
	if(bufPtr)
	{
		GO_CHECK_GL_ERROR();
		memcpy(bufPtr, m_RenderImage.ppPlane[0], static_cast<size_t>(dataSize));
		int randomRow = rand() % (m_RenderImage.height - 5);
		memset(bufPtr + randomRow * m_RenderImage.width * 4, 188,
        static_cast<size_t>(m_RenderImage.width * 4 * 5));
		glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
	}
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	END_TIME("PBOSample::UploadPixels Update Image data")

#else
    NativeImage nativeImage = m_RenderImage;
	NativeImageUtil::AllocNativeImage(&nativeImage);
	BEGIN_TIME("PBOSample::UploadPixels Update Image data")
		//update image data
		int randomRow = rand() % (m_RenderImage.height - 5);
		memset(m_RenderImage.ppPlane[0] + randomRow * m_RenderImage.width * 4, 188,
		static_cast<size_t>(m_RenderImage.width * 4 * 5));
        NativeImageUtil::CopyNativeImage(&m_RenderImage, &nativeImage);
	END_TIME("PBOSample::UploadPixels Update Image data")
	NativeImageUtil::FreeNativeImage(&nativeImage);
#endif

}

void PBOSample::DownloadPixels() {
    int dataSize = m_RenderImage.width * m_RenderImage.height * 4;
	NativeImage nativeImage = m_RenderImage;
	nativeImage.format = IMAGE_FORMAT_RGBA;


	uint8_t *pBuffer = new uint8_t[dataSize];
	nativeImage.ppPlane[0] = pBuffer;
	BEGIN_TIME("DownloadPixels glReadPixels without PBO")
		glReadPixels(0, 0, nativeImage.width, nativeImage.height, GL_RGBA, GL_UNSIGNED_BYTE, pBuffer);
	//NativeImageUtil::DumpNativeImage(&nativeImage, "/sdcard/DCIM", "Normal");
	END_TIME("DownloadPixels glReadPixels without PBO")
    delete []pBuffer;

    int index = m_FrameIndex % 2;
    int nextIndex = (index + 1) % 2;

    BEGIN_TIME("DownloadPixels glReadPixels with PBO")
    glBindBuffer(GL_PIXEL_PACK_BUFFER, m_DownloadPboIds[index]);
    glReadPixels(0, 0, m_RenderImage.width, m_RenderImage.height, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    END_TIME("DownloadPixels glReadPixels with PBO")

    BEGIN_TIME("DownloadPixels PBO glMapBufferRange")
    glBindBuffer(GL_PIXEL_PACK_BUFFER, m_DownloadPboIds[nextIndex]);
    GLubyte *bufPtr = static_cast<GLubyte *>(glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0,
                                                           dataSize,
                                                           GL_MAP_READ_BIT));

    if (bufPtr) {
        nativeImage.ppPlane[0] = bufPtr;
        //NativeImageUtil::DumpNativeImage(&nativeImage, "/sdcard/DCIM", "PBO");
        glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
    }
    END_TIME("DownloadPixels PBO glMapBufferRange")
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

}
