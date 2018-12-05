#include "pch.h"
#include "GLCoreRender.h"
#include "Core.h"
#include "ResourceManager.h"
#include "GLShader.h"
#include "GLMesh.h"
#include "GLTexture.h"

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

vector<UBO> UBOpool;

GLMesh *getGLMesh(IMesh *mesh)
{
	ICoreMesh *cm = getCoreMesh(mesh);
	return static_cast<GLMesh*>(cm);
}

GLTexture *getGLTexture(ITexture *tex)
{
	ICoreTexture *ct = getCoreTexture(tex);
	return static_cast<GLTexture*>(ct);
}

GLShader *getGLShader(IShader *shader)
{
	ICoreShader *cs = getCoreShader(shader);
	return static_cast<GLShader*>(cs);
}

GLRenderTarget *getGLRenderTarget(IRenderTarget *rt)
{
	ICoreRenderTarget *crt = getCoreRenderTarget(rt);
	return static_cast<GLRenderTarget*>(crt);
}

PIXELFORMATDESCRIPTOR pfd{};

void CHECK_GL_ERRORS()
{
#ifdef _DEBUG
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
		LOG_FATAL(error.c_str());
		assert(false);
	}
#endif
}

bool GLCoreRender::check_shader_errors(int id, GLenum constant)
{
	int status = GL_TRUE;

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

		char *error_message = new char[length];

		if (constant == GL_COMPILE_STATUS)
			glGetShaderInfoLog(id, length, &length, error_message);
		else if (constant == GL_LINK_STATUS)
			glGetProgramInfoLog(id, length, &length, error_message);

		LOG_FATAL_FORMATTED("GLCoreRender::check_shader_errors(): failed to compile shader. Error:");
		LOG_FATAL_FORMATTED("%s", error_message);

		delete error_message;

		return false;
	}
	return true;
}

bool GLCoreRender::create_shader(GLuint& id, GLenum type, const char* pText, GLuint programID)
{
	GLuint _fileID = glCreateShader(type);
	glShaderSource(_fileID, 1, (const GLchar **)&pText, nullptr);
	glCompileShader(_fileID);

	if (!check_shader_errors(_fileID, GL_COMPILE_STATUS))
	{
		glDeleteShader(_fileID);
		return false;
	}
	glAttachShader(programID, _fileID);
	glLinkProgram(programID);
	if (!check_shader_errors(programID, GL_LINK_STATUS))
		return false;

	id = _fileID;

	return true;
}

GLCoreRender::GLCoreRender()
{
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 24;
	pfd.iLayerType = PFD_MAIN_PLANE;
}

GLCoreRender::~GLCoreRender()
{
}

API GLCoreRender::GetName(OUT const char **pTxt)
{
	*pTxt = "GLCoreRender";
	return S_OK;
}

