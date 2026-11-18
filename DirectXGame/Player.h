#pragma once

#include "KamataEngine.h"

class TopDownMap;

// XZ 平面を移動するプレイヤー。モデル・カメラ・マップの所有権は GameScene にある。
class Player {
public:
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera,
		const TopDownMap* map, const KamataEngine::Vector3& position);
	void Update();
	void Draw();

	const KamataEngine::WorldTransform& GetWorldTransform() const { return worldTransform_; }
	bool IsDodging() const { return dodgeTimeRemaining_ > 0.0f; }

private:
	static constexpr float kMoveSpeed = 7.0f;
	static constexpr float kDodgeSpeed = 20.0f;
	static constexpr float kDodgeDuration = 0.22f;
	static constexpr float kDodgeCooldown = 0.75f;

	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Camera* camera_ = nullptr;
	const TopDownMap* map_ = nullptr;
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::ObjectColor color_;
	float facingX_ = 0.0f;
	float facingZ_ = 1.0f;
	float dodgeDirectionX_ = 0.0f;
	float dodgeDirectionZ_ = 1.0f;
	float dodgeTimeRemaining_ = 0.0f;
	float dodgeCooldownRemaining_ = 0.0f;

	void MoveAxis(float distance, bool horizontal);
	void UpdateMatrix();
};
