#include "GLCoreRender.h"

#include <sstream>

#include <GL\glew.h>
#include <GL\wglew.h>

#include "Core.h"
#include "ResourceManager.h"
#include "GLShader.h"
#include "GLMesh.h"

using string = std::string;

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

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

bool GLCoreRender::_check_shader_errors(int id, GLenum constant)
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

bool GLCoreRender::_create_shader(GLuint& id, GLenum type, const char** pText, int numLines, GLuint programID)
{
	GLuint _id = glCreateShader(type);
	glShaderSource(_id, numLines, pText, nullptr);
	glCompileShader(_id);

	if (!_check_shader_errors(_id, GL_COMPILE_STATUS))
	{
		glDeleteShader(_id);
		return false;
	}
	glAttachShader(programID, _id);
	glLinkProgram(programID);
	if (!_check_shader_errors(programID, GL_LINK_STATUS))
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

API GLCoreRender::Init(const WinHandle* handle)
{
	const int major_version = 4;
	const int minor_version = 5;

	_pCore->GetSubSystem((ISubSystem*&)_pResMan, SUBSYSTEM_TYPE::RESOURCE_MANAGER);

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
			char _log_fbx_buffer[sizeof(OGLI) + 4];
			glGetIntegerv(GL_MAJOR_VERSION, &major);
			glGetIntegerv(GL_MINOR_VERSION, &minor);
			sprintf_s(_log_fbx_buffer, OGLI"%i.%i", major, minor);

			LOG(_log_fbx_buffer);
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
	glDisable(GL_CULL_FACE);

	glEnable(GL_DEPTH_TEST);
	glClearDepth(1.0f);

	CHECK_GL_ERRORS();

	LOG("GLCoreRender initalized");

	return S_OK;
}

API GLCoreRender::CreateMesh(ICoreMesh *&pMesh, const MeshDataDesc &dataDesc, const MeshIndexDesc &indexDesc, VERTEX_TOPOLOGY mode)
{
	const int indexes = indexDesc.format != MESH_INDEX_FORMAT::NOTHING;
	const int normals = dataDesc.normalsPresented;
	const int texCoords = dataDesc.texCoordPresented;
	const int colors = dataDesc.colorPresented;
	const int bytes = (12 + texCoords * 8 + normals * 12 + colors * 12) * dataDesc.numberOfVertex;	
	GLuint vao = 0, vbo = 0, ibo = 0;

	CHECK_GL_ERRORS();

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	const GLenum glBufferType = GL_STATIC_DRAW; // TODO: GL_DYNAMIC_DRAW;
	glBufferData(GL_ARRAY_BUFFER, bytes, reinterpret_cast<const void*>(dataDesc.pData), glBufferType); // send data to VRAM

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, dataDesc.positionStride, reinterpret_cast<const void*>((long long)dataDesc.positionOffset));
	glEnableVertexAttribArray(0);
	
	if (normals)
	{
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, dataDesc.normalStride, reinterpret_cast<const void*>((long long)dataDesc.normalOffset));
		glEnableVertexAttribArray(1);
	}
	
	if (texCoords)
	{
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, dataDesc.texCoordStride, reinterpret_cast<const void*>((long long)dataDesc.texCoordOffset));
		glEnableVertexAttribArray(2);
	}

	if (colors)
	{
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, dataDesc.colorStride, reinterpret_cast<const void*>((long long)dataDesc.colorOffset));
		glEnableVertexAttribArray(3);
	}
	
	if (indexes)
	{
		int idxSize = 0;
		switch (indexDesc.format)
		{
			case MESH_INDEX_FORMAT::INT32: idxSize = 32; break;
			case MESH_INDEX_FORMAT::INT16: idxSize = 16; break;
		}
		const int idxBytes = idxSize * indexDesc.number;

		glGenBuffers(1, &ibo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, idxBytes, reinterpret_cast<void*>(indexDesc.pData), glBufferType); // send data to VRAM
	}

	glBindVertexArray(0);

	CHECK_GL_ERRORS();

	INPUT_ATTRUBUTE a = INPUT_ATTRUBUTE::POSITION;
	if (dataDesc.normalsPresented)
		a = a | INPUT_ATTRUBUTE::NORMAL;
	if (dataDesc.texCoordPresented)
		a = a | INPUT_ATTRUBUTE::TEX_COORD;
	if (dataDesc.colorPresented)
		a = a | INPUT_ATTRUBUTE::COLOR;

	GLMesh *pGLMesh = new GLMesh(vao, vbo, ibo, dataDesc.numberOfVertex, indexDesc.number, indexDesc.format, mode, a);
	pMesh = pGLMesh;

	return S_OK;
}

