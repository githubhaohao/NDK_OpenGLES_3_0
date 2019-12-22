#ifndef _BYTE_FLOW_GL_UTILS_H_
#define _BYTE_FLOW_GL_UTILS_H_

#include <GLES3/gl3.h>

class GLUtils
{
public:
	static GLuint LoadShader(GLenum shaderType, const char *pSource);

	static GLuint CreateProgram(const char *pVertexShaderSource, const char *pFragShaderSource, GLuint &vertexShaderHandle,
								GLuint &fragShaderHandle);

	static GLuint CreateProgram(const char *pVertexShaderSource, const char *pFragShaderSource);

	static GLuint CreateProgramWithFeedback(const char *pVertexShaderSource, const char *pFragShaderSource, GLuint &vertexShaderHandle,
											GLuint &fragShaderHandle, const GLchar **varying, int varyingCount);

	static void DeleteProgram(GLuint &program);

	static void CheckGLError(const char *pGLOperation);

};
#endif // _BYTE_FLOW_GL_UTILS_H_