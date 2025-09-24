//  This file is part of Snake3D by Or Avnat: https://github.com/oravnat/Snake3D_public
//
//  SnakeRenderer.cpp
//  Snake3D
//
//  Created by Or Avnat on 22/01/2025.
//

#include "SnakeRenderer.hpp"

#define _USE_MATH_DEFINES

#include "Vector.hpp"
#include "Matrix.hpp"
#include <math.h>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <iostream>
#include "OpenGL.hpp"

#if TARGET_OS_OSX
#define GLSL_VERSION "410"
#elif TARGET_OS_IOS
#define GLSL_VERSION "300 es"
#else
#define GLSL_VERSION "320 es"
#endif

static bool g_glewInit = false;
extern void SnakeLogErrorFmt(const char* format, ...);
extern void SnakeLogError(const char* text);

// Radar 2D shaders:

static const char RADAR_VERTEX_SHADER_ES2[] =
        "#version 120\n"
        "uniform mat4 m;\n"
        "uniform float point_size;\n"
        "attribute vec4 color;\n"
        "attribute vec4 pos;\n"
        "varying vec4 vColor;\n"
        "void main() {\n"
        "    gl_Position = m * pos;\n"
        "    gl_PointSize = point_size;\n"
        "    vColor = color;\n"
        "}\n";

// the screen coordinates are corresponded to x and y in gl_Position in the range (-1, 1)

static const char RADAR_FRAGMENT_SHADER_ES2[] =
        "#version 120\n"
        //"precision mediump float;\n"
        "varying vec4 vColor;\n"
        "void main() {\n"
        "    gl_FragColor = vColor;\n"
        "}\n";

// Main 3D shaders:

static const char VERTEX_SHADER_ES2[] =
        "#version 120\n"
        "attribute vec4 pos;\n"
        "attribute vec3    myNormal;\n"
        "attribute vec3    color;\n"
        "uniform mat4 myPMVMatrix;\n" // ProjectionModelviewMatrix
        //"uniform mediump mat4 normalMatrix;\n"
        "uniform mat4 modelMatrix; \n"
        //"uniform mediump mat3 myModelViewIT;\n"
        "uniform vec3 lightPos;\n" // light position in world coordinates
        "uniform vec3 lightColor;\n"
        "varying vec4 vColor;\n"
        "void main() {\n"
        "    gl_Position = myPMVMatrix * pos;\n"
        "    vec4 position = modelMatrix * pos;\n" // position in world coordinates
        "    mat3 m = mat3(modelMatrix);\n"
        "    vec3 transNormal = m * myNormal;\n"
        "    vec3 myLightDirection = normalize(position.xyz - lightPos);\n"
        "    float varDot = max( dot(transNormal, -myLightDirection), 0.0 );\n"
        "    vColor = vec4(varDot * lightColor * color, 1);\n"
        //"    vColor = vec4(color, 1);\n"
        //"    vColor = vec4(0, 1, 0, 1);\n"
        //"    vColor = vec4(myNormal, 1);\n"
        "}\n";

// the screen coordinates are corresponded to x and y in gl_Position in the range (-1, 1)

static const char FRAGMENT_SHADER_ES2[] =
        "#version 120\n"
        //"precision mediump float;\n"
        "varying vec4 vColor;\n"
        "void main() {\n"
        "    gl_FragColor = vColor;\n"
        "}\n";

static const char VERTEX_SHADER_TEXURE[] =
    "#version " GLSL_VERSION "\n"
    // Input vertex data, different for all executions of this shader.
    //"layout(location = 0) in vec3 vertexPosition_modelspace;\n"
    //"layout(location = 1) in vec2 vertexUV;\n"
    "in vec3 vertexPosition_modelspace;\n"
    "in vec2 vertexUV;\n"
    // Output data ; will be interpolated for each fragment.
    "out vec2 UV; \n"
    // Values that stay constant for the whole mesh.
    "uniform mat4 MVP; \n"
    "void main() { \n"
        // Output position of the vertex, in clip space : MVP * position
        "gl_Position = MVP * vec4(vertexPosition_modelspace, 1);\n"
        // UV of the vertex. No special space for this one.
        "UV = vertexUV;\n"
    "}\n";