API GLCoreRender::Init(const WindowHandle* handle, int MSAASamples, int VSyncOn)
{
	const int major_version = 4;
	const int minor_version = 5;

	const bool msaa = MSAASamples == 2 || MSAASamples == 4 || MSAASamples == 8 || MSAASamples == 16 || MSAASamples == 32; // set true to init with MSAA

	_hWnd = *handle;

	_pCore->GetSubSystem((ISubSystem**)&_pResMan, SUBSYSTEM_TYPE::RESOURCE_MANAGER);

	_hdc = GetDC(_hWnd);

	if (!msaa)
	{
		_pixel_format = ChoosePixelFormat(_hdc, &pfd);

		if (_pixel_format == 0)
		{
			LOG_FATAL("Wrong ChoosePixelFormat() result");
			return E_FAIL;
		}

		if (!SetPixelFormat(_hdc, _pixel_format, &pfd))
		{
			LOG_FATAL("Wrong SetPixelFormat() result");
			return E_FAIL;
		}

		HGLRC hrc_fake = wglCreateContext(_hdc);
		wglMakeCurrent(_hdc, hrc_fake);

		if (glewInit() != GLEW_OK)
		{
			LOG_FATAL("Couldn't initialize GLEW");
			wglMakeCurrent(nullptr, nullptr);
			wglDeleteContext(hrc_fake);
			return E_FAIL;
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
			return E_FAIL;
		}

		// New way create default framebuffer
		if (!WGLEW_ARB_pixel_format)
		{
			LOG_FATAL("Extension WGLEW_ARB_pixel_format didn't found in driver");
			return E_FAIL;
		}

		// Select highest MSAA support
		int samples = MSAASamples;

		while (samples > 0)
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
				WGL_SAMPLES_ARB, samples,
				0
			};

			int numFormats = 0;
			int chosen = wglChoosePixelFormatARB(_hdc, iPixelFormatAttribList, NULL, 1, &_pixel_format, (UINT*)&numFormats);
			if (chosen && numFormats > 0) break;

			samples /= 2;
		}

		if (_pixel_format == 0)
		{
			LOG_FATAL("Wrong wglChoosePixelFormatARB() result");
			return E_FAIL;
		}

		if (samples != MSAASamples)
		{
			string need_msaa = msaa_to_string(MSAASamples);
			string actially_msaa = msaa_to_string(samples);
			LOG_WARNING_FORMATTED("GLCoreRender::Init() DirectX doesn't support %s MSAA. Now using %s MSAA", need_msaa.c_str(), actially_msaa.c_str());
		}

		wglMakeCurrent(nullptr, nullptr);
		wglDeleteContext(hrc_fake);
		ReleaseDC(hwnd_fake, hdc_fake);
		DestroyWindow(hwnd_fake);
	}

	if (!SetPixelFormat(_hdc, _pixel_format, &pfd))
	{
		LOG_FATAL("Wrong SetPixelFormat() result");
		return E_FAIL;
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
				return E_FAIL;
			}

			
			#define OGLI "OpenGL context created at version " 
			GLint major, minor;
			char gl_version_buffer[sizeof(OGLI) + 4];
			glGetIntegerv(GL_MAJOR_VERSION, &major);
			glGetIntegerv(GL_MINOR_VERSION, &minor);
			sprintf_s(gl_version_buffer, OGLI"%i.%i", major, minor);

			LOG(gl_version_buffer);
		}
		else
		{
			LOG_FATAL("Couldn't create OpenGL context with wglCreateContextAttribsARB()");
			return E_FAIL;
		}
	}
	else
	{
		LOG_FATAL("Extension WGLEW_ARB_create_context didn't found in driver");
		return E_FAIL;
	}

	CHECK_GL_ERRORS();

	// Fill state
	//
	GLboolean blending;
	glGetBooleanv(GL_BLEND, &blending);
	assert(_state.blending == (bool)blending && "Incorrect default state");

	GLint src, dest;
	glGetIntegerv(GL_BLEND_SRC_RGB, &src);
	glGetIntegerv(GL_BLEND_DST_RGB, &dest);
	assert(_state.srcBlend == src && "Incorrect default state");
	assert(_state.dstBlend == dest && "Incorrect default state");

	GLfloat c[4];
	glGetFloatv(GL_COLOR_CLEAR_VALUE, c);

	assert(_state.culling == (bool)glIsEnabled(GL_CULL_FACE) && "Incorrect default state");

	GLint cullMode;
	glGetIntegerv(GL_CULL_FACE_MODE, &cullMode);
	assert(_state.cullingMode == cullMode && "Incorrect default state");

	GLint i[2];
	glGetIntegerv(GL_POLYGON_MODE, i);
	assert(_state.polygonMode == i[0] && "Incorrect default state");

	GLboolean depthTest;
	glGetBooleanv(GL_DEPTH_TEST, &depthTest);
	assert(_state.depthTest == (bool)depthTest && "Incorrect default state");

	GLint vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);
	_state.viewportX = vp[0];
	_state.viewportY = vp[1];
	_state.viewportWidth = vp[2];
	_state.viewportHeigth = vp[3];

	//vsync
	if (VSyncOn)
		wglSwapIntervalEXT(1);
	else
		wglSwapIntervalEXT(0);

	glClearDepth(1.0f);
	glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE); // to DirectX conformity

	CHECK_GL_ERRORS();

	LOG("GLCoreRender initalized");

	return S_OK;
}

