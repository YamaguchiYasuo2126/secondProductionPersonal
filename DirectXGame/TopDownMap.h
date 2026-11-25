#pragma once

#include <cstdint>
#include <string>
#include <vector>

// 見下ろしシーン専用の XZ 平面マップ。
// Tiled の上の行ほど +Z に配置する。
class TopDownMap {
public:
	static constexpr float kTileSize = 4.0f;
	static constexpr float kPlayerRadius = 0.40f;

	void Load(const std::string& path);
	int32_t Width() const { return width_; }
	int32_t Height() const { return height_; }
	bool HasFloor(int32_t column, int32_t row) const;
	bool IsWall(int32_t column, int32_t row) const;
	float CellX(int32_t column) const;
	float CellZ(int32_t row) const;
	float SpawnX() const { return spawnX_; }
	float SpawnZ() const { return spawnZ_; }
	bool CollidesWithWall(float x, float z) const;

private:
	int32_t width_ = 0;
	int32_t height_ = 0;
	std::vector<uint8_t> floors_;
	std::vector<uint8_t> walls_;
	float spawnX_ = 0.0f;
	float spawnZ_ = 0.0f;
};
