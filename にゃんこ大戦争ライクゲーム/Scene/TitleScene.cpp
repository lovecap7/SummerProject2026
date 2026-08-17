#include "TitleScene.h"
#include <Dxlib.h>
#include  "../General/Input.h"
#include "SceneController.h"
#include "GameScene.h"
#include "../General/Fader.h"

namespace
{

}

TitleScene::TitleScene(SceneController& controller):
	SceneBase(controller)
{
}

TitleScene::~TitleScene()
{

}

void TitleScene::Init()
{
	Fader::GetInstance().FadeIn();
}

void TitleScene::Update()
{
	auto& fader = Fader::GetInstance();

	if(Input::GetInstance().IsTrigger("A") && !fader.IsFadeNow())
	{
		fader.FadeOut();
	}
	if (fader.IsFinishFadeOut())
	{
		m_controller.ChangeScene(std::make_unique<GameScene>(m_controller));
		return;
	}
}

void TitleScene::Draw()
{
	DebugDraw();
}

void TitleScene::End()
{
	
}

void TitleScene::DebugDraw() const
{
#if _DEBUG
	DrawString(0, 0, L"Title Scene", 0xffffff);
	DrawString(0, 16, L"[D]ÉLÅ[Ç≈ Debug Scene", 0xffffff);
#endif
}

