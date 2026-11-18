#include "../TopDownMap.h"

#include <cassert>
#include <cmath>

int main()
{
	TopDownMap map;
	map.Load("Resources/maps/rooms/topdown_room.tmj");

	assert(map.Width() == 25);
	assert(map.Height() == 25);
	assert(std::abs(map.SpawnX()) < 0.001f);
	assert(std::abs(map.SpawnZ()) < 0.001f);
	assert(map.HasFloor(12, 12));
	assert(!map.CollidesWithWall(map.SpawnX(), map.SpawnZ()));

	// 壁の中心、側面、角のそれぞれでプレイヤーの円形判定を確認する。
	assert(map.IsWall(9, 11));
	assert(map.CollidesWithWall(-12.0f, 4.0f));
	assert(map.CollidesWithWall(-9.61f, 4.0f));
	assert(!map.CollidesWithWall(-9.59f, 4.0f));
	assert(map.CollidesWithWall(-9.75f, 6.25f));
	assert(!map.CollidesWithWall(-9.70f, 6.30f));

	// 外周の壁とマップ外も通行できない。
	assert(map.CollidesWithWall(-28.0f, 0.0f));
	assert(map.IsWall(-1, 0));
	assert(map.CollidesWithWall(49.8f, 0.0f));

	// Tiled で床・壁・スポーンを編集した別サイズの部屋も同じ読み込み処理で反映する。
	TopDownMap editedMap;
	editedMap.Load("Tests/fixtures/topdown_room_small.tmj");
	assert(editedMap.Width() == 5 && editedMap.Height() == 5);
	assert(editedMap.HasFloor(2, 2));
	assert(!editedMap.HasFloor(3, 2));
	assert(editedMap.IsWall(1, 2));
	assert(std::abs(editedMap.SpawnX() - 4.0f) < 0.001f);
	assert(std::abs(editedMap.SpawnZ() - 4.0f) < 0.001f);
	assert(editedMap.CollidesWithWall(-4.0f, 0.0f));
	assert(editedMap.CollidesWithWall(4.0f, 0.0f));
	assert(!editedMap.CollidesWithWall(editedMap.SpawnX(), editedMap.SpawnZ()));
}
