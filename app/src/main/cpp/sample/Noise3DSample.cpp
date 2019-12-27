//
// Created by ByteFlow on 2019/7/30.
//

#include <gtc/matrix_transform.hpp>
#include <stdlib.h>
#include "Noise3DSample.h"
#include "../util/GLUtils.h"

// Attribute locations
#define ATTRIB_LOCATION_POS      0
#define ATTRIB_LOCATION_COLOR    1
#define ATTRIB_LOCATION_TEXCOORD 2

#define NOISE_TABLE_MASK   255

#define FLOOR(x)           ((int)(x) - ((x) < 0 && (x) != (int)(x)))
#define smoothstep(t)      ( t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f ) )
#define lerp(t, a, b)      ( a + t * (b - a) )
#ifdef _WIN32
#define srandom srand
#define random rand
#endif

// lattice gradients 3D noise
static float gradientTable[256 * 3];

// permTable describes a random permutatin of 8-bit values from 0 to 255.
static unsigned char permTable[256] =
		{
				0xE1, 0x9B, 0xD2, 0x6C, 0xAF, 0xC7, 0xDD, 0x90, 0xCB, 0x74, 0x46, 0xD5, 0x45, 0x9E,
				0x21, 0xFC,
				0x05, 0x52, 0xAD, 0x85, 0xDE, 0x8B, 0xAE, 0x1B, 0x09, 0x47, 0x5A, 0xF6, 0x4B, 0x82,
				0x5B, 0xBF,
				0xA9, 0x8A, 0x02, 0x97, 0xC2, 0xEB, 0x51, 0x07, 0x19, 0x71, 0xE4, 0x9F, 0xCD, 0xFD,
				0x86, 0x8E,
				0xF8, 0x41, 0xE0, 0xD9, 0x16, 0x79, 0xE5, 0x3F, 0x59, 0x67, 0x60, 0x68, 0x9C, 0x11,
				0xC9, 0x81,
				0x24, 0x08, 0xA5, 0x6E, 0xED, 0x75, 0xE7, 0x38, 0x84, 0xD3, 0x98, 0x14, 0xB5, 0x6F,
				0xEF, 0xDA,
				0xAA, 0xA3, 0x33, 0xAC, 0x9D, 0x2F, 0x50, 0xD4, 0xB0, 0xFA, 0x57, 0x31, 0x63, 0xF2,
				0x88, 0xBD,
				0xA2, 0x73, 0x2C, 0x2B, 0x7C, 0x5E, 0x96, 0x10, 0x8D, 0xF7, 0x20, 0x0A, 0xC6, 0xDF,
				0xFF, 0x48,
				0x35, 0x83, 0x54, 0x39, 0xDC, 0xC5, 0x3A, 0x32, 0xD0, 0x0B, 0xF1, 0x1C, 0x03, 0xC0,
				0x3E, 0xCA,
				0x12, 0xD7, 0x99, 0x18, 0x4C, 0x29, 0x0F, 0xB3, 0x27, 0x2E, 0x37, 0x06, 0x80, 0xA7,
				0x17, 0xBC,
				0x6A, 0x22, 0xBB, 0x8C, 0xA4, 0x49, 0x70, 0xB6, 0xF4, 0xC3, 0xE3, 0x0D, 0x23, 0x4D,
				0xC4, 0xB9,
				0x1A, 0xC8, 0xE2, 0x77, 0x1F, 0x7B, 0xA8, 0x7D, 0xF9, 0x44, 0xB7, 0xE6, 0xB1, 0x87,
				0xA0, 0xB4,
				0x0C, 0x01, 0xF3, 0x94, 0x66, 0xA6, 0x26, 0xEE, 0xFB, 0x25, 0xF0, 0x7E, 0x40, 0x4A,
				0xA1, 0x28,
				0xB8, 0x95, 0xAB, 0xB2, 0x65, 0x42, 0x1D, 0x3B, 0x92, 0x3D, 0xFE, 0x6B, 0x2A, 0x56,
				0x9A, 0x04,
				0xEC, 0xE8, 0x78, 0x15, 0xE9, 0xD1, 0x2D, 0x62, 0xC1, 0x72, 0x4E, 0x13, 0xCE, 0x0E,
				0x76, 0x7F,
				0x30, 0x4F, 0x93, 0x55, 0x1E, 0xCF, 0xDB, 0x36, 0x58, 0xEA, 0xBE, 0x7A, 0x5F, 0x43,
				0x8F, 0x6D,
				0x89, 0xD6, 0x91, 0x5D, 0x5C, 0x64, 0xF5, 0x00, 0xD8, 0xBA, 0x3C, 0x53, 0x69, 0x61,
				0xCC, 0x34,
		};

