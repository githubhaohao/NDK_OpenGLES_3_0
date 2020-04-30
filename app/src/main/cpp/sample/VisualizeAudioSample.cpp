//
// Created by ByteFlow on 2020/4/30.
//

#include <gtc/matrix_transform.hpp>
#include "VisualizeAudioSample.h"
#include "../util/GLUtils.h"

VisualizeAudioSample::VisualizeAudioSample() {

    m_SamplerLoc = GL_NONE;
    m_MVPMatLoc = GL_NONE;

    m_TextureId = GL_NONE;
    m_VaoId = GL_NONE;

    m_AngleX = 0;
    m_AngleY = 0;

    m_ScaleX = 1.0f;
    m_ScaleY = 1.0f;

    m_pAudioData = nullptr;
    m_AudioDataSize = 0;

    m_pTextureCoords = nullptr;
    m_pVerticesCoords = nullptr;

    memset(m_VboIds, 0, sizeof(GLuint) * 2);
}

VisualizeAudioSample::~VisualizeAudioSample() {
    NativeImageUtil::FreeNativeImage(&m_RenderImage);

    if (m_pAudioData != nullptr) {
        free(m_pAudioData);
        m_pAudioData = nullptr;
    }

    if (m_pTextureCoords != nullptr) {
        free(m_pTextureCoords);
        m_pTextureCoords = nullptr;
    }

    if (m_pVerticesCoords != nullptr) {
        free(m_pVerticesCoords);
        m_pVerticesCoords = nullptr;
    }


}

void VisualizeAudioSample::Init() {
    if (m_ProgramObj)
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
            "layout(location = 1) in vec2 a_texCoord;\n"
            "uniform mat4 u_MVPMatrix;\n"
            "out vec2 v_texCoord;\n"
            "void main()\n"
            "{\n"
            "    gl_Position = u_MVPMatrix * a_position;\n"
            "    v_texCoord = a_texCoord;\n"
            "}";

    char fShaderStr[] =
            "#version 300 es                                     \n"
            "precision mediump float;                            \n"
            "in vec2 v_texCoord;                                 \n"
            "layout(location = 0) out vec4 outColor;             \n"
            "uniform sampler2D s_TextureMap;                     \n"
            "void main()                                         \n"
            "{                                                   \n"
            "  outColor = texture(s_TextureMap, v_texCoord);     \n"
            "}                                                   \n";

    m_ProgramObj = GLUtils::CreateProgram(vShaderStr, fShaderStr, m_VertexShader, m_FragmentShader);
    if (m_ProgramObj) {
        m_SamplerLoc = glGetUniformLocation(m_ProgramObj, "s_TextureMap");
        m_MVPMatLoc = glGetUniformLocation(m_ProgramObj, "u_MVPMatrix");
    } else {
        LOGCATE("VisualizeAudioSample::Init create program fail");
    }

    //upload RGBA image data
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_TextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_RenderImage.width, m_RenderImage.height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, m_RenderImage.ppPlane[0]);
    glBindTexture(GL_TEXTURE_2D, GL_NONE);

}

void VisualizeAudioSample::LoadImage(NativeImage *pImage) {
    LOGCATE("VisualizeAudioSample::LoadImage pImage = %p", pImage->ppPlane[0]);
    if (pImage) {
        m_RenderImage.width = pImage->width;
        m_RenderImage.height = pImage->height;
        m_RenderImage.format = pImage->format;
        NativeImageUtil::CopyNativeImage(pImage, &m_RenderImage);
    }

}


void VisualizeAudioSample::Draw(int screenW, int screenH) {
    LOGCATE("VisualizeAudioSample::Draw()");

    if (m_ProgramObj == GL_NONE || m_TextureId == GL_NONE) return;

    if (m_pVerticesCoords == nullptr || m_pTextureCoords == nullptr) return;

    UpdateMVPMatrix(m_MVPMatrix, m_AngleX, m_AngleY, (float) screenW / screenH);


    std::unique_lock<std::mutex> lock(m_Mutex);
    // Generate VBO Ids and load the VBOs with data
    if(m_VboIds[0] == 0)
    {
        glGenBuffers(2, m_VboIds);
    }
    glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * m_AudioDataSize * 6 * 3, m_pVerticesCoords, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * m_AudioDataSize * 6 * 2, m_pTextureCoords, GL_STATIC_DRAW);
    lock.unlock();

    if(m_VaoId == GL_NONE)
    {
        glGenVertexArrays(1, &m_VaoId);
        glBindVertexArray(m_VaoId);

        glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[0]);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (const void *) 0);
        glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);

        glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[1]);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (const void *) 0);
        glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);

        glBindVertexArray(GL_NONE);
    }


    // Use the program object
    glUseProgram(m_ProgramObj);

    glBindVertexArray(m_VaoId);

    glUniformMatrix4fv(m_MVPMatLoc, 1, GL_FALSE, &m_MVPMatrix[0][0]);

    // Bind the RGBA map
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_TextureId);
    glUniform1i(m_SamplerLoc, 0);

    glDrawArrays(GL_TRIANGLES, 0, m_AudioDataSize * 6);

}

