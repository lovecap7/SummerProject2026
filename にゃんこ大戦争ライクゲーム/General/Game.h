#pragma once

//基本16:9
namespace Game//属性
{
	constexpr int kScreenWidth = 1280;
	constexpr int kScreenHeight = 720;
	constexpr int kColorBitNum = 32;
	constexpr int kScreenCenterX = kScreenWidth / 2;
	constexpr int kScreenCenterY = kScreenHeight / 2;

	//プレイヤーのモンスターの初期座標
	constexpr float kPlayerMonsterPosX = kScreenWidth - 200;
	constexpr float kPlayerMonsterPosY = kScreenCenterY + 160;

	//敵のモンスターの初期座標
	constexpr float kEnemyMonsterPosX = 200;
	constexpr float kEnemyMonsterPosY = kScreenCenterY + 160;

	//家の座標
	constexpr float kPlayerHousePosX = kPlayerMonsterPosX + 10;
	constexpr float kPlayerHousePosY = kPlayerMonsterPosY - 80;
	constexpr float kEnemyHousePosX = kEnemyMonsterPosX - 10;
	constexpr float kEnemyHousePosY = kEnemyMonsterPosY - 80;

	//プレイヤー側か敵側か
	enum class Team
	{
		Player,
		Enemy
	};
}