void initNoiseTable()
{
	int i;
	float a;
	float x, y, z, r, theta;
	float gradients[256 * 3];
	unsigned int *p, *psrc;

	srandom(0);

	// build gradient table for 3D noise
	for (i = 0; i < 256; i++)
	{
		/*
		* calculate 1 - 2 * random number
		*/
		a = (random() % 32768) / 32768.0f;
		z = (1.0f - 2.0f * a);

		r = sqrtf(1.0f - z * z); // r is radius of circle

		a = (random() % 32768) / 32768.0f;
		theta = (2.0f * (float) M_PI * a);
		x = (r * cosf(a));
		y = (r * sinf(a));

		gradients[i * 3] = x;
		gradients[i * 3 + 1] = y;
		gradients[i * 3 + 2] = z;
	}

	// use the index in the permutation table to load the
	// gradient values from gradients to gradientTable
	p = (unsigned int *) gradientTable;
	psrc = (unsigned int *) gradients;

	for (i = 0; i < 256; i++)
	{
		int indx = permTable[i];
		p[i * 3] = psrc[indx * 3];
		p[i * 3 + 1] = psrc[indx * 3 + 1];
		p[i * 3 + 2] = psrc[indx * 3 + 2];
	}
}

//
// generate the value of gradient noise for a given lattice point
//
// (ix, iy, iz) specifies the 3D lattice position
// (fx, fy, fz) specifies the fractional part
//
static float glattice3D(int ix, int iy, int iz, float fx, float fy, float fz)
{
	float *g;
	int indx, y, z;

	z = permTable[iz & NOISE_TABLE_MASK];
	y = permTable[(iy + z) & NOISE_TABLE_MASK];
	indx = (ix + y) & NOISE_TABLE_MASK;
	g = &gradientTable[indx * 3];

	return (g[0] * fx + g[1] * fy + g[2] * fz);
}

//
// generate the 3D noise value
// f describes the input (x, y, z) position for which the noise value needs to be computed
// noise3D returns the scalar noise value
//
float noise3D(float *f)
{
	int ix, iy, iz;
	float fx0, fx1, fy0, fy1, fz0, fz1;
	float wx, wy, wz;
	float vx0, vx1, vy0, vy1, vz0, vz1;

	ix = FLOOR (f[0]);
	fx0 = f[0] - ix;
	fx1 = fx0 - 1;
	wx = smoothstep (fx0);

	iy = FLOOR (f[1]);
	fy0 = f[1] - iy;
	fy1 = fy0 - 1;
	wy = smoothstep (fy0);

	iz = FLOOR (f[2]);
	fz0 = f[2] - iz;
	fz1 = fz0 - 1;
	wz = smoothstep (fz0);

	vx0 = glattice3D(ix, iy, iz, fx0, fy0, fz0);
	vx1 = glattice3D(ix + 1, iy, iz, fx1, fy0, fz0);
	vy0 = lerp (wx, vx0, vx1);
	vx0 = glattice3D(ix, iy + 1, iz, fx0, fy1, fz0);
	vx1 = glattice3D(ix + 1, iy + 1, iz, fx1, fy1, fz0);
	vy1 = lerp (wx, vx0, vx1);
	vz0 = lerp (wy, vy0, vy1);

	vx0 = glattice3D(ix, iy, iz + 1, fx0, fy0, fz1);
	vx1 = glattice3D(ix + 1, iy, iz + 1, fx1, fy0, fz1);
	vy0 = lerp (wx, vx0, vx1);
	vx0 = glattice3D(ix, iy + 1, iz + 1, fx0, fy1, fz1);
	vx1 = glattice3D(ix + 1, iy + 1, iz + 1, fx1, fy1, fz1);
	vy1 = lerp (wx, vx0, vx1);
	vz1 = lerp (wy, vy0, vy1);

	return lerp (wz, vz0, vz1);;
}

