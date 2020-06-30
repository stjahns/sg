#include "ModelLoader.h"

void* LoaderThreadCB(void* data)
{
    ModelLoader *loader = static_cast<ModelLoader *>(data);

    loader->Run();

    return nullptr;
}

pthread_t ModelLoader::CreateThread()
{
	// Create the thread using POSIX routines.
	pthread_attr_t attr;
	int returnVal;

	returnVal = pthread_attr_init(&attr);
	assert(!returnVal);

	int threadError = pthread_create(&m_ThreadId, &attr, &LoaderThreadCB, this);

	returnVal = pthread_attr_destroy(&attr);
	assert(!returnVal);
	if (threadError != 0)
	{
		// Report an error.
	}

	return m_ThreadId;
}

void ModelLoader::Run()
{
    while (!m_bShutdown)
    {
        pthread_mutex_lock(&m_Mutex);
        if (m_Models.size() > 0)
        {
            Model *model = m_Models.back();
            if (model && !model->IsLoaded())
            {
                model->Load();
            }
        }
        pthread_mutex_unlock(&m_Mutex);
    }
}

void ModelLoader::Load(const char *path)
{
    pthread_mutex_lock(&m_Mutex);
    m_Models.push_back(new Model(path));
    pthread_mutex_unlock(&m_Mutex);
}

Model *ModelLoader::GetLoadedModel()
{
    pthread_mutex_lock(&m_Mutex);

    Model *loadedModel = nullptr;

    if (m_Models.size() > 0)
    {
        Model *model = m_Models.back();
        if (model && model->IsLoaded())
        {
            loadedModel = model;
            m_Models.pop_back();
        }
    }

    pthread_mutex_unlock(&m_Mutex);

    return loadedModel;
}

void ModelLoader::Shutdown()
{
    m_bShutdown = true;
    pthread_join(m_ThreadId, nullptr);
}
