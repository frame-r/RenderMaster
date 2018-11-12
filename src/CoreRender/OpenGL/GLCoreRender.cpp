#include "pch.h"
#include "GLCoreRender.h"
#include "Core.h"
#include "ResourceManager.h"
#include "GLShader.h"
#include "GLMesh.h"

using string = string;

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

namespace
{
	PIXELFORMATDESCRIPTOR pfd{};
};

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

		LOG_FATAL(error_message);

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

API GLCoreRender::Init(const WindowHandle* handle, int MSAASamples)
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

	glEnable(GL_DEPTH_TEST);
	glClearDepth(1.0f);
	glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE); // to DirectX conformity

	glDisable(GL_CULL_FACE);

	// Fill state
	//
	_currentState.blending_enabled = 0;
	GLint blend_factor;
	glGetIntegerv(GL_BLEND_SRC_RGB, &blend_factor);
	_currentState.src_blend_factor = blend_factor;
	glGetIntegerv(GL_BLEND_DST_ALPHA, &blend_factor);
	_currentState.src_blend_factor = blend_factor;

	glGetFloatv(GL_COLOR_CLEAR_VALUE, &_currentState.clear_color[0]);

	GLint i[2];
	glGetIntegerv(GL_POLYGON_MODE, i);
	_currentState.poligon_mode = i[0];

	_currentState.culling_enabled = glIsEnabled(GL_CULL_FACE);
	glGetIntegerv(GL_CULL_FACE_MODE, &_currentState.culling_mode);

	glGetBooleanv(GL_DEPTH_TEST, &_currentState.depth_test_enabled);

	GLint vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);
	_currentState.viewport_x = vp[0];
	_currentState.viewport_y = vp[1];
	_currentState.viewport_w = vp[2];
	_currentState.viewport_h = vp[3];

	CHECK_GL_ERRORS();

	LOG("GLCoreRender initalized");

	return S_OK;
}

