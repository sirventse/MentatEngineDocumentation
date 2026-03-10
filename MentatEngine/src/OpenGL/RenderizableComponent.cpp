#include <GL/glew.h>  
#include <iostream>  
#include <../include/MentatEngine/OpenGL/RenderizableComponent.hpp>

namespace ME {
	RenderizableComponent::RenderizableComponent(
		const Vec3* vertices,
		const size_t& vert_num,
		const unsigned int* indexes,
		const size_t& index_count,
		const Vec3 color,
		std::string path
	)
		: vao_()
		, vbo_(GL_ARRAY_BUFFER)
		, ebo_(GL_ELEMENT_ARRAY_BUFFER)
		, vertex_amount_{ static_cast<GLuint>(vert_num) }
		, index_count_{ static_cast<GLuint>(index_count) }
		, color_{ color }
		, mesh_path_{ path }
	{
		original_vertices_.assign(vertices, vertices + vert_num);
		original_indices_.assign(indexes, indexes + index_count);

		// 1) calcula normales
		std::vector<Vec3> normals;
		ComputeSmoothNormals(original_vertices_, original_indices_, normals);

		// 2) construye interleaved
		vertices_pn_.resize(vert_num);
		for (size_t i = 0; i < vert_num; ++i) {
			vertices_pn_[i].pos = original_vertices_[i];
			vertices_pn_[i].normal = normals[i];
		}

		// 3) VAO + VBO + layout
		vao_.Bind();

		vbo_.Bind();
		vbo_.SetData(vertices_pn_.data(), static_cast<GLuint>(vertices_pn_.size() * sizeof(VertexPN)), GL_STATIC_DRAW);

		// location 0: position
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPN), (void*)offsetof(VertexPN, pos));
		glEnableVertexAttribArray(0);

		// location 1: normal
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPN), (void*)offsetof(VertexPN, normal));
		glEnableVertexAttribArray(1);

		// EBO
		ebo_.Bind();
		ebo_.SetData(indexes, static_cast<GLuint>(index_count) * sizeof(unsigned int), GL_STATIC_DRAW);

		vao_.Unbind();
		vbo_.Unbind();
		ebo_.Unbind();

		is_color_active_ = true;
		isVisible_ = true;
		texure_path_ = "";
	}

	RenderizableComponent::~RenderizableComponent() {
		// Si tus clases Buffer y VertexArray
		// manejan su propio cleanup (glDeleteBuffers, etc.),
		// no necesitas nada aquí.
	}

	void RenderizableComponent::ReplaceMesh(
		const Vec3* vertices,
		size_t vert_num,
		const unsigned int* indexes,
		size_t index_count,
		char* new_path
	) {
		// Actualizamos los contadores de la clase
		vertex_amount_ = static_cast<GLuint>(vert_num);
		index_count_ = static_cast<GLuint>(index_count);

		if (new_path) {
			mesh_path_ = new_path;
		}

		// Sincronizamos los vectores de la CPU
		original_vertices_.assign(vertices, vertices + vert_num);
		original_indices_.assign(indexes, indexes + index_count);

		// Recalculamos normales (necesario si la forma cambia)
		std::vector<Vec3> normals;
		ComputeSmoothNormals(original_vertices_, original_indices_, normals);

		// Reconstruimos el formato interleaved (Posición + Normal)
		vertices_pn_.clear();
		vertices_pn_.resize(vert_num);
		for (size_t i = 0; i < vert_num; ++i) {
			vertices_pn_[i].pos = original_vertices_[i];
			vertices_pn_[i].normal = normals[i];
		}

		// --- ACTUALIZACIÓN DE GPU ---
		vao_.Bind();

		// IMPORTANTE: Usamos SetData (glBufferData) para que la GPU 
		// pueda reservar un espacio de memoria distinto si el tamaño cambió.
		vbo_.Bind();
		vbo_.SetData(vertices_pn_.data(),
			static_cast<GLuint>(vertices_pn_.size() * sizeof(VertexPN)),
			GL_STATIC_DRAW);

		ebo_.Bind();
		ebo_.SetData(indexes,
			static_cast<GLuint>(index_count) * sizeof(unsigned int),
			GL_STATIC_DRAW);

		vao_.Unbind();
		vbo_.Unbind();
		ebo_.Unbind();

		std::cout << "Mesh replaced successfully: " << (new_path ? new_path : "internal") << std::endl;
	}

	void RenderizableComponent::Bind() const {
		vao_.Bind();
	}

	void RenderizableComponent::Unbind() const {
		vao_.Unbind();
	}

	void RenderizableComponent::UpdateGeometry(Vec3* vertices, unsigned int vert_num) {
		if (vert_num != vertex_amount_) return; // o ajusta si quieres soportar cambios de tamaño

		// actualiza posiciones
		original_vertices_.assign(vertices, vertices + vert_num);

		// recalcula normales
		std::vector<Vec3> normals;
		ComputeSmoothNormals(original_vertices_, original_indices_, normals);

		// reconstruye interleaved
		vertices_pn_.resize(vert_num);
		for (size_t i = 0; i < vert_num; ++i) {
			vertices_pn_[i].pos = original_vertices_[i];
			vertices_pn_[i].normal = normals[i];
		}

		// sube todo el buffer
		vbo_.Bind();
		vbo_.UpdateData(vertices_pn_.data(), vert_num * sizeof(VertexPN), GL_STATIC_DRAW);
		vbo_.Unbind();
	}

	GLuint RenderizableComponent::GetVAO() {
		return vao_.GetVAO();
	}

	GLuint RenderizableComponent::GetVertexAmount() {
		return vertex_amount_;
	}

	GLuint RenderizableComponent::GetIndexCount() {
		return index_count_;
	}

	Vec3 RenderizableComponent::GetColor() {
		return color_;
	}

	bool RenderizableComponent::IsColorActive()
	{
		return is_color_active_;
	}
}