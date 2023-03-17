#include "TestGame.h"
#include "engine/Simulation.h"
#include "platform/win32/WinEntry.h"

class TestGame : public Daybreak::Simulation {
public:
	TestGame();
	~TestGame();

	void settings();
	void initialize();
	void update();
};

ENTRYAPP(TestGame);

TestGame::TestGame() {
}

TestGame::~TestGame() {
}

void TestGame::settings() {
	GameSettings::set_game_name(IDS_PERGAMENAME);
	GameSettings::set_game_short_name(IDS_SHORTNAME);
	GameSettings::set_game_icon(IDI_MAINICON);
	GameSettings::SetSplashURL(IDS_SPLASHURL);
}

void TestGame::initialize() {}

void TestGame::update() {}
