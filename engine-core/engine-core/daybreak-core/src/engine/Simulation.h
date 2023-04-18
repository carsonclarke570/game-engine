#pragma once

namespace Daybreak {

	class DAYBREAK_API Simulation : public win32::IApplication, public win32::Window {

		public:
			struct UpdateEvent {
				uint64_t										frameCounter;
				double											totalTime;
				double											elapsedSeconds;
				std::chrono::high_resolution_clock::time_point	lastUpdate;
			};

			struct ResizeEvent {
				int newWidth;
				int newHeight;
			};

			struct RenderEvent {

			};

			Simulation();
			~Simulation();
	
			void Tick();

			virtual void PreInitialize() override;
			virtual void Update() override;
			virtual void Teardown() override;

			virtual LRESULT MessageHandler(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) override;
	
			virtual void OnUpdate(UpdateEvent event) = 0;
			virtual void OnRender(RenderEvent event) = 0;
			virtual void OnResize(ResizeEvent event) = 0;

		private:
			std::vector<Window*>				m_windows;
			std::chrono::high_resolution_clock	m_clock;

			UpdateEvent							m_currentUpdate;

			void Resize();
			
	};
}

