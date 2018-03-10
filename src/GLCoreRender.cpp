#include <string>
#include <sstream>

#include <GL\glew.h>
#include <GL\wglew.h>

#include "GLShader.h"
#include "GLCoreRender.h"
#include "Core.h"
#include "GLMesh.h"


extern Core *_pCore;

using string = std::string;

#define LOG_FATAL(msg) _pCore->Log(msg, LOG_TYPE::LT_FATAL);
void CHECK_GL_ERRORS()
{
	GLenum err = glGetError();
	if (err != GL_NO_ERROR)
	{
		string error;

		switch (err) 
		{
			case GL_INVALID_OPERATION:      error = "INVALID_OPERATION";      break;
			case GL_INVALID_ENUM:           error = "INVALID_ENUM";           break;
			case GL_INVALID_VALUE:          error = "INVALID_VALUE";          break;
			case GL_OUT_OF_MEMORY:          error = "OUT_OF_MEMORY";          break;
			case GL_INVALID_FRAMEBUFFER_OPERATION:  error = "INVALID_FRAMEBUFFER_OPERATION";  break;
			default:
			{
				std::ostringstream oss;
				oss << err;
				error = "OpenGL error: " + oss.str();
			}break;
		}
		_pCore->Log(error.c_str(), LOG_TYPE::LT_WARNING);
	}	
}

bool GLCoreRender::_check_chader_errors(int id, GLenum constant)
{
	int status;

	if (constant == GL_COMPILE_STATUS)
		glGetShaderiv(id, GL_COMPILE_STATUS, &status);
	else if (constant == GL_LINK_STATUS)
		glGetProgramiv(id, GL_LINK_STATUS, &status);

	if (status == GL_FALSE)
	{
		GLint length = 0;

		if (constant == GL_COMPILE_STATUS)
			glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
		else if (constant == GL_LINK_STATUS)
			glGetProgramiv(id, GL_INFO_LOG_LENGTH, &length);

		char *buf = new char[length];

		if (constant == GL_COMPILE_STATUS)
			glGetShaderInfoLog(id, length, &length, buf);
		else if (constant == GL_LINK_STATUS)
			glGetProgramInfoLog(id, length, &length, buf);

		_pCore->Log(buf, LOG_TYPE::LT_FATAL);

		delete buf;

		return false;
	}
	return true;
}

bool GLCoreRender::_create_shader(GLuint& id, GLenum type, const char** pData, int numLines, GLuint programID)
{
	GLuint _id = glCreateShader(type);
	glShaderSource(_id, numLines, pData, nullptr);
	glCompileShader(_id);

	if (!_check_chader_errors(_id, GL_COMPILE_STATUS))
	{
		glDeleteShader(_id);
		return false;
	}
	glAttachShader(programID, _id);
	glLinkProgram(programID);
	if (!_check_chader_errors(programID, GL_LINK_STATUS))
		return false;

	id = _id;

	return true;
}

GLCoreRender::GLCoreRender()
{
}


GLCoreRender::~GLCoreRender()
{
}

API GLCoreRender::GetName(const char *& pTxt)
{
	pTxt = "GLCoreRender";
	return S_OK;
}

