//
// Created by ByteFlow on 2020/4/30.
//

#include <gtc/matrix_transform.hpp>
#include <thread>
#include "VisualizeAudioSample.h"
#include "../util/GLUtils.h"

VisualizeAudioSample::VisualizeAudioSample() {

    m_SamplerLoc = GL_NONE;
    m_MVPMatLoc = GL_NONE;

    m_VaoId = GL_NONE;

    m_AngleX = 0;
    m_AngleY = 0;

    m_ScaleX = 1.0f;
    m_ScaleY = 1.0f;

    m_pCurAudioData = nullptr;
    m_AudioDataSize = 0;

    m_pTextureCoords = nullptr;
    m_pVerticesCoords = nullptr;

    memset(m_VboIds, 0, sizeof(GLuint) * 2);

    m_FrameIndex = 0;
    m_pAudioBuffer = nullptr;
    m_bAudioDataReady = false;
}

VisualizeAudioSample::~VisualizeAudioSample() {
    if (m_pAudioBuffer != nullptr) {
        delete [] m_pAudioBuffer;
        m_pAudioBuffer = nullptr;
    }

    if (m_pTextureCoords != nullptr) {
        delete [] m_pTextureCoords;
        m_pTextureCoords = nullptr;
    }

    if (m_pVerticesCoords != nullptr) {
        delete [] m_pVerticesCoords;
        m_pVerticesCoords = nullptr;
    }


}

void VisualizeAudioSample::Init() {
    if (m_ProgramObj)
        return;
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
            "    gl_PointSize = 4.0f;\n"
            "}";

    char fShaderStr[] =
            "#version 300 es                                     \n"
            "precision mediump float;                            \n"
            "in vec2 v_texCoord;                                 \n"
            "layout(location = 0) out vec4 outColor;             \n"
            "uniform float drawType;                             \n"
            "void main()                                         \n"
            "{                                                   \n"
            "  if(drawType == 1.0)                               \n"
            "  {                                                 \n"
            "      outColor = vec4(1.5 - v_texCoord.y, 0.3, 0.3, 1.0); \n"
            "  }                                                 \n"
            "  else                                              \n"
            "  {                                                 \n"
            "      outColor = vec4(1.0, 1.0, 1.0, 1.0);          \n"
            "  }                                                 \n"
            "}                                                   \n";

    m_ProgramObj = GLUtils::CreateProgram(vShaderStr, fShaderStr, m_VertexShader, m_FragmentShader);
    if (m_ProgramObj) {
        m_MVPMatLoc = glGetUniformLocation(m_ProgramObj, "u_MVPMatrix");
    } else {
        LOGCATE("VisualizeAudioSample::Init create program fail");
    }

    //upload RGBA image data
//    glActiveTexture(GL_TEXTURE0);
//    glBindTexture(GL_TEXTURE_2D, m_TextureId);
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_RenderImage.width, m_RenderImage.height, 0, GL_RGBA,
//                 GL_UNSIGNED_BYTE, m_RenderImage.ppPlane[0]);
//    glBindTexture(GL_TEXTURE_2D, GL_NONE);

}

void VisualizeAudioSample::LoadImage(NativeImage *pImage) {
    LOGCATE("VisualizeAudioSample::LoadImage pImage = %p", pImage->ppPlane[0]);

}


void VisualizeAudioSample::Draw(int screenW, int screenH) {
    LOGCATE("VisualizeAudioSample::Draw()");
    glClearColor(1.0f, 1.0f, 1.0f, 1.0);
    if (m_ProgramObj == GL_NONE) return;
    std::this_thread::sleep_for(std::chrono::milliseconds(25));
    std::unique_lock<std::mutex> lock(m_Mutex);
    if (!m_bAudioDataReady) return;
    UpdateMesh();
    lock.unlock();

    UpdateMVPMatrix(m_MVPMatrix, m_AngleX, m_AngleY, (float) screenW / screenH);

    // Generate VBO Ids and load the VBOs with data
    if(m_VboIds[0] == 0)
    {
        glGenBuffers(2, m_VboIds);

        glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * m_RenderDataSize * 6 * 3, m_pVerticesCoords, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[1]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * m_RenderDataSize * 6 * 2, m_pTextureCoords, GL_DYNAMIC_DRAW);
    }
    else
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[0]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * m_RenderDataSize * 6 * 3, m_pVerticesCoords);

        glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[1]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * m_RenderDataSize * 6 * 2, m_pTextureCoords);
    }

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
    GLUtils::setFloat(m_ProgramObj, "drawType", 1.0f);
    glDrawArrays(GL_TRIANGLES, 0, m_RenderDataSize * 6);
    GLUtils::setFloat(m_ProgramObj, "drawType", 0.0f);
    glDrawArrays(GL_LINES, 0, m_RenderDataSize * 6);


}

