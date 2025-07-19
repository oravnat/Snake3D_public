//  This file is part of Snake3D by Or Avnat: https://github.com/oravnat/Snake3D_public

//
// Created by Or Avnat on 15/07/2020.
//

#ifndef SNAKE3D3_SNAKERENDERER_H
#define SNAKE3D3_SNAKERENDERER_H

#define _USE_MATH_DEFINES

#include "Vector.hpp"
#include "Matrix.hpp"
#include <math.h>
#include <cstddef>
#include <cstdint>

#define SNAKE_POINTS 0x0000
#define SNAKE_LINES 0x0001
#define SNAKE_LINE_LOOP 0x0002
#define SNAKE_LINE_STRIP 0x0003
#define SNAKE_TRIANGLES 0x0004
#define SNAKE_TRIANGLE_STRIP 0x0005

class Texture
{
public:
    int m_textHandle;
public:
    Texture() : m_textHandle(0) {}
    Texture(int texture) : m_textHandle(texture) {}
    bool IsValid() { return m_textHandle != 0; }
};

class Program
{
public:
    int mProgram;
    // attributes are -1 if not used
    int mPosAttrib;
    int mNormalAttrib;
    int mColorAttrib;
    // uniforms are -1 if not used
    int mPMVMatrixUniform;
    int mModelMatrixUniform;
    int mLightPosUniform;
    int mLightColorUniform;
    int mAmbientlightColorUniform;
    int mTextUVAttrib;
    int mTextSamplerUniform;
    int mRadarPointSizeUniform;
public:
    Program() : mProgram(0), 
        mPosAttrib(-1), mNormalAttrib(-1), mColorAttrib(-1),
        mPMVMatrixUniform(-1), mModelMatrixUniform(-1), mLightPosUniform(-1), mLightColorUniform(-1), mAmbientlightColorUniform(-1),
        mTextUVAttrib(-1), mTextSamplerUniform(-1),
        mRadarPointSizeUniform(-1)

    {

    }
    ~Program();

    bool Create(const char* vtxSrc, const char* fragSrc);

    bool CreateTextureProgram(const char* vtxSrc, const char* fragSrc);

    bool CreateRadarProgram(const char* vtxSrc, const char* fragSrc);
};

class SnakeRenderer
{
	static const int STACK_SIZE = 20;
public:
    SnakeRenderer() : m_stackTop(0), mVB(0), mIndicesBuffer(0), m_pActiveProgram(nullptr), m_view(Matrix::I()), m_bShadow(false), mVBState(0)
    {
		//m_screenRotation = 0;
	}

    ~SnakeRenderer() {
        //glDeleteBuffers(1, &mVB);
    }


    bool Init();

    void SetViewMatrix(const Matrix& m)
    {
        m_view = m;
    }
    void SetProjectionMatrix(const Matrix& m)
    {
        m_projection = m;
    }
    void SetPerspective(float fovy, float aspect, float zNear, float zFar)
    {
        Matrix m = gluPerspective(fovy, aspect, zNear, zFar);
        SetProjectionMatrix(m);
    }
	void LoadIdentity()
	{
		m_modelView = Matrix::I();
		//m_inverseModelView = Matrix::I();
		m_normalMatrix = Matrix::I();
		m_model = Matrix::I();
	}

	void PushMatrix()
	{
		m_normalsStack[m_stackTop] = m_normalMatrix;
		m_nmodelStack[m_stackTop] = m_model;
		m_stack[m_stackTop++] = m_modelView;
	}

	void PopMatrix()
	{
		m_modelView = m_stack[--m_stackTop];
		m_model = m_nmodelStack[m_stackTop];
		m_normalMatrix = m_normalsStack[m_stackTop];
	}

    void UseProgram(Program& program);

    void Start3DRendering();
	
    void Start2DRendering();

    void Use3DProgram()
    {
        UseProgram(mMainProgram);
    }

    void UseTexture(uint32_t texture);

    void UseTexture(const Texture& t);

	float Deg2Rad(float deg)
	{
		return (float)(deg * (M_PI / 180.0f));
	}

    // angle is in degrees
	void RotateX(float angleDeg)
	{
		Matrix m = Matrix::RotateX(Deg2Rad(angleDeg));
		MultMatrix(m);
		//MultNormalMatrix(m);
	}

    // angle is in degrees
    void RotateY(float angleDeg)
	{
		Matrix m = Matrix::RotateY(Deg2Rad(angleDeg));
		MultMatrix(m);
		//MultNormalMatrix(m);
	}

    // angle is in degrees
    void RotateZ(float angleDeg)
	{
		Matrix m = Matrix::RotateZ(Deg2Rad(angleDeg));
		MultMatrix(m);
		//MultNormalMatrix(m);
	}

    // If the transform before the call was C x v, after the call it will be C x M x v
    // bModel should be true for translation and rotation
    void MultMatrix(const Matrix& m, bool bModel = true )
    {
        m_modelView = m_modelView * m;
        if (bModel)
            m_model = m_model * m;
    }