static const char FRAGMENT_SHADER_TEXTURE[] =
    "#version " GLSL_VERSION "\n"
    // Interpolated values from the vertex shaders
    "in lowp vec2 UV;\n"
    // Ouput data
    "out lowp vec4 color;\n"
    // Values that stay constant for the whole mesh.
    "uniform sampler2D myTextureSampler;\n"
    "void main() {\n"
        // Output color = color of the texture at the specified UV
        "color = texture(myTextureSampler, UV).rgba; \n"
        //"color.r = texture(myTextureSampler, UV).r; \n"
        //"color.g = texture(myTextureSampler, UV).r; \n"
        //"color.b = texture(myTextureSampler, UV).r; \n"
        //"color.a = texture(myTextureSampler, UV).g; \n"
    //"color = vec3(1, 0, 0); \n"
    "}\n";

// The version statement has come on first line.
static const char* VertexShaderGlsl =
    "#version " GLSL_VERSION "\n"
    R"_(
    in vec4 pos;
    in vec3    myNormal;
    in vec3 color;

    out vec4 PSVertexColor;

    uniform mat4 myPMVMatrix;
    uniform mat4 modelMatrix;
    uniform vec3 lightPos;
    uniform vec3 lightColor;
    uniform vec3 ambientLightColor;

    void main() {
       gl_Position = myPMVMatrix * pos;
       vec4 position = modelMatrix * pos;
       mat3 m = mat3(modelMatrix);
       //mat3 m = mat3(transpose(inverse(modelMatrix)));
       vec3 transNormal = m * myNormal;
       vec3 myLightDirection = normalize(position.xyz - lightPos);
       float varDot = max( dot(transNormal, -myLightDirection), 0.0 );
       PSVertexColor = vec4((varDot * lightColor + ambientLightColor) * color, 1);
    }
    )_";

// The version statement has come on first line.
static const char* FragmentShaderGlsl =
    "#version " GLSL_VERSION "\n"
    R"_(
    in lowp vec4 PSVertexColor;
    out lowp vec4 FragColor;

    void main() {
       FragColor = PSVertexColor;
    }
    )_";

bool checkGlError(const char* funcName) {
    GLint err = glGetError();
    if (err != GL_NO_ERROR) {
        SnakeLogErrorFmt("GL error after %s(): 0x%08x\n", funcName, err);
        return true;
    }
    return false;
}

GLuint createShader(GLenum shaderType, const char* src) {
    GLuint shader = glCreateShader(shaderType);
    if (!shader) {
        checkGlError("glCreateShader");
        return 0;
    }
    glShaderSource(shader, 1, &src, NULL);

    GLint compiled = GL_FALSE;
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        GLint infoLogLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLen);
        if (infoLogLen > 0) {
            GLchar* infoLog = (GLchar*)malloc(infoLogLen);
            if (infoLog) {
                glGetShaderInfoLog(shader, infoLogLen, NULL, infoLog);
                SnakeLogErrorFmt("Could not compile %s shader:\n%s\n",
                    shaderType == GL_VERTEX_SHADER ? "vertex" : "fragment",
                    infoLog);
                free(infoLog);
            }
        }
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

GLuint createProgram(const char* vtxSrc, const char* fragSrc) {
    GLuint vtxShader = 0;
    GLuint fragShader = 0;
    GLuint program = 0;
    GLint linked = GL_FALSE;

    vtxShader = createShader(GL_VERTEX_SHADER, vtxSrc);
    if (!vtxShader)
        goto exit;

    fragShader = createShader(GL_FRAGMENT_SHADER, fragSrc);
    if (!fragShader)
        goto exit;

    program = glCreateProgram();
    if (!program) {
        checkGlError("glCreateProgram");
        goto exit;
    }
    glAttachShader(program, vtxShader);
    glAttachShader(program, fragShader);

    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked) {
        SnakeLogError("Could not link program");
        GLint infoLogLen = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLen);
        if (infoLogLen) {
            GLchar* infoLog = (GLchar*)malloc(infoLogLen);
            if (infoLog) {
                glGetProgramInfoLog(program, infoLogLen, NULL, infoLog);
                SnakeLogErrorFmt("Could not link program:\n%s\n", infoLog);
                free(infoLog);
            }
        }
        glDeleteProgram(program);
        program = 0;
    }

