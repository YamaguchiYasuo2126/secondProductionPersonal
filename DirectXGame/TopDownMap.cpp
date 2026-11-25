#include "TopDownMap.h"

#include "ThirdParty/nlohmann/json.hpp"

#include <algorithm>
#include <fstream>
#include <functional>
#include <stdexcept>
#include <utility>

using json = nlohmann::json;

namespace {

constexpr uint32_t kTiledFlipFlags = 0xE0000000u;
constexpr int32_t kMaxCells = 4096;

std::vector<uint8_t> ReadTileLayer(const json& layer, int32_t width, int32_t height,
                                   const std::string& path)
{
	if (layer.value("x", 0) != 0 || layer.value("y", 0) != 0 ||
		layer.value("width", width) != width || layer.value("height", height) != height ||
		!layer.contains("data") || !layer["data"].is_array() ||
		layer["data"].size() != static_cast<size_t>(width) * height)
	{
		throw std::runtime_error("Top-down tile layer must be a finite CSV array matching the map size: " + path);
	}

	std::vector<uint8_t> cells(static_cast<size_t>(width) * height);
	for (size_t index = 0; index < cells.size(); ++index)
	{
		const uint32_t gid = layer["data"][index].get<uint32_t>() & ~kTiledFlipFlags;
		cells[index] = gid == 0 ? 0u : 1u;
	}
	return cells;
}

} // namespace

void TopDownMap::Load(const std::string& path)
{
	std::ifstream file(path);
	if (!file)
	{
		throw std::runtime_error("Top-down Tiled map could not be opened: " + path);
	}

	json map;
	file >> map;
	if (map.value("orientation", "") != "orthogonal" || map.value("infinite", false))
	{
		throw std::runtime_error("Top-down map must be a finite orthogonal Tiled map: " + path);
	}

	const int32_t width = map.at("width").get<int32_t>();
	const int32_t height = map.at("height").get<int32_t>();
	const int32_t tileWidth = map.at("tilewidth").get<int32_t>();
	const int32_t tileHeight = map.at("tileheight").get<int32_t>();
	if (width <= 0 || height <= 0 || tileWidth <= 0 || tileHeight <= 0 ||
		static_cast<int64_t>(width) * height > kMaxCells)
	{
		throw std::runtime_error("Top-down map has an unsupported size: " + path);
	}

	bool foundFloor = false;
	bool foundWalls = false;
	bool foundSpawn = false;
	std::vector<uint8_t> floors;
	std::vector<uint8_t> walls;
	float spawnX = 0.0f;
	float spawnZ = 0.0f;

	std::function<void(const json&)> readLayers = [&](const json& layers)
	{
		for (const json& layer : layers)
		{
			const std::string type = layer.value("type", "");
			const std::string name = layer.value("name", "");
			if (type == "group")
			{
				readLayers(layer.at("layers"));
			}
			else if (type == "tilelayer" && name == "Floor")
			{
				if (foundFloor)
				{
					throw std::runtime_error("Top-down map has multiple Floor layers: " + path);
				}
				floors = ReadTileLayer(layer, width, height, path);
				foundFloor = true;
			}
			else if (type == "tilelayer" && name == "CollisionSolid")
			{
				if (foundWalls)
				{
					throw std::runtime_error("Top-down map has multiple CollisionSolid layers: " + path);
				}
				walls = ReadTileLayer(layer, width, height, path);
				foundWalls = true;
			}
			else if (type == "objectgroup" && name == "Spawns")
			{
				for (const json& object : layer.at("objects"))
				{
					std::string objectType = object.value("type", "");
					if (objectType.empty())
					{
						objectType = object.value("class", "");
					}
					if (objectType != "PlayerSpawn")
					{
						continue;
					}
					if (foundSpawn || !object.value("point", false))
					{
						throw std::runtime_error("Top-down map needs one PlayerSpawn point: " + path);
					}
					const float pixelX = object.at("x").get<float>();
					const float pixelY = object.at("y").get<float>();
					spawnX = (pixelX / tileWidth - width * 0.5f) * kTileSize;
					spawnZ = (height * 0.5f - pixelY / tileHeight) * kTileSize;
					foundSpawn = true;
				}
			}
		}
	};
	readLayers(map.at("layers"));

	if (!foundFloor || !foundWalls || !foundSpawn)
	{
		throw std::runtime_error("Top-down map needs Floor, CollisionSolid, and PlayerSpawn: " + path);
	}

	width_ = width;
	height_ = height;
	floors_ = std::move(floors);
	walls_ = std::move(walls);
	spawnX_ = spawnX;
	spawnZ_ = spawnZ;
	if (CollidesWithWall(spawnX_, spawnZ_))
	{
		throw std::runtime_error("PlayerSpawn overlaps a wall or has no floor: " + path);
	}
}

bool TopDownMap::HasFloor(int32_t column, int32_t row) const
{
	return column >= 0 && column < width_ && row >= 0 && row < height_ &&
		floors_[static_cast<size_t>(row) * width_ + column] != 0;
}

bool TopDownMap::IsWall(int32_t column, int32_t row) const
{
	if (column < 0 || column >= width_ || row < 0 || row >= height_)
	{
		return true;
	}
	return walls_[static_cast<size_t>(row) * width_ + column] != 0;
}

float TopDownMap::CellX(int32_t column) const
{
	return (column - (width_ - 1) * 0.5f) * kTileSize;
}

float TopDownMap::CellZ(int32_t row) const
{
	return ((height_ - 1) * 0.5f - row) * kTileSize;
}

bool TopDownMap::CollidesWithWall(float x, float z) const
{
	// 床のないマスとマップ外も通行不可にする。
	const float halfWidth = width_ * kTileSize * 0.5f;
	const float halfHeight = height_ * kTileSize * 0.5f;
	if (x - kPlayerRadius < -halfWidth || x + kPlayerRadius > halfWidth ||
		z - kPlayerRadius < -halfHeight || z + kPlayerRadius > halfHeight)
	{
		return true;
	}

	constexpr float kHalfTile = kTileSize * 0.5f;
	for (int32_t row = 0; row < height_; ++row)
	{
		for (int32_t column = 0; column < width_; ++column)
		{
			if (HasFloor(column, row) && !IsWall(column, row))
			{
				continue;
			}
			const float wallX = CellX(column);
			const float wallZ = CellZ(row);
			const float nearestX = std::clamp(x, wallX - kHalfTile, wallX + kHalfTile);
			const float nearestZ = std::clamp(z, wallZ - kHalfTile, wallZ + kHalfTile);
			const float differenceX = x - nearestX;
			const float differenceZ = z - nearestZ;
			if (differenceX * differenceX + differenceZ * differenceZ < kPlayerRadius * kPlayerRadius)
			{
				return true;
			}
		}
	}
	return false;
}
