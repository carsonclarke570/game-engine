#pragma once

namespace Daybreak {

	class DAYBREAK_API RenderStateManager {
	public:
		static void Create();
		static void Destroy();
		
		static dx12::AttachmentPoint GetCurrentAttachment();
		static void SetCurrentAttachment(dx12::AttachmentPoint attachment);

		private:
			RenderStateManager();
			~RenderStateManager();

			static dx12::AttachmentPoint		g_currentAttachmentPoint;
			static RenderStateManager*			g_renderStateManager;
			static std::mutex					g_renderStateMutex;
	};
}