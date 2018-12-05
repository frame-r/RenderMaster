#include "pch.h"
#include "GLShader.h"
#include "GLCoreRender.h"
#include "Core.h"

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

extern vector<UBO> UBOpool;

GLShader::GLShader(GLuint programID, GLuint vertID, GLuint geomID, GLuint fragID) :
	_programID(programID), _vertID(vertID), _geomID(geomID), _fragID(fragID)
{
	// Get all unoform variables from all Uniform Buffer Objects for current shader

	glUseProgram(_programID);

	GLint numUBO = 0;
	glGetProgramiv(_programID, GL_ACTIVE_UNIFORM_BLOCKS, &numUBO);

	for (int i = 0; i < numUBO; i++)
	{
		char nameUBO[1024];
		GLint bytesUBO;
		GLint numIndices, indicesArray[2048];

		glGetActiveUniformBlockName(_programID, i, sizeof(nameUBO), NULL, nameUBO);
		glGetActiveUniformBlockiv(_programID, i, GL_UNIFORM_BLOCK_DATA_SIZE, &bytesUBO);
		glGetActiveUniformBlockiv(_programID, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &numIndices);

		if (numIndices == 0)
			continue;

		glGetActiveUniformBlockiv(_programID, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, indicesArray);

		vector<UBO::UBOParameter> parametersUBO;

		// active UBO parameters
		for (int j = 0; j < numIndices; j++)
		{
			GLuint index = (GLuint)indicesArray[j];

			uint bytesVariable;

			GLint elementsNum;
			GLenum typeVariable;
			char nameVariable[1024];
			glGetActiveUniform(_programID, index, sizeof(nameVariable), NULL, &elementsNum, &typeVariable, nameVariable);
			
			GLint offsetVariable;						
			glGetActiveUniformsiv(_programID, 1, &index, GL_UNIFORM_OFFSET, &offsetVariable);

			char *s = strchr(nameVariable, '[');
			if (s != nullptr)
				*s = '\0';

			switch (typeVariable)
			{
				case GL_FLOAT:				bytesVariable = elementsNum * 4;	break;
				case GL_FLOAT_VEC2:			bytesVariable = elementsNum * 8;	break;
				case GL_FLOAT_VEC3:			bytesVariable = elementsNum * 12;	break;
				case GL_FLOAT_VEC4:			bytesVariable = elementsNum * 16;	break;
				case GL_DOUBLE:				bytesVariable = elementsNum * 8;	break;
				case GL_DOUBLE_VEC2:		bytesVariable = elementsNum * 16;	break;
				case GL_DOUBLE_VEC3:		bytesVariable = elementsNum * 24;	break;
				case GL_DOUBLE_VEC4:		bytesVariable = elementsNum * 32;	break;
				case GL_INT:				bytesVariable = elementsNum * 4;	break;
				case GL_INT_VEC2:			bytesVariable = elementsNum * 8;	break;
				case GL_INT_VEC3:			bytesVariable = elementsNum * 12;	break;
				case GL_INT_VEC4:			bytesVariable = elementsNum * 16;	break;
				case GL_UNSIGNED_INT:		bytesVariable = elementsNum * 4;	break;
				case GL_UNSIGNED_INT_VEC2:	bytesVariable = elementsNum * 8;	break;
				case GL_UNSIGNED_INT_VEC3:	bytesVariable = elementsNum * 12;	break;
				case GL_UNSIGNED_INT_VEC4:	bytesVariable = elementsNum * 16;	break;
				case GL_BOOL_VEC2:			bytesVariable = elementsNum * 8;	break;
				case GL_BOOL_VEC3:			bytesVariable = elementsNum * 12;	break;
				case GL_BOOL_VEC4:			bytesVariable = elementsNum * 16;	break;
				case GL_FLOAT_MAT3:			bytesVariable = elementsNum * 36;	break;
				case GL_FLOAT_MAT4:			bytesVariable = elementsNum * 64;	break;
				case GL_DOUBLE_MAT3:		bytesVariable = elementsNum * 72;	break;
				case GL_DOUBLE_MAT4:		bytesVariable = elementsNum * 128;	break;
				default: assert(0 && "GLShader::GLShader(): unknown parameter type");
			}


			UBO::UBOParameter p;
			p.name = nameVariable;
			p.bytes = bytesVariable;
			p.offset = offsetVariable;
			p.elements = elementsNum;

			parametersUBO.push_back(p);
		}

		// sort by offset for pretty view
		#ifdef _DEBUG
			std::sort(parametersUBO.begin(), parametersUBO.end(), [](const UBO::UBOParameter& a, const UBO::UBOParameter& b) -> bool
			{ 
				return a.offset < b.offset; 
			});
		#endif

		// find existing buffer with same memory layout
		int indexFound = -1;
		{
			for (int j = 0; indexFound == -1 && j < UBOpool.size(); j++)
			{
				bool isEqual = true;

				if (UBOpool[j].name != nameUBO)
					isEqual = false;

				if (UBOpool[j].bytes != bytesUBO)
					isEqual = false;

				if (UBOpool[j].parameters.size() != parametersUBO.size())
					isEqual = false;

				for (int k = 0; isEqual && k < parametersUBO.size(); k++)
				{
					if (UBOpool[j].parameters[k].name != parametersUBO[k].name ||
						UBOpool[j].parameters[k].bytes != parametersUBO[k].bytes ||
						UBOpool[j].parameters[k].offset != parametersUBO[k].offset ||
						UBOpool[j].parameters[k].elements != parametersUBO[k].elements)
					{
						isEqual = 0;
					}
				}

				if (isEqual)
					indexFound = j;
			}
		}		

		if (indexFound != -1) // buffer found
		{
			_bufferIndicies.push_back(indexFound);

			for (int i = 0; i < parametersUBO.size(); i++)
			{
				_parameters[parametersUBO[i].name] = {(int)indexFound, (int)i};
			}
		} else // not found => create new
		{
			GLuint id;

			glGenBuffers(1, &id);
			glBindBuffer(GL_UNIFORM_BUFFER, id);
			vector<char> data(bytesUBO, '\0');
			glBufferData(GL_UNIFORM_BUFFER, bytesUBO, &data[0], GL_DYNAMIC_DRAW);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);

			_bufferIndicies.push_back(UBOpool.size());

			for (int i = 0; i < parametersUBO.size(); i++)
			{
				_parameters[parametersUBO[i].name] = {(int)UBOpool.size(), (int)i};
			}

			UBOpool.emplace_back(std::move(UBO(id, bytesUBO, nameUBO, parametersUBO)));			
		}
	}
}