    void Translate(float x, float y, float z)
    {
        Matrix m = Matrix::Translate(x, y, z);
        MultMatrix(m);
    }

    void TranslateTo(const Vector& v)
    {
        Translate(v.v[0], v.v[1], v.v[2]);
    }

    void Scale(float x, float y, float z)
    {
        Matrix m = Matrix::Scale(x, y, z);
        MultMatrix(m, false);
    }

    void Color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha);

    // used in 2D radar mode
    void DrawVectors(Vector a[], int count, uint32_t mode);

    void BindState();

    unsigned GetError();

    void PrepareDrawVertex3f3f();

    // used in 3D light mode
    void DrawVertex3f3f(const Vertex3f3f a[], size_t count, unsigned mode = SNAKE_TRIANGLES);

    // used in 3D light mode
    void DrawVertex3f3f(const Vertex3f3f a[], size_t vertex_count, const int indices[], size_t index_count, unsigned mode = SNAKE_TRIANGLES);

    // used in 3D light mode with texture
    void DrawVertex3f3f2f(const Vertex3f3f2f a[], size_t count);

    // used in 3D no-light mode with texture
    void DrawVertex3f2f(const Vertex3f2f a[], int count, unsigned mode = SNAKE_TRIANGLES);

    // used in 3D no-light mode with texture
    void DrawVertex3f2f(const Vertex3f2f a[], size_t vertex_count, const int indices[], size_t index_count, unsigned mode = SNAKE_TRIANGLES);

    
    void SetData(Vertex3f3f a[], size_t count);

    // used in 2D radar mode
    void SetBlendData(VertexColor2f a[], size_t count);
    
    void DrawArrays(unsigned mode, int first, int count);
	
    void PointSize(float size);

    void SetLight(int iLight, const Vector& position, const struct Color& color);

    void SetAmbientLight(const struct Color& color);

    void ShadowMode(bool bShadow);

    void PrepareStencil();

    void UseStencil();

    void DisableStencil();

    void DisableDepthTest();

    void Viewport(int x, int y, int width, int height);

    void LineWidth(float width);

    static Matrix gluPerspective(float fovy, float aspect, float zNear, float zFar)
    {
        float sine, cotangent, deltaZ;
        float radians = fovy / 2 * (float)M_PI / 180.0f;

        deltaZ = zFar - zNear;
        sine = sinf(radians);
        if ((deltaZ == 0) || (sine == 0) || (aspect == 0))
            return Matrix::I();
        cotangent = cosf(radians) / sine;
        Matrix m = Matrix::I();

        m[0][0] = cotangent / aspect;
        m[1][1] = cotangent;
        m[2][2] = -(zFar + zNear) / deltaZ;
        m[2][3] = -1;
        m[3][2] = -2 * zNear * zFar / deltaZ;
        m[3][3] = 0;

        return m;
    }

    void gluLookAt(float eyex, float eyey, float eyez, float centerx,
                   float centery, float centerz, float upx, float upy,
                   float upz)
    {
        Vector forward, side, up;

        forward[0] = centerx - eyex;
        forward[1] = centery - eyey;
        forward[2] = centerz - eyez;

        up[0] = upx;
        up[1] = upy;
        up[2] = upz;

        forward.Normalize();

        /* Side = forward x up */
        side = forward.CrossProduct(up);
        side.Normalize();

        /* Recompute up as: up = side x forward */
        up = side.CrossProduct(forward);

        Matrix m = Matrix::I();

        m[0][0] = side[0];
        m[1][0] = side[1];
        m[2][0] = side[2];

        m[0][1] = up[0];
        m[1][1] = up[1];
        m[2][1] = up[2];

        m[0][2] = -forward[0];
        m[1][2] = -forward[1];
        m[2][2] = -forward[2];

        MultMatrix(m);
        Translate(-eyex, -eyey, -eyez);

        m_model = Matrix::I();
    }

    void SetModelView(const Matrix& modelView)
    {
        m_modelView = modelView;
    }

    Vector TransformVector(Vector& v)
    {
        return m_projection * m_modelView * v;
    }

    Matrix GetView()
    {
        return m_view;
    }

    Matrix GetProjection()
    {
        return m_projection;
    }

    void ResetModelTrasfrom()
    {
        LoadIdentity();
        MultMatrix(m_view, false);
    }

private:
    bool m_bShadow;
    Matrix m_model;
    Matrix m_modelView;
    Matrix m_view;
	Matrix m_projection;
	Matrix m_normalMatrix;
    Vector m_lightPosition;
    struct Color m_lightColor;
    struct Color m_ambientLightColor;

	Matrix m_stack[STACK_SIZE];
	Matrix m_normalsStack[STACK_SIZE];
	Matrix m_nmodelStack[STACK_SIZE];
	int m_stackTop;

    Program* m_pActiveProgram;
    Program mMainProgram;
    Program mTextureProgram;
    Program mRadarProgram;

    uint32_t mVB;
    uint32_t mIndicesBuffer;
    uint32_t mVBState;
};

#endif //SNAKE3D3_SNAKERENDERER_H
