#pragma once

#include "KamataEngine.h"
#include <array>
#include <cstdint>

// XZ を地面、Y を高さとする斜め見下ろし操作の最小シーン。
class TopDownScene {
public:
	TopDownScene() = default;
	~TopDownScene();

	void Initialize();
	void Update();
	void Draw();

private:
	static constexpr int32_t kFloorTilesPerSide = 25;
	static constexpr float kTileSize = 4.0f;
	static constexpr float kMoveSpeed = 7.0f;
	static constexpr float kPlayerLimit = 20.0f;
	static constexpr KamataEngine::Vector3 kCameraOffset = {0.0f, 12.0f, -9.0f};

	KamataEngine::Camera camera_;
	KamataEngine::Model* playerModel_ = nullptr;
	KamataEngine::Model* floorModel_ = nullptr;
	KamataEngine::WorldTransform playerTransform_;
	std::array<KamataEngine::WorldTransform, kFloorTilesPerSide * kFloorTilesPerSide> floorTransforms_;
	KamataEngine::ObjectColor floorColors_[2];
	uint32_t whiteTextureHandle_ = 0;

	static void UpdateTransform(KamataEngine::WorldTransform& transform);
};
