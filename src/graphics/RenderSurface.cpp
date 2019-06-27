#include <pch.h>

#include "RenderSurface.h"

namespace ALZ {

	static ShaderProgram* ImageShader = nullptr;
	static bool ImageShaderInitilized = false;

	RenderSurface::RenderSurface()
	{
		m_FrameBuffer = Renderer::CreateFrameBuffer();

		m_FrameBuffer->Bind();

		glGenTextures(1, &m_Texture);
		glBindTexture(GL_TEXTURE_2D, m_Texture);

		m_Size = Window::WindowSize;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (GLsizei)m_Size.x, (GLsizei)m_Size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_Texture, 0);

		glGenVertexArrays(1, &m_VertexArray);
		glBindVertexArray(m_VertexArray);
		glGenBuffers(1, &m_VertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);

		float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
			// positions   // texCoords
			-1.0f,  1.0f,  0.0f, 1.0f,
			-1.0f, -1.0f,  0.0f, 0.0f,
			 1.0f, -1.0f,  1.0f, 0.0f,

			-1.0f,  1.0f,  0.0f, 1.0f,
			 1.0f, -1.0f,  1.0f, 0.0f,
			 1.0f,  1.0f,  1.0f, 1.0f
		};

		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

		m_FrameBuffer->Unbind();

		if (ImageShaderInitilized == false)
		{
			ImageShader = Renderer::CreateShaderProgram("shaders/Image.vert", "shaders/Image.frag").release();
			ImageShaderInitilized = true;
		}
	}

	RenderSurface::~RenderSurface()
	{
		glDeleteTextures(1, &m_Texture);
		glDeleteBuffers(1, &m_VertexBuffer);
		glDeleteVertexArrays(1, &m_VertexArray);
	}

	void RenderSurface::Render()
	{
		glBindVertexArray(m_VertexArray);
		ImageShader->SetUniform1i("screenTexture", 0);
		ImageShader->Bind();
		glBindTexture(GL_TEXTURE_2D, m_Texture);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	void RenderSurface::Render(ShaderProgram & shader)
	{
		glBindVertexArray(m_VertexArray);
		shader.Bind();
		glBindTexture(GL_TEXTURE_2D, m_Texture);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	void RenderSurface::RenderToScreen()
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_FrameBuffer->GetRendererID());
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

		glBlitFramebuffer(0, 0, (GLint)m_Size.x, (GLint)m_Size.y,
						  0, 0, (GLint)m_Size.x, (GLint)m_Size.y,
						  GL_COLOR_BUFFER_BIT,
						  GL_LINEAR);

		glViewport(0, 0, (GLsizei)Window::WindowSize.x, (GLsizei)Window::WindowSize.y);
	}

	void RenderSurface::SetSize(const glm::vec2 & size)
	{
		m_Size = size;
		m_FrameBuffer->Bind();
		glDeleteTextures(1, &m_Texture);

		glGenTextures(1, &m_Texture);
		glBindTexture(GL_TEXTURE_2D, m_Texture);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (GLsizei)size.x, (GLsizei)size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_Texture, 0);
		m_FrameBuffer->Unbind();
	}
}