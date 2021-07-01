/**
 *
 * Created by 公众号：字节流动 on 2020/4/18.
 * https://github.com/githubhaohao/NDK_OpenGLES_3_0
 * 最新文章首发于公众号：字节流动，有疑问或者技术交流可以添加微信 Byte-Flow ,领取视频教程, 拉你进技术交流群
 *
 * */

#include <gtc/matrix_transform.hpp>
#include "TextRenderSample.h"
#include "../util/GLUtils.h"
static const wchar_t BYTE_FLOW[] = L"微信公众号字节流动，欢迎关注交流学习。";
static const int MAX_SHORT_VALUE = 65536;

TextRenderSample::TextRenderSample()
{

	m_SamplerLoc = GL_NONE;
	m_MVPMatLoc = GL_NONE;

	m_TextureId = GL_NONE;
	m_VaoId = GL_NONE;
	m_VboId = GL_NONE;

	m_AngleX = 0;
	m_AngleY = 0;

	m_ScaleX = 1.0f;
	m_ScaleY = 1.0f;
}

TextRenderSample::~TextRenderSample()
{
	NativeImageUtil::FreeNativeImage(&m_RenderImage);

}

void TextRenderSample::Init()
{
	if(m_ProgramObj)
		return;

	LoadFacesByASCII();

	LoadFacesByUnicode(BYTE_FLOW, sizeof(BYTE_FLOW)/sizeof(BYTE_FLOW[0]) - 1);

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
            "layout(location = 0) in vec4 a_position;// <vec2 pos, vec2 tex>\n"
            "uniform mat4 u_MVPMatrix;\n"
            "out vec2 v_texCoord;\n"
            "void main()\n"
            "{\n"
            "    gl_Position = u_MVPMatrix * vec4(a_position.xy, 0.0, 1.0);;\n"
            "    v_texCoord = a_position.zw;\n"
            "}";

	char fShaderStr[] =
			"#version 300 es\n"
            "precision mediump float;\n"
            "in vec2 v_texCoord;\n"
            "layout(location = 0) out vec4 outColor;\n"
            "uniform sampler2D s_textTexture;\n"
            "uniform vec3 u_textColor;\n"
            "\n"
            "void main()\n"
            "{\n"
            "    vec4 color = vec4(1.0, 1.0, 1.0, texture(s_textTexture, v_texCoord).r);\n"
            "    outColor = vec4(u_textColor, 1.0) * color;\n"
            "}";

	m_ProgramObj = GLUtils::CreateProgram(vShaderStr, fShaderStr);
	if (m_ProgramObj)
	{
		m_SamplerLoc = glGetUniformLocation(m_ProgramObj, "s_textTexture");
		m_MVPMatLoc = glGetUniformLocation(m_ProgramObj, "u_MVPMatrix");
	}
	else
	{
		LOGCATE("TextRenderSample::Init create program fail");
	}

	// Generate VAO Id
	glGenVertexArrays(1, &m_VaoId);
	// Generate VBO Ids and load the VBOs with data
	glGenBuffers(1, &m_VboId);

	glBindVertexArray(m_VaoId);
	glBindBuffer(GL_ARRAY_BUFFER, m_VboId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);
    glBindVertexArray(GL_NONE);

	//upload RGBA image data
	glBindTexture(GL_TEXTURE_2D, m_TextureId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_RenderImage.width, m_RenderImage.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_RenderImage.ppPlane[0]);
	glBindTexture(GL_TEXTURE_2D, GL_NONE);

}

void TextRenderSample::LoadImage(NativeImage *pImage)
{
	LOGCATE("TextRenderSample::LoadImage pImage = %p", pImage->ppPlane[0]);
	if (pImage)
	{
		m_RenderImage.width = pImage->width;
		m_RenderImage.height = pImage->height;
		m_RenderImage.format = pImage->format;
		NativeImageUtil::CopyNativeImage(pImage, &m_RenderImage);
	}

}