exit:
    glDeleteShader(vtxShader);
    glDeleteShader(fragShader);
    return program;
}

bool InitOpenGL()
{
#if !defined(__ANDROID_API__) && !defined(__APPLE__)
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        /* Problem: glewInit failed, something is seriously wrong. */
        std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
        return false;
    }
    std::cout << "Status: Using GLEW " << glewGetString(GLEW_VERSION) << std::endl;
#endif
    return true;
}

Program::~Program()
{
    if (mProgram != 0)
        glDeleteProgram(mProgram);
}
bool Program::Create(const char* vtxSrc, const char* fragSrc)
{
    mProgram = createProgram(vtxSrc, fragSrc);
    if (!mProgram)
        return false;

    // glGetAttribLocation() returns -1 if it can't find the attribute:
    mPosAttrib = glGetAttribLocation(mProgram, "pos");
    mNormalAttrib = glGetAttribLocation(mProgram, "myNormal");
    mColorAttrib = glGetAttribLocation(mProgram, "color");
    // glGetUniformLocation() returns -1 if it can't find the uniform:
    mPMVMatrixUniform = glGetUniformLocation(mProgram, "myPMVMatrix");
    mModelMatrixUniform = glGetUniformLocation(mProgram, "modelMatrix");
    mLightPosUniform = glGetUniformLocation(mProgram, "lightPos");
    mLightColorUniform = glGetUniformLocation(mProgram, "lightColor");
    mAmbientlightColorUniform = glGetUniformLocation(mProgram, "ambientLightColor");

    return true;
}

bool Program::CreateTextureProgram(const char* vtxSrc, const char* fragSrc)
{
    mProgram = createProgram(vtxSrc, fragSrc);
    if (!mProgram)
        return false;

    mPosAttrib = glGetAttribLocation(mProgram, "vertexPosition_modelspace");
    mTextUVAttrib = glGetAttribLocation(mProgram, "vertexUV");
    mPMVMatrixUniform = glGetUniformLocation(mProgram, "MVP");
    mTextSamplerUniform = glGetUniformLocation(mProgram, "myTextureSampler");

    return true;
}

bool Program::CreateRadarProgram(const char* vtxSrc, const char* fragSrc)
{
    mProgram = createProgram(vtxSrc, fragSrc);
    if (!mProgram)
        return false;

    mPosAttrib = glGetAttribLocation(mProgram, "pos");
    mColorAttrib = glGetAttribLocation(mProgram, "color");
    //mColorUniform = glGetUniformLocation(mRadarProgram, "color");
    mPMVMatrixUniform = glGetUniformLocation(mProgram, "m");
    mRadarPointSizeUniform = glGetUniformLocation(mProgram, "point_size");

    return true;
}


bool SnakeRenderer::Init()
{
    if (!g_glewInit && !InitOpenGL())
        return false;
    g_glewInit = true;

    glGenVertexArrays(1, &mVBState);
    BindState();

    //if (!mRadarProgram.CreateRadarProgram(RADAR_VERTEX_SHADER_ES2, RADAR_FRAGMENT_SHADER_ES2))
        //return false;

    //mMainProgram = createProgram(VERTEX_SHADER_ES2, FRAGMENT_SHADER_ES2);
    //mMainProgram = createProgram(VertexShaderGlsl, FragmentShaderGlsl);
    if (!mMainProgram.Create(VertexShaderGlsl, FragmentShaderGlsl))
        return false;

    if (!mTextureProgram.CreateTextureProgram(VERTEX_SHADER_TEXURE, FRAGMENT_SHADER_TEXTURE))
        return false;

    glGenBuffers(1, &mVB);
    glBindBuffer(GL_ARRAY_BUFFER, mVB);
#if __APPLE__
    glGenBuffers(1, &mIndicesBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndicesBuffer);
#endif

    UseProgram(mMainProgram);
   
    return true;
}