API GLCoreRender::CreateShader(ICoreShader*& pShader, const ShaderText& shaderDesc)
{
	GLuint vertID = 0;
	GLuint geomID = 0;
	GLuint fragID = 0;

	CHECK_GL_ERRORS();

	GLuint programID = glCreateProgram();
	
	if (!_create_shader(vertID, GL_VERTEX_SHADER, shaderDesc.pVertText, shaderDesc.vertNumLines, programID))
	{
		glDeleteProgram(programID);
		return S_FALSE;
	}
	
	if (shaderDesc.pGeomText != nullptr && shaderDesc.geomNumLines > 0)
	{
		if (!_create_shader(geomID, GL_GEOMETRY_SHADER, shaderDesc.pGeomText, shaderDesc.geomNumLines, programID))
		{
			glDeleteProgram(programID);
			return S_FALSE;
		}
	}
	
	if (!_create_shader(fragID, GL_FRAGMENT_SHADER, shaderDesc.pFragText, shaderDesc.fragNumLines, programID))
	{
		glDeleteProgram(programID);
		return S_FALSE;
	}

	CHECK_GL_ERRORS();

	GLShader *pGLShader = new GLShader(programID, vertID, geomID, fragID);
	pShader = pGLShader;

	return S_OK;
}

API GLCoreRender::SetShader(const ICoreShader* pShader)
{
	if (_current_shader == pShader) return S_OK;

	CHECK_GL_ERRORS();
	
	if (!pShader)
		glUseProgram(0);
	else
	{
		const GLShader *pGLShader = reinterpret_cast<const GLShader*>(pShader);
		glUseProgram(pGLShader->programID());
	}
	
	_current_shader = pShader;

	CHECK_GL_ERRORS();
	
	return S_OK;
}

API GLCoreRender::SetUniform(const char* name, const void* pData, const ICoreShader* pShader, SHADER_VARIABLE_TYPE type)
{
	CHECK_GL_ERRORS();

	const GLShader *pGLShader = reinterpret_cast<const GLShader*>(pShader);
	const GLuint ID = glGetUniformLocation(pGLShader->programID(), name);

	switch (type)
	{

	case SHADER_VARIABLE_TYPE::SVT_INT:
		{
			const int *i = reinterpret_cast<const int*>(pData);
			glUniform1i(ID, *i);
		}
		break;

	case SHADER_VARIABLE_TYPE::SVT_FLOAT:
		{
			const float *f = reinterpret_cast<const float*>(pData);
			glUniform1f(ID, *f);
		}
		break;
	case SHADER_VARIABLE_TYPE::SVT_VECTOR3:
		{
			const Vector3 *v3 = reinterpret_cast<const Vector3*>(pData);
			glUniform3f(ID, v3->x, v3->y, v3->z);
		}
		break;
	case SHADER_VARIABLE_TYPE::SVT_VECTOR4:
		{
			const Vector4 *v4 = reinterpret_cast<const Vector4*>(pData);
			glUniform4f(ID, v4->x, v4->y, v4->z, v4->w);
		}
		break;

	case SHADER_VARIABLE_TYPE::SVT_MATRIX3X3:
		{
			const Matrix3x3 *m3 = reinterpret_cast<const Matrix3x3*>(pData);
			glUniformMatrix3fv(ID, 1, GL_TRUE, &m3->el_1D[0]);
		}
		break;

	case SHADER_VARIABLE_TYPE::SVT_MATRIX4X4:
		{
			const Matrix4x4 *m4 = reinterpret_cast<const Matrix4x4*>(pData);
			glUniformMatrix4fv(ID, 1, GL_TRUE, &m4->el_1D[0]);
		}
		break;

	default:
		LOG_WARNING("GLCoreRender::SetUniform(): unknown shader variable type");
		return S_FALSE;


	//case SHADER_VARIABLE_TYPE::SVT_TEXTURE:
	//	tex = reinterpret_cast<const Texture*>(pData);
	//	glActiveTexture(GL_TEXTURE0 + tex_sampler); // activate next avaliable texture unit
	//	glBindTexture(GL_TEXTURE_2D, tex->ID); // attach texture to texture unit as GL_TEXTURE_2D
	//	glUniform1i(ID, tex_sampler);
	//	//_freeTextureUnit++;
	//	break;

	//case SVT_TEXTURE3D:
	//	ERR_GUARDS();
	//	tex = reinterpret_cast<const Texture*>(pData);
	//	glActiveTexture(GL_TEXTURE0 + tex_sampler);
	//	glBindTexture(GL_TEXTURE_3D, tex->ID);
	//	glUniform1i(ID, tex_sampler);
	//	ERR_GUARDS();
	//	break;

	}

	CHECK_GL_ERRORS();

	return S_OK;
}