void TextRenderSample::Draw(int screenW, int screenH)
{
	m_SurfaceWidth = screenW;
	m_SurfaceHeight = screenH;
	LOGCATE("TextRenderSample::Draw()");
	if(m_ProgramObj == GL_NONE) return;

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); //禁用byte-alignment限制
	glEnable(GL_BLEND);
	//glEnable(GL_CULL_FACE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glm::vec2 viewport(m_SurfaceWidth, m_SurfaceHeight);

	UpdateMVPMatrix(m_MVPMatrix, m_AngleX, m_AngleY, viewport.x / viewport.y);
	glUniformMatrix4fv(m_MVPMatLoc, 1, GL_FALSE, &m_MVPMatrix[0][0]);

	// (x,y)为屏幕坐标系的位置，即原点位于屏幕中心，x(-1.0,1.0), y(-1.0,1.0)
	RenderText("My WeChat ID is Byte-Flow.", -0.9f, 0.2f, 1.0f, glm::vec3(0.8, 0.1f, 0.1f), viewport);
	RenderText("Welcome to add my WeChat.", -0.9f, 0.0f, 2.0f, glm::vec3(0.2, 0.4f, 0.7f), viewport);

	RenderText(BYTE_FLOW, sizeof(BYTE_FLOW)/sizeof(BYTE_FLOW[0]) - 1, -0.9f, -0.2f, 1.0f, glm::vec3(0.7, 0.4f, 0.2f), viewport);

}

void TextRenderSample::UpdateTransformMatrix(float rotateX, float rotateY, float scaleX, float scaleY)
{
	GLSampleBase::UpdateTransformMatrix(rotateX, rotateY, scaleX, scaleY);
	m_AngleX = static_cast<int>(rotateX);
	m_AngleY = static_cast<int>(rotateY);
	m_ScaleX = scaleX;
	m_ScaleY = scaleY;
}

void TextRenderSample::RenderText(std::string text, GLfloat x, GLfloat y, GLfloat scale,
                                  glm::vec3 color, glm::vec2 viewport) {
	// 激活合适的渲染状态
	glUseProgram(m_ProgramObj);
	glUniform3f(glGetUniformLocation(m_ProgramObj, "u_textColor"), color.x, color.y, color.z);
	glBindVertexArray(m_VaoId);
	GO_CHECK_GL_ERROR();
	// 对文本中的所有字符迭代
	std::string::const_iterator c;
	x *= viewport.x;
	y *= viewport.y;
	for (c = text.begin(); c != text.end(); c++)
	{
		Character ch = m_Characters[*c];

		GLfloat xpos = x + ch.bearing.x * scale;
		GLfloat ypos = y - (ch.size.y - ch.bearing.y) * scale;

		xpos /= viewport.x;
		ypos /= viewport.y;

		GLfloat w = ch.size.x * scale;
		GLfloat h = ch.size.y * scale;

		w /= viewport.x;
		h /= viewport.y;

		LOGCATE("TextRenderSample::RenderText [xpos,ypos,w,h]=[%f, %f, %f, %f], ch.advance >> 6 = %d", xpos, ypos, w, h, ch.advance >> 6);

		// 当前字符的VBO
		GLfloat vertices[6][4] = {
				{ xpos,     ypos + h,   0.0, 0.0 },
				{ xpos,     ypos,       0.0, 1.0 },
				{ xpos + w, ypos,       1.0, 1.0 },

				{ xpos,     ypos + h,   0.0, 0.0 },
				{ xpos + w, ypos,       1.0, 1.0 },
				{ xpos + w, ypos + h,   1.0, 0.0 }
		};

		// 在方块上绘制字形纹理
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, ch.textureID);
		glUniform1i(m_SamplerLoc, 0);
		GO_CHECK_GL_ERROR();
		// 更新当前字符的VBO
		glBindBuffer(GL_ARRAY_BUFFER, m_VboId);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        GO_CHECK_GL_ERROR();
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		// 绘制方块
		glDrawArrays(GL_TRIANGLES, 0, 6);
		GO_CHECK_GL_ERROR();
		// 更新位置到下一个字形的原点，注意单位是1/64像素
		x += (ch.advance >> 6) * scale; //(2^6 = 64)
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}