void SnakeRenderer::UseProgram(Program& program)
{
    GLenum error = 0;
    m_pActiveProgram = &program;
    glUseProgram(m_pActiveProgram->mProgram);
    error = glGetError();
}

void SnakeRenderer::Start3DRendering()
{
    GLenum error = 0;
    UseProgram(mMainProgram);

    // specify the back of the buffer as clear depth (0 closest, 1 farmost)
    glClearDepth(1.0f);
    // enable depth testing (Enable zbuffer - hidden surface removal)
    glEnable( GL_DEPTH_TEST );
    // Set default background as dark blue color (skies).
    glClearColor(0.0f, 0.0f, 0.2f, 1.0f); // dark blue
    //glClearColor(0.0, 0.0, 1.0, 1.0f); // blue

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);        // clear screen and zbuffer
    error = glGetError();

    //glEnable(GL_CULL_FACE);
    glDisable(GL_CULL_FACE);
#if __APPLE__ && TARGET_OS_OSX
    glPrimitiveRestartIndex(0xFFFFFFFF);
    glEnable(GL_PRIMITIVE_RESTART);
#else
    glEnable(GL_PRIMITIVE_RESTART_FIXED_INDEX);
#endif

    LoadIdentity();
    glFrontFace(GL_CCW); // GL_CCW is also the default
    SetProjectionMatrix(Matrix::I());
    m_bShadow = false;
}

void SnakeRenderer::Start2DRendering()
{
    UseProgram(mRadarProgram);

    glDisable(GL_DEPTH_TEST);
    Matrix m = Matrix::I();
    SetProjectionMatrix(m);
    LoadIdentity();
    glFrontFace(GL_CCW); // GL_CCW is also the default
#ifdef WIN32 // todo: add linux ifdef
    glEnable(GL_PROGRAM_POINT_SIZE);
#endif

    //No need for projection matrix in this mode/shader.
    // z coordinate will be ignored
}

void SnakeRenderer::UseTexture(uint32_t texture)
{
    UseProgram(mTextureProgram);

    // Bind our texture in Texture Unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    // Set our "myTextureSampler" sampler to use Texture Unit 0
    glUniform1i(mTextureProgram.mTextSamplerUniform, 0);
}

void SnakeRenderer::UseTexture(const Texture& t)
{
    UseProgram(mTextureProgram);

    // Bind our texture in Texture Unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, t.m_textHandle);
    // Set our "myTextureSampler" sampler to use Texture Unit 0
    glUniform1i(mTextureProgram.mTextSamplerUniform, 0);
}

void SnakeRenderer::Color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
{
    if (m_pActiveProgram && m_pActiveProgram->mColorAttrib != -1 && !m_bShadow)
    {
        //glUniform4f(mColorUniform, red / 255.0f, green / 255.0f, blue / 255.0f, alpha / 255.0f);
        glVertexAttrib4f(m_pActiveProgram->mColorAttrib, red / 255.0f, green / 255.0f, blue / 255.0f, alpha / 255.0f);
        glDisableVertexAttribArray(m_pActiveProgram->mColorAttrib);
    }
}

// used in 2D radar mode
void SnakeRenderer::DrawVectors(Vector a[], int count, uint32_t mode)
{
    glBindBuffer(GL_ARRAY_BUFFER, mVB);
    glBufferData(GL_ARRAY_BUFFER, count*sizeof(Vector), &a[0], GL_STREAM_DRAW);

    glUniformMatrix4fv(m_pActiveProgram->mPMVMatrixUniform, 1, GL_FALSE, m_modelView);

    glEnableVertexAttribArray(m_pActiveProgram->mPosAttrib);
    glDisableVertexAttribArray(m_pActiveProgram->mColorAttrib);
    glVertexAttribPointer(m_pActiveProgram->mPosAttrib, 3, SN_FLOAT, GL_FALSE, 0, 0);

    glDrawArrays(mode, 0, count);
}

