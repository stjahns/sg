#pragma once

#include <GLFW/glfw3.h>
#include <vector>

#include "Model.h"

class ModelLoader
{
public:
	ModelLoader() : m_bShutdown(false)
	{
		//CreateThread();
	}

	~ModelLoader() 
	{
		Shutdown();
	}

	void Run();

	void Load(const char* path);

	Model* GetLoadedModel();

private:

	void Shutdown();
	//pthread_t CreateThread();

	//pthread_t m_ThreadId;
	bool m_bShutdown;

	std::vector<Model *> m_Models;

	//pthread_mutex_t m_Mutex;
};