void TextRenderSample::LoadFacesByASCII() {
	// FreeType
	FT_Library ft;
	// All functions return a value different than 0 whenever an error occurred
	if (FT_Init_FreeType(&ft))
		LOGCATE("TextRenderSample::LoadFacesByASCII FREETYPE: Could not init FreeType Library");

	// Load font as face
	FT_Face face;
    std::string path(DEFAULT_OGL_ASSETS_DIR);
	if (FT_New_Face(ft, (path + "/fonts/Antonio-Regular.ttf").c_str(), 0, &face))
		LOGCATE("TextRenderSample::LoadFacesByASCII FREETYPE: Failed to load font");

	// Set size to load glyphs as
	FT_Set_Pixel_Sizes(face, 0, 96);

	// Disable byte-alignment restriction
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Load first 128 characters of ASCII set
	for (unsigned char c = 0; c < 128; c++)
	{
		// Load character glyph
		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
		{
		LOGCATE("TextRenderSample::LoadFacesByASCII FREETYTPE: Failed to load Glyph");
			continue;
		}
		// Generate texture
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_LUMINANCE,
				face->glyph->bitmap.width,
				face->glyph->bitmap.rows,
				0,
				GL_LUMINANCE,
				GL_UNSIGNED_BYTE,
				face->glyph->bitmap.buffer
		);
//		NativeImage image;
//		image.width = face->glyph->bitmap.width;
//		image.height = face->glyph->bitmap.rows;
//		image.format = 8;
//		image.ppPlane[0] = face->glyph->bitmap.buffer;
//		NativeImageUtil::DumpNativeImage(&image, "/sdcard/DCIM", "TextRenderSample");
        LOGCATE("TextRenderSample::LoadFacesByASCII [w,h,buffer]=[%d, %d, %p], ch.advance >> 6 = %d", face->glyph->bitmap.width,face->glyph->bitmap.rows, face->glyph->bitmap.buffer,face->glyph->advance.x >> 6);
		// Set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// Now store character for later use
		Character character = {
				texture,
				glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
				glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
				static_cast<GLuint>(face->glyph->advance.x)
		};
		m_Characters.insert(std::pair<GLint, Character>(c, character));
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	// Destroy FreeType once we're finished
	FT_Done_Face(face);
	FT_Done_FreeType(ft);

}

void TextRenderSample::LoadFacesByUnicode(const wchar_t* text, int size) {
	// FreeType
	FT_Library ft;
	// All functions return a value different than 0 whenever an error occurred
	if (FT_Init_FreeType(&ft))
		LOGCATE("TextRenderSample::LoadFacesByUnicode FREETYPE: Could not init FreeType Library");

	// Load font as face
	FT_Face face;
    std::string path(DEFAULT_OGL_ASSETS_DIR);
    if (FT_New_Face(ft, (path + "/fonts/msyh.ttc").c_str(), 0, &face))
		LOGCATE("TextRenderSample::LoadFacesByUnicode FREETYPE: Failed to load font");

	// Set size to load glyphs as
	FT_Set_Pixel_Sizes(face, 96, 96);
	FT_Select_Charmap(face, ft_encoding_unicode);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	for (int i = 0; i < size; ++i) {
		//int index =  FT_Get_Char_Index(face,unicodeArr[i]);
		if (FT_Load_Glyph(face, FT_Get_Char_Index(face, text[i]), FT_LOAD_DEFAULT))
		{
			LOGCATE("TextRenderSample::LoadFacesByUnicode FREETYTPE: Failed to load Glyph");
			continue;
		}

		FT_Glyph glyph;
		FT_Get_Glyph(face->glyph, &glyph );

		//Convert the glyph to a bitmap.
		FT_Glyph_To_Bitmap(&glyph, ft_render_mode_normal, 0, 1 );
		FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;

		//This reference will make accessing the bitmap easier
		FT_Bitmap& bitmap = bitmap_glyph->bitmap;

		// Generate texture
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_LUMINANCE,
				bitmap.width,
				bitmap.rows,
				0,
				GL_LUMINANCE,
				GL_UNSIGNED_BYTE,
				bitmap.buffer
		);

		LOGCATE("TextRenderSample::LoadFacesByUnicode text[i]=%d [w,h,buffer]=[%d, %d, %p], advance.x=%ld", text[i], bitmap.width, bitmap.rows, bitmap.buffer, glyph->advance.x / MAX_SHORT_VALUE);
		// Set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// Now store character for later use
		Character character = {
				texture,
				glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
				glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
				static_cast<GLuint>((glyph->advance.x / MAX_SHORT_VALUE) << 6)
		};
		m_Characters.insert(std::pair<GLint, Character>(text[i], character));

	}
	glBindTexture(GL_TEXTURE_2D, 0);
	// Destroy FreeType once we're finished
	FT_Done_Face(face);
	FT_Done_FreeType(ft);
}

