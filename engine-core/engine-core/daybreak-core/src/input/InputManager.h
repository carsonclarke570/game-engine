#include "daybreak.h"

#pragma once

class DAYBREAK_API InputManager {
	
	public:
		/*
			Handles checking if an exit command has been issued.

			Returns true if an exit command has been issued, false otherwise.
		*/
		virtual bool handle_exit() = 0;

		/*
			Performs any logic necessary to consume input.
		*/
		virtual bool handle_input() = 0;
};

class DAYBREAK_API DX12InputManager : public InputManager {
	
	public:
		DX12InputManager();
		
		bool handle_exit();
		bool handle_input();
	
	private:
		MSG current_msg;
};

class DAYBREAK_API InputManagerFactory {
	
	public:
		static InputManager& create();
};