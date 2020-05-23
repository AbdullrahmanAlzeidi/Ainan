#pragma once

#include "renderer/ShaderProgram.h"
#include "renderer/RendererAPI.h"

#include <d3d11.h>

namespace Ainan {
	namespace D3D11 {
		class D3D11RendererContext;

		class D3D11ShaderProgram : public ShaderProgram
		{
		public:
			D3D11ShaderProgram(const std::string& vertPath, const std::string& fragPath, RendererContext* context);
			virtual ~D3D11ShaderProgram();

			virtual void Bind() const override;
			virtual void Unbind() const override;
			virtual void SetUniform1i(const char* name, const int& value) override;
			virtual int GetUniformLocation(const char* name) override;
			virtual int GetRendererID() const override;

		public:
			D3D11RendererContext* Context;
			ID3D11VertexShader* VertexShader;
			ID3D11PixelShader* FragmentShader;


			//this is needed for creating vertex buffers
			uint8_t* VertexByteCode = nullptr;
			uint32_t VertexByteCodeSize = 0;

			// Inherited via ShaderProgram
			virtual void BindUniformBuffer(const char* name, uint32_t slot) override;
			//there is no reason to keep the fragment shader code so it is immdediately freed after use
		};
	}
}