API GLCoreRender::SetUniformArray(const char* name, const void* pData, const ICoreShader* pShader, SHADER_VARIABLE_TYPE type, uint number)
{
	CHECK_GL_ERRORS();

	const GLShader *pGLShader = reinterpret_cast<const GLShader*>(pShader);
	const GLuint ID = glGetUniformLocation(pGLShader->programID(), name);

	switch (type)
	{

	case SHADER_VARIABLE_TYPE::SVT_INT:
	{
		const int *i = reinterpret_cast<const int*>(pData);
		glUniform1iv(ID, number, i);
	}
	break;

	case SHADER_VARIABLE_TYPE::SVT_FLOAT:
	{
		const float *f = reinterpret_cast<const float*>(pData);
		glUniform1fv(ID, number, f);
	}
	break;
	case SHADER_VARIABLE_TYPE::SVT_VECTOR3:
	{
		const Vector3 *v3 = reinterpret_cast<const Vector3*>(pData);
		glUniform3fv(ID, number, &v3->x);
	}
	break;
	case SHADER_VARIABLE_TYPE::SVT_VECTOR4:
	{
		const Vector4 *v4 = reinterpret_cast<const Vector4*>(pData);
		glUniform4fv(ID, number, &v4->x);
	}
	break;

	case SHADER_VARIABLE_TYPE::SVT_MATRIX3X3:
	{
		const Matrix3x3 *m3 = reinterpret_cast<const Matrix3x3*>(pData);
		glUniformMatrix3fv(ID, number, GL_TRUE, &m3->el_1D[0]);
	}
	break;

	case SHADER_VARIABLE_TYPE::SVT_MATRIX4X4:
	{
		const Matrix4x4 *m4 = reinterpret_cast<const Matrix4x4*>(pData);
		glUniformMatrix4fv(ID, number, GL_TRUE, &m4->el_1D[0]);
	}
	break;

	default:
		LOG_FATAL("GLCoreRender::SetUniformArray(): unknown shader variable typre");
		return S_FALSE;
	}

	CHECK_GL_ERRORS();

	return S_OK;
}

API GLCoreRender::SetMesh(const ICoreMesh* mesh)
{
	CHECK_GL_ERRORS();

	if (!mesh)
		glBindVertexArray(0);
	else
	{
		const GLMesh *glMesh = reinterpret_cast<const GLMesh*>(mesh);
		glBindVertexArray(glMesh->VAO_ID());
	}

	CHECK_GL_ERRORS();

	return S_OK;
}

API GLCoreRender::Draw(ICoreMesh *mesh)
{
	CHECK_GL_ERRORS();
	if (!mesh)
		glBindVertexArray(0);
	else
	{
		GLMesh *pGLMesh = reinterpret_cast<GLMesh*>(mesh);
		glBindVertexArray(pGLMesh->VAO_ID());

		uint vertecies;
		mesh->GetNumberOfVertex(vertecies);

		VERTEX_TOPOLOGY topology;
		mesh->GetVertexTopology(topology);

		if (topology == VERTEX_TOPOLOGY::TRIANGLES)
			glDrawArrays(GL_TRIANGLES, 0, vertecies);
		else
			glDrawArrays(GL_LINES, 0, vertecies);
	}
	CHECK_GL_ERRORS();
	return S_OK;
}

API GLCoreRender::SetDepthState(int enabled)
{
	CHECK_GL_ERRORS();

	if (enabled)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);

	CHECK_GL_ERRORS();

	return S_OK;
}

API GLCoreRender::SetViewport(uint w, uint h)
{
	glViewport(0, 0, w, h);
	return S_OK;
}

API GLCoreRender::Clear()
{
	CHECK_GL_ERRORS();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	CHECK_GL_ERRORS();
	return S_OK;
}

API GLCoreRender::SwapBuffers()
{
	CHECK_GL_ERRORS();
	::SwapBuffers(_hdc);
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