API GLCoreRender::Free()
{
	UBOpool.clear();

	wglMakeCurrent(nullptr, nullptr);
	wglDeleteContext(_hRC);
	ReleaseDC(_hWnd, GetDC(_hWnd));

	LOG("GLCoreRender::Free()");

	return S_OK;
}

API GLCoreRender::MakeCurrent(const WindowHandle* handle)
{
	HDC new_hdc = GetDC(*handle);

	int new_dc_pixel_format = GetPixelFormat(new_hdc);

	if (new_dc_pixel_format != _pixel_format)
	{
		int closest_pixel_format = ChoosePixelFormat(new_hdc, &pfd);

		if (closest_pixel_format == 0)
		{
			LOG_FATAL("Wrong ChoosePixelFormat() result");
			return S_FALSE;
		}

		if (!SetPixelFormat(new_hdc, closest_pixel_format, &pfd))
		{
			LOG_FATAL("Wrong SetPixelFormat() result");
			return S_FALSE;
		}
	}

	if (!wglMakeCurrent(new_hdc, _hRC))
	{
		LOG_FATAL("Couldn't perform wglMakeCurrent(_hdc, _hRC);");
		return S_FALSE;
	}

	CHECK_GL_ERRORS();

	_hWnd = *handle;
	_hdc = new_hdc;

	return S_OK;
}

API GLCoreRender::SwapBuffers()
{
	CHECK_GL_ERRORS();

	::SwapBuffers(_hdc);

	CHECK_GL_ERRORS();
	return S_OK;
}

API GLCoreRender::ClearState()
{
	_state = State();
	// TODO: actualy clear state
	return S_OK;
}

API GLCoreRender::PushStates()
{
	_statesStack.push(_state);
	return S_OK;
}

API GLCoreRender::PopStates()
{
	State& state = _statesStack.top();
	_statesStack.pop();

	// Blending
	//
	if (_state.blending != state.blending)
	{
		if (state.blending)
			glEnable(GL_BLEND);
		else
			glDisable(GL_BLEND);
	}

	if (_state.srcBlend != state.srcBlend || _state.dstBlend != state.dstBlend)
		glBlendFunc(state.srcBlend, state.dstBlend);

	// Rasterizer
	//
	if (state.culling != _state.culling)
	{
		if (state.culling)
		{
			glEnable(GL_CULL_FACE);
			if (state.cullingMode != _state.cullingMode)
				glCullFace(state.cullingMode);
		}
		else
			glDisable(GL_CULL_FACE);
	}

	if (state.polygonMode != _state.polygonMode)
		glPolygonMode(GL_FRONT_AND_BACK, _state.polygonMode);


	// Depth/Stencil
	//
	if (state.depthTest != _state.depthTest)
	{
		if (state.depthTest)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);
	}

	// Viewport
	//
	if (state.viewportX != _state.viewportX || state.viewportY != _state.viewportY || state.viewportWidth != _state.viewportWidth || state.viewportHeigth != _state.viewportHeigth)
		glViewport(state.viewportX, state.viewportY, state.viewportWidth, state.viewportHeigth);

	// Shader
	//
	if (state.shader.Get() != _state.shader.Get())
	{		
		if (state.shader.Get())
		{
			GLShader *glShader = getGLShader(state.shader.Get());
			glUseProgram(glShader->programID());
		} else
			glUseProgram(0);
	}

	// Textures
	//
	{
		GLShader *glShader = getGLShader(_state.shader.Get());
		GLuint programID = glShader->programID();

		for (int i = 0; i < 16; i++)
		{
			if (state.texShaderBindings[i].Get() != _state.texShaderBindings[i].Get())
			{
				GLTexture *glTex = getGLTexture(_state.texShaderBindings[i].Get());

				// now work with slot == i
				// TODO: make other types (not only GL_TEXTURE_2D)!
				glActiveTexture(GL_TEXTURE0 + i);

				// texture id -> slot
				glBindTexture(GL_TEXTURE_2D, glTex->textureID());

				// shader variable id -> slot
				GLuint tex_ID = glGetUniformLocation(programID, ("_texture_" + std::to_string(i)).c_str());
				glUniform1i(glTex->textureID(), i);
			}
		}
	}

	// Framebuffer
	// TODO

	// Clear
	//
	if (state.clearColor[0] != _state.clearColor[0] || state.clearColor[1] != _state.clearColor[1] ||
		state.clearColor[2] != _state.clearColor[2] || state.clearColor[3] != _state.clearColor[3])
		glClearColor(state.clearColor[0], state.clearColor[1], state.clearColor[2], state.clearColor[3]);

	_state = state;

	return S_OK;
}

