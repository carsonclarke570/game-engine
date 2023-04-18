#pragma once

#include "platform/dx12/RootSignature.h"

namespace gfx {

	struct DAYBREAK_API RenderPassShaders {
		std::wstring GeometryVertex;
		std::wstring GeometryPixel;
	};

	enum GeometryRootParameters {
		MATRICES_CB,        // ConstantBuffer<Mat> MatCB : register(b0);
		TEXTURES,           // Texture2D DiffuseTexture : register( t2 );
		NUM_PARAMS
	};

	class DAYBREAK_API Renderer {
		public:
			Renderer(const RenderPassShaders& shaders);
			~Renderer();

			void Initialize(const dx12::CommandList& commandList, int initialWidth, int initialHeight);
		
			void BeginRender(std::shared_ptr<dx12::CommandList> commandList);
			void EndRender(std::shared_ptr<dx12::CommandList> commandList, std::shared_ptr<dx12::CommandQueue> commandQueue);

			void Resize(int width, int height);

			dx12::RenderTarget& GetRenderTarget() { return m_renderTarget; }

		private:
			dx12::Texture CreateRenderColorTexture(int initialWidth, int initialHeight, DXGI_FORMAT format, DXGI_SAMPLE_DESC sampleDesc, const std::wstring& name);
			dx12::Texture CreateRenderDepthTexture(int initialWidth, int initialHeight, DXGI_FORMAT format, DXGI_SAMPLE_DESC sampleDesc, const std::wstring& name);

			void Clear(std::shared_ptr<dx12::CommandList> commandList);
			void DepthPass(std::shared_ptr<dx12::CommandList> commandList);
			void LightingPass(std::shared_ptr<dx12::CommandList> commandList);
			void PostProcessingPass(std::shared_ptr<dx12::CommandList> commandList);

			RenderPassShaders	m_shaderPaths;
			dx12::RenderTarget	m_renderTarget;
			D3D12_VIEWPORT		m_viewport;
			D3D12_RECT			m_scissorRect;

			// Geometry Pass Resources
			struct GeometryPipelineStateStream {
				CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
				CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
				CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
				CD3DX12_PIPELINE_STATE_STREAM_VS VS;
				CD3DX12_PIPELINE_STATE_STREAM_PS PS;
				CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
				CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
				CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC SampleDesc;
			};
			
			dx12::RootSignature			m_geometryRootSignature;
			GeometryPipelineStateStream m_geometryPipelineStream;
			ComPtr<ID3D12PipelineState> m_geometryPipelineState;
			ComPtr<ID3DBlob>			m_geometryVertexShader;
			ComPtr<ID3DBlob>			m_geometryPixelShader;
	};
}