void VisualizeAudioSample::Destroy() {
    if (m_ProgramObj) {
        glDeleteProgram(m_ProgramObj);
        glDeleteBuffers(2, m_VboIds);
        glDeleteVertexArrays(1, &m_VaoId);
        glDeleteTextures(1, &m_TextureId);
    }

}


/**
 * @param angleX 绕X轴旋转度数
 * @param angleY 绕Y轴旋转度数
 * @param ratio 宽高比
 * */
void
VisualizeAudioSample::UpdateMVPMatrix(glm::mat4 &mvpMatrix, int angleX, int angleY, float ratio) {
    LOGCATE("VisualizeAudioSample::UpdateMVPMatrix angleX = %d, angleY = %d, ratio = %f", angleX,
            angleY, ratio);
    angleX = angleX % 360;
    angleY = angleY % 360;

    //转化为弧度角
    float radiansX = static_cast<float>(MATH_PI / 180.0f * angleX);
    float radiansY = static_cast<float>(MATH_PI / 180.0f * angleY);


    // Projection matrix
    //glm::mat4 Projection = glm::ortho(-ratio, ratio, -1.0f, 1.0f, 0.1f, 100.0f);
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
    Model = glm::scale(Model, glm::vec3(m_ScaleX, m_ScaleY, 1.0f));
    Model = glm::rotate(Model, radiansX, glm::vec3(1.0f, 0.0f, 0.0f));
    Model = glm::rotate(Model, radiansY, glm::vec3(0.0f, 1.0f, 0.0f));
    Model = glm::translate(Model, glm::vec3(0.0f, 0.0f, 0.0f));

    mvpMatrix = Projection * View * Model;

}

void VisualizeAudioSample::UpdateTransformMatrix(float rotateX, float rotateY, float scaleX,
                                                 float scaleY) {
    GLSampleBase::UpdateTransformMatrix(rotateX, rotateY, scaleX, scaleY);
    m_AngleX = static_cast<int>(rotateX);
    m_AngleY = static_cast<int>(rotateY);
    m_ScaleX = scaleX;
    m_ScaleY = scaleY;
}

void VisualizeAudioSample::LoadShortArrData(short *const pShortArr, int arrSize) {
    //GLSampleBase::LoadShortArrData(pShortArr, arrSize);
    LOGCATE("VisualizeAudioSample::LoadShortArrData pShortArr=%p, arrSize=%d", pShortArr, arrSize);
    if (pShortArr == nullptr || arrSize == 0)
        return;

    std::unique_lock<std::mutex> lock(m_Mutex);
    if (m_pAudioData == nullptr) {
        m_AudioDataSize = arrSize;
        m_pAudioData = new short[m_AudioDataSize];

        m_pVerticesCoords = new vec3[m_AudioDataSize * 6]; //(x,y,z) * 6 points
        m_pTextureCoords = new vec2[m_AudioDataSize * 6]; //(x,y) * 6 points

    }
    memcpy(m_pAudioData, pShortArr, sizeof(short) * m_AudioDataSize);

    float dy = 0.5f / MAX_AUDIO_LEVEL;
    float dx = 1.0f / m_AudioDataSize;
    for (int i = 0; i < m_AudioDataSize; ++i) {
        vec2 p1(i * dx, 0);
        vec2 p2(i * dx, m_pAudioData[i] * dy);
        vec2 p3((i + 1) * dx, m_pAudioData[i] * dy);
        vec2 p4((i + 1) * dx, 0);

        m_pTextureCoords[i * 6 + 0] = p1;
        m_pTextureCoords[i * 6 + 1] = p2;
        m_pTextureCoords[i * 6 + 2] = p3;
        m_pTextureCoords[i * 6 + 3] = p1;
        m_pTextureCoords[i * 6 + 4] = p3;
        m_pTextureCoords[i * 6 + 5] = p4;

        m_pVerticesCoords[i * 6 + 0] = GLUtils::texCoordToVertexCoord(p1);
        m_pVerticesCoords[i * 6 + 1] = GLUtils::texCoordToVertexCoord(p2);
        m_pVerticesCoords[i * 6 + 2] = GLUtils::texCoordToVertexCoord(p3);
        m_pVerticesCoords[i * 6 + 3] = GLUtils::texCoordToVertexCoord(p1);
        m_pVerticesCoords[i * 6 + 4] = GLUtils::texCoordToVertexCoord(p3);
        m_pVerticesCoords[i * 6 + 5] = GLUtils::texCoordToVertexCoord(p4);
    }

}
