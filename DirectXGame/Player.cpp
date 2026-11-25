#include "Player.h"

#include "GameTime.h"
#include "MyMathUtility.h"
#include "TopDownMap.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>

using namespace KamataEngine;

void Player::Initialize(Model* model, Camera* camera, const TopDownMap* map,
	const Vector3& position)
{
	assert(model && camera && map);
	model_ = model;
	camera_ = camera;
	map_ = map;
	facingX_ = 0.0f;
	facingZ_ = 1.0f;
	dodgeDirectionX_ = 0.0f;
	dodgeDirectionZ_ = 1.0f;
	dodgeTimeRemaining_ = 0.0f;
	dodgeCooldownRemaining_ = 0.0f;
	color_.Initialize();
	color_.SetColor({1.0f, 1.0f, 1.0f, 1.0f});
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	UpdateMatrix();
}

void Player::MoveAxis(float distance, bool horizontal)
{
	if (distance == 0.0f)
	{
		return;
	}

	// 回避中も壁や床のないマスを飛び越えないよう、移動量を分割する。
	constexpr float kMaxStep = TopDownMap::kTileSize * 0.25f;
	const int32_t steps = (std::max)(1, static_cast<int32_t>(std::ceil(std::abs(distance) / kMaxStep)));
	const float stepDistance = distance / steps;
	for (int32_t step = 0; step < steps; ++step)
	{
		float& coordinate = horizontal ? worldTransform_.translation_.x : worldTransform_.translation_.z;
		const float start = coordinate;
		const auto isBlocked = [this, horizontal, start, stepDistance](float ratio)
		{
			const float x = horizontal ? start + stepDistance * ratio : worldTransform_.translation_.x;
			const float z = horizontal ? worldTransform_.translation_.z : start + stepDistance * ratio;
			return map_->CollidesWithWall(x, z);
		};

		if (!isBlocked(1.0f))
		{
			coordinate = start + stepDistance;
			continue;
		}

		// 壁に当たったときは接触する手前まで移動する。
		float freeRatio = 0.0f;
		float blockedRatio = 1.0f;
		for (int32_t i = 0; i < 10; ++i)
		{
			const float middle = (freeRatio + blockedRatio) * 0.5f;
			if (isBlocked(middle))
			{
				blockedRatio = middle;
			}
			else
			{
				freeRatio = middle;
			}
		}
		coordinate = start + stepDistance * freeRatio;
		break;
	}
}

void Player::Update()
{
	Input* input = Input::GetInstance();
	const int32_t horizontal =
		static_cast<int32_t>(input->PushKey(DIK_D) || input->PushKey(DIK_RIGHT)) -
		static_cast<int32_t>(input->PushKey(DIK_A) || input->PushKey(DIK_LEFT));
	const int32_t vertical =
		static_cast<int32_t>(input->PushKey(DIK_W) || input->PushKey(DIK_UP)) -
		static_cast<int32_t>(input->PushKey(DIK_S) || input->PushKey(DIK_DOWN));
	const float deltaSeconds = GameTime::GetDeltaSeconds();
	dodgeCooldownRemaining_ = (std::max)(0.0f, dodgeCooldownRemaining_ - deltaSeconds);

	float directionX = facingX_;
	float directionZ = facingZ_;
	if (horizontal != 0 || vertical != 0)
	{
		const float inverseLength = 1.0f / std::sqrt(static_cast<float>(horizontal * horizontal + vertical * vertical));
		directionX = horizontal * inverseLength;
		directionZ = vertical * inverseLength;
	}

	if (!IsDodging() && input->TriggerKey(DIK_SPACE) && dodgeCooldownRemaining_ <= 0.0f)
	{
		// 移動入力がないときは最後に向いた方向へ回避する。
		dodgeDirectionX_ = directionX;
		dodgeDirectionZ_ = directionZ;
		facingX_ = directionX;
		facingZ_ = directionZ;
		dodgeTimeRemaining_ = kDodgeDuration;
		dodgeCooldownRemaining_ = kDodgeCooldown;
		worldTransform_.rotation_.y = std::atan2(directionX, directionZ);
	}

	if (IsDodging())
	{
		const float moveSeconds = (std::min)(deltaSeconds, dodgeTimeRemaining_);
		const float distance = kDodgeSpeed * moveSeconds;
		MoveAxis(dodgeDirectionX_ * distance, true);
		MoveAxis(dodgeDirectionZ_ * distance, false);
		dodgeTimeRemaining_ -= moveSeconds;
		color_.SetColor({0.65f, 0.90f, 1.0f, 1.0f});
	}
	else
	{
		if (horizontal != 0 || vertical != 0)
		{
			facingX_ = directionX;
			facingZ_ = directionZ;
			const float distance = kMoveSpeed * deltaSeconds;
			MoveAxis(directionX * distance, true);
			MoveAxis(directionZ * distance, false);
			worldTransform_.rotation_.y = std::atan2(directionX, directionZ);
		}
		color_.SetColor({1.0f, 1.0f, 1.0f, 1.0f});
	}
	UpdateMatrix();
}

void Player::UpdateMatrix()
{
	worldTransform_.matWorld_ = MyMathUtility::MakeAffineMatrix(
		worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();
}

void Player::Draw()
{
	model_->Draw(worldTransform_, *camera_, &color_);
}
