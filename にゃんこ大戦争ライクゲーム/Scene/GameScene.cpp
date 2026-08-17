#include "GameScene.h"
#include "TitleScene.h"
#include <Dxlib.h>
#include  "../General/Input.h"
#include  "../General/Fader.h"
#include  "../General/Game.h"
#include "SceneController.h"
#include "../Monster/MonsterBase.h"
#include "../Monster/Mushroom.h"
#include "../Monster/Takenoko.h"
#include "../Game/House.h"
#include "../General/Math/MyMath.h"


namespace
{
	//ボタンの大きさ
	constexpr float kButtonWidth = 200.0f;
	constexpr float kButtonHeight = 100.0f;

	//ボタン１の座標
	constexpr float kButton1PosX = Game::kScreenCenterX - 330;
	//ボタン２の座標
	constexpr float kButton2PosX = Game::kScreenCenterX - 110;
	//ボタン３の座標
	constexpr float kButton3PosX = Game::kScreenCenterX + 110;
	//ボタン４の座標
	constexpr float kButton4PosX = Game::kScreenCenterX + 330;
	//ボタンのY座標
	constexpr float kButtonPosY = 630;

	//敵の出現間隔
	constexpr int kEnemySpawnInterval = 6 * 60;

	//ランダム出現間隔
	constexpr int kEnemySpawnRandomRange = 60;
}

GameScene::GameScene(SceneController& controller) :
	SceneBase(controller)
{
}

GameScene::~GameScene()
{
	//ボタン画像削除
	for(auto& image : m_buttonImageList)
	{
		DeleteGraph(image);
	}
	//背景削除
	DeleteGraph(m_backgroundImage);
}

void GameScene::Init()
{
	Fader::GetInstance().FadeIn();

	//ボタンの画像と配置
	for (int i = 0; i < 4; ++i)
	{
		m_buttonImageList.push_back(LoadGraph(L"Image/MonsterButton.png"));
	}
	m_buttonPosList.resize(4);
	m_buttonPosList[0] = Vector2(kButton1PosX, kButtonPosY);
	m_buttonPosList[1] = Vector2(kButton2PosX, kButtonPosY);
	m_buttonPosList[2] = Vector2(kButton3PosX, kButtonPosY);
	m_buttonPosList[3] = Vector2(kButton4PosX, kButtonPosY);

	//背景画像
	m_backgroundImage = LoadGraph(L"Image/Background.png");


	//プレイヤーの家を作成
	m_playerHouse = std::make_shared<House>(Game::Team::Player);
	m_playerHouse->Init();
	//敵の家を作成
	m_enemyHouse = std::make_shared<House>(Game::Team::Enemy);
	m_enemyHouse->Init();

	//敵の出現フレームを初期化
	m_enemy1SpawnInterval = kEnemySpawnInterval + MyMath::GetRand(-kEnemySpawnRandomRange, kEnemySpawnRandomRange);
}

void GameScene::Update()
{
	//敵のモンスターを生成
	CreateEnemyMonster();

	auto& fader = Fader::GetInstance();

	if (Input::GetInstance().IsTrigger("A") && !fader.IsFadeNow())
	{
		//ボタンを押したときの処理
		int index = CheckPushButton();
		if(index != -1)
		{
			//ボタンが押された場合の処理
			CreatePlayerMonster(static_cast<MonsterIndex>(index));
		}
	}

	//モンスターの更新
	for(auto& monster : m_enemyMonsterList)
	{
		monster->Update();
	}
	for(auto& monster : m_playerMonsterList)
	{
		monster->Update();
	}

	//家の更新
	m_playerHouse->Update();
	m_enemyHouse->Update();

	//攻撃範囲チェック
	for (auto& monster : m_enemyMonsterList)
	{
		monster->CheckAttackRange(m_playerMonsterList,m_playerHouse);
	}
	for (auto& monster : m_playerMonsterList)
	{
		monster->CheckAttackRange(m_enemyMonsterList, m_enemyHouse);
	}

	//死亡チェック
	m_enemyMonsterList.erase(std::remove_if(m_enemyMonsterList.begin(), m_enemyMonsterList.end(),
		[](const std::shared_ptr<MonsterBase>& monster) { return monster->IsDead(); }), m_enemyMonsterList.end());
	m_playerMonsterList.erase(std::remove_if(m_playerMonsterList.begin(), m_playerMonsterList.end(),
		[](const std::shared_ptr<MonsterBase>& monster) { return monster->IsDead(); }), m_playerMonsterList.end());
}