void TextRenderSample::Destroy()
{
	if (m_ProgramObj)
	{
		glDeleteProgram(m_ProgramObj);
		glDeleteBuffers(1, &m_VboId);
		glDeleteVertexArrays(1, &m_VaoId);
		glDeleteTextures(1, &m_TextureId);

		std::map<GLint, Character>::const_iterator iter;
		for (iter = m_Characters.begin(); iter != m_Characters.end(); iter++)
		{
			glDeleteTextures(1, &m_Characters[iter->first].textureID);
		}
	}
}

/**
 * @param angleX 绕X轴旋转度数
 * @param angleY 绕Y轴旋转度数
 * @param ratio 宽高比
 * */
void TextRenderSample::UpdateMVPMatrix(glm::mat4 &mvpMatrix, int angleX, int angleY, float ratio)
{
	LOGCATE("TextRenderSample::UpdateMVPMatrix angleX = %d, angleY = %d, ratio = %f", angleX, angleY, ratio);
	angleX = angleX % 360;
	angleY = angleY % 360;

	//转化为弧度角
	float radiansX = static_cast<float>(MATH_PI / 180.0f * angleX);
	float radiansY = static_cast<float>(MATH_PI / 180.0f * angleY);


	// Projection matrix
	glm::mat4 Projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 100.0f);
	//glm::mat4 Projection = glm::frustum(-ratio, ratio, -1.0f, 1.0f, 4.0f, 100.0f);
	//glm::mat4 Projection = glm::perspective(45.0f,ratio, 0.1f,100.f);

	// View matrix
	glm::mat4 View = glm::lookAt(
			glm::vec3(0, 0, 4), // Camera is at (0,0,1), in World Space
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

void TextRenderSample::RenderText(const wchar_t *text, int textLen, GLfloat x, GLfloat y, GLfloat scale,
								  glm::vec3 color, glm::vec2 viewport) {
	// 激活合适的渲染状态
	glUseProgram(m_ProgramObj);
	glUniform3f(glGetUniformLocation(m_ProgramObj, "u_textColor"), color.x, color.y, color.z);
	glBindVertexArray(m_VaoId);
	GO_CHECK_GL_ERROR();
	x *= viewport.x;
	y *= viewport.y;
	for (int i = 0; i < textLen; ++i)
	{
		Character ch = m_Characters[text[i]];

		GLfloat xpos = x + ch.bearing.x * scale;
		GLfloat ypos = y - (ch.size.y - ch.bearing.y) * scale;

		xpos /= viewport.x;
		ypos /= viewport.y;

		GLfloat w = ch.size.x * scale;
		GLfloat h = ch.size.y * scale;

		w /= viewport.x;
		h /= viewport.y;

		LOGCATE("TextRenderSample::RenderText [xpos,ypos,w,h]=[%f, %f, %f, %f]", xpos, ypos, w, h);

		// 当前字符的VBO
		GLfloat vertices[6][4] = {
				{ xpos,     ypos + h,   0.0, 0.0 },
				{ xpos,     ypos,       0.0, 1.0 },
				{ xpos + w, ypos,       1.0, 1.0 },

				{ xpos,     ypos + h,   0.0, 0.0 },
				{ xpos + w, ypos,       1.0, 1.0 },
				{ xpos + w, ypos + h,   1.0, 0.0 }
		};

		// 在方块上绘制字形纹理
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, ch.textureID);
		glUniform1i(m_SamplerLoc, 0);
		GO_CHECK_GL_ERROR();
		// 更新当前字符的VBO
		glBindBuffer(GL_ARRAY_BUFFER, m_VboId);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		GO_CHECK_GL_ERROR();
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		// 绘制方块
		glDrawArrays(GL_TRIANGLES, 0, 6);
		GO_CHECK_GL_ERROR();
		// 更新位置到下一个字形的原点，注意单位是1/64像素
		x += (ch.advance >> 6) * scale; //(2^6 = 64)
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}
