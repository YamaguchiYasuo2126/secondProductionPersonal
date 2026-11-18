#include "TopDownScene.h"

#include "GameTime.h"
#include "MyMathUtility.h"

#include <algorithm>
#include <cmath>

using namespace KamataEngine;

TopDownScene::~TopDownScene()
{
	delete playerModel_;
	delete floorModel_;
}

void TopDownScene::UpdateTransform(WorldTransform& transform)
{
	transform.matWorld_ = MyMathUtility::MakeAffineMatrix(
		transform.scale_, transform.rotation_, transform.translation_);
	transform.TransferMatrix();
}

void TopDownScene::Initialize()
{
	GameTime::SetTimeScale(1.0f);
	playerModel_ = Model::CreateFromOBJ("player", true);
	floorModel_ = Model::CreateFromOBJ("mountainStone", true);
	whiteTextureHandle_ = TextureManager::Load("white1x1.png");

	floorColors_[0].Initialize();
	floorColors_[0].SetColor({0.30f, 0.40f, 0.47f, 1.0f});
	floorColors_[1].Initialize();
	floorColors_[1].SetColor({0.38f, 0.48f, 0.53f, 1.0f});

	// キューブの上面が Y=-0.05、プレイヤーの足がほぼ Y=0 になる。
	for (int32_t z = 0; z < kFloorTilesPerSide; ++z)
	{
		for (int32_t x = 0; x < kFloorTilesPerSide; ++x)
		{
			WorldTransform& tile = floorTransforms_[z * kFloorTilesPerSide + x];
			tile.Initialize();
			tile.scale_ = {kTileSize * 1.030f, 0.15f, kTileSize * 1.030f};
			tile.translation_ = {
				(x - kFloorTilesPerSide / 2) * kTileSize,
				-0.20f,
				(z - kFloorTilesPerSide / 2) * kTileSize};
			UpdateTransform(tile);
		}
	}

	playerTransform_.Initialize();
	playerTransform_.translation_ = {0.0f, 0.40f, 0.0f};
	UpdateTransform(playerTransform_);

	// +Z が画面の奥になる向きで、プレイヤーを斜め上から見下ろす。
	camera_.rotation_ = {std::atan2(kCameraOffset.y, -kCameraOffset.z), 0.0f, 0.0f};
	camera_.translation_ = {
		playerTransform_.translation_.x + kCameraOffset.x,
		playerTransform_.translation_.y + kCameraOffset.y,
		playerTransform_.translation_.z + kCameraOffset.z};
	camera_.Initialize();
	camera_.UpdateMatrix();
}

void TopDownScene::Update()
{
	Input* input = Input::GetInstance();
	const int32_t horizontal =
		static_cast<int32_t>(input->PushKey(DIK_D) || input->PushKey(DIK_RIGHT)) -
		static_cast<int32_t>(input->PushKey(DIK_A) || input->PushKey(DIK_LEFT));
	const int32_t vertical =
		static_cast<int32_t>(input->PushKey(DIK_W) || input->PushKey(DIK_UP)) -
		static_cast<int32_t>(input->PushKey(DIK_S) || input->PushKey(DIK_DOWN));

	if (horizontal != 0 || vertical != 0)
	{
		// 斜め入力を正規化し、縦横と同じ移動速度にする。
		const float inverseLength = 1.0f / std::sqrt(static_cast<float>(horizontal * horizontal + vertical * vertical));
		const float directionX = horizontal * inverseLength;
		const float directionZ = vertical * inverseLength;
		const float distance = kMoveSpeed * GameTime::GetDeltaSeconds();
		playerTransform_.translation_.x = std::clamp(
			playerTransform_.translation_.x + directionX * distance, -kPlayerLimit, kPlayerLimit);
		playerTransform_.translation_.z = std::clamp(
			playerTransform_.translation_.z + directionZ * distance, -kPlayerLimit, kPlayerLimit);
		playerTransform_.rotation_.y = std::atan2(directionX, directionZ);
		UpdateTransform(playerTransform_);
	}

	// 回転は固定し、XZ の座標だけを滑らかに追従させる。
	const float followRate = GameTime::GetLerpFactor(0.06f);
	const float targetX = playerTransform_.translation_.x + kCameraOffset.x;
	const float targetZ = playerTransform_.translation_.z + kCameraOffset.z;
	camera_.translation_.x += (targetX - camera_.translation_.x) * followRate;
	camera_.translation_.z += (targetZ - camera_.translation_.z) * followRate;
	camera_.UpdateMatrix();
}

void TopDownScene::Draw()
{
	Model::PreDraw();
	for (int32_t z = 0; z < kFloorTilesPerSide; ++z)
	{
		for (int32_t x = 0; x < kFloorTilesPerSide; ++x)
		{
			const WorldTransform& tile = floorTransforms_[z * kFloorTilesPerSide + x];
			floorModel_->Draw(tile, camera_, whiteTextureHandle_, &floorColors_[(x + z) % 2]);
		}
	}
	playerModel_->Draw(playerTransform_, camera_);
	Model::PostDraw();
}