API GLCoreRender::Init(WinHandle* handle)
{
	const int major_version = 4;
	const int minor_version = 5;

	_hWnd = *handle;

	PIXELFORMATDESCRIPTOR pfd = {};

	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 24;
	pfd.iLayerType = PFD_MAIN_PLANE;

	_hdc = GetDC(_hWnd);

	int closest_pixel_format = 0;
	const bool msaa = false;
	const int msaa_samples = 16;

	if (!msaa)
	{
		closest_pixel_format = ChoosePixelFormat(_hdc, &pfd);

		if (closest_pixel_format == 0)
		{
			LOG_FATAL("Wrong ChoosePixelFormat() result");
			return S_FALSE;
		}

		if (!SetPixelFormat(_hdc, closest_pixel_format, &pfd))
		{
			LOG_FATAL("Wrong SetPixelFormat() result");
			return S_FALSE;
		}

		HGLRC hrc_fake = wglCreateContext(_hdc);
		wglMakeCurrent(_hdc, hrc_fake);

		if (glewInit() != GLEW_OK)
		{
			LOG_FATAL("Couldn't initialize GLEW");
			wglMakeCurrent(nullptr, nullptr);
			wglDeleteContext(hrc_fake);
			return S_FALSE;
		}

		wglMakeCurrent(nullptr, nullptr);
		wglDeleteContext(hrc_fake);

	}
	else
	{
		HWND hwnd_fake = CreateWindowEx(0, TEXT("STATIC"), NULL, 0, 0, 0, 0, 0, 0, 0, 0, NULL);
		HDC hdc_fake = GetDC(hwnd_fake);
		int closest_pixel_format_temp = ChoosePixelFormat(hdc_fake, &pfd);
		SetPixelFormat(hdc_fake, closest_pixel_format_temp, &pfd);
		HGLRC hrc_fake = wglCreateContext(hdc_fake);
		wglMakeCurrent(hdc_fake, hrc_fake);

		if (glewInit() != GLEW_OK)
		{
			LOG_FATAL("Couldn't initialize GLEW");
			wglMakeCurrent(nullptr, nullptr);
			wglDeleteContext(hrc_fake);
			ReleaseDC(hwnd_fake, hdc_fake);
			DestroyWindow(hwnd_fake);
			return S_FALSE;
		}

		// New way create default framebuffer
		if (WGLEW_ARB_pixel_format)
		{

			const int iPixelFormatAttribList[] =
			{
				WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
				WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
				WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
				WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
				WGL_COLOR_BITS_ARB, 32,
				WGL_DEPTH_BITS_ARB, 24,
				WGL_STENCIL_BITS_ARB, 8,
				WGL_SAMPLE_BUFFERS_ARB, 1, //Number of buffers (must be 1 at time of writing)
				WGL_SAMPLES_ARB, msaa_samples,
				0
			};

			int numFormats = 0;
			int chosen = wglChoosePixelFormatARB(_hdc, iPixelFormatAttribList, NULL, 1, &closest_pixel_format, (UINT*)&numFormats);
			if (!chosen || numFormats <= 0)
			{
				LOG_FATAL("Wrong wglChoosePixelFormatARB() result");
				return S_FALSE;
			}
		}
		else
		{
			LOG_FATAL("Extension WGLEW_ARB_pixel_format didn't found in driver");
			return S_FALSE;
		}

		wglMakeCurrent(nullptr, nullptr);
		wglDeleteContext(hrc_fake);
		ReleaseDC(hwnd_fake, hdc_fake);
		DestroyWindow(hwnd_fake);
	}

	if (!SetPixelFormat(_hdc, closest_pixel_format, &pfd))
	{
		LOG_FATAL("Wrong SetPixelFormat() result");
		return S_FALSE;
	}

	if (WGLEW_ARB_create_context)
	{
		const int context_attribs[] =
		{
			WGL_CONTEXT_MAJOR_VERSION_ARB, major_version,
			WGL_CONTEXT_MINOR_VERSION_ARB, minor_version,
			WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
			0 // end
		};

		_hRC = wglCreateContextAttribsARB(_hdc, nullptr, context_attribs); // context

		if (_hRC)
		{
			if (!wglMakeCurrent(_hdc, _hRC))
			{
				wglDeleteContext(_hRC);
				ReleaseDC(_hWnd, _hdc);
				LOG_FATAL("Couldn't perform wglMakeCurrent(_hdc, _hRC);");
				return S_FALSE;
			}

			
			#define OGLI "OpenGL context created at version " 
			GLint major, minor;
			char buffer[sizeof(OGLI) + 4];
			glGetIntegerv(GL_MAJOR_VERSION, &major);
			glGetIntegerv(GL_MINOR_VERSION, &minor);
			sprintf_s(buffer, OGLI"%i.%i", major, minor);

			_pCore->Log(buffer, LOG_TYPE::LT_NORMAL);
		}
		else
		{
			LOG_FATAL("Couldn't create OpenGL context with wglCreateContextAttribsARB()");
			return S_FALSE;
		}
	}
	else
	{
		LOG_FATAL("Extension WGLEW_ARB_create_context didn't found in driver");
		return S_FALSE;
	}

	CHECK_GL_ERRORS();
	
	// dbg
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	//glViewport(0, 0, 100, 100);

	CHECK_GL_ERRORS();

	_pCore->Log("GLCoreRender initalized");

	return S_OK;
}

