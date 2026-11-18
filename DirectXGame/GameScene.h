#pragma once

#include "KamataEngine.h"
#include "Player.h"
#include "TopDownMap.h"

#include <cstdint>
#include <memory>

// 見下ろしゲームのマップ、プレイヤー、固定角度カメラを組み立てる。
class GameScene {
public:
	GameScene() = default;
	~GameScene();

	void Initialize();
	void Update();
	void Draw();

private:
	static constexpr KamataEngine::Vector3 kCameraOffset = {0.0f, 12.0f, -9.0f};

	TopDownMap map_;
	KamataEngine::Camera camera_;
	std::unique_ptr<Player> player_;
	KamataEngine::Model* playerModel_ = nullptr;
	KamataEngine::Model* tileModel_ = nullptr;
	std::unique_ptr<KamataEngine::WorldTransform[]> floorTransforms_;
	std::unique_ptr<KamataEngine::WorldTransform[]> wallTransforms_;
	KamataEngine::ObjectColor floorColors_[2];
	KamataEngine::ObjectColor wallColor_;
	uint32_t whiteTextureHandle_ = 0;

	static void UpdateTransform(KamataEngine::WorldTransform& transform);
};