void GameScene::Draw()
{
	DebugDraw();

	//背景の描画
	DrawGraph(0, 0, m_backgroundImage, true);

	//家の描画
	m_playerHouse->Draw();
	m_enemyHouse->Draw();

	//敵の描画
	for(auto& monster : m_enemyMonsterList)
	{
		monster->Draw();
	}

	//プレイヤーのモンスター描画
	for(auto& monster : m_playerMonsterList)
	{
		monster->Draw();
	}


	//ボタンの描画
	for(int i = 0; i < m_buttonPosList.size(); ++i)
	{
		SetDrawBright(255, 255, 255);
		//選ばれてるとき少し拡大
		double scale = 1.0;
		if(CheckPushButton() == i)
		{
			scale = 1.1;

			//クリックもしてるなら
			if(Input::GetInstance().IsPress("A"))
			{
				// 半分くらいの明るさ
				SetDrawBright(128, 128, 128);
			}
		}
		DrawRotaGraphF(m_buttonPosList[i].x, m_buttonPosList[i].y, scale, 0.0, m_buttonImageList[i], true);
		SetDrawBright(255, 255, 255);
	}
}

void GameScene::End()
{
	
}

void GameScene::Restart()
{
}

void GameScene::DebugDraw() const
{
#if _DEBUG
	DrawString(0, 0, L"Game Scene", 0xffffff);
	DrawString(0, 16, L"[D]キーで Debug Scene", 0xffffff);
#endif
}

int GameScene::CheckPushButton()
{
	Vector2 mousePos = Input::GetInstance().GetMousePos();

	for(int i = 0; i < m_buttonPosList.size(); ++i)
	{
		Vector2 buttonPos = m_buttonPosList[i];
		if(mousePos.x >= buttonPos.x - kButtonWidth / 2 &&
		   mousePos.x <= buttonPos.x + kButtonWidth / 2 &&
		   mousePos.y >= buttonPos.y - kButtonHeight / 2 &&
		   mousePos.y <= buttonPos.y + kButtonHeight / 2)
		{
			return i;
		}
	}
	// どのボタンも押されていない場合は -1 を返す
	return -1;
}

void GameScene::CreatePlayerMonster(MonsterIndex index)
{
	//範囲外チェック
	if (index < MonsterIndex::Mushroom1 && index >= MonsterIndex::MaxNum)return;

	switch (index)
	{
	case MonsterIndex::Mushroom1:
		m_playerMonsterList.push_back(std::make_shared<Mushroom>());
		break;
	case MonsterIndex::Mushroom2:
		m_playerMonsterList.push_back(std::make_shared<Mushroom>());
		break;
	case MonsterIndex::Mushroom3:
		m_playerMonsterList.push_back(std::make_shared<Mushroom>());
		break;
	case MonsterIndex::Mushroom4:
		m_playerMonsterList.push_back(std::make_shared<Mushroom>());
		break;
	default:
		break;
	}
}
void GameScene::CreateEnemyMonster()
{
	if(m_enemyHouse->GetHp() <= 0)
	{
		return;
	}
	m_enemySpawnFrame++;

	//たけのこ
	if (m_enemySpawnFrame % m_enemy1SpawnInterval == 0)
	{
		//敵の出現フレームを初期化
		m_enemy1SpawnInterval = kEnemySpawnInterval + MyMath::GetRand(-kEnemySpawnRandomRange, kEnemySpawnRandomRange);
		//敵モンスターを生成
		m_enemyMonsterList.push_back(std::make_shared<Takenoko>());
	}
}