void VisualizeAudioSample::Destroy() {
    std::unique_lock<std::mutex> lock(m_Mutex);
    m_Cond.notify_all();
    lock.unlock();

    if (m_ProgramObj) {
        glDeleteProgram(m_ProgramObj);
        glDeleteBuffers(2, m_VboIds);
        glDeleteVertexArrays(1, &m_VaoId);
    }

}

void VisualizeAudioSample::UpdateMVPMatrix(glm::mat4 &mvpMatrix, int angleX, int angleY, float ratio) {
    LOGCATE("VisualizeAudioSample::UpdateMVPMatrix angleX = %d, angleY = %d, ratio = %f", angleX,
            angleY, ratio);
    angleX = angleX % 360;
    angleY = angleY % 360;

    //转化为弧度角
    float radiansX = static_cast<float>(MATH_PI / 180.0f * angleX);
    float radiansY = static_cast<float>(MATH_PI / 180.0f * angleY);


    // Projection matrix
    glm::mat4 Projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 100.0f);
    //glm::mat4 Projection = glm::frustum(-ratio, ratio, -1.0f, 1.0f, 4.0f, 100.0f);
    //glm::mat4 Projection = glm::perspective(45.0f, ratio, 0.1f, 100.f);

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

    float radiansZ = static_cast<float>(MATH_PI / 2.0f);

    //Model = glm::rotate(Model, radiansZ, glm::vec3(0.0f, 0.0f, 1.0f));
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
    m_FrameIndex++;
    std::unique_lock<std::mutex> lock(m_Mutex);
    if(m_FrameIndex == 1)
    {
        m_pAudioBuffer = new short[arrSize * 2];
        memcpy(m_pAudioBuffer, pShortArr, sizeof(short) * arrSize);
        m_AudioDataSize = arrSize;
        return;
    }

    if(m_FrameIndex == 2)
    {
        memcpy(m_pAudioBuffer + arrSize, pShortArr, sizeof(short) * arrSize);
        m_RenderDataSize = m_AudioDataSize / RESAMPLE_LEVEL;
        m_pVerticesCoords = new vec3[m_RenderDataSize * 6]; //(x,y,z) * 6 points
        m_pTextureCoords = new vec2[m_RenderDataSize * 6]; //(x,y) * 6 points
    }

    if(m_FrameIndex > 2)
    {
        memcpy(m_pAudioBuffer, m_pAudioBuffer + arrSize, sizeof(short) * arrSize);
        memcpy(m_pAudioBuffer + arrSize, pShortArr, sizeof(short) * arrSize);
    }
    LOGCATE("VisualizeAudioSample::Draw() m_bAudioDataReady = true");
    m_bAudioDataReady = true;
    m_pCurAudioData = m_pAudioBuffer;
    m_Cond.wait(lock);

}

void VisualizeAudioSample::UpdateMesh() {
    //calculate mesh
    int step = m_AudioDataSize / 64;
    if(m_pAudioBuffer + m_AudioDataSize - m_pCurAudioData >= step)
    {
        LOGCATE("VisualizeAudioSample::Draw() m_pAudioBuffer + m_AudioDataSize - m_pCurAudioData >= step");
        float dy = 0.5f / MAX_AUDIO_LEVEL;
        float dx = 1.0f / m_RenderDataSize;
        for (int i = 0; i < m_RenderDataSize; ++i) {
            int index = i * RESAMPLE_LEVEL;
            float y = m_pCurAudioData[index] * dy * -1;
            y = y < 0 ? y : -y;
            vec2 p1(i * dx, 0 + 1.0f);
            vec2 p2(i * dx, y + 1.0f);
            vec2 p3((i + 1) * dx, y + 1.0f);
            vec2 p4((i + 1) * dx, 0 + 1.0f);

            m_pTextureCoords[i * 6 + 0] = p1;
            m_pTextureCoords[i * 6 + 1] = p2;
            m_pTextureCoords[i * 6 + 2] = p4;
            m_pTextureCoords[i * 6 + 3] = p4;
            m_pTextureCoords[i * 6 + 4] = p2;
            m_pTextureCoords[i * 6 + 5] = p3;

            m_pVerticesCoords[i * 6 + 0] = GLUtils::texCoordToVertexCoord(p1);
            m_pVerticesCoords[i * 6 + 1] = GLUtils::texCoordToVertexCoord(p2);
            m_pVerticesCoords[i * 6 + 2] = GLUtils::texCoordToVertexCoord(p4);
            m_pVerticesCoords[i * 6 + 3] = GLUtils::texCoordToVertexCoord(p4);
            m_pVerticesCoords[i * 6 + 4] = GLUtils::texCoordToVertexCoord(p2);
            m_pVerticesCoords[i * 6 + 5] = GLUtils::texCoordToVertexCoord(p3);
        }
        m_pCurAudioData += step;
    }
    else
    {
        LOGCATE("VisualizeAudioSample::Draw() m_pAudioBuffer + m_AudioDataSize - m_pCurAudioData < step");
        m_bAudioDataReady = false;
        m_Cond.notify_all();
        return;
    }
}