void SnakeRenderer::BindState()
{
    glBindVertexArray(mVBState);
}

unsigned SnakeRenderer::GetError()
{
    return glGetError();
}

void SnakeRenderer::PrepareDrawVertex3f3f()
{
    Matrix m = m_projection * m_modelView; // m is the model-view-projection matrix
    //Matrix m = m_modelView;
    glUniformMatrix4fv(m_pActiveProgram->mPMVMatrixUniform, 1, GL_FALSE, m);
    glUniformMatrix4fv(m_pActiveProgram->mModelMatrixUniform, 1, GL_FALSE, m_model);

    glEnableVertexAttribArray(m_pActiveProgram->mPosAttrib);
    glEnableVertexAttribArray(m_pActiveProgram->mNormalAttrib);
    // glVertexAttribPointer exist in opengl versions: {2.0, 2.1, 3.0, 3.1, 3.2, 3.3, 4.1, 4.2, 4.3, 4.4, 4.5}
    glVertexAttribPointer(m_pActiveProgram->mPosAttrib, 3, SN_FLOAT, GL_FALSE, sizeof(Vertex3f3f), (const GLvoid*)offsetof(Vertex3f3f, pos));
    glVertexAttribPointer(m_pActiveProgram->mNormalAttrib, 3, SN_FLOAT, GL_FALSE, sizeof(Vertex3f3f), (const GLvoid*)offsetof(Vertex3f3f, normal));
    glDisableVertexAttribArray(m_pActiveProgram->mColorAttrib);
}

// used in 3D light mode
void SnakeRenderer::DrawVertex3f3f(const Vertex3f3f a[], size_t count, unsigned mode)
{
    glBindBuffer(GL_ARRAY_BUFFER, mVB);
    glBufferData(GL_ARRAY_BUFFER, count*sizeof(Vertex3f3f), &a[0], GL_STREAM_DRAW);
    PrepareDrawVertex3f3f();

    glDrawArrays(mode, 0, (GLsizei)count);
}

// used in 3D light mode
void SnakeRenderer::DrawVertex3f3f(const Vertex3f3f a[], size_t vertex_count, const int indices[], size_t index_count, unsigned mode)
{
    glBindBuffer(GL_ARRAY_BUFFER, mVB);
    glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(Vertex3f3f), &a[0], GL_STREAM_DRAW);
#if __APPLE__
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndicesBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count * sizeof(int), &indices[0], GL_STREAM_DRAW);
    indices = NULL;
#endif
    PrepareDrawVertex3f3f();

    glDrawElements(mode, (GLsizei)index_count, GL_UNSIGNED_INT, indices);
}

// used in 3D light mode with texture
void SnakeRenderer::DrawVertex3f3f2f(const Vertex3f3f2f a[], size_t count)
{
    glBindBuffer(GL_ARRAY_BUFFER, mVB);
    glBufferData(GL_ARRAY_BUFFER, count * sizeof(Vertex3f3f2f), &a[0], GL_STREAM_DRAW);

    Matrix m = m_projection * m_modelView; // m is the model-view-projection matrix
    glUniformMatrix4fv(m_pActiveProgram->mPMVMatrixUniform, 1, GL_FALSE, m);

    glEnableVertexAttribArray(m_pActiveProgram->mPosAttrib);
    glVertexAttribPointer(m_pActiveProgram->mPosAttrib, 3, SN_FLOAT, GL_FALSE, sizeof(Vertex3f3f2f), (const GLvoid*)offsetof(Vertex3f3f2f, pos));
    if (m_pActiveProgram->mTextUVAttrib != -1)
    {
        glEnableVertexAttribArray(m_pActiveProgram->mTextUVAttrib);
        glVertexAttribPointer(m_pActiveProgram->mTextUVAttrib, 2, SN_FLOAT, GL_FALSE, sizeof(Vertex3f3f2f), (const GLvoid*)offsetof(Vertex3f3f2f, u));
    }
    if (m_pActiveProgram->mNormalAttrib != -1)
    {
        glEnableVertexAttribArray(m_pActiveProgram->mNormalAttrib);
        glVertexAttribPointer(m_pActiveProgram->mNormalAttrib, 3, SN_FLOAT, GL_FALSE, sizeof(Vertex3f3f2f), (const GLvoid*)offsetof(Vertex3f3f2f, normal));
    }

    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)count);
}