API GLCoreRender::CreateMesh(OUT ICoreMesh **pMesh, const MeshDataDesc *dataDesc, const MeshIndexDesc *indexDesc, VERTEX_TOPOLOGY mode)
{
	const int indexes = indexDesc->format != MESH_INDEX_FORMAT::NOTHING;
	const int normals = dataDesc->normalsPresented;
	const int texCoords = dataDesc->texCoordPresented;
	const int colors = dataDesc->colorPresented;
	const int bytes = (16 + 16 * normals + 8 * texCoords + 16 * colors) * dataDesc->numberOfVertex;
	GLuint vao = 0, vbo = 0, ibo = 0;

	INPUT_ATTRUBUTE attribs = INPUT_ATTRUBUTE::POSITION;
	if (dataDesc->normalsPresented)
		attribs = attribs | INPUT_ATTRUBUTE::NORMAL;
	if (dataDesc->texCoordPresented)
		attribs = attribs | INPUT_ATTRUBUTE::TEX_COORD;
	if (dataDesc->colorPresented)
		attribs = attribs | INPUT_ATTRUBUTE::COLOR;


	CHECK_GL_ERRORS();

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	const GLenum glBufferType = GL_STATIC_DRAW; // TODO: GL_DYNAMIC_DRAW;
	glBufferData(GL_ARRAY_BUFFER, bytes, reinterpret_cast<const void*>(dataDesc->pData), glBufferType); // send data to VRAM

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, dataDesc->positionStride, reinterpret_cast<const void*>((long long)dataDesc->positionOffset));
	glEnableVertexAttribArray(0);
	
	if (normals)
	{
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, dataDesc->normalStride, reinterpret_cast<const void*>((long long)dataDesc->normalOffset));
		glEnableVertexAttribArray(1);
	}
	
	if (texCoords)
	{
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, dataDesc->texCoordStride, reinterpret_cast<const void*>((long long)dataDesc->texCoordOffset));
		glEnableVertexAttribArray(2);
	}

	if (colors)
	{
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, dataDesc->colorStride, reinterpret_cast<const void*>((long long)dataDesc->colorOffset));
		glEnableVertexAttribArray(3);
	}
	
	if (indexes)
	{
		int idxSize = 0;
		switch (indexDesc->format)
		{
			case MESH_INDEX_FORMAT::INT32: idxSize = 4; break;
			case MESH_INDEX_FORMAT::INT16: idxSize = 2; break;
		}
		const int idxBytes = idxSize * indexDesc->number;

		glGenBuffers(1, &ibo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, idxBytes, reinterpret_cast<void*>(indexDesc->pData), glBufferType); // send data to VRAM
	}

	glBindVertexArray(0);

	CHECK_GL_ERRORS();

	GLMesh *pGLMesh = new GLMesh(vao, vbo, ibo, dataDesc->numberOfVertex, indexDesc->number, indexDesc->format, mode, attribs);
	*pMesh = pGLMesh;

	return S_OK;
}

