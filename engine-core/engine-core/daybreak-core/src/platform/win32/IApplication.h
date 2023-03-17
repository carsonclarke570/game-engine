#pragma once

#define ENTRYAPP(x) win32::IApplication* entry_point() { return new x; }

namespace win32 {

	class DAYBREAK_API IApplication {
	public:
		IApplication();
		virtual ~IApplication() {};

		/*
		* Sets up the game settings
		*/
		virtual void settings() = 0;

		virtual void PreInitialize() = 0;

		/*
		* Initializes the application.
		*/
		virtual void initialize() = 0;

		/*
		* Updates the apllication.
		*/
		virtual void update() = 0;
	};

	IApplication* entry_point();
}