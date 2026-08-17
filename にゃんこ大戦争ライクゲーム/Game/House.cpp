#include "House.h"
#include <DxLib.h>

namespace
{
	//—h‚ç‚·ƒtƒŒ[ƒ€
	constexpr int kShakeFrame = 6;

	//—h‚ç‚·—Ê
	constexpr float kShakeAmount = 5.0f;
}

House::House(Game::Team team):
	m_team(team)
{
}

House::~House()
{
	DeleteGraph(m_houseImage);
}
void House::Init()
{
	if(m_team == Game::Team::Player)
	{
		m_houseImage = LoadGraph(L"Image/Mushroom/MushroomHouse.png");
	}
	else
	{
		m_houseImage = LoadGraph(L"Image/Takenoko/TakenokoHouse.png");
	}
	

	if(m_team == Game::Team::Player)
	{
		m_pos = Vector2(Game::kPlayerHousePosX, Game::kPlayerHousePosY);
	}
	else
	{
		m_pos = Vector2(Game::kEnemyHousePosX, Game::kEnemyHousePosY);
	}
}
void House::Update()
{
	if(m_damageTimer > 0)
	{
		m_damageTimer--;
	}
}
void House::Draw() const
{
	float shakeOffsetX = 0.0f;
	if(m_damageTimer > 0)
	{
		//—h‚ç‚·
		if(m_damageTimer % 2 == 0)
		{
			shakeOffsetX = kShakeAmount;
		}
		else
		{
			shakeOffsetX = -kShakeAmount;
		}
	}

	//‰Æ‚Ì•`‰æ
	DrawRotaGraphF(m_pos.x + shakeOffsetX, m_pos.y, 1.0, 0.0, m_houseImage, true, m_team == Game::Team::Enemy);
	//‘Ì—Í‚Ì•`‰æ
	DrawBox(m_pos.x - 50, m_pos.y - 80, m_pos.x + 50, m_pos.y - 70, 0x000000, true);

	if(m_team == Game::Team::Player)
	{
		DrawBox(m_pos.x - 50, m_pos.y - 80, m_pos.x - 50 + (100 * m_hp / 100), m_pos.y - 70, 0x00ff00, true);
	}
	else
	{
		DrawBox(m_pos.x - 50, m_pos.y - 80, m_pos.x - 50 + (100 * m_hp / 100), m_pos.y - 70, 0xff0000, true);
	}
}

void House::Damage(int damage)
{
	m_hp -= damage;
	if(m_hp < 0)
	{
		m_hp = 0;
	}
	if(m_damageTimer <= 0)
	{
		//ƒ_ƒ[ƒW‚ð‹ò‚ç‚Á‚½Žž‚É—h‚ç‚·
		m_damageTimer = kShakeFrame;
	}
}