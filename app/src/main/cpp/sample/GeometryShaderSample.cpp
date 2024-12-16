/**
 *
 * Created by 公众号：字节流动 on 2024/12/02.
 * https://github.com/githubhaohao/NDK_OpenGLES_3_0
 * 最新文章首发于公众号：字节流动，有疑问或者技术交流可以添加微信 Byte-Flow ,领取视频教程, 拉你进技术交流群
 *
 * */

#include <GLUtils.h>
#include <LogUtil.h>
#include "GeometryShaderSample.h"

GeometryShaderSample::GeometryShaderSample()
{
	m_TextureId = 0;
    m_FrameIndex = 0;
}

GeometryShaderSample::~GeometryShaderSample()
{
	NativeImageUtil::FreeNativeImage(&m_RenderImage);
}

void GeometryShaderSample::Init()
{
    if(m_ProgramObj) return;

	//create RGBA texture
	glGenTextures(1, &m_TextureId);
	glBindTexture(GL_TEXTURE_2D, m_TextureId);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, GL_NONE);

	char vShaderStr[] = "#version 320 es\n"
                        "layout (location = 0) in vec2 aPos;\n"
                        "layout (location = 1) in vec3 aColor;\n"
                        "\n"
                        "out VS_OUT {\n"
                        "    vec3 color;\n"
                        "} vs_out;\n"
                        "\n"
                        "void main()\n"
                        "{\n"
                        "    vs_out.color = aColor;\n"
                        "    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);\n"
                        "}";

    char gShaderStr[] = "#version 320 es\n"
                        "layout (points) in;\n"
                        "layout (triangle_strip, max_vertices = 4) out;\n"
                        "\n"
                        "in VS_OUT {\n"
                        "    vec3 color;\n"
                        "} gs_in[];\n"
                        "\n"
                        "out vec3 gsColor;\n"
                        "out vec2 v_texCoord;\n"
                        "\n"
                        "uniform float iTime;\n"
                        "\n"
                        "void main() {\n"
                        "    gsColor = gs_in[0].color; // gs_in[0] since there's only one input vertex\n"
                        "    float offset = abs(sin(iTime * 0.25)) * 0.5;\n"
                        "    gl_Position = gl_in[0].gl_Position + vec4(-offset, -offset, 0.0, 0.0); // 1:bottom-left\n"
                        "    v_texCoord = vec2(0.0, 1.0);\n"
                        "    EmitVertex();\n"
                        "    gl_Position = gl_in[0].gl_Position + vec4( offset, -offset, 0.0, 0.0); // 2:bottom-right\n"
                        "    v_texCoord = vec2(1.0, 1.0);\n"
                        "    EmitVertex();\n"
                        "    gl_Position = gl_in[0].gl_Position + vec4(-offset,  offset, 0.0, 0.0); // 3:top-left\n"
                        "    v_texCoord = vec2(0.0, 0.0);\n"
                        "    EmitVertex();\n"
                        "    gl_Position = gl_in[0].gl_Position + vec4( offset,  offset, 0.0, 0.0); // 4:top-right\n"
                        "    v_texCoord = vec2(1.0, 0.0);\n"
                        "    EmitVertex();\n"
                        "    EndPrimitive();\n"
                        "}";

	char fShaderStr[] = "#version 320 es\n"
                        "precision highp float;\n"
                        "uniform sampler2D sTexture;\n"
                        "in vec2 v_texCoord;\n"
                        "out vec4 outColor;\n"
                        "in vec3 gsColor;\n"
                        "void main()\n"
                        "{\n"
                        "    outColor = texture(sTexture, v_texCoord) + vec4(gsColor, 1.0) * 0.15;\n"
                        "}";

	m_ProgramObj = GLUtils::CreateProgramWithGeometryShader(vShaderStr, fShaderStr, gShaderStr);
	if (!m_ProgramObj)
	{
		LOGCATE("GeometryShaderSample::Init create program fail");
	}

    //upload RGBA image data
    glBindTexture(GL_TEXTURE_2D, m_TextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_RenderImage.width, m_RenderImage.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_RenderImage.ppPlane[0]);
    glBindTexture(GL_TEXTURE_2D, GL_NONE);

    GLfloat vertices[] = {
             -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, // top-left
              0.5f,  0.5f, 0.0f, 1.0f, 0.0f, // top-right
             0.5f, -0.5f, 0.0f, 0.0f, 1.0f, // bottom-right
            -0.5f, -0.5f, 1.0f, 1.0f, 0.0f  // bottom-left
    };

    glGenBuffers(1, &m_VboId);
    glGenVertexArrays(1, &m_VaoId);
    glBindVertexArray(m_VaoId);
    glBindBuffer(GL_ARRAY_BUFFER, m_VboId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
    glBindVertexArray(0);
}

void GeometryShaderSample::Draw(int screenW, int screenH)
{
	LOGCATE("GeometryShaderSample::Draw()");

	if(m_ProgramObj == GL_NONE || m_TextureId == GL_NONE) return;

	glClearColor(0.1, 0.1, 0.1, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	// Use the program object
	glUseProgram (m_ProgramObj);
    glBindVertexArray(m_VaoId);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_TextureId);
    GLUtils::setInt(m_ProgramObj, "sTexture", 0);

    GLUtils::setFloat(m_ProgramObj, "iTime", m_FrameIndex * 0.015f);

    glDrawArrays(GL_POINTS, 0, 4);

    m_FrameIndex++;
}

void GeometryShaderSample::LoadImage(NativeImage *pImage)
{
	LOGCATE("GeometryShaderSample::LoadImage pImage = %p", pImage->ppPlane[0]);
	if (pImage)
	{
		m_RenderImage.width = pImage->width;
		m_RenderImage.height = pImage->height;
		m_RenderImage.format = pImage->format;
		NativeImageUtil::CopyNativeImage(pImage, &m_RenderImage);
	}
}

void GeometryShaderSample::Destroy()
{
	if (m_ProgramObj)
	{
		glDeleteProgram(m_ProgramObj);
		glDeleteTextures(1, &m_TextureId);
        glDeleteBuffers(1, &m_VboId);
        glDeleteVertexArrays(1, &m_VaoId);
		m_ProgramObj = GL_NONE;
        m_VboId = GL_NONE;
        m_VaoId = GL_NONE;
	}
}
