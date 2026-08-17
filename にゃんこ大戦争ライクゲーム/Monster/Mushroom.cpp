#include "Mushroom.h"
#include "../General/Game.h"
#include <DxLib.h>

namespace
{
	//画像サイズ
	int kImageWidth = 80;
	int kImageHeight = 64;

	//初期ステ
	int kInitHp = 10;
	int kInitAttack = 1;
	int kInitAttackSpeed = 1 * 60;
	int kInitSpeed = 1;
	int kInitRange = 50;
	int kInitAnimationSpeed = 6;
}

Mushroom::Mushroom():
	MonsterBase(Game::Team::Player)
{
	Init();
}

Mushroom::~Mushroom()
{
	DeleteGraph(m_walkAnim);
	DeleteGraph(m_attackAnim);
	DeleteGraph(m_hitAnim);
}

void Mushroom::Init()
{
	//初期位置
	m_pos = Vector2(Game::kPlayerMonsterPosX, Game::kPlayerMonsterPosY);

	//画像読み込み
	m_walkAnim = LoadGraph(L"Image/Mushroom/Mushroom-Run.png");
	m_attackAnim = LoadGraph(L"Image/Mushroom/Mushroom-Attack.png");
	m_hitAnim = LoadGraph(L"Image/Mushroom/Mushroom-Hit.png");

	int width, height;
	//歩きアニメーションのフレーム数を計算
	GetGraphSize(m_walkAnim, &width, &height);
	m_walkAnimFrameCount = width / kImageWidth;
	//攻撃アニメーションのフレーム数を計算
	GetGraphSize(m_attackAnim, &width, &height);
	m_attackAnimFrameCount = width / kImageWidth;
	//やられアニメーションのフレーム数を計算
	GetGraphSize(m_hitAnim, &width, &height);
	m_hitAnimFrameCount = width / kImageWidth;

	//初期ステータス
	m_hp = kInitHp;
	m_attack = kInitAttack;
	m_attackSpeed = kInitAttackSpeed;
	m_speed = kInitSpeed;
	m_range = kInitRange;
	m_animationSpeed = kInitAnimationSpeed;
}

void Mushroom::Draw() const
{
	DrawRectGraph(
		m_pos.x - kImageWidth / 2,   // 左上X = 足元X - 半分
		m_pos.y - kImageHeight,      // 左上Y = 足元Y - 高さ
		m_animationIndex * kImageWidth,
		0,
		kImageWidth,
		kImageHeight,
		m_currentAnim,
		TRUE);
}
