#include "ResultScene.h"
#include "SceneController.h"
#include "../General/Fader.h"
#include "../General/AssetManager.h"

ResultScene::ResultScene(SceneController& controller, bool isWin):
	SceneBase(controller),
	m_isWin(isWin)
{
}

ResultScene::~ResultScene()
{
}

void ResultScene::Init()
{
	Fader::GetInstance().FadeIn();
	
}

void ResultScene::Update()
{
}

void ResultScene::Draw()
{
}

void ResultScene::End()
{
}

void ResultScene::Restart()
{
}

void ResultScene::DebugDraw() const
{
}
