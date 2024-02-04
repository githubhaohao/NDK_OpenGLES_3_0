/**
 *
 * Created by 公众号：字节流动 on 2024/01/12.
 * https://github.com/githubhaohao/NDK_OpenGLES_3_0
 * 最新文章首发于公众号：字节流动，有疑问或者技术交流可以添加微信 Byte-Flow ,领取视频教程, 拉你进技术交流群
 *
 * */
#include <GLUtils.h>
#include "RenderI444Sample.h"
#include "YUVP010Example.h"

void RenderI444Sample::LoadImage(NativeImage *pImage)
{
	LOGCATE("RenderI444Sample::LoadImage pImage = %p", pImage->ppPlane[0]);
	if (pImage)
	{
		m_RenderImage.width = pImage->width;
		m_RenderImage.height = pImage->height;
		m_RenderImage.format = pImage->format;
		NativeImageUtil::CopyNativeImage(pImage, &m_RenderImage);
	}
}

void RenderI444Sample::Init()
{
	char vShaderStr[] =R"(
			#version 300 es
			layout(location = 0) in vec4 a_position;
			layout(location = 1) in vec2 a_texCoord;
			out vec2 v_texCoord;
			void main()
			{
			   gl_Position = a_position;
			   v_texCoord = a_texCoord;
			})";

	char fShaderStr[] =R"(
		#version 300 es
		precision highp float;
		in vec2 v_texCoord;
		uniform sampler2D y_texture;
		uniform vec2 inputSize;
		out vec4 outColor;
		void main() {
			vec2 uv = v_texCoord;
			uv.y *= 1.0 / 3.0;
			float y = texture(y_texture, uv).r - 0.063;

			uv.y += 1.0 / 3.0;
			float u = texture(y_texture, uv).r - 0.502;

			uv.y += 1.0 / 3.0;
			float v = texture(y_texture, uv).r - 0.502;
			vec3 yuv = vec3(y,u,v);

			highp vec3 rgb = mat3(1.164, 1.164, 1.164,
			0, 		 -0.392, 	2.017,
			1.596,   -0.813,    0.0) * yuv;
			outColor = vec4(rgb, 1.0);
		}
)";

	// Load the shaders and get a linked program object
	m_ProgramObj = GLUtils::CreateProgram(vShaderStr, fShaderStr, m_VertexShader, m_FragmentShader);

	// Get the sampler location
	m_TextureLoc = glGetUniformLocation (m_ProgramObj, "y_texture" );

	//create textures
	GLuint textureIds[1] = {0};
	glGenTextures(1, textureIds);

	m_TextureId = textureIds[0];

	//upload Y plane data
	glBindTexture(GL_TEXTURE_2D, m_TextureId);
	//GL_R16F, GL_RED, GL_HALF_FLOAT
	glTexImage2D ( GL_TEXTURE_2D, 0, GL_LUMINANCE, m_RenderImage.width, m_RenderImage.height * 3, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, m_RenderImage.ppPlane[0]);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, GL_NONE);
}

void RenderI444Sample::Draw(int screenW, int screenH)
{
	LOGCATE("RenderI444Sample::Draw()");
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	if(m_ProgramObj == GL_NONE || m_TextureId == GL_NONE) return;

	GLfloat verticesCoords[] = {
			-1.0f,  1.0f, 0.0f,  // Position 0
			-1.0f, -1.0f, 0.0f,  // Position 1
			 1.0f, -1.0f, 0.0f,  // Position 2
			 1.0f, 1.0f, 0.0f,  // Position 3
	};

	GLfloat textureCoords[] = {
			0.0f,  0.0f,        // TexCoord 0
			0.0f,  1.0f,        // TexCoord 1
			1.0f,  1.0f,        // TexCoord 2
			1.0f,  0.0f         // TexCoord 3
	};

	GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

	// Use the program object
	glUseProgram (m_ProgramObj);

	// Load the vertex position
	glVertexAttribPointer (0, 3, GL_FLOAT,
						   GL_FALSE, 3 * sizeof (GLfloat), verticesCoords);
	// Load the texture coordinate
	glVertexAttribPointer (1, 2, GL_FLOAT,
						   GL_FALSE, 2 * sizeof (GLfloat), textureCoords);

	glEnableVertexAttribArray (0);
	glEnableVertexAttribArray (1);

	// Bind the Y plane map
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_TextureId);

	// Set the Y plane sampler to texture unit to 0
	glUniform1i(m_TextureLoc, 0);

	GLUtils::setVec2(m_ProgramObj, "inputSize", m_RenderImage.width, m_RenderImage.height);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
}

void RenderI444Sample::Destroy()
{
	if (m_ProgramObj)
	{
		glDeleteProgram(m_ProgramObj);
		glDeleteTextures(1, &m_TextureId);
		m_ProgramObj = GL_NONE;
	}
}
