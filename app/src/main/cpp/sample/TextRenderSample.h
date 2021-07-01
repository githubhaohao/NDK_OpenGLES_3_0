/**
 *
 * Created by 公众号：字节流动 on 2020/4/18.
 * https://github.com/githubhaohao/NDK_OpenGLES_3_0
 * 最新文章首发于公众号：字节流动，有疑问或者技术交流可以添加个人微信 Byte-Flow ,领取视频教程, 拉你进技术交流群, 坑位有限。
 *
 * */

#ifndef NDK_OPENGLES_3_0_TEXTRENDERSAMPLE_H
#define NDK_OPENGLES_3_0_TEXTRENDERSAMPLE_H

#include <detail/type_mat.hpp>
#include <detail/type_mat4x4.hpp>
#include "GLSampleBase.h"
#include "ft2build.h"
#include <freetype/ftglyph.h>
#include <string>
#include <map>

/// Holds all state information relevant to a character as loaded using FreeType
struct Character {
	GLuint textureID;   // ID handle of the glyph texture
	glm::ivec2 size;    // Size of glyph
	glm::ivec2 bearing;  // Offset from baseline to left/top of glyph
	GLuint advance;    // Horizontal offset to advance to next glyph
};

class TextRenderSample : public GLSampleBase
{
public:
	TextRenderSample();

	virtual ~TextRenderSample();

	virtual void LoadImage(NativeImage *pImage);

	virtual void Init();
	virtual void Draw(int screenW, int screenH);

	virtual void Destroy();

	virtual void UpdateTransformMatrix(float rotateX, float rotateY, float scaleX, float scaleY);

	void UpdateMVPMatrix(glm::mat4 &mvpMatrix, int angleX, int angleY, float ratio);

private:
	/**
	 * (x,y)为屏幕坐标系的位置，即原点位于屏幕中心，x(-1.0,1.0), y(-1.0,1.0)
	 * */
	void RenderText(std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color, glm::vec2 viewport);

	void RenderText(const wchar_t* text, int textLen, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color, glm::vec2 viewport);

	void LoadFacesByASCII();
	void LoadFacesByUnicode(const wchar_t* text, int size);

	GLuint m_TextureId;
	GLint m_SamplerLoc;
	GLint m_MVPMatLoc;
	GLuint m_VaoId;
	GLuint m_VboId;
	NativeImage m_RenderImage;
	glm::mat4 m_MVPMatrix;

	int m_AngleX;
	int m_AngleY;
	float m_ScaleX;
	float m_ScaleY;

	std::map<GLint, Character> m_Characters;

};


#endif //NDK_OPENGLES_3_0_TEXTRENDERSAMPLE_H
