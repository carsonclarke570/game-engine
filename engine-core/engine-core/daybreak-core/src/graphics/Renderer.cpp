#include "daybreak.h"

#include "Renderer.h"
#include "graphics/Mesh.h"
#include "engine/manager/RenderStateManager.h"

namespace gfx {
	Renderer::Renderer(const RenderPassShaders& shaders) :
		m_shaderPaths(shaders),
		m_renderTarget(),
		m_scissorRect(CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX)),
		m_viewport(CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(DEFAULT_WIDTH), static_cast<float>(DEFAULT_HEIGHT))),
		m_geometryVertexShader(),
		m_geometryPixelShader(),
		m_geometryPipelineStream(),
		m_geometryPipelineState()
	{}

	Renderer::~Renderer() {}

	void Renderer::Initialize(const dx12::CommandList& commandList, int initialWidth, int initialHeight) {
		auto device = dx12::Application::Device();

		Logger::info(L"[Renderer::Initialize] Setup shared pipeline state...\n");
		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData;
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
		if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData)))) {
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
		}

		CD3DX12_STATIC_SAMPLER_DESC linearRepeatSampler(0, D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR);
		CD3DX12_STATIC_SAMPLER_DESC anisotropicSampler(0, D3D12_FILTER_ANISOTROPIC);
		DXGI_FORMAT backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D32_FLOAT;
		DXGI_SAMPLE_DESC sampleDesc = dx12::Application::Get()->GetMultisampleQualityLevels(backBufferFormat, D3D12_MAX_MULTISAMPLE_SAMPLE_COUNT);

		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	
		Logger::info(L"[Renderer::Initialize] Loading vertex shader for Geometry Pass...\n");
		ThrowOnFailure(D3DReadFileToBlob(m_shaderPaths.GeometryVertex.c_str(), &m_geometryVertexShader));

		// Load the pixel shader.
		Logger::info(L"[Renderer::Initialize] Loading pixel shader for Geometry Pass...\n");
		ThrowOnFailure(D3DReadFileToBlob(m_shaderPaths.GeometryPixel.c_str(), &m_geometryPixelShader));

		Logger::info(L"[Renderer::Initialize] Creating root signature for Geometry Pass...\n");
		CD3DX12_DESCRIPTOR_RANGE1 gpDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);

		CD3DX12_ROOT_PARAMETER1 gpRootParameters[GeometryRootParameters::NUM_PARAMS - 1];
		gpRootParameters[GeometryRootParameters::MATRICES_CB].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);
		// gpRootParameters[GeometryRootParameters::TEXTURES].InitAsDescriptorTable(1, &gpDescriptorRange, D3D12_SHADER_VISIBILITY_PIXEL);

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC gpRootSignatureDescription;
		gpRootSignatureDescription.Init_1_1(_countof(gpRootParameters), gpRootParameters, 1, &linearRepeatSampler, rootSignatureFlags);
		m_geometryRootSignature.SetRootSignatureDesc(gpRootSignatureDescription.Desc_1_1, featureData.HighestVersion);

		Logger::info(L"[Renderer::Initialize] Setup pipeline state for Geometry Pass...\n");
		D3D12_RT_FORMAT_ARRAY rtvFormats = {};
		rtvFormats.NumRenderTargets = 3;
		rtvFormats.RTFormats[0] = backBufferFormat;
		rtvFormats.RTFormats[1] = backBufferFormat;
		rtvFormats.RTFormats[2] = backBufferFormat;

		m_geometryPipelineStream.pRootSignature = m_geometryRootSignature.Signature().Get();
		m_geometryPipelineStream.InputLayout = { gfx::VertexData::InputElements, gfx::VertexData::InputElementCount };
		m_geometryPipelineStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		m_geometryPipelineStream.VS = CD3DX12_SHADER_BYTECODE(m_geometryVertexShader.Get());
		m_geometryPipelineStream.PS = CD3DX12_SHADER_BYTECODE(m_geometryPixelShader.Get());
		m_geometryPipelineStream.DSVFormat = depthBufferFormat;
		m_geometryPipelineStream.RTVFormats = rtvFormats;
		m_geometryPipelineStream.SampleDesc = sampleDesc;

		D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
			sizeof(GeometryPipelineStateStream), &m_geometryPipelineStream
		};
		ThrowOnFailure(device->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&m_geometryPipelineState)));

		Logger::info(L"[Renderer::Initialize] Creating diffuse color buffer...\n");
		dx12::Texture diffuseTexture = CreateRenderColorTexture(initialWidth, initialHeight, backBufferFormat, sampleDesc, L"Diffuse Buffer");

		Logger::info(L"[Renderer::Initialize] Creating normal color buffer...\n");
		dx12::Texture normalTexture = CreateRenderColorTexture(initialWidth, initialHeight, backBufferFormat, sampleDesc, L"Normal Buffer");

		Logger::info(L"[Renderer::Initialize] Creating position color buffer...\n");
		dx12::Texture positionTexture = CreateRenderColorTexture(initialWidth, initialHeight, backBufferFormat, sampleDesc, L"Position Buffer");

		Logger::info(L"[Renderer::Initialize] Creating depth buffer...\n");
		dx12::Texture depthTexture = CreateRenderDepthTexture(initialWidth, initialHeight, depthBufferFormat, sampleDesc, L"Depth Buffer");

		Logger::info(L"[TestGame::Initialize] Attaching depth & color buffers...\n");
		m_renderTarget.AttachTexture(dx12::AttachmentPoint::COLOR_0, diffuseTexture);
		m_renderTarget.AttachTexture(dx12::AttachmentPoint::COLOR_1, normalTexture);
		m_renderTarget.AttachTexture(dx12::AttachmentPoint::COLOR_2, positionTexture);
		m_renderTarget.AttachTexture(dx12::AttachmentPoint::DEPTH_STENCIL, depthTexture);
	}

	void Renderer::BeginRender(std::shared_ptr<dx12::CommandList> commandList) {
		Clear(commandList);

		commandList->SetPipelineState(m_geometryPipelineState.Get());
		commandList->SetGraphicsRootSignature(m_geometryRootSignature);

		commandList->SetViewport(m_viewport);
		commandList->SetScissorRect(m_scissorRect);
		commandList->SetRenderTarget(m_renderTarget);
	}

	void Renderer::EndRender(std::shared_ptr<dx12::CommandList> commandList, std::shared_ptr<dx12::CommandQueue> commandQueue) {
		commandQueue->ExecuteCommandList(commandList);
		dx12::Application::Get()->Present(m_renderTarget.GetTexture(Daybreak::RenderStateManager::GetCurrentAttachment()));
	}

	void Renderer::Resize(int width, int height) {
		m_viewport = CD3DX12_VIEWPORT(
			0.0f, 0.0f,
			static_cast<float>(width),
			static_cast<float>(height)
		);
		m_renderTarget.Resize(width, height);
	}

	void Renderer::Clear(std::shared_ptr<dx12::CommandList> commandList) {
		FLOAT clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		commandList->ClearTexture(m_renderTarget.GetTexture(dx12::AttachmentPoint::COLOR_0), clearColor);
		commandList->ClearTexture(m_renderTarget.GetTexture(dx12::AttachmentPoint::COLOR_1), clearColor);
		commandList->ClearTexture(m_renderTarget.GetTexture(dx12::AttachmentPoint::COLOR_2), clearColor);

		FLOAT depthClearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		commandList->ClearDepthStencilTexture(m_renderTarget.GetTexture(dx12::AttachmentPoint::DEPTH_STENCIL), D3D12_CLEAR_FLAG_DEPTH);
	}

	dx12::Texture Renderer::CreateRenderColorTexture(int initialWidth, int initialHeight, DXGI_FORMAT format, DXGI_SAMPLE_DESC sampleDesc, const std::wstring& name)  {
		auto colorDesc = CD3DX12_RESOURCE_DESC::Tex2D(
			format,
			initialWidth, initialHeight,
			1, 1,
			sampleDesc.Count, sampleDesc.Quality,
			D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
		);
		D3D12_CLEAR_VALUE colorClearValue;
		colorClearValue.Format = colorDesc.Format;
		colorClearValue.Color[0] = 0.0f;
		colorClearValue.Color[1] = 0.0f;
		colorClearValue.Color[2] = 0.0f;
		colorClearValue.Color[3] = 1.0f;

		return dx12::Texture(
			colorDesc, &colorClearValue,
			gfx::TextureType::RENDER_TARGET,
			name
		);
	}

	dx12::Texture Renderer::CreateRenderDepthTexture(int initialWidth, int initialHeight, DXGI_FORMAT format, DXGI_SAMPLE_DESC sampleDesc, const std::wstring& name) {
		auto depthDesc = CD3DX12_RESOURCE_DESC::Tex2D(
			format,
			initialWidth, initialHeight,
			1, 1,
			sampleDesc.Count, sampleDesc.Quality,
			D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
		);

		D3D12_CLEAR_VALUE depthClearValue;
		depthClearValue.Format = depthDesc.Format;
		depthClearValue.DepthStencil = { 1.0f, 0 };

		return dx12::Texture(
			depthDesc, &depthClearValue,
			gfx::TextureType::DEPTH,
			name
		);
	}

	void Renderer::DepthPass(std::shared_ptr<dx12::CommandList> commandList)
	{
	}

	void Renderer::LightingPass(std::shared_ptr<dx12::CommandList> commandList)
	{
	}
	void Renderer::PostProcessingPass(std::shared_ptr<dx12::CommandList> commandList)
	{
	}
}