// used in 3D no-light mode with texture
void SnakeRenderer::DrawVertex3f2f(const Vertex3f2f a[], int count, unsigned mode)
{
    glBindBuffer(GL_ARRAY_BUFFER, mVB);
    glBufferData(GL_ARRAY_BUFFER, count * sizeof(Vertex3f2f), &a[0], GL_STREAM_DRAW);

    Matrix m = m_projection * m_modelView; // m is the model-view-projection matrix
    glUniformMatrix4fv(m_pActiveProgram->mPMVMatrixUniform, 1, GL_FALSE, m);

    glEnableVertexAttribArray(m_pActiveProgram->mPosAttrib);
    glEnableVertexAttribArray(m_pActiveProgram->mTextUVAttrib);
    glVertexAttribPointer(m_pActiveProgram->mPosAttrib, 3, SN_FLOAT, GL_FALSE, sizeof(Vertex3f2f), (const GLvoid*)offsetof(Vertex3f2f, pos));
    glVertexAttribPointer(m_pActiveProgram->mTextUVAttrib, 2, SN_FLOAT, GL_FALSE, sizeof(Vertex3f2f), (const GLvoid*)offsetof(Vertex3f2f, u));

    glDrawArrays(mode, 0, count);
}

// used in 3D no-light mode with texture
void SnakeRenderer::DrawVertex3f2f(const Vertex3f2f a[], size_t vertex_count, const int indices[], size_t index_count, unsigned mode)
{
    glBindBuffer(GL_ARRAY_BUFFER, mVB);
    glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(Vertex3f2f), &a[0], GL_STREAM_DRAW);
    
#if __APPLE__
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndicesBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count * sizeof(int), &indices[0], GL_STREAM_DRAW);
    indices = NULL;
#endif
    
    Matrix m = m_projection * m_modelView; // m is the model-view-projection matrix
    glUniformMatrix4fv(m_pActiveProgram->mPMVMatrixUniform, 1, GL_FALSE, m);

    glEnableVertexAttribArray(m_pActiveProgram->mPosAttrib);
    glEnableVertexAttribArray(m_pActiveProgram->mTextUVAttrib);
    glVertexAttribPointer(m_pActiveProgram->mPosAttrib, 3, SN_FLOAT, GL_FALSE, sizeof(Vertex3f2f), (const GLvoid*)offsetof(Vertex3f2f, pos));
    glVertexAttribPointer(m_pActiveProgram->mTextUVAttrib, 2, SN_FLOAT, GL_FALSE, sizeof(Vertex3f2f), (const GLvoid*)offsetof(Vertex3f2f, u));

    glDrawElements(mode, (GLsizei)index_count, GL_UNSIGNED_INT, indices);
}


void SnakeRenderer::SetData(Vertex3f3f a[], size_t count)
{
    glBindBuffer(GL_ARRAY_BUFFER, mVB);
    glBufferData(GL_ARRAY_BUFFER, count*sizeof(Vertex3f3f), &a[0], GL_STREAM_DRAW);
    glEnableVertexAttribArray(m_pActiveProgram->mPosAttrib);
    glEnableVertexAttribArray(m_pActiveProgram->mNormalAttrib);
    glVertexAttribPointer(m_pActiveProgram->mPosAttrib, 3, SN_FLOAT, GL_FALSE, sizeof(Vertex3f3f), (const GLvoid*)offsetof(Vertex3f3f, pos));
    glVertexAttribPointer(m_pActiveProgram->mNormalAttrib, 3, SN_FLOAT, GL_FALSE, sizeof(Vertex3f3f), (const GLvoid*)offsetof(Vertex3f3f, normal));
    glDisableVertexAttribArray(m_pActiveProgram->mColorAttrib);
}

