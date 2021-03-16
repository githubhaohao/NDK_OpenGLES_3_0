/**
 *
 * Created by 公众号：字节流动 on 2020/4/18.
 * https://github.com/githubhaohao/NDK_OpenGLES_3_0
 * 最新文章首发于公众号：字节流动，有疑问或者技术交流可以添加微信 Byte-Flow ,领取视频教程, 拉你进技术交流群
 *
 * */

#include <gtc/matrix_transform.hpp>
#include "CloudSample.h"
#include "../util/GLUtils.h"

CloudSample::CloudSample()
{

	m_SamplerLoc = GL_NONE;
	m_MVPMatLoc = GL_NONE;

	m_TextureId = GL_NONE;
	m_VaoId = GL_NONE;

	m_AngleX = 0;
	m_AngleY = 0;

	m_ScaleX = 1.0f;
	m_ScaleY = 1.0f;
}

CloudSample::~CloudSample()
{
	NativeImageUtil::FreeNativeImage(&m_RenderImage);

}

void CloudSample::Init()
{
	if(m_ProgramObj)
		return;
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
            "layout(location = 0) in vec4 a_position;\n"
            "uniform mat4 u_MVPMatrix;\n"
            "void main()\n"
            "{\n"
            "    gl_Position = u_MVPMatrix * a_position;\n"
            "}";

	char fShaderStr[] =
			"#version 300 es\n"
            "precision highp float;\n"
            "layout(location = 0) out vec4 outColor;\n"
            "uniform float u_time;\n"
            "uniform vec2 u_screenSize;\n"
            "const float cloudscale = 1.1;\n"
            "const float speed = 0.03;\n"
            "const float clouddark = 0.5;\n"
            "const float cloudlight = 0.3;\n"
            "const float cloudcover = 0.2;\n"
            "const float cloudalpha = 8.0;\n"
            "const float skytint = 0.5;\n"
            "const vec3 skycolour1 = vec3(0.2, 0.4, 0.6);\n"
            "const vec3 skycolour2 = vec3(0.4, 0.7, 1.0);\n"
            "\n"
            "const mat2 m = mat2( 1.6,  1.2, -1.2,  1.6 );\n"
            "\n"
            "vec2 hash( vec2 p ) {\n"
            "    p = vec2(dot(p,vec2(127.1,311.7)), dot(p,vec2(269.5,183.3)));\n"
            "    return -1.0 + 2.0*fract(sin(p)*43758.5453123);\n"
            "}\n"
            "\n"
            "float noise( in vec2 p ) {\n"
            "    const float K1 = 0.366025404; // (sqrt(3)-1)/2;\n"
            "    const float K2 = 0.211324865; // (3-sqrt(3))/6;\n"
            "    vec2 i = floor(p + (p.x+p.y)*K1);\n"
            "    vec2 a = p - i + (i.x+i.y)*K2;\n"
            "    vec2 o = (a.x>a.y) ? vec2(1.0,0.0) : vec2(0.0,1.0); //vec2 of = 0.5 + 0.5*vec2(sign(a.x-a.y), sign(a.y-a.x));\n"
            "    vec2 b = a - o + K2;\n"
            "    vec2 c = a - 1.0 + 2.0*K2;\n"
            "    vec3 h = max(0.5-vec3(dot(a,a), dot(b,b), dot(c,c) ), 0.0 );\n"
            "    vec3 n = h*h*h*h*vec3( dot(a,hash(i+0.0)), dot(b,hash(i+o)), dot(c,hash(i+1.0)));\n"
            "    return dot(n, vec3(70.0));\n"
            "}\n"
            "\n"
            "float fbm(vec2 n) {\n"
            "    float total = 0.0, amplitude = 0.1;\n"
            "    for (int i = 0; i < 7; i++) {\n"
            "        total += noise(n) * amplitude;\n"
            "        n = m * n;\n"
            "        amplitude *= 0.4;\n"
            "    }\n"
            "    return total;\n"
            "}\n"
            "\n"
            "// -----------------------------------------------\n"
            "\n"
            "void main() {\n"
            "    vec2 fragCoord = gl_FragCoord.xy;\n"
            "    vec2 p = fragCoord.xy / u_screenSize.xy;\n"
            "    vec2 uv = p*vec2(u_screenSize.x/u_screenSize.y,1.0);\n"
            "    float time = u_time * speed;\n"
            "    float q = fbm(uv * cloudscale * 0.5);\n"
            "\n"
            "    //ridged noise shape\n"
            "    float r = 0.0;\n"
            "    uv *= cloudscale;\n"
            "    uv -= q - time;\n"
            "    float weight = 0.8;\n"
            "    for (int i=0; i<8; i++){\n"
            "        r += abs(weight*noise( uv ));\n"
            "        uv = m*uv + time;\n"
            "        weight *= 0.7;\n"
            "    }\n"
            "\n"
            "    //noise shape\n"
            "    float f = 0.0;\n"
            "    uv = p*vec2(u_screenSize.x/u_screenSize.y,1.0);\n"
            "    uv *= cloudscale;\n"
            "    uv -= q - time;\n"
            "    weight = 0.7;\n"
            "    for (int i=0; i<8; i++){\n"
            "        f += weight*noise( uv );\n"
            "        uv = m*uv + time;\n"
            "        weight *= 0.6;\n"
            "    }\n"
            "\n"
            "    f *= r + f;\n"
            "\n"
            "    //noise colour\n"
            "    float c = 0.0;\n"
            "    time = u_time * speed * 2.0;\n"
            "    uv = p*vec2(u_screenSize.x/u_screenSize.y,1.0);\n"
            "    uv *= cloudscale*2.0;\n"
            "    uv -= q - time;\n"
            "    weight = 0.4;\n"
            "    for (int i=0; i<7; i++){\n"
            "        c += weight*noise( uv );\n"
            "        uv = m*uv + time;\n"
            "        weight *= 0.6;\n"
            "    }\n"
            "\n"
            "    //noise ridge colour\n"
            "    float c1 = 0.0;\n"
            "    time = u_time * speed * 3.0;\n"
            "    uv = p*vec2(u_screenSize.x/u_screenSize.y,1.0);\n"
            "    uv *= cloudscale*3.0;\n"
            "    uv -= q - time;\n"
            "    weight = 0.4;\n"
            "    for (int i=0; i<7; i++){\n"
            "        c1 += abs(weight*noise( uv ));\n"
            "        uv = m*uv + time;\n"
            "        weight *= 0.6;\n"
            "    }\n"
            "\n"
            "    c += c1;\n"
            "\n"
            "    vec3 skycolour = mix(skycolour2, skycolour1, p.y);\n"
            "    vec3 cloudcolour = vec3(1.1, 1.1, 0.9) * clamp((clouddark + cloudlight*c), 0.0, 1.0);\n"
            "\n"
            "    f = cloudcover + cloudalpha*f*r;\n"
            "\n"
            "    vec3 result = mix(skycolour, clamp(skytint * skycolour + cloudcolour, 0.0, 1.0), clamp(f + c, 0.0, 1.0));\n"
            "\n"
            "    outColor = vec4( result, 1.0 );\n"
            "}";


	m_ProgramObj = GLUtils::CreateProgram(vShaderStr, fShaderStr, m_VertexShader, m_FragmentShader);
	if (m_ProgramObj)
	{
		m_SamplerLoc = glGetUniformLocation(m_ProgramObj, "s_TextureMap");
		m_MVPMatLoc = glGetUniformLocation(m_ProgramObj, "u_MVPMatrix");
		m_SizeLoc = glGetUniformLocation(m_ProgramObj, "u_screenSize");
		m_TimeLoc = glGetUniformLocation(m_ProgramObj, "u_time");
	}
	else
	{
		LOGCATE("CloudSample::Init create program fail");
	}

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

	// Generate VBO Ids and load the VBOs with data
	glGenBuffers(3, m_VboIds);
	glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesCoords), verticesCoords, GL_STATIC_DRAW);

