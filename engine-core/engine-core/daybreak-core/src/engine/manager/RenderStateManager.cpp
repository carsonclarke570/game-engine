#include "daybreak.h"
#include "RenderStateManager.h"

namespace Daybreak {

	dx12::AttachmentPoint	RenderStateManager::g_currentAttachmentPoint = dx12::AttachmentPoint::COLOR_0;
	RenderStateManager*		RenderStateManager::g_renderStateManager = nullptr;
	std::mutex				RenderStateManager::g_renderStateMutex;

	RenderStateManager::RenderStateManager() {}

	RenderStateManager::~RenderStateManager() {}

	void RenderStateManager::Create() {
		const std::lock_guard<std::mutex> lock(g_renderStateMutex);
		if (g_renderStateManager) {
			throw std::exception("RenderStateManager already created!");
		}
		g_renderStateManager = new RenderStateManager();
	}

	void RenderStateManager::Destroy() {
		const std::lock_guard<std::mutex> lock(g_renderStateMutex);
		if (!g_renderStateManager) {
			throw std::exception("RenderStateManager already destroyed!");
		}
		delete g_renderStateManager;
		g_renderStateManager = 0;
	}
	
	dx12::AttachmentPoint RenderStateManager::GetCurrentAttachment() {
		const std::lock_guard<std::mutex> lock(g_renderStateMutex);
		return g_currentAttachmentPoint;
	}

	void RenderStateManager::SetCurrentAttachment(dx12::AttachmentPoint attachment) {
		const std::lock_guard<std::mutex> lock(g_renderStateMutex);
		g_currentAttachmentPoint = attachment;
	}
}