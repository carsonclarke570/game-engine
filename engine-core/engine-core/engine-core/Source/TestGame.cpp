#include "TestGame.h"

#include "platform/dx12/Application.h"
#include "engine/Simulation.h"
#include "platform/win32/WinEntry.h"

#include "platform/dx12/RenderTarget.h"
#include "platform/dx12/RootSignature.h"
#include "platform/dx12/Texture.h"
#include "platform/dx12/CommandList.h"
#include "graphics/TextureType.h"
#include "graphics/Mesh.h"
#include "graphics/Model.h"
#include "graphics/Renderer.h"
#include "platform/dx12/Texture.h"

using namespace DirectX;

class TestGame : public Daybreak::Simulation {
	public:
		TestGame();
		~TestGame();

		void Settings();
		void Initialize();

		void OnUpdate(UpdateEvent event);
		void OnRender(RenderEvent event);
		void OnResize(ResizeEvent event);

	private:
		std::shared_ptr<gfx::Model> m_cube;
		FXMVECTOR m_cubePos;

		dx12::Texture m_testTexture;

		// Renderer
		gfx::Renderer m_renderer;

		float m_fov;
		FXMVECTOR m_cameraPos;
		FXMVECTOR m_cameraDir;
		FXMVECTOR m_cameraUp;

		XMMATRIX m_model;
		XMMATRIX m_view;
		XMMATRIX m_projection;
};

ENTRYAPP(TestGame);

struct Mat {
	XMMATRIX Model;
	XMMATRIX View;
	XMMATRIX Projection;
};

enum RootParameters {
	MATRICES_CB,        // ConstantBuffer<Mat> MatCB : register(b0);
	TEXTURES,           // Texture2D DiffuseTexture : register( t2 );
	NUM_PARAMS
};

TestGame::TestGame() : 
	m_fov(45.0),
	m_cameraPos(),
	m_cameraDir(),
	m_cameraUp(),
	m_projection(),
	m_cubePos(),
	m_renderer({
		L"../bin/Debug/VertexShader.cso",
		L"../bin/Debug/PixelShader.cso"
	}) {
	float aspectRatio = DEFAULT_WIDTH / (float)DEFAULT_HEIGHT;
	m_projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(m_fov), aspectRatio, 0.1f, 100.0f); 
}

TestGame::~TestGame() {
}

void TestGame::Settings() {
	GameSettings::set_game_name(IDS_PERGAMENAME);
	GameSettings::set_game_short_name(IDS_SHORTNAME);
	GameSettings::set_game_icon(IDI_MAINICON);
	GameSettings::SetSplashURL(IDS_SPLASHURL);
}

void TestGame::Initialize() {
	auto device = dx12::Application::Device();
	auto commandQueue = dx12::Application::Get()->CommandQueue(D3D12_COMMAND_LIST_TYPE_COPY);
	auto commandList = commandQueue->CommandList();

	// Upload vertex buffer data
	Logger::info(L"[TestGame::Initialize] Creating cube mesh...\n");
	m_cube = gfx::Model::LoadFromFile(*commandList, "./Assets/Models/Backpack/backpack.obj");

	Logger::info(L"[TestGame::Initialize] Creating view & projection matrix...\n");
	const XMVECTOR eyePosition = XMVectorSet(0, 0, -10, 1);
	const XMVECTOR focusPoint = XMVectorSet(0, 0, 0, 1);
	const XMVECTOR upDirection = XMVectorSet(0, 1, 0, 0);
	m_view = XMMatrixLookAtLH(eyePosition, focusPoint, upDirection);

	// Initialize renderer
	m_renderer.Initialize(*commandList, m_size.cx, m_size.cy);

	Logger::info(L"[TestGame::Initialize] Executing command list...\n");
	auto fenceValue = commandQueue->ExecuteCommandList(commandList);
	commandQueue->WaitForFenceValue(fenceValue);
}

void TestGame::OnUpdate(UpdateEvent event) {

	float angle = static_cast<float>(event.elapsedSeconds * 360.0f);
	const XMVECTOR rotationAxis = XMVectorSet(0, 1, 1, 0);
	XMMATRIX m_rotationMatrix = XMMatrixRotationAxis(rotationAxis, XMConvertToRadians(angle));
	XMMATRIX translationMatrix = XMMatrixIdentity();
	XMMATRIX scaleMatrix = XMMatrixIdentity();
	m_model = scaleMatrix * m_rotationMatrix * translationMatrix;
}

void TestGame::OnRender(RenderEvent event) {
	dx12::Application* app = dx12::Application::Get();

	auto commandQueue = app->CommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
	auto commandList = commandQueue->CommandList();

	{
		m_renderer.BeginRender(commandList);

		Mat matrices;
		matrices.Model = XMMatrixTranspose(m_model);
		matrices.View = XMMatrixTranspose(m_view);
		matrices.Projection = XMMatrixTranspose(m_projection);

		commandList->SetGraphicsDynamicConstantBuffer(RootParameters::MATRICES_CB, matrices);
		// commandList->SetGraphicsDynamicConstantBuffer(RootParameters::MATRICES_CB, Material::White);
		// commandList->SetShaderResourceView(RootParameters::TEXTURES, 0, m_MonaLisaTexture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		m_cube->Draw(*commandList);

		m_renderer.EndRender(commandList, commandQueue);
	}
}

void TestGame::OnResize(ResizeEvent event) {
	if (m_size.cx != event.newWidth || m_size.cy != event.newHeight) {
		Logger::info(L"Resizing to %d x %d\n", event.newWidth, event.newHeight);
		m_size = { std::max(1, event.newWidth), std::max(1, event.newHeight) };

		float aspectRatio = event.newWidth / (float)event.newHeight;
		m_projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(m_fov), aspectRatio, 0.1f, 100.0f);

		m_renderer.Resize(event.newWidth, event.newHeight);
	}
}