//	glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[1]);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(textureCoords), textureCoords, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VboIds[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Generate VAO Id
	glGenVertexArrays(1, &m_VaoId);
	glBindVertexArray(m_VaoId);

	glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[0]);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (const void *)0);
	glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);

//	glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[1]);
//	glEnableVertexAttribArray(1);
//	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (const void *)0);
//	glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VboIds[2]);

	glBindVertexArray(GL_NONE);

}

void CloudSample::LoadImage(NativeImage *pImage)
{
	LOGCATE("CloudSample::LoadImage pImage = %p", pImage->ppPlane[0]);
	if (pImage)
	{
		m_RenderImage.width = pImage->width;
		m_RenderImage.height = pImage->height;
		m_RenderImage.format = pImage->format;
		NativeImageUtil::CopyNativeImage(pImage, &m_RenderImage);
	}

}

void CloudSample::Draw(int screenW, int screenH)
{
	LOGCATE("CloudSample::Draw()");

	if(m_ProgramObj == GL_NONE || m_RenderImage.ppPlane[0] == nullptr) return;

	UpdateMVPMatrix(m_MVPMatrix, m_AngleX, m_AngleY, (float)screenW / screenH);

	//upload RGBA image data
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_TextureId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_RenderImage.width, m_RenderImage.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_RenderImage.ppPlane[0]);
	glBindTexture(GL_TEXTURE_2D, GL_NONE);

	// Use the program object
	glUseProgram (m_ProgramObj);

	glBindVertexArray(m_VaoId);

	static int sFrameIndex = 0;
	sFrameIndex ++;

	glUniformMatrix4fv(m_MVPMatLoc, 1, GL_FALSE, &m_MVPMatrix[0][0]);
	float time = sFrameIndex * 0.04f;
    LOGCATE("CloudSample::Draw() time=%f",time);
	glUniform1f(m_TimeLoc, time);
    glUniform2f(m_SizeLoc, screenW, screenH);
	// Bind the RGBA map
//	glActiveTexture(GL_TEXTURE0);
//	glBindTexture(GL_TEXTURE_2D, m_TextureId);
//	glUniform1i(m_SamplerLoc, 0);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (const void *)0);

}

void CloudSample::Destroy()
{
	if (m_ProgramObj)
	{
		glDeleteProgram(m_ProgramObj);
		glDeleteBuffers(3, m_VboIds);
		glDeleteVertexArrays(1, &m_VaoId);
		glDeleteTextures(1, &m_TextureId);
	}
}

void CloudSample::UpdateMVPMatrix(glm::mat4 &mvpMatrix, int angleX, int angleY, float ratio)
{
	LOGCATE("CloudSample::UpdateMVPMatrix angleX = %d, angleY = %d, ratio = %f", angleX, angleY, ratio);
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

void CloudSample::UpdateTransformMatrix(float rotateX, float rotateY, float scaleX, float scaleY)
{
	GLSampleBase::UpdateTransformMatrix(rotateX, rotateY, scaleX, scaleY);
	m_AngleX = static_cast<int>(rotateX);
	m_AngleY = static_cast<int>(rotateY);
	m_ScaleX = scaleX;
	m_ScaleY = scaleY;
}