void Create3DNoiseTexture(GLuint &textureId)
{
	int textureSize = 64; // Size of the 3D nosie texture
	float frequency = 5.0f; // Frequency of the noise.
	GLfloat *texBuf = (GLfloat *) malloc(sizeof(GLfloat) * textureSize * textureSize * textureSize);
	GLubyte *texBufUbyte = (GLubyte *) malloc(
			sizeof(GLubyte) * textureSize * textureSize * textureSize);
	int x, y, z;
	int index = 0;
	float min = 1000;
	float max = -1000;
	float range;

	initNoiseTable();

	for (z = 0; z < textureSize; z++)
	{
		for (y = 0; y < textureSize; y++)
		{
			for (x = 0; x < textureSize; x++)
			{
				float noiseVal;
				float pos[3] = {
						(float) x / (float) textureSize, (float) y / (float) textureSize,
						(float) z / (float) textureSize
				};
				pos[0] *= frequency;
				pos[1] *= frequency;
				pos[2] *= frequency;
				noiseVal = noise3D(pos);

				if (noiseVal < min)
				{
					min = noiseVal;
				}

				if (noiseVal > max)
				{
					max = noiseVal;
				}

				texBuf[index++] = noiseVal;
			}
		}
	}

	// Normalize to the [0, 1] range
	range = (max - min);
	index = 0;

	for (z = 0; z < textureSize; z++)
	{
		for (y = 0; y < textureSize; y++)
		{
			for (x = 0; x < textureSize; x++)
			{
				float noiseVal = texBuf[index];
				noiseVal = (noiseVal - min) / range;
				texBufUbyte[index++] = (GLubyte) (noiseVal * 255.0f);
			}
		}
	}

	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_3D, textureId);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, textureSize, textureSize, textureSize, 0,
				 GL_RED, GL_UNSIGNED_BYTE, texBufUbyte);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_MIRRORED_REPEAT);

	glBindTexture(GL_TEXTURE_3D, 0);

	free(texBuf);
	free(texBufUbyte);
}

