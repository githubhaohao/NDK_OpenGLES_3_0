/**
 *
 * Created by 公众号：字节流动 on 2021/3/12.
 * https://github.com/githubhaohao/NDK_OpenGLES_3_0
 * 最新文章首发于公众号：字节流动，有疑问或者技术交流可以添加微信 Byte-Flow ,领取视频教程, 拉你进技术交流群
 *
 * */

#ifndef NDK_OPENGLES_3_0_GLSAMPLEBASE_H
#define NDK_OPENGLES_3_0_GLSAMPLEBASE_H

#include "stdint.h"
#include <GLES3/gl3.h>
#include <ImageDef.h>
#include <ByteFlowLock.h>

//For PI define
#define MATH_PI 3.1415926535897932384626433832802

#define SAMPLE_TYPE                             200
#define SAMPLE_TYPE_KEY_TRIANGLE                SAMPLE_TYPE + 0
#define SAMPLE_TYPE_KEY_TEXTURE_MAP             SAMPLE_TYPE + 1
#define SAMPLE_TYPE_KEY_YUV_TEXTURE_MAP         SAMPLE_TYPE + 2
#define SAMPLE_TYPE_KEY_VAO                     SAMPLE_TYPE + 3
#define SAMPLE_TYPE_KEY_FBO                     SAMPLE_TYPE + 4
#define SAMPLE_TYPE_KEY_FBO_LEG_LENGTHEN        SAMPLE_TYPE + 6
#define SAMPLE_TYPE_KEY_COORD_SYSTEM            SAMPLE_TYPE + 7
#define SAMPLE_TYPE_KEY_BASIC_LIGHTING          SAMPLE_TYPE + 8
#define SAMPLE_TYPE_KEY_TRANSFORM_FEEDBACK      SAMPLE_TYPE + 9
#define SAMPLE_TYPE_KEY_MULTI_LIGHTS            SAMPLE_TYPE + 10
#define SAMPLE_TYPE_KEY_DEPTH_TESTING           SAMPLE_TYPE + 11
#define SAMPLE_TYPE_KEY_INSTANCING              SAMPLE_TYPE + 12
#define SAMPLE_TYPE_KEY_STENCIL_TESTING         SAMPLE_TYPE + 13
#define SAMPLE_TYPE_KEY_BLENDING                SAMPLE_TYPE + 14
#define SAMPLE_TYPE_KEY_PARTICLES               SAMPLE_TYPE + 15
#define SAMPLE_TYPE_KEY_SKYBOX                  SAMPLE_TYPE + 16
#define SAMPLE_TYPE_KEY_3D_MODEL                SAMPLE_TYPE + 17
#define SAMPLE_TYPE_KEY_PBO                     SAMPLE_TYPE + 18
#define SAMPLE_TYPE_KEY_BEATING_HEART           SAMPLE_TYPE + 19
#define SAMPLE_TYPE_KEY_CLOUD                   SAMPLE_TYPE + 20
#define SAMPLE_TYPE_KEY_TIME_TUNNEL             SAMPLE_TYPE + 21
#define SAMPLE_TYPE_KEY_BEZIER_CURVE            SAMPLE_TYPE + 22
#define SAMPLE_TYPE_KEY_BIG_EYES                SAMPLE_TYPE + 23
#define SAMPLE_TYPE_KEY_FACE_SLENDER            SAMPLE_TYPE + 24
#define SAMPLE_TYPE_KEY_BIG_HEAD                SAMPLE_TYPE + 25
#define SAMPLE_TYPE_KEY_RATARY_HEAD             SAMPLE_TYPE + 26
#define SAMPLE_TYPE_KEY_VISUALIZE_AUDIO         SAMPLE_TYPE + 27
#define SAMPLE_TYPE_KEY_SCRATCH_CARD            SAMPLE_TYPE + 28
#define SAMPLE_TYPE_KEY_AVATAR                  SAMPLE_TYPE + 29
#define SAMPLE_TYPE_KEY_SHOCK_WAVE              SAMPLE_TYPE + 30
#define SAMPLE_TYPE_KEY_MRT                     SAMPLE_TYPE + 31
#define SAMPLE_TYPE_KEY_FBO_BLIT                SAMPLE_TYPE + 32
#define SAMPLE_TYPE_KEY_TBO                     SAMPLE_TYPE + 33
#define SAMPLE_TYPE_KEY_UBO                     SAMPLE_TYPE + 34
#define SAMPLE_TYPE_KEY_RGB2YUV                 SAMPLE_TYPE + 35
#define SAMPLE_TYPE_KEY_MULTI_THREAD_RENDER     SAMPLE_TYPE + 36
#define SAMPLE_TYPE_KEY_TEXT_RENDER     		SAMPLE_TYPE + 37
#define SAMPLE_TYPE_KEY_STAY_COLOR       		SAMPLE_TYPE + 38


#define SAMPLE_TYPE_KEY_SET_TOUCH_LOC           SAMPLE_TYPE + 999
#define SAMPLE_TYPE_SET_GRAVITY_XY              SAMPLE_TYPE + 1000

#define DEFAULT_OGL_ASSETS_DIR "/sdcard/Android/data/com.byteflow.app/files/Download"
class GLSampleBase
{
public:
	GLSampleBase()
	{
		m_ProgramObj = 0;
		m_VertexShader = 0;
		m_FragmentShader = 0;

		m_SurfaceWidth = 0;
		m_SurfaceHeight = 0;

	}

	virtual ~GLSampleBase()
	{

	}

	virtual void LoadImage(NativeImage *pImage)
	{};

	virtual void LoadMultiImageWithIndex(int index, NativeImage *pImage)
	{};

	virtual void LoadShortArrData(short *const pShortArr, int arrSize)
	{}

	virtual void UpdateTransformMatrix(float rotateX, float rotateY, float scaleX, float scaleY)
	{}

	virtual void SetTouchLocation(float x, float y)
	{}

	virtual void SetGravityXY(float x, float y)
	{}

	virtual void Init() = 0;
	virtual void Draw(int screenW, int screenH) = 0;

	virtual void Destroy() = 0;

protected:
	GLuint m_VertexShader;
	GLuint m_FragmentShader;
	GLuint m_ProgramObj;
	MySyncLock m_Lock;
	int m_SurfaceWidth;
	int m_SurfaceHeight;
};


#endif //NDK_OPENGLES_3_0_GLSAMPLEBASE_H