GLShader::~GLShader()
{
	if (_vertID != 0) {	glDeleteShader(_vertID); _vertID = 0; }
	if (_fragID != 0) {	glDeleteShader(_fragID); _fragID = 0; }
	if (_geomID != 0) { glDeleteShader(_geomID); _geomID = 0; }
	if (_programID != 0) { glDeleteProgram(_programID); _programID = 0; }
}

void GLShader::setParameter(const char *name, const void *data)
{
	auto it = _parameters.find(name);
	if (it == _parameters.end())
	{
		auto &p = _parameters[name];
		LOG_WARNING_FORMATTED("GLShader::setParameter() unable find parameter \"%s\"", name);
	}

	Parameter &p = _parameters[name];

	if (p.bufferIndex < 0 || p.parameterIndex < 0)
		return;

	UBO &ubo = UBOpool[p.bufferIndex];
	UBO::UBOParameter &pUBO = ubo.parameters[p.parameterIndex];
	uint8 *pointer = ubo.data.get() + pUBO.offset;
	if (memcmp(pointer, data, pUBO.bytes))
	{
		memcpy(pointer, data, pUBO.bytes);
		ubo.needFlush = true;
	}
}

void GLShader::bind()
{
	glUseProgram(_programID);
	for (int i = 0; i < _bufferIndicies.size(); i++)
	{
		UBO &ubo = UBOpool[_bufferIndicies[i]];
		glBindBufferBase(GL_UNIFORM_BUFFER, i, ubo._ID);
		glUniformBlockBinding(_programID, i, i);
	}
}

API GLShader::SetFloatParameter(const char *name, float value)
{
	setParameter(name, &value);
	return S_OK;
}

API GLShader::SetVec4Parameter(const char *name, const vec4 *value)
{
	setParameter(name, value);
	return S_OK;
}

API GLShader::SetMat4Parameter(const char *name, const mat4 *value)
{
	setParameter(name, value);
	return S_OK;
}

API GLShader::SetUintParameter(const char *name, uint value)
{
	setParameter(name, &value);
	return S_OK;
}

API GLShader::FlushParameters()
{
	for (auto& idx : _bufferIndicies)
	{
		UBO &ubo = UBOpool[idx];
		if (ubo.needFlush)
		{
			glNamedBufferSubData(ubo._ID, 0, ubo.bytes, ubo.data.get());
			ubo.needFlush = false;
		}
	}

	return S_OK;
}

