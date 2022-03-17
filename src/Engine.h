//--[Nelson Engine]----------------------------------------------------------//
// 
// FILE:     Engine.h
// 
// AUTHOR:   Lincoln Scheer
// 
// CREATED:  03-16-2022
// 
// PURPOSE:  Runs A Scene with Nelson Editor
// 
//---------------------------------------------------------------------------//

# pragma once

#include "Window.h"

class Engine {
public:
	// Public API
	Engine();		// Wake Up Engine (Instantiate Systems)
	void init();		// Initialize Systems
	void start();		// Load World
	int run();		// Game Loop
	void update();		// Game Loop Update
	void dispose();		// Free Allocated Resources


private:
	// Engine State
	bool exit		= false;

	// Message System and System Pointers
	MessageBus* messageBus	= new MessageBus();
	Console* console	= new Console(messageBus);
	Window* window		= new Window(messageBus);
	Renderer* renderer	= new Renderer(messageBus);
	Editor* editor		= new Editor(messageBus);


	// Scene Pointer
	Scene* scene		= nullptr;
		

	// Timing
	float deltaTime = 0.0f;	// Delta Time = Sync() - Last Sync()
	float lastFrame = 0.0f;	// Last Sync() Call
	void sync();		// Syncs Timer Since Last Call
};