int GenCubeVertexData(float scale, GLfloat **vertices, GLfloat **normals,
					  GLfloat **texCoords, GLuint **indices)
{
	int i;
	int numVertices = 24;
	int numIndices = 36;

	GLfloat cubeVerts[] =
			{
					-0.5f, -0.5f, -0.5f,
					-0.5f, -0.5f, 0.5f,
					0.5f, -0.5f, 0.5f,
					0.5f, -0.5f, -0.5f,
					-0.5f, 0.5f, -0.5f,
					-0.5f, 0.5f, 0.5f,
					0.5f, 0.5f, 0.5f,
					0.5f, 0.5f, -0.5f,
					-0.5f, -0.5f, -0.5f,
					-0.5f, 0.5f, -0.5f,
					0.5f, 0.5f, -0.5f,
					0.5f, -0.5f, -0.5f,
					-0.5f, -0.5f, 0.5f,
					-0.5f, 0.5f, 0.5f,
					0.5f, 0.5f, 0.5f,
					0.5f, -0.5f, 0.5f,
					-0.5f, -0.5f, -0.5f,
					-0.5f, -0.5f, 0.5f,
					-0.5f, 0.5f, 0.5f,
					-0.5f, 0.5f, -0.5f,
					0.5f, -0.5f, -0.5f,
					0.5f, -0.5f, 0.5f,
					0.5f, 0.5f, 0.5f,
					0.5f, 0.5f, -0.5f,
			};

	GLfloat cubeNormals[] =
			{
					0.0f, -1.0f, 0.0f,
					0.0f, -1.0f, 0.0f,
					0.0f, -1.0f, 0.0f,
					0.0f, -1.0f, 0.0f,
					0.0f, 1.0f, 0.0f,
					0.0f, 1.0f, 0.0f,
					0.0f, 1.0f, 0.0f,
					0.0f, 1.0f, 0.0f,
					0.0f, 0.0f, -1.0f,
					0.0f, 0.0f, -1.0f,
					0.0f, 0.0f, -1.0f,
					0.0f, 0.0f, -1.0f,
					0.0f, 0.0f, 1.0f,
					0.0f, 0.0f, 1.0f,
					0.0f, 0.0f, 1.0f,
					0.0f, 0.0f, 1.0f,
					-1.0f, 0.0f, 0.0f,
					-1.0f, 0.0f, 0.0f,
					-1.0f, 0.0f, 0.0f,
					-1.0f, 0.0f, 0.0f,
					1.0f, 0.0f, 0.0f,
					1.0f, 0.0f, 0.0f,
					1.0f, 0.0f, 0.0f,
					1.0f, 0.0f, 0.0f,
			};

	GLfloat cubeTex[] =
			{
					0.0f, 0.0f,
					0.0f, 1.0f,
					1.0f, 1.0f,
					1.0f, 0.0f,
					1.0f, 0.0f,
					1.0f, 1.0f,
					0.0f, 1.0f,
					0.0f, 0.0f,
					0.0f, 0.0f,
					0.0f, 1.0f,
					1.0f, 1.0f,
					1.0f, 0.0f,
					0.0f, 0.0f,
					0.0f, 1.0f,
					1.0f, 1.0f,
					1.0f, 0.0f,
					0.0f, 0.0f,
					0.0f, 1.0f,
					1.0f, 1.0f,
					1.0f, 0.0f,
					0.0f, 0.0f,
					0.0f, 1.0f,
					1.0f, 1.0f,
					1.0f, 0.0f,
			};

	// Allocate memory for buffers
	if (vertices != NULL)
	{
		*vertices = static_cast<GLfloat *>(malloc(sizeof(GLfloat) * 3 * numVertices));
		memcpy(*vertices, cubeVerts, sizeof(cubeVerts));

		for (i = 0; i < numVertices * 3; i++)
		{
			(*vertices)[i] *= scale;
		}
	}

	if (normals != NULL)
	{
		*normals = static_cast<GLfloat *>(malloc(sizeof(GLfloat) * 3 * numVertices));
		memcpy(*normals, cubeNormals, sizeof(cubeNormals));
	}

	if (texCoords != NULL)
	{
		*texCoords = static_cast<GLfloat *>(malloc(sizeof(GLfloat) * 2 * numVertices));
		memcpy(*texCoords, cubeTex, sizeof(cubeTex));
	}


	// Generate the indices
	if (indices != NULL)
	{
		GLuint cubeIndices[] =
				{
						0, 2, 1,
						0, 3, 2,
						4, 5, 6,
						4, 6, 7,
						8, 9, 10,
						8, 10, 11,
						12, 15, 14,
						12, 14, 13,
						16, 17, 18,
						16, 18, 19,
						20, 23, 22,
						20, 22, 21
				};

		*indices = static_cast<GLuint *>(malloc(sizeof(GLuint) * numIndices));
		memcpy(*indices, cubeIndices, sizeof(cubeIndices));
	}

	return numIndices;
}

Noise3DSample::Noise3DSample()
{

	m_SamplerLoc = GL_NONE;
	m_MVPMatLoc = GL_NONE;
	m_ModelMatrixLoc = GL_NONE;
	m_FogMinDistLoc = GL_NONE;

	m_TextureId = GL_NONE;

	m_AngleX = 0;
	m_AngleY = 0;

	m_ScaleX = 1.0f;
	m_ScaleY = 1.0f;

	m_ModelMatrix = glm::mat4(0.0f);
}

Noise3DSample::~Noise3DSample()
{
	NativeImageUtil::FreeNativeImage(&m_RenderImage);

	if (m_VertexData.pIndices)
	{
		free(m_VertexData.pIndices);
	}

	if (m_VertexData.pTexCoords)
	{
		free(m_VertexData.pTexCoords);
	}

	if (m_VertexData.pVertices)
	{
		free(m_VertexData.pVertices);
	}

}

void Noise3DSample::Init()
{
	if (m_ProgramObj)
	{
		return;
	}

	char vShaderStr[] =
			"#version 300 es\n"
			"uniform mat4 u_mvpMatrix;\n"
			"uniform mat4 u_mvMatrix;\n"
			"layout(location = 0) in vec4 a_position;\n"
			"layout(location = 1) in vec4 a_color;\n"
			"layout(location = 2) in vec2 a_texCoord;\n"
			"out vec4 v_color;\n"
			"out vec2 v_texCoord;\n"
			"out vec4 v_eyePos;\n"
			"void main()\n"
			"{\n"
			"   // Transform vertex to view-space\n"
			"   v_eyePos = u_mvMatrix * a_position;\n"
			"   v_color = a_color;\n"
			"   v_texCoord = a_texCoord;\n"
			"   gl_Position = u_mvpMatrix * a_position;\n"
			"}";

	char fShaderStr[] =
			"#version 300 es\n"
			"precision mediump float;\n"
			"uniform mediump sampler3D s_noiseTex;\n"
			"uniform float u_fogMaxDist;\n"
			"uniform float u_fogMinDist;\n"
			"uniform vec4  u_fogColor;\n"
			"uniform float u_time;\n"
			"in vec4 v_color;\n"
			"in vec2 v_texCoord;\n"
			"in vec4 v_eyePos;\n"
			"layout(location = 0) out vec4 outColor;\n"
			"\n"
			"float computeLinearFogFactor()\n"
			"{\n"
			"  float factor;\n"
			"  // Compute linear fog equation\n"
			"  float dist = distance( v_eyePos,\n"
			"                 vec4( 0.0, 0.0, 0.0, 1.0 ) );\n"
			"  factor = (u_fogMaxDist - dist) /\n"
			"           (u_fogMaxDist - u_fogMinDist );\n"
			"  // Clamp in the [0,1] range\n"
			"  factor = clamp( factor, 0.0, 1.0 );\n"
			"  return factor;\n"
			"}\n"
			"\n"
			"void main( void )\n"
			"{\n"
			"  float fogFactor = computeLinearFogFactor();\n"
			"  vec3 noiseCoord =vec3( v_texCoord.xy - u_time, u_time );\n"
			"  fogFactor -=\n"
			"     texture(s_noiseTex, noiseCoord).r * 0.25;\n"
			"  fogFactor = clamp(fogFactor, 0.0, 1.0);\n"
			"  vec4 baseColor = v_color;\n"
			"  outColor = baseColor * fogFactor +\n"
			"            u_fogColor * (1.0 - fogFactor);\n"
			"}";

	m_ProgramObj = GLUtils::CreateProgram(vShaderStr, fShaderStr, m_VertexShader, m_FragmentShader);
	if (m_ProgramObj)
	{
		m_SamplerLoc = glGetUniformLocation(m_ProgramObj, "s_noiseTex");
		m_MVPMatLoc = glGetUniformLocation(m_ProgramObj, "u_mvpMatrix");
		m_ModelMatrixLoc = glGetUniformLocation(m_ProgramObj, "u_mvMatrix");
		m_FogMinDistLoc = glGetUniformLocation(m_ProgramObj, "u_fogMinDist");
		m_FogMaxDistLoc = glGetUniformLocation(m_ProgramObj, "u_fogMaxDist");
		m_TimeLoc = glGetUniformLocation(m_ProgramObj, "u_time");
		m_FogColorLoc = glGetUniformLocation(m_ProgramObj, "u_fogColor");
	}
	else
	{
		LOGCATE("Noise3DSample::Init create program fail");
		return;
	}

	Create3DNoiseTexture(m_TextureId);

	m_VertexData.indicesNum = GenCubeVertexData(3.0, &m_VertexData.pVertices, nullptr,
												&m_VertexData.pTexCoords, &m_VertexData.pIndices);
}

void Noise3DSample::LoadImage(NativeImage *pImage)
{
	LOGCATE("Noise3DSample::LoadImage pImage = %p", pImage->ppPlane[0]);
	if (pImage)
	{
		m_RenderImage.width = pImage->width;
		m_RenderImage.height = pImage->height;
		m_RenderImage.format = pImage->format;
		NativeImageUtil::CopyNativeImage(pImage, &m_RenderImage);
	}

}

