#pragma once

namespace FPSCounter {

	void UpdateFPS(double fps);
	void UpdateFrameTime(double framTimeMs);
	void UpdateTotalTime(double totalTime);
	void UpdateFrameCount(int frameCount);

	double FPS();
	double FrameTime();
	double TotalTime();
	int FrameCount();
}