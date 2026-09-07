#include "pch.h"
#include "ServerNavigation.h"

#include <algorithm>
#include <cmath>
#include <fstream>

ServerNavigation GNavigation;

bool ServerNavigation::Init()
{
	const char* candidates[] = {
		"Model/HeightMapTexture/terrain.raw",
		"../Model/HeightMapTexture/terrain.raw",
		"../../Model/HeightMapTexture/terrain.raw"
	};

	for (const char* path : candidates)
	{
		if (LoadHeightMap(path))
		{
			std::cout << "[NAVIGATION] Height map loaded: " << path << std::endl;
			return true;
		}
	}

	std::cerr << "[NAVIGATION] Could not find Model/HeightMapTexture/terrain.raw" << std::endl;
	return false;
}

bool ServerNavigation::LoadHeightMap(const std::string& path)
{
	std::ifstream file(path, std::ios::binary);
	if (!file.is_open())
		return false;

	const size_t pixelCount = static_cast<size_t>(HeightMapWidth) * HeightMapLength;
	std::vector<char> source(pixelCount);
	file.read(source.data(), static_cast<std::streamsize>(source.size()));
	if (file.gcount() != static_cast<std::streamsize>(pixelCount))
		return false;

	_heightMap.resize(pixelCount);
	for (int32 z = 0; z < HeightMapLength; ++z)
	{
		for (int32 x = 0; x < HeightMapWidth; ++x)
		{
			const size_t sourceIndex = static_cast<size_t>(x) + static_cast<size_t>(z) * HeightMapWidth;
			const size_t destinationIndex = static_cast<size_t>(x)
				+ static_cast<size_t>(HeightMapLength - 1 - z) * HeightMapWidth;
			_heightMap[destinationIndex] = static_cast<uint8_t>(
				static_cast<unsigned char>(source[sourceIndex]));
		}
	}
	return true;
}

bool ServerNavigation::IsFinite(const ServerPosition& position) const
{
	return std::isfinite(position.x) && std::isfinite(position.y) && std::isfinite(position.z);
}

bool ServerNavigation::IsInsideWorld(float x, float z) const
{
	return x >= PlayMin && x <= PlayMax && z >= PlayMin && z <= PlayMax;
}

void ServerNavigation::MarkGridDirty()
{
	std::lock_guard<std::mutex> guard(_gridMutex);
	_gridDirty = true;
}

void ServerNavigation::UpdateWalkableGrid(const std::vector<NavigationObstacle>& obstacles)
{
	std::lock_guard<std::mutex> guard(_gridMutex);
	if (!_gridDirty)
		return;

	_walkableGrid.assign(static_cast<size_t>(GridWidth) * GridHeight, 1);
	for (const NavigationObstacle& obstacle : obstacles)
	{
		const float cellHalf = CellSize * 0.5f;
		const float minX = obstacle.minX - PlayerRadius - cellHalf;
		const float maxX = obstacle.maxX + PlayerRadius + cellHalf;
		const float minZ = obstacle.minZ - PlayerRadius - cellHalf;
		const float maxZ = obstacle.maxZ + PlayerRadius + cellHalf;

		for (int32 gridZ = 0; gridZ < GridHeight; ++gridZ)
		{
			const float worldZ = PlayMin + (static_cast<float>(gridZ) + 0.5f) * CellSize;
			if (worldZ < minZ || worldZ > maxZ)
				continue;

			for (int32 gridX = 0; gridX < GridWidth; ++gridX)
			{
				const float worldX = PlayMin + (static_cast<float>(gridX) + 0.5f) * CellSize;
				if (worldX >= minX && worldX <= maxX)
					_walkableGrid[static_cast<size_t>(gridZ) * GridWidth + gridX] = 0;
			}
		}
	}

	_gridDirty = false;
	std::cout << "[NAVIGATION] Walkable grid rebuilt. obstacles=" << obstacles.size()
		<< " cells=" << _walkableGrid.size() << std::endl;
}

bool ServerNavigation::WorldToGrid(float x, float z, int32& gridX, int32& gridZ) const
{
	if (!IsInsideWorld(x, z))
		return false;

	gridX = static_cast<int32>((x - PlayMin) / CellSize);
	gridZ = static_cast<int32>((z - PlayMin) / CellSize);
	gridX = std::clamp(gridX, 0, GridWidth - 1);
	gridZ = std::clamp(gridZ, 0, GridHeight - 1);
	return true;
}

bool ServerNavigation::IsWalkable(float x, float z) const
{
	int32 gridX = 0;
	int32 gridZ = 0;
	if (!WorldToGrid(x, z, gridX, gridZ))
		return false;
	if (_walkableGrid.empty())
		return true;

	return _walkableGrid[static_cast<size_t>(gridZ) * GridWidth + gridX] != 0;
}