API GLCoreRender::CreateShader(OUT ICoreShader **pShader, const char *vertText, const char *fragText, const char *geomText)
{
	GLuint vertID = 0;
	GLuint geomID = 0;
	GLuint fragID = 0;

	CHECK_GL_ERRORS();

	GLuint programID = glCreateProgram();
	
	if (!create_shader(vertID, GL_VERTEX_SHADER, vertText, programID))
	{
		glDeleteProgram(programID);
		return E_VERTEX_SHADER_FAILED_COMPILE;
	}
	
	if (geomText != nullptr)
	{
		if (!create_shader(geomID, GL_GEOMETRY_SHADER, geomText, programID))
		{
			glDeleteProgram(programID);
			glDeleteShader(vertID);
			return E_GEOM_SHADER_FAILED_COMPILE;
		}
	}
	
	if (!create_shader(fragID, GL_FRAGMENT_SHADER, fragText, programID))
	{
		glDeleteProgram(programID);
		glDeleteShader(vertID);
		if (geomID)
			glDeleteShader(geomID);
		return E_FRAGMENT_SHADER_FAILED_COMPILE;
	}

	CHECK_GL_ERRORS();

	GLShader *pGLShader = new GLShader(programID, vertID, geomID, fragID);
	*pShader = pGLShader;

	return S_OK;
}

void get_gl_formats(TEXTURE_FORMAT format, GLint& VRAMFormat, GLenum& sourceFormat, GLenum& sourceType)
{
	switch (format)
	{
	case TEXTURE_FORMAT::R8:		VRAMFormat = GL_R8;		sourceFormat = GL_RED;			sourceType = GL_UNSIGNED_BYTE; return;
	case TEXTURE_FORMAT::RG8:		VRAMFormat = GL_RG8;	sourceFormat = GL_RG;			sourceType = GL_UNSIGNED_BYTE; return;
	case TEXTURE_FORMAT::RGB8:		VRAMFormat = GL_RGB8;	sourceFormat = GL_RGB;			sourceType = GL_UNSIGNED_BYTE; return;
	case TEXTURE_FORMAT::RGBA8:		VRAMFormat = GL_RGBA8;	sourceFormat = GL_RGBA;			sourceType = GL_UNSIGNED_BYTE; return;
	case TEXTURE_FORMAT::R16F:		VRAMFormat = GL_R16F;	sourceFormat = GL_RED;			sourceType = GL_HALF_FLOAT; return;
	case TEXTURE_FORMAT::RG16F:		VRAMFormat = GL_RG16F;	sourceFormat = GL_RG;			sourceType = GL_HALF_FLOAT; return;
	case TEXTURE_FORMAT::RGB16F:	VRAMFormat = GL_RGB16F;	sourceFormat = GL_RGB;			sourceType = GL_HALF_FLOAT; return;
	case TEXTURE_FORMAT::RGBA16F:	VRAMFormat = GL_RGBA16F;sourceFormat = GL_RGBA;			sourceType = GL_HALF_FLOAT; return;
	case TEXTURE_FORMAT::R32F:		VRAMFormat = GL_R32F;	sourceFormat = GL_R;			sourceType = GL_FLOAT; return;
	case TEXTURE_FORMAT::RG32F:		VRAMFormat = GL_RG32F;	sourceFormat = GL_RG;			sourceType = GL_FLOAT; return;
	case TEXTURE_FORMAT::RGB32F:	VRAMFormat = GL_RGB32F;	sourceFormat = GL_RGB;			sourceType = GL_FLOAT; return;
	case TEXTURE_FORMAT::RGBA32F:	VRAMFormat = GL_RGBA32F;sourceFormat = GL_RGBA;			sourceType = GL_FLOAT; return;
	case TEXTURE_FORMAT::R32UI:		VRAMFormat = GL_R32UI;	sourceFormat = GL_RED_INTEGER;	sourceType = GL_UNSIGNED_INT; return;
	case TEXTURE_FORMAT::DXT1:		VRAMFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;	/* no sense */ return;
	case TEXTURE_FORMAT::DXT3:		VRAMFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;	/* no sense */ return;
	case TEXTURE_FORMAT::DXT5:		VRAMFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;	/* no sense */ return;
	case TEXTURE_FORMAT::D24S8:		VRAMFormat = GL_DEPTH24_STENCIL8; sourceFormat = GL_DEPTH_STENCIL; sourceType = GL_UNSIGNED_INT_24_8; return;
	}

	LOG_WARNING("get_gl_formats(): unknown format\n");
}

