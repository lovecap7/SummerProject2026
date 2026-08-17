#pragma once
#include "../General/Math//Vector2.h"
#include "../General/Game.h"
class House
{
public :
	House(Game::Team team);
	~House();
	void Init();
	void Update();
	void Draw() const;
	Game::Team GetTeam() const { return m_team; }
	Vector2 GetPos() const { return m_pos; }
	void Damage(int damage);
	int GetHp() const { return m_hp; }
private:
	int m_houseImage = -1;
	Vector2 m_pos;
	Game::Team m_team = Game::Team::Player;
	int m_hp = 100;

	//ダメージを喰らった時に揺らす用のタイマー
	int m_damageTimer = 0;

};