float ServerNavigation::GetTerrainHeight(float worldX, float worldZ) const
{
	if (_heightMap.empty())
		return 0.0f;

	float mapX = std::clamp((worldX + WorldOffset) / ScaleX, 0.0f,
		static_cast<float>(HeightMapWidth - 2));
	float mapZ = std::clamp((worldZ + WorldOffset) / ScaleZ, 0.0f,
		static_cast<float>(HeightMapLength - 2));
	const int32 x = static_cast<int32>(mapX);
	const int32 z = static_cast<int32>(mapZ);
	const float xRatio = mapX - static_cast<float>(x);
	const float zRatio = mapZ - static_cast<float>(z);

	const auto heightAt = [this](int32 hx, int32 hz)
	{
		const size_t index = static_cast<size_t>(hx) + static_cast<size_t>(hz) * HeightMapWidth;
		return static_cast<float>(_heightMap[index]);
	};

	const float bottom = heightAt(x, z) * (1.0f - xRatio) + heightAt(x + 1, z) * xRatio;
	const float top = heightAt(x, z + 1) * (1.0f - xRatio) + heightAt(x + 1, z + 1) * xRatio;
	return (bottom * (1.0f - zRatio) + top * zRatio) * HeightScale;
}

bool ServerNavigation::IsBlocked(float x, float z,
	const std::vector<NavigationObstacle>& obstacles) const
{
	for (const NavigationObstacle& obstacle : obstacles)
	{
		if (x >= obstacle.minX - PlayerRadius && x <= obstacle.maxX + PlayerRadius
			&& z >= obstacle.minZ - PlayerRadius && z <= obstacle.maxZ + PlayerRadius)
			return true;
	}
	return false;
}

bool ServerNavigation::IsPathBlocked(const ServerPosition& from, const ServerPosition& to,
	const std::vector<NavigationObstacle>& obstacles) const
{
	const float dx = to.x - from.x;
	const float dz = to.z - from.z;
	const float distance = std::sqrt(dx * dx + dz * dz);
	constexpr float sampleInterval = 0.25f;
	const int32 sampleCount = std::max(1, static_cast<int32>(std::ceil(distance / sampleInterval)));

	for (int32 i = 1; i <= sampleCount; ++i)
	{
		const float ratio = static_cast<float>(i) / sampleCount;
		const float sampleX = from.x + dx * ratio;
		const float sampleZ = from.z + dz * ratio;
		if (!IsWalkable(sampleX, sampleZ) || IsBlocked(sampleX, sampleZ, obstacles))
			return true;
	}
	return false;
}

ServerPosition ServerNavigation::ResolveCollision(const ServerPosition& current,
	const ServerPosition& requested, const std::vector<NavigationObstacle>& obstacles,
	bool& collided) const
{
	collided = false;
	if (!IsPathBlocked(current, requested, obstacles))
		return requested;

	collided = true;
	ServerPosition slideX = current;
	slideX.x = requested.x;
	slideX.y = requested.y;
	if (!IsPathBlocked(current, slideX, obstacles))
		return slideX;

	ServerPosition slideZ = current;
	slideZ.z = requested.z;
	slideZ.y = requested.y;
	if (!IsPathBlocked(current, slideZ, obstacles))
		return slideZ;

	return current;
}

MoveValidationResult ServerNavigation::ValidateMove(const ServerPosition& current,
	const ServerPosition& requested, float elapsedSeconds,
	const std::vector<NavigationObstacle>& dynamicObstacles) const
{
	std::lock_guard<std::mutex> guard(_gridMutex);
	MoveValidationResult result;
	result.position = current;
	if (!IsFinite(requested))
	{
		result.corrected = true;
		result.reason = MoveRejectReason::InvalidNumber;
		return result;
	}
	if (!IsInsideWorld(requested.x, requested.z))
	{
		result.corrected = true;
		result.reason = MoveRejectReason::OutOfBounds;
		return result;
	}

	elapsedSeconds = std::clamp(elapsedSeconds, 0.001f, 0.5f);
	const float dx = requested.x - current.x;
	const float dz = requested.z - current.z;
	const float distance = std::sqrt(dx * dx + dz * dz);
	if (distance > MaximumMoveSpeed * elapsedSeconds + NetworkTolerance)
	{
		result.corrected = true;
		result.reason = MoveRejectReason::TooFast;
		return result;
	}

	ServerPosition adjusted = requested;
	const float terrainHeight = GetTerrainHeight(requested.x, requested.z);
	// Keep the existing client-side jump functional; clamp only impossible Y values.
	if (adjusted.y < terrainHeight - 0.25f || adjusted.y > terrainHeight + 20.0f)
	{
		adjusted.y = terrainHeight;
		result.corrected = true;
		result.reason = MoveRejectReason::TerrainMismatch;
	}

	bool collided = false;
	result.position = ResolveCollision(current, adjusted, dynamicObstacles, collided);
	if (collided)
	{
		result.corrected = true;
		result.reason = MoveRejectReason::Collision;
	}
	return result;
}