int byte_per_pixel(TEXTURE_FORMAT format)
{
	switch(format)
	{
		case TEXTURE_FORMAT::R8:		return 1;
		case TEXTURE_FORMAT::RG8:		return 2;
		case TEXTURE_FORMAT::RGB8:		return 3;
		case TEXTURE_FORMAT::RGBA8:		return 4;
		case TEXTURE_FORMAT::R16F:		return 1;
		case TEXTURE_FORMAT::RG16F:		return 2;
		case TEXTURE_FORMAT::RGB16F:	return 3;
		case TEXTURE_FORMAT::RGBA16F:	return 4;
		case TEXTURE_FORMAT::R32F:		return 4;
		case TEXTURE_FORMAT::RG32F:		return 8;
		case TEXTURE_FORMAT::RGB32F:	return 12;
		case TEXTURE_FORMAT::RGBA32F:	return 16;
		case TEXTURE_FORMAT::R32UI:		return 4;
		case TEXTURE_FORMAT::D24S8:		return 4;
		default: assert(false); // no sense
	}
	return 1;
}

int calculate_data_size(TEXTURE_FORMAT format, uint width, uint height)
{
	//TODO: support align
	if (is_compressed_format(format))
	{
		int blockSize;
		if (format == TEXTURE_FORMAT::DXT1)
			blockSize = 8;
		else
			blockSize = 16;
		if (width < 4 || height < 4) return blockSize;
		return (width / 4) * (height / 4) * blockSize;
	}

	int n = width * height * byte_per_pixel(format);
	return n;
}

API GLCoreRender::CreateTexture(OUT ICoreTexture **pTexture, uint8 *pData, uint width, uint height, TEXTURE_TYPE type, TEXTURE_FORMAT format, TEXTURE_CREATE_FLAGS flags, int mipmapsPresented)
{
	CHECK_GL_ERRORS();

	GLuint id;
	glGenTextures(1, &id);

	glBindTexture(GL_TEXTURE_2D, id);

	// filter
	{
		GLint glMinFilter;
		glMinFilter = GL_NEAREST;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, glMinFilter);
		CHECK_GL_ERRORS();

		GLint glMagFilter;
		glMagFilter = GL_NEAREST;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, glMagFilter);
		CHECK_GL_ERRORS();
	}

	// wrap
	{
		GLint glWrap;
		glWrap = GL_REPEAT;	
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, glWrap);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, glWrap);
		CHECK_GL_ERRORS();
	}

	GLint internalFormat;
	GLenum sourceFormat;
	GLenum sourceType;
	get_gl_formats(format, internalFormat, sourceFormat, sourceType);		

	if (is_compressed_format(format))
	{
		int dataSize = calculate_data_size(format, width, height);
		glCompressedTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataSize, pData);
	}
	else
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, sourceFormat, sourceType, pData);		

	CHECK_GL_ERRORS();

	glBindTexture(GL_TEXTURE_2D, 0);

	GLTexture *glTex = new GLTexture(id, format);
	*pTexture = glTex;

	CHECK_GL_ERRORS();	

	return S_OK;
}

API GLCoreRender::CreateRenderTarget(OUT ICoreRenderTarget **pRenderTarget)
{
	GLuint id;
	glGenFramebuffers(1, &id);
	glBindFramebuffer(GL_FRAMEBUFFER, id);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	*pRenderTarget = new GLRenderTarget(id);

	return S_OK;
}

