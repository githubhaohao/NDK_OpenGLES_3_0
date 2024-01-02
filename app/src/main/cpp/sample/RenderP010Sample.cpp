/**
 *
 * Created by 公众号：字节流动 on 2023/3/12.
 * https://github.com/githubhaohao/NDK_OpenGLES_3_0
 * 最新文章首发于公众号：字节流动，有疑问或者技术交流可以添加微信 Byte-Flow ,领取视频教程, 拉你进技术交流群
 *
 * */
#include <GLUtils.h>
#include "RenderP010Sample.h"
#include "YUVP010Example.h"

#define IMAGE_PATH "/sdcard/Android/data/com.byteflow.app/files/Download/yuv/IMAGE_4406x3108.P010"

void RenderP010Sample::LoadImage(NativeImage *pImage)
{
	NativeImageUtil::FreeNativeImage(&m_RenderImage);
	if(m_RenderImage.ppPlane[0] == nullptr) {
		m_RenderImage.width = 4406;
		m_RenderImage.height = 3108;
		m_RenderImage.format = IMAGE_FORMAT_P010;
		NativeImageUtil::AllocNativeImage(&m_RenderImage);
	}
	NativeImageUtil::LoadNativeImage(&m_RenderImage, IMAGE_PATH);

	//YUVP010Example::YUVP010Test();
}

void RenderP010Sample::Init()
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
				uniform sampler2D uv_texture;
				out vec4 outColor;
				void main() {
					vec4 yCol = texture(y_texture, v_texCoord);
					vec4 uvCol = texture(uv_texture, v_texCoord);

					float val = 255.0 * yCol.r + yCol.a * 255.0 * pow(2.0, 8.0);
					float yVal = val / 65535.0 - 0.063;

					val = 255.0 * uvCol.r + uvCol.g * 255.0 * pow(2.0, 8.0);
					float vVal = val / 65535.0 - 0.502;

					val = 255.0 * uvCol.b + uvCol.a * 255.0 * pow(2.0, 8.0);
					float uVal = val / 65535.0 - 0.502;

					highp vec3 rgb = mat3(1.164, 1.164, 1.164,
											  0, -0.392, 2.017,
										  1.596, -0.813,  0.0) * vec3(yVal, uVal, vVal);
					outColor = vec4(rgb, 1.0);
				}
)";

	// Load the shaders and get a linked program object
	m_ProgramObj= GLUtils::CreateProgram(vShaderStr, fShaderStr, m_VertexShader, m_FragmentShader);

	// Get the sampler location
	m_ySamplerLoc = glGetUniformLocation (m_ProgramObj, "y_texture" );
	m_uvSamplerLoc = glGetUniformLocation(m_ProgramObj, "uv_texture");


	//create textures
	GLuint textureIds[2] = {0};
	glGenTextures(2, textureIds);

	m_yTextureId = textureIds[0];
	m_uvTextureId = textureIds[1];

	LoadImage(nullptr);

	//upload Y plane data
	glBindTexture(GL_TEXTURE_2D, m_yTextureId);
	//GL_R16F, GL_RED, GL_HALF_FLOAT
	glTexImage2D ( GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, m_RenderImage.width, m_RenderImage.height, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, m_RenderImage.ppPlane[0]);

	//glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, m_RenderImage.width, m_RenderImage.height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, m_RenderImage.ppPlane[0]);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, GL_NONE);

	//update UV plane data
	glBindTexture(GL_TEXTURE_2D, m_uvTextureId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_RenderImage.width >> 1, m_RenderImage.height >> 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_RenderImage.ppPlane[1]);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, GL_NONE);
}

void RenderP010Sample::Draw(int screenW, int screenH)
{
	LOGCATE("RenderP010Sample::Draw()");

	if(m_ProgramObj == GL_NONE || m_yTextureId == GL_NONE) return;

	GLfloat verticesCoords[] = {
			-1.0f,  1.0f, 0.0f,  // Position 0
			-1.0f, -1.0f, 0.0f,  // Position 1
			1.0f,  -1.0f, 0.0f,  // Position 2
			1.0f,   1.0f, 0.0f,  // Position 3
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
	glBindTexture(GL_TEXTURE_2D, m_yTextureId);

	// Set the Y plane sampler to texture unit to 0
	glUniform1i(m_ySamplerLoc, 0);

	// Bind the UV plane map
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_uvTextureId);

	// Set the UV plane sampler to texture unit to 1
	glUniform1i(m_uvSamplerLoc, 1);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
}

void RenderP010Sample::Destroy()
{
	if (m_ProgramObj)
	{
		glDeleteProgram(m_ProgramObj);
		glDeleteTextures(1, &m_yTextureId);
		glDeleteTextures(1, &m_uvTextureId);
		m_ProgramObj = GL_NONE;
	}
}