// used in 2D radar mode
void SnakeRenderer::SetBlendData(VertexColor2f a[], size_t count)
{
    glBindBuffer(GL_ARRAY_BUFFER, mVB);
    glBufferData(GL_ARRAY_BUFFER, count * sizeof(VertexColor2f), &a[0], GL_STREAM_DRAW);
    glEnableVertexAttribArray(m_pActiveProgram->mColorAttrib);
    glVertexAttribPointer(m_pActiveProgram->mPosAttrib, 2, SN_FLOAT, GL_FALSE, sizeof(VertexColor2f), (const GLvoid*)offsetof(VertexColor2f, x));
    glVertexAttribPointer(m_pActiveProgram->mColorAttrib, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(VertexColor2f), (const GLvoid*)offsetof(VertexColor2f, r));
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void SnakeRenderer::DrawArrays(GLenum mode, int first, int count)
{
    if (m_pActiveProgram == &mMainProgram)
    {
        Matrix m = m_projection * m_modelView; // m is the model-view-projection matrix
        glUniformMatrix4fv(m_pActiveProgram->mPMVMatrixUniform, 1, GL_FALSE, m);
        glUniformMatrix4fv(m_pActiveProgram->mModelMatrixUniform, 1, GL_FALSE, m_model);
    }
    /*if (m_pActiveProgram == mRadarProgram)
    {
        glUniformMatrix4fv(mScaleRotUniform, 1, GL_FALSE, m_modelView);
    }*/
    glDrawArrays(mode, first, count);
}

void SnakeRenderer::PointSize(float size)
{
    glUniform1f(m_pActiveProgram->mRadarPointSizeUniform, size);
}

void SnakeRenderer::SetLight(int iLight, const Vector& position, const struct Color& color)
{
    if (iLight == 0)
    {
        // Option 1: multiplying light position by the view matrix (m_modelView should have only view transform
        // when SetLight is called). the normals would be multiplied by the model-view matrix:
        //m_lightPosition = m_modelView * position;
        // Option 2: copy the light position as is. The normals would be multiplied by the model matrix:
        m_lightPosition = position;
        m_lightColor = color;
        glUniform3f(m_pActiveProgram->mLightPosUniform, m_lightPosition[0], m_lightPosition[1], m_lightPosition[2]);
        glUniform3fv(m_pActiveProgram->mLightColorUniform, 1, m_lightColor.c);
    }
}

void SnakeRenderer::SetAmbientLight(const struct Color& color)
{
    m_ambientLightColor = color;
    glUniform3fv(m_pActiveProgram->mAmbientlightColorUniform, 1, m_ambientLightColor.c);
}

void SnakeRenderer::ShadowMode(bool bShadow)
{
    if (bShadow) {
        Color(0, 0, 0, 1);
        glDisable(GL_DEPTH_TEST);
    }
    else
        glEnable( GL_DEPTH_TEST );
    m_bShadow = bShadow;
}

void SnakeRenderer::PrepareStencil()
{
    glEnable(GL_STENCIL_TEST);
    glClear(GL_STENCIL_BUFFER_BIT);
    glStencilFunc (GL_ALWAYS, 0x1, 0x1);
    glStencilOp (GL_REPLACE, GL_REPLACE, GL_REPLACE);
}

void SnakeRenderer::UseStencil()
{
    glStencilOp (GL_KEEP, GL_KEEP, GL_KEEP);
    glStencilFunc (GL_EQUAL, 0x1, 0x1);
}

void SnakeRenderer::DisableStencil()
{
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_BLEND);
    glDisableVertexAttribArray(m_pActiveProgram->mColorAttrib);
}

void SnakeRenderer::DisableDepthTest()
{
    glDisable(GL_DEPTH_TEST);
}

void SnakeRenderer::Viewport(int x, int y, int width, int height)
{
    glViewport(x, y, width, height);
}

void SnakeRenderer::LineWidth(float width)
{
    glLineWidth(width);
}
