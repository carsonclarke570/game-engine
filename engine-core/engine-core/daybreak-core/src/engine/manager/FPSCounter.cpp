#include "daybreak.h"
#include "FPSCounter.h"

namespace FPSCounter {

	double g_fps = 0.0f;
	std::mutex g_fpsMutex;

	double g_frameTime = 0.0f;
	std::mutex g_frameTimeMutex;

	double g_totalTime = 0.0f;
	std::mutex g_totalTimeMutex;

	int g_frameCount = 0;
	std::mutex g_frameCountMutex;

	void UpdateFPS(double fps) {
		const std::lock_guard<std::mutex> lock(g_fpsMutex);
		g_fps = fps;
	}

	void UpdateFrameTime(double framTimeMs) {
		const std::lock_guard<std::mutex> lock(g_frameTimeMutex);
		g_frameTime = framTimeMs;
	}

	void UpdateTotalTime(double totalTime) {
		const std::lock_guard<std::mutex> lock(g_totalTimeMutex);
		g_totalTime = totalTime;
	}

	void UpdateFrameCount(int frameCount) {
		const std::lock_guard<std::mutex> lock(g_frameCountMutex);
		g_frameCount = frameCount;
	}

	double FPS() {
		return g_fps;
	}

	double FrameTime() {
		return g_frameTime;
	}

	double TotalTime() {
		return g_totalTime;
	}

	int FrameCount() {
		return g_frameCount;
	}
}