API GLCoreRender::Free()
{
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

API GLCoreRender::PushStates()
{
	_statesStack.push(_currentState);
	return S_OK;
}

API GLCoreRender::PopStates()
{
	State& state = _statesStack.top();
	_statesStack.pop();

	if (_currentState.blending_enabled != state.blending_enabled)
		if (state.blending_enabled)
			glEnable(GL_BLEND);
		else
			glDisable(GL_BLEND);

	if (_currentState.src_blend_factor != state.src_blend_factor || _currentState.dst_blend_factor != state.dst_blend_factor)
		glBlendFunc(state.src_blend_factor, state.dst_blend_factor);

	if (state.shader_program_id != _currentState.shader_program_id)
		glUseProgram(state.shader_program_id);

	for (int i = 0; i < 16; i++)
	{
		if (state.tex_slots_bindings[i].tex_id != _currentState.tex_slots_bindings[i].tex_id ||
			state.tex_slots_bindings[i].shader_variable_id != _currentState.tex_slots_bindings[i].shader_variable_id)
		{
			// TODO: make other types (not only GL_TEXTURE_2D)!
			glActiveTexture(GL_TEXTURE0 + i);									// now work with slot == i
			glBindTexture(GL_TEXTURE_2D, state.tex_slots_bindings[i].tex_id);	// slot <- texture id
			glUniform1i(state.tex_slots_bindings[i].shader_variable_id, i);			// slot <- shader variable id
		}
	}

	if (state.clear_color[0] != _currentState.clear_color[0] ||
		state.clear_color[1] != _currentState.clear_color[1] ||
		state.clear_color[2] != _currentState.clear_color[2] ||
		state.clear_color[3] != _currentState.clear_color[3])
		glClearColor(state.clear_color[0], state.clear_color[1], state.clear_color[2], state.clear_color[3]);

	if (state.poligon_mode != _currentState.poligon_mode)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	if (state.culling_enabled != _currentState.culling_enabled)
	{
		if (state.culling_enabled)
			glEnable(GL_CULL_FACE);
		else
			glDisable(GL_CULL_FACE);
	}
	if (state.culling_mode != _currentState.culling_mode)
		glCullFace(state.culling_mode);

	if (state.depth_test_enabled != _currentState.depth_test_enabled)
	{
		if (state.depth_test_enabled)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);
	}

	if (state.viewport_x != _currentState.viewport_x ||
		state.viewport_y != _currentState.viewport_y ||
		state.viewport_w != _currentState.viewport_w ||
		state.viewport_h != _currentState.viewport_h)
		glViewport(state.viewport_x, state.viewport_y, state.viewport_w, state.viewport_h);

	_currentState = state;

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

API GLCoreRender::CreateShader(OUT ICoreShader **pShader, const char *vert, const char *frag, const char *geom)
{
	GLuint vertID = 0;
	GLuint geomID = 0;
	GLuint fragID = 0;

	CHECK_GL_ERRORS();

	GLuint programID = glCreateProgram();
	
	if (!create_shader(vertID, GL_VERTEX_SHADER, vert, programID))
	{
		glDeleteProgram(programID);
		return S_FALSE;
	}
	
	if (geom != nullptr)
	{
		if (!create_shader(geomID, GL_GEOMETRY_SHADER, geom, programID))
		{
			glDeleteProgram(programID);
			glDeleteShader(vertID);
			return S_FALSE;
		}
	}
	
	if (!create_shader(fragID, GL_FRAGMENT_SHADER, frag, programID))
	{
		glDeleteProgram(programID);
		glDeleteShader(vertID);
		if (geomID)
			glDeleteShader(geomID);
		return S_FALSE;
	}

	CHECK_GL_ERRORS();

	GLShader *pGLShader = new GLShader(programID, vertID, geomID, fragID);
	*pShader = pGLShader;

	return S_OK;
}

API GLCoreRender::CreateUniformBuffer(OUT IUniformBuffer **pBuffer, uint size)
{
	GLuint ubo = 0;
	glGenBuffers(1, &ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, ubo);
	vector<char> data(size, '\0');
	glBufferData(GL_UNIFORM_BUFFER, size, &data[0], GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	*pBuffer = static_cast<IUniformBuffer*>(new GLUniformBuffer(ubo, size));

	return S_OK;
}

API GLCoreRender::CreateTexture(OUT ICoreTexture ** pTexture, uint8 * pData, uint width, uint height, TEXTURE_TYPE type, TEXTURE_FORMAT format, TEXTURE_CREATE_FLAGS flags, int mipmapsPresented)
{
	return E_NOTIMPL;
}

API GLCoreRender::SetShader(const ICoreShader* pShader)
{
	CHECK_GL_ERRORS();

	const GLShader *pGLShader = dynamic_cast<const GLShader*>(pShader);
	
	if (pShader == nullptr)
	{
		if (_currentState.shader_program_id != 0u)
		{
			glUseProgram(0);
			_currentState.shader_program_id = 0u;
			return S_OK;
		}
	}

	if (_currentState.shader_program_id == pGLShader->programID())
		return S_OK;
	
	glUseProgram(pGLShader->programID());
	_currentState.shader_program_id = pGLShader->programID();

	CHECK_GL_ERRORS();
			
	return S_OK;
}

API GLCoreRender::SetMesh(const ICoreMesh* mesh)
{
	CHECK_GL_ERRORS();

	if (mesh == nullptr)
		glBindVertexArray(0);
	else
	{
		const GLMesh *glMesh = reinterpret_cast<const GLMesh*>(mesh);
		glBindVertexArray(glMesh->VAO_ID());
	}

	CHECK_GL_ERRORS();

	return S_OK;
}

API GLCoreRender::SetUniformBuffer(const IUniformBuffer *pBuffer, uint slot)
{
	assert(_currentState.shader_program_id != 0 && "GLCoreRender::SetUniformBuffer(): shader not set");

	CHECK_GL_ERRORS();

	// uniform buffer -> UBO slot
	const GLUniformBuffer *glBuf = reinterpret_cast<const GLUniformBuffer*>(pBuffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, slot, glBuf->ID());

	// shader -> UBO slot
	string s = "const_buffer_" + std::to_string(slot);
	unsigned int block_index = glGetUniformBlockIndex(_currentState.shader_program_id, s.c_str());
	glUniformBlockBinding(_currentState.shader_program_id, block_index, slot);

	CHECK_GL_ERRORS();

	return S_OK;
}

API GLCoreRender::SetUniformBufferData(IUniformBuffer *pBuffer, const void *pData)
{
	CHECK_GL_ERRORS();

	const GLUniformBuffer *glBuf = reinterpret_cast<const GLUniformBuffer*>(pBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, glBuf->ID());
	GLvoid* p = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
	memcpy(p, pData, glBuf->size());
	glUnmapBuffer(GL_UNIFORM_BUFFER);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	CHECK_GL_ERRORS();

	return S_OK;
}

API GLCoreRender::Draw(ICoreMesh *mesh)
{
	assert(_currentState.shader_program_id != 0 && "GLCoreRender::SetUniformBuffer(): shader not set");

	CHECK_GL_ERRORS();

	if (!mesh)
		glBindVertexArray(0);
	else
	{
		GLMesh *pGLMesh = reinterpret_cast<GLMesh*>(mesh);
		glBindVertexArray(pGLMesh->VAO_ID());

		uint vertecies;
		mesh->GetNumberOfVertex(&vertecies);

		VERTEX_TOPOLOGY topology;
		mesh->GetVertexTopology(&topology);

		// glDrawElements - for index! Why it works?
		glDrawArrays((topology == VERTEX_TOPOLOGY::TRIANGLES) ? GL_TRIANGLES : GL_LINES, 0, vertecies);
	}
	
	CHECK_GL_ERRORS();
	
	return S_OK;
}

API GLCoreRender::SetDepthState(int enabled)
{
	CHECK_GL_ERRORS();

	if (bool(enabled) == bool(_currentState.depth_test_enabled))
		return S_OK;

	if (enabled)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);

	_currentState.depth_test_enabled = enabled;

	CHECK_GL_ERRORS();

	return S_OK;
}

API GLCoreRender::SetViewport(uint wIn, uint hIn)
{
	if (wIn == _currentState.viewport_w && hIn == _currentState.viewport_h) 
		return S_OK;

	glViewport(0, 0, wIn, hIn);

	_currentState.viewport_w = wIn;
	_currentState.viewport_h = hIn;

	return S_OK;
}

API GLCoreRender::GetViewport(OUT uint* wOut, OUT uint* hOut)
{
	CHECK_GL_ERRORS();

	*wOut = _currentState.viewport_w;
	*hOut = _currentState.viewport_h;

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

API GLUniformBuffer::Free()
{
	if (_UBO) glDeleteBuffers(1, &_UBO);
	_UBO = 0;
	IResourceManager *rm = getResourceManager(_pCore);
	rm->RemoveUniformBuffer(this);

	return S_OK;
}