void Noise3DSample::Draw(int screenW, int screenH)
{
	LOGCATE("Noise3DSample::Draw()");

	if (m_ProgramObj == GL_NONE || m_TextureId == GL_NONE) return;

	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glDisable(GL_BLEND);

	UpdateMVPMatrix(m_MVPMatrix, m_AngleX, m_AngleY, (float) screenW / screenH);

	m_CurTime += (random() % 4000) / 1000.0f;

	// Use the program object
	glUseProgram(m_ProgramObj);

	glEnableVertexAttribArray(ATTRIB_LOCATION_POS);
	glVertexAttribPointer(ATTRIB_LOCATION_POS, 3, GL_FLOAT,
						  GL_FALSE, 3 * sizeof(GLfloat), m_VertexData.pVertices);

	glVertexAttrib4f(ATTRIB_LOCATION_COLOR, 1.0f, 0.0f, 0.0f, 1.0f);

	glEnableVertexAttribArray(ATTRIB_LOCATION_TEXCOORD);
	glVertexAttribPointer(ATTRIB_LOCATION_TEXCOORD, 2, GL_FLOAT,
						  GL_FALSE, 2 * sizeof(GLfloat), m_VertexData.pTexCoords);

	glUniformMatrix4fv(m_MVPMatLoc, 1, GL_FALSE, &m_MVPMatrix[0][0]);
	glUniformMatrix4fv(m_ModelMatrixLoc, 1, GL_FALSE, &m_ModelMatrix[0][0]);

	float fogColor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
	float fogMinDist = 1.22f;
	float fogMaxDist = 1.88f;
	glUniform1f(m_FogMinDistLoc, fogMinDist);
	glUniform1f(m_FogMaxDistLoc, fogMaxDist);

	glUniform4fv(m_FogColorLoc, 1, fogColor);
	glUniform1f(m_TimeLoc, m_CurTime * 0.001f);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, m_TextureId);
	glUniform1i(m_SamplerLoc, 0);

	glDrawElements(GL_TRIANGLES, m_VertexData.indicesNum, GL_UNSIGNED_INT, m_VertexData.pIndices);
	GO_CHECK_GL_ERROR();
}

void Noise3DSample::Destroy()
{
	if (m_ProgramObj)
	{
		glDeleteProgram(m_ProgramObj);
		glDeleteTextures(1, &m_TextureId);

		m_ProgramObj = GL_NONE;
		m_TextureId = GL_NONE;
	}

}


/**
 * @param angleX 绕X轴旋转度数
 * @param angleY 绕Y轴旋转度数
 * @param ratio 宽高比
 * */
void Noise3DSample::UpdateMVPMatrix(glm::mat4 &mvpMatrix, int angleX, int angleY, float ratio)
{
	LOGCATE("Noise3DSample::UpdateMVPMatrix angleX = %d, angleY = %d, ratio = %f", angleX,
			angleY, ratio);
	angleX = angleX % 360;
	angleY = angleY % 360;

	//转化为弧度角
	float radiansX = static_cast<float>(MATH_PI / 180.0f * angleX);
	float radiansY = static_cast<float>(MATH_PI / 180.0f * angleY);


	// Projection matrix
	//glm::mat4 Projection = glm::ortho(-ratio, ratio, -1.0f, 1.0f, 0.0f, 100.0f);
	//glm::mat4 Projection = glm::frustum(-ratio, ratio, -1.0f, 1.0f, 4.0f, 100.0f);
	glm::mat4 Projection = glm::perspective(45.0f, ratio, 0.1f, 100.f);

	// View matrix
	glm::mat4 View = glm::lookAt(
			glm::vec3(0, 0, 4), // Camera is at (0,0,1), in World Space
			glm::vec3(0, 0, 0), // and looks at the origin
			glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
	);

	// Model matrix
	glm::mat4 Model = glm::mat4(1.0f);
	Model = glm::scale(Model, glm::vec3(1.0f, 1.0f, 1.0f));
	Model = glm::rotate(Model, radiansX, glm::vec3(1.0f, 0.0f, 0.0f));
	Model = glm::rotate(Model, radiansY, glm::vec3(0.0f, 1.0f, 0.0f));
	Model = glm::translate(Model, glm::vec3(0.0f, 0.0f, 0.0f));

	m_ModelMatrix = Model;

	mvpMatrix = Projection * View * Model;

}

void Noise3DSample::UpdateTransformMatrix(float rotateX, float rotateY, float scaleX, float scaleY)
{
	GLSampleBase::UpdateTransformMatrix(rotateX, rotateY, scaleX, scaleY);
	//m_AngleX = static_cast<int>(rotateX);
	//m_AngleY = static_cast<int>(rotateY);
	m_ScaleX = scaleX;
	m_ScaleY = scaleY;
}