API GLCoreRender::SetCurrentRenderTarget(IRenderTarget *pRenderTarget)
{
	_state.renderTarget = WRL::ComPtr<IRenderTarget>(pRenderTarget);

	GLRenderTarget *glRT = getGLRenderTarget(pRenderTarget);
	glBindFramebuffer(GL_FRAMEBUFFER, glRT->ID());
	glRT->bind();

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		if (status == GL_FRAMEBUFFER_UNSUPPORTED)
			LOG_FATAL("GLFrameBuffer::enable(): unsupported\n");
		else if (status == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT)
			LOG_FATAL("GLFrameBuffer::enable(): incomplete attachment\n");
		else if (status == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT)
			LOG_FATAL("GLFrameBuffer::enable(): incomplete missing attachment\n");
		else if (status == GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER)
			LOG_FATAL("GLFrameBuffer::enable(): incomplete draw buffer\n");
		else if (status == GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER)
			LOG_FATAL("GLFrameBuffer::enable(): incomplete read buffer\n");
		else if (status == GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE)
			LOG_FATAL("GLFrameBuffer::enable(): incomplete multisample\n");
		else if (status == GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS)
			LOG_FATAL("GLFrameBuffer::enable(): incomplete layer targets\n");
		else
			LOG_FATAL_FORMATTED("GLFrameBuffer::enable(): failed 0x%04X\n", status);
	}

	CHECK_GL_ERRORS();

	return S_OK;
}

API GLCoreRender::RestoreDefaultRenderTarget()
{
	_state.renderTarget = WRL::ComPtr<IRenderTarget>();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return S_OK;
}

API GLCoreRender::SetShader(IShader* pShader)
{
	if (_state.shader.Get() == pShader)
		return S_OK;

	_state.shader = WRL::ComPtr<IShader>(pShader);

	CHECK_GL_ERRORS();	
	
	if (pShader == nullptr)
		glUseProgram(0);
	else
	{
		GLShader *pGLShader = getGLShader(pShader);
		pGLShader->bind();
	}

	CHECK_GL_ERRORS();
			
	return S_OK;
}

API GLCoreRender::SetMesh(IMesh* mesh)
{
	if (_state.mesh.Get() == mesh)
		return S_OK;

	CHECK_GL_ERRORS();

	_state.mesh = WRL::ComPtr<IMesh>(mesh);

	if (mesh == nullptr)
		glBindVertexArray(0);
	else
	{
		const GLMesh *glMesh = getGLMesh(mesh);
		glBindVertexArray(glMesh->VAO_ID());
	}

	CHECK_GL_ERRORS();

	return S_OK;
}

API GLCoreRender::Draw(IMesh *mesh)
{
	assert(_state.shader.Get() && "GLCoreRender::Draw(): shader not set");

	CHECK_GL_ERRORS();

	if (_state.mesh.Get() != mesh)
		SetMesh(mesh);

	uint vertecies;
	mesh->GetNumberOfVertex(&vertecies);

	VERTEX_TOPOLOGY topology;
	mesh->GetVertexTopology(&topology);

	// glDrawElements - for index! Why it works?
	glDrawArrays((topology == VERTEX_TOPOLOGY::TRIANGLES) ? GL_TRIANGLES : GL_LINES, 0, vertecies);
	
	CHECK_GL_ERRORS();
	
	return S_OK;
}

API GLCoreRender::SetDepthState(int enabled)
{
	CHECK_GL_ERRORS();

	if (bool(enabled) == bool(_state.depthTest))
		return S_OK;

	if (enabled)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);

	_state.depthTest = enabled;

	CHECK_GL_ERRORS();

	return S_OK;
}

API GLCoreRender::SetViewport(uint wIn, uint hIn)
{
	if (wIn == _state.viewportWidth && hIn == _state.viewportHeigth) 
		return S_OK;

	glViewport(0, 0, wIn, hIn);

	_state.viewportWidth = wIn;
	_state.viewportHeigth = hIn;

	return S_OK;
}

