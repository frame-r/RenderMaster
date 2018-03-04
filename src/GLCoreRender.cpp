#include <string>

#include <GL\glew.h>
#include <GL\wglew.h>

#include "GLCoreRender.h"
#include "Core.h"

extern Core *_pCore;

using string = std::string;

#define LOG_FATAL(msg) _pCore->Log(msg, LOG_TYPE::LT_FATAL);
void E_GUARDS()
{
	GLenum err = glGetError();
	if (err != GL_NO_ERROR)
	{
		string error;

		switch (err) {
		case GL_INVALID_OPERATION:      error = "INVALID_OPERATION";      break;
		case GL_INVALID_ENUM:           error = "INVALID_ENUM";           break;
		case GL_INVALID_VALUE:          error = "INVALID_VALUE";          break;
		case GL_OUT_OF_MEMORY:          error = "OUT_OF_MEMORY";          break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:  error = "INVALID_FRAMEBUFFER_OPERATION";  break;
		}
		_pCore->Log(error.c_str(), LOG_TYPE::LT_WARNING);
	}
}

GLCoreRender::GLCoreRender()
{
	_pCore->Log("GLCoreRender initalized");
}


GLCoreRender::~GLCoreRender()
{
}

API GLCoreRender::GetName(const char *& pTxt)
{
	pTxt = "GLCoreRender";
	return S_OK;
}

API GLCoreRender::Init(WinHandle& handle)
{
	const int major_version = 4;
	const int minor_version = 5;

	_hWnd = handle;

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

	E_GUARDS();

	// dbg
	if (wglGetCurrentContext() != _hRC)
		if (!wglMakeCurrent(_hdc, _hRC))
			LOG_FATAL("some error");
	
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glViewport(0, 0, 100, 100);
	E_GUARDS();

	return S_OK;
}

API GLCoreRender::Clear()
{
	E_GUARDS();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	
	//dbg
	SwapBuffers(_hdc);
	
	E_GUARDS();
	return S_OK;
}

API GLCoreRender::Free()
{
	wglMakeCurrent(nullptr, nullptr);
	wglDeleteContext(_hRC);
	ReleaseDC(_hWnd, GetDC(_hWnd));

	return S_OK;
}
