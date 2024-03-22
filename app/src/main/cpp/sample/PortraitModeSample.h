/**
 *
 * Created by 公众号：字节流动 on 2020/4/18.
 * https://github.com/githubhaohao/NDK_OpenGLES_3_0
 * 最新文章首发于公众号：字节流动，有疑问或者技术交流可以添加微信 Byte-Flow ,领取视频教程, 拉你进技术交流群
 *
 * */

#ifndef NDK_OPENGLES_3_0_PORTRAITMODESAMPLE_H
#define NDK_OPENGLES_3_0_PORTRAITMODESAMPLE_H

#include "GLSampleBase.h"
#include "../util/ImageDef.h"

static const char* fBlendShaderStr = "#version 300 es\n"
						   "precision highp float;\n"
						   "in vec2 v_texCoord;\n"
						   "uniform sampler2D sTexture;\n"
						   "uniform sampler2D sPortraitTexture;\n"
						   "layout(location = 0) out vec4 outColor;\n"
						   "\n"
						   "void main() {\n"
						   "    vec4 portraitCol = texture(sPortraitTexture, v_texCoord);\n"
						   "    vec4 srcCol = texture(sTexture, v_texCoord);\n"
						   "    srcCol *= srcCol.a;\n"
						   "    portraitCol *= portraitCol.a;\n"
						   "    outColor = mix(srcCol, portraitCol, portraitCol.a);\n"
						   "    outColor.rgb /= max(0.000001, outColor.a);\n"
						   "}";

static const char* fCircleBokehShaderStr = "#version 300 es\n"
								 "precision highp float;\n"
								 "in vec2 v_texCoord;\n"
								 "uniform sampler2D sTexture;\n"
								 "uniform sampler2D sPortraitTexture;\n"
								 "uniform highp vec2 inputSize;\n"
								 "layout(location = 0) out vec4 outColor;\n"
								 "\n"
								 "float sdCircle(in vec2 p, in float r) {\n"
								 "    return length(p)-r;\n"
								 "}\n"
								 "\n"
								 "void main() {\n"
								 "    float effectValue = 0.4;\n"
								 "    vec2 uv = v_texCoord;\n"
								 "    vec2 d = 1.0 / inputSize;\n"
								 "    float scale = (inputSize.x + inputSize.y)/1080.0;\n"
								 "    float kernel = floor(20.0 * scale * effectValue);\n"
								 "    vec4 maxCol = vec4(0.0);\n"
								 "    vec4 mask = texture(sPortraitTexture, uv);\n"
								 "    for(float x = -kernel;x<kernel;x++) {\n"
								 "        for(float y = -kernel;y<kernel;y++) {\n"
								 "            vec2 xy = vec2(x,y)/kernel;\n"
								 "            if(sdCircle(xy, 0.5 + effectValue * 0.2) < 0.0) {\n"
								 "                vec4 col = texture(sTexture, uv + d * vec2(x,y));\n"
								 "                maxCol = max(maxCol, col * mask.a);\n"
								 "            }\n"
								 "        }\n"
								 "    }\n"
								 "    outColor = maxCol;\n"
								 "}";

static const char* fFastGaussianBlurStr = "#version 300 es\n"
								"precision highp float;\n"
								"in vec2 v_texCoord;\n"
								"uniform sampler2D sTexture;\n"
								"uniform highp vec2 inputSize;\n"
								"layout(location = 0) out vec4 outColor;\n"
								"\n"
								"#define BMIX 0.4\n"
								"#define OMIX 0.1\n"
								"#define MAX_BLUR 20.0\n"
								"\n"
								"const float PI = 3.141592653589793238462;\n"
								"const float inv_sqrt_2pi = 0.3989422804014327;\n"
								"float soft_light(float base, float blend)\n"
								"{\n"
								"    return (blend < 0.5) ? (2.0 * base * blend + base * base * (1.0 - 2.0 * blend)) : (sqrt(base) * (2.0 * blend - 1.0) + 2.0 * base * (1.0 - blend));\n"
								"}\n"
								"vec3 soft_light(vec3 base, vec3 blend)\n"
								"{\n"
								"    return vec3(soft_light(base.x,blend.x),soft_light(base.y,blend.y),soft_light(base.z,blend.z));\n"
								"}\n"
								"vec3 screen( vec3 s, vec3 d )\n"
								"{\n"
								"    return s + d - s * d;\n"
								"}\n"
								"float luma(vec4 color) {\n"
								"    return clamp(dot(color.rgb, vec3(0.299, 0.587, 0.114)),0.0,1.0);\n"
								"}\n"
								"float gaussianf(float sigma, float dist)\n"
								"{\n"
								"    float a = dist / sigma;\n"
								"    return (inv_sqrt_2pi / sigma ) * exp( -0.5 * a * a );\n"
								"}\n"
								"\n"
								"vec4 fast_gaussian_blur(sampler2D image, vec2 uv,vec2 resolution, vec2 direction, float blur)\n"
								"{\n"
								"    vec4 color = vec4(0.0);\n"
								"    float coef = gaussianf(blur,0.0);\n"
								"    color += texture(image, uv) * coef;\n"
								"    float kernel_s = blur * 4.0;\n"
								"    //removed precomputing weights by onehade\n"
								"    float prev = gaussianf(blur, 0.0);\n"
								"    for (float i = 0.0; i < kernel_s; i++) {\n"
								"        float w1 = prev;\n"
								"        float w2 = gaussianf(blur, i + 1.0);\n"
								"        prev = w2; //storing redudant weight for next loop\n"
								"        float c = w1 + w2;\n"
								"        vec2 off1 = vec2((i * w1 + (i + 1.0) * w2) / c) * direction / resolution;\n"
								"        color += texture(image, uv - off1 ) * c;\n"
								"        color += texture(image, uv + off1 ) * c;\n"
								"        coef += c + c;\n"
								"    }\n"
								"\n"
								"    return color/coef;\n"
								"}\n"
								"\n"
								"void main() {\n"
								"    float effectValue = 0.5;\n"
								"    vec2 direction = vec2(0.0, 0.6);\n"
								"    vec2 uv = v_texCoord;\n"
								"    outColor = fast_gaussian_blur(sTexture, uv, inputSize, direction, effectValue * MAX_BLUR);;\n"
								"}";

class PortraitModeSample : public GLSampleBase
{
public:
	PortraitModeSample();

	virtual ~PortraitModeSample();

	virtual void LoadMultiImageWithIndex(int index, NativeImage *pImage);

	virtual void Init();
	virtual void Draw(int screenW, int screenH);

	virtual void Destroy();

	bool CreateFrameBufferObj();

	int RenderToFrameBuffer(GLuint program, GLuint srcTexId, GLuint dstTexId);

private:
	GLuint m_SrcTexId;
	GLuint m_PortraitTexId;
	GLuint m_FboTextureId;
	GLuint m_FboId;
	GLuint m_VaoIds[2];
	GLuint m_VboIds[4];
	NativeImage m_RenderImages[2];
	GLuint m_BlendProgramObj;
	GLuint m_CircleBokehProgramObj;
	GLuint m_FastGaussianBlurProgramObj;
};


#endif //NDK_OPENGLES_3_0_PORTRAITMODESAMPLE_H