API GLCoreRender::GetViewport(OUT uint* wOut, OUT uint* hOut)
{
	CHECK_GL_ERRORS();

	*wOut = _state.viewportWidth;
	*hOut = _state.viewportHeigth;

	CHECK_GL_ERRORS();
	return S_OK;
}

API GLCoreRender::Clear()
{
	CHECK_GL_ERRORS();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	CHECK_GL_ERRORS();
	return S_OK;
}

API GLCoreRender::ReadPixel2D(ICoreTexture *tex, OUT void *out, OUT uint *readPixel, uint x, uint y)
{
	GLTexture *glTex = static_cast<GLTexture*>(tex);

	TEXTURE_FORMAT format;
	glTex->GetFormat(&format);

	glBindTexture(GL_TEXTURE_2D, glTex->textureID());

	GLint internalFormat;
	GLenum sourceFormat;
	GLenum sourceType;
	get_gl_formats(format, internalFormat, sourceFormat, sourceType);

	uint w, h;
	glTex->GetWidth(&w);
	glTex->GetHeight(&h);

	int pixelBytes = byte_per_pixel(format);
	int allBytes = w * h * pixelBytes;

	uint8 *p = new uint8[allBytes];

	// Inefficient! Don't use every frame because We send all texture
	glGetnTexImage(GL_TEXTURE_2D, 0, sourceFormat, sourceType, allBytes, p);

	// Reverese by Y for for conformity directx
	y = h - y;

	uint8 *p_read  = p + (y * w + x) * pixelBytes; 

	memcpy(out, p_read, pixelBytes);
	*readPixel = pixelBytes;

	delete[] p;

	return S_OK;
}

GLRenderTarget::GLRenderTarget(GLuint idIn) : _ID(idIn)
{
	for (int i = 0; i < 8; i++)
		_colors[i] = 0u;
}

void GLRenderTarget::bind()
{
	CHECK_GL_ERRORS();

	//int targets;
	for (int i = 0; i < 8; i++)
	{
		if (!_colors[i])
			break;
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, _colors[i], 0);
		//targets++;
	}

	CHECK_GL_ERRORS();

	if (_depth)
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, _depth, 0);

	CHECK_GL_ERRORS();
}

API GLRenderTarget::SetColorTexture(uint slot, ITexture *tex)
{
	assert(slot < 8 && "GLRenderTarget::SetColorTexture() slot must be 0..7");

	if (tex)
	{
		ICoreTexture *coreTex;
		tex->GetCoreTexture(&coreTex);
		GLTexture *glTex = static_cast<GLTexture*>(coreTex);

		_colors[slot] = glTex->textureID();
	} else
	{
		if (_colors[slot])
		{
			glBindFramebuffer(GL_FRAMEBUFFER, _ID);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + slot, GL_TEXTURE_2D, 0, 0);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
		_colors[slot] = 0u;
	}
	
	return S_OK;
}

API GLRenderTarget::SetDepthTexture(ITexture *tex)
{
	if (tex)
	{
		ICoreTexture *coreTex;
		tex->GetCoreTexture(&coreTex);
		GLTexture *glTex = static_cast<GLTexture*>(coreTex);	
		_depth = glTex->textureID();
	} else
	{
		if (_depth)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, _ID);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, 0, 0);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
		_depth = 0u;
	}

	return S_OK;
}

API GLRenderTarget::UnbindColorTexture(uint slot)
{
	assert(slot < 8 && "GLRenderTarget::UnbindColorTexture() slot must be 0..7");

	glBindFramebuffer(GL_FRAMEBUFFER, _ID);
	_colors[slot] = 0;
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + slot, GL_TEXTURE_2D, 0, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	CHECK_GL_ERRORS();

	return S_OK;
}

API GLRenderTarget::UnbindAll()
{
	glBindFramebuffer(GL_FRAMEBUFFER, _ID);
	for (int i = 0; i < 8; i++)
	{
		_colors[i] = 0;
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, 0, 0);
	}
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,  0, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	CHECK_GL_ERRORS();

	return S_OK;
}
