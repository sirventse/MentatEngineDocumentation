#include <GL/glew.h>  
#include <fstream>
#include <sstream>
#include <iostream>  
#include <../include/MentatEngine/OpenGL/Shader.hpp>

static std::string ReadTextFile(const char* path) {
	std::ifstream file(path, std::ios::in);
	if (!file.is_open()) {
		std::cerr << "Shader file not found: " << (path ? path : "(null)") << "\n";
		return {};
	}
	std::stringstream ss;
	ss << file.rdbuf();
	return ss.str();
}

static void CompileAndLog(GLuint shader, const char* label) {
	glCompileShader(shader);
	GLint compiled = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

	if (!compiled) {
		GLint logLen = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);

		std::string log;
		log.resize((logLen > 1) ? logLen : 1);

		glGetShaderInfoLog(shader, logLen, nullptr, log.data());

		std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n";
		std::cerr << log << "\n";
	}
}

namespace ME {
	Shader::Shader(const char* vertexPath, const char* fragmentPath)
		: vertexShader_(0), fragmentShader_(0)
	{
		// ---------- VERTEX ----------
		if (vertexPath) {
			std::string src = ReadTextFile(vertexPath);

			if (!src.empty()) {
				vertexShader_ = glCreateShader(GL_VERTEX_SHADER);

				const char* csrc = src.c_str();
				glShaderSource(vertexShader_, 1, &csrc, nullptr);
				CompileAndLog(vertexShader_, vertexPath);
			}
			else {
				std::cerr << "Vertex shader EMPTY: " << vertexPath << "\n";
			}
		}

		// ---------- FRAGMENT ----------
		if (fragmentPath) {
			std::string src = ReadTextFile(fragmentPath);

			if (!src.empty()) {
				fragmentShader_ = glCreateShader(GL_FRAGMENT_SHADER);

				const char* csrc = src.c_str();
				glShaderSource(fragmentShader_, 1, &csrc, nullptr);
				CompileAndLog(fragmentShader_, fragmentPath);
			}
			else {
				std::cerr << "Fragment shader EMPTY: " << fragmentPath << "\n";
			}
		}
	}

	Shader::Shader(const char* vertex, const char* fragment, const char* geometry)
	{
		// ---------- VERTEX ----------
		if (vertex) {
			std::string src = ReadTextFile(vertex);

			if (!src.empty()) {
				vertexShader_ = glCreateShader(GL_VERTEX_SHADER);

				const char* csrc = src.c_str();
				glShaderSource(vertexShader_, 1, &csrc, nullptr);
				CompileAndLog(vertexShader_, vertex);
			}
			else {
				std::cerr << "Vertex shader EMPTY: " << vertex << "\n";
			}
		}

		// ---------- FRAGMENT ----------
		if (fragment) {
			std::string src = ReadTextFile(fragment);

			if (!src.empty()) {
				fragmentShader_ = glCreateShader(GL_FRAGMENT_SHADER);

				const char* csrc = src.c_str();
				glShaderSource(fragmentShader_, 1, &csrc, nullptr);
				CompileAndLog(fragmentShader_, fragment);
			}
			else {
				std::cerr << "Fragment shader EMPTY: " << fragment << "\n";
			}
		}

		// ---------- GEOMETRY ----------
		if (geometry) {
			std::string src = ReadTextFile(geometry);

			if (!src.empty()) {
				geometryShader_ = glCreateShader(GL_GEOMETRY_SHADER);

				const char* csrc = src.c_str();
				glShaderSource(geometryShader_, 1, &csrc, nullptr);
				CompileAndLog(geometryShader_, geometry);
			}
			else {
				std::cerr << "Fragment shader EMPTY: " << geometry << "\n";
			}
		}
	}

	ME::Shader::Shader(Shader&& other) noexcept
		: vertexShader_(other.vertexShader_),
		fragmentShader_(other.fragmentShader_),
		geometryShader_(other.geometryShader_){
		other.vertexShader_ = 0;
		other.fragmentShader_ = 0;
		other.geometryShader_ = 0;
	}

	ME::Shader& ME::Shader::operator=(Shader&& other) noexcept {
		if (this != &other) {
			CleanShader();
			vertexShader_ = other.vertexShader_;
			fragmentShader_ = other.fragmentShader_;
			geometryShader_ = other.geometryShader_;
			other.vertexShader_ = 0;
			other.fragmentShader_ = 0;
			other.geometryShader_ = 0;
		}
		return *this;
	}

	Shader::~Shader() {
		CleanShader();
	}

	GLuint Shader::GetVertexShader() const {
		return vertexShader_;
	}

	GLuint Shader::GetFragmentShader() const {
		return fragmentShader_;
	}

	GLuint Shader::GetGeometryShader() const
	{
		return geometryShader_;
	}

	void Shader::CleanShader() {
		if (vertexShader_) {
			glDeleteShader(vertexShader_);
			vertexShader_ = 0;
		}
		if (fragmentShader_) {
			glDeleteShader(fragmentShader_);
			fragmentShader_ = 0;
		}
		if (geometryShader_) {
			glDeleteShader(geometryShader_);
			geometryShader_ = 0;
		}
	}
}
