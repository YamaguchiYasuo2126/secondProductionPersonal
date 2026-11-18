#include "GameScene.h"

#include "GameTime.h"
#include "MyMathUtility.h"

#include <cmath>

using namespace KamataEngine;

GameScene::~GameScene()
{
	player_.reset();
	delete playerModel_;
	delete tileModel_;
}

void GameScene::UpdateTransform(WorldTransform& transform)
{
	transform.matWorld_ = MyMathUtility::MakeAffineMatrix(
		transform.scale_, transform.rotation_, transform.translation_);
	transform.TransferMatrix();
}

void GameScene::Initialize()
{
	GameTime::SetTimeScale(1.0f);
	map_.Load("Resources/maps/rooms/topdown_room.tmj");
	playerModel_ = Model::CreateFromOBJ("player", true);
	tileModel_ = Model::CreateFromOBJ("mountainStone", true);
	whiteTextureHandle_ = TextureManager::Load("white1x1.png");

	floorColors_[0].Initialize();
	floorColors_[0].SetColor({0.30f, 0.40f, 0.47f, 1.0f});
	floorColors_[1].Initialize();
	floorColors_[1].SetColor({0.38f, 0.48f, 0.53f, 1.0f});
	wallColor_.Initialize();
	wallColor_.SetColor({0.20f, 0.26f, 0.32f, 1.0f});

	const size_t cellCount = static_cast<size_t>(map_.Width()) * map_.Height();
	floorTransforms_ = std::make_unique<WorldTransform[]>(cellCount);
	wallTransforms_ = std::make_unique<WorldTransform[]>(cellCount);

	for (int32_t row = 0; row < map_.Height(); ++row)
	{
		for (int32_t column = 0; column < map_.Width(); ++column)
		{
			const int32_t index = row * map_.Width() + column;
			if (map_.HasFloor(column, row))
			{
				WorldTransform& tile = floorTransforms_[index];
				tile.Initialize();
				tile.scale_ = {TopDownMap::kTileSize * 1.030f, 0.15f, TopDownMap::kTileSize * 1.030f};
				tile.translation_ = {map_.CellX(column), -0.20f, map_.CellZ(row)};
				UpdateTransform(tile);
			}

			if (map_.IsWall(column, row))
			{
				WorldTransform& wall = wallTransforms_[index];
				wall.Initialize();
				wall.scale_ = {TopDownMap::kTileSize * 1.02f, 3.0f, TopDownMap::kTileSize * 1.02f};
				wall.translation_ = {map_.CellX(column), 1.35f, map_.CellZ(row)};
				UpdateTransform(wall);
			}
		}
	}

	camera_.rotation_ = {std::atan2(kCameraOffset.y, -kCameraOffset.z), 0.0f, 0.0f};
	camera_.Initialize();
	player_ = std::make_unique<Player>();
	player_->Initialize(playerModel_, &camera_, &map_,
		{map_.SpawnX(), 0.40f, map_.SpawnZ()});
	const Vector3& playerPosition = player_->GetWorldTransform().translation_;
	camera_.translation_ = {
		playerPosition.x + kCameraOffset.x,
		playerPosition.y + kCameraOffset.y,
		playerPosition.z + kCameraOffset.z};
	camera_.UpdateMatrix();
}

void GameScene::Update()
{
	player_->Update();
	const Vector3& playerPosition = player_->GetWorldTransform().translation_;
	const float followRate = GameTime::GetLerpFactor(0.06f);
	const float targetX = playerPosition.x + kCameraOffset.x;
	const float targetZ = playerPosition.z + kCameraOffset.z;
	camera_.translation_.x += (targetX - camera_.translation_.x) * followRate;
	camera_.translation_.z += (targetZ - camera_.translation_.z) * followRate;
	camera_.UpdateMatrix();
}

void GameScene::Draw()
{
	Model::PreDraw();
	for (int32_t row = 0; row < map_.Height(); ++row)
	{
		for (int32_t column = 0; column < map_.Width(); ++column)
		{
			if (map_.HasFloor(column, row))
			{
				const int32_t index = row * map_.Width() + column;
				tileModel_->Draw(floorTransforms_[index], camera_, whiteTextureHandle_, &floorColors_[(column + row) % 2]);
			}
		}
	}
	for (int32_t row = 0; row < map_.Height(); ++row)
	{
		for (int32_t column = 0; column < map_.Width(); ++column)
		{
			if (map_.IsWall(column, row))
			{
				const int32_t index = row * map_.Width() + column;
				tileModel_->Draw(wallTransforms_[index], camera_, whiteTextureHandle_, &wallColor_);
			}
		}
	}
	player_->Draw();
	Model::PostDraw();
}
