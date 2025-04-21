#include "pch.h"
#include "SceneManager.h"

void SceneManager::Update()
{
	if(currentScene)
		currentScene->Update();
}