API GLCoreRender::CreateMesh(ICoreMesh *&pMesh, MeshDataDesc &dataDesc, MeshIndexDesc &indexDesc, DRAW_MODE mode)
{
	const int indexes = indexDesc.format != MESH_INDEX_FORMAT::MID_NOTHING;
	const int texCoords = dataDesc.texCoordPresented;
	const int normals = dataDesc.normalsPresented;
	const int bytes = (12 + texCoords * 8 + normals * 12) * dataDesc.number;
	
	GLuint vao = 0, vbo = 0, ibo = 0;

	CHECK_GL_ERRORS();

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	const GLenum glBufferType = GL_STATIC_DRAW; // TODO: GL_DYNAMIC_DRAW;
	glBufferData(GL_ARRAY_BUFFER, bytes, reinterpret_cast<const void*>(dataDesc.pData), glBufferType); // send data to VRAM

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, dataDesc.positionStride, reinterpret_cast<void*>(dataDesc.pData + dataDesc.positionOffset));

	if (texCoords)
	{
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, dataDesc.texCoordStride, reinterpret_cast<void*>(dataDesc.pData + dataDesc.texCoordOffset));
	}

	if (normals)
	{
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, dataDesc.normalStride, reinterpret_cast<void*>(dataDesc.pData + dataDesc.normalOffset));
	}

	if (indexes)
	{
		int idxSize = 0;
		switch (indexDesc.format)
		{
			case MESH_INDEX_FORMAT::MID_INT32: idxSize = 32; break;
			case MESH_INDEX_FORMAT::MID_INT16: idxSize = 16; break;
		}
		const int idxBytes = idxSize * indexDesc.number;

		glGenBuffers(1, &ibo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, idxBytes, reinterpret_cast<void*>(indexDesc.pData), glBufferType); // send data to VRAM
	}

	glBindVertexArray(0);

	CHECK_GL_ERRORS();

	GLMesh *pGlMesh = new GLMesh(vao, vbo, ibo, dataDesc.number, indexDesc.number, indexDesc.format, mode);
	pMesh = pGlMesh;

	return S_OK;
}

API GLCoreRender::CreateShader(ICoreShader*& pShader, ShaderDesc& shaderDesc)
{
	GLuint vertID = 0;
	GLuint geomID = 0;
	GLuint fragID = 0;
	GLuint programID = glCreateProgram();
	
	if (!_create_shader(vertID, GL_VERTEX_SHADER, shaderDesc.pVertStr, shaderDesc.vertNumLines, programID))
	{
		glDeleteProgram(programID);
		return S_FALSE;
	}
	
	if (shaderDesc.pGeomStr != nullptr && shaderDesc.geomNumLines > 0)
	{
		if (!_create_shader(geomID, GL_GEOMETRY_SHADER, shaderDesc.pGeomStr, shaderDesc.geomNumLines, programID))
		{
			glDeleteProgram(programID);
			return S_FALSE;
		}
	}
	
	if (!_create_shader(fragID, GL_FRAGMENT_SHADER, shaderDesc.pFragStr, shaderDesc.fragNumLines, programID))
	{
		glDeleteProgram(programID);
		return S_FALSE;
	}

	GLShader *pGLShader = new GLShader(programID, vertID, geomID, fragID);
	pShader = pGLShader;

	return S_OK;
}

API GLCoreRender::Clear()
{
	CHECK_GL_ERRORS();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	
	//dbg
	SwapBuffers(_hdc);
	
	CHECK_GL_ERRORS();
	return S_OK;
}

API GLCoreRender::Free()
{
	wglMakeCurrent(nullptr, nullptr);
	wglDeleteContext(_hRC);
	ReleaseDC(_hWnd, GetDC(_hWnd));

	return S_OK;
}
