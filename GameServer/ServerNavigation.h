#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct ServerPosition
{
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
};

struct NavigationObstacle
{
	uint64 objectId = 0;
	float minX = 0.0f;
	float maxX = 0.0f;
	float minZ = 0.0f;
	float maxZ = 0.0f;
};

enum class MoveRejectReason
{
	None,
	InvalidNumber,
	OutOfBounds,
	TooFast,
	TerrainMismatch,
	Collision
};

struct MoveValidationResult
{
	ServerPosition position;
	MoveRejectReason reason = MoveRejectReason::None;
	bool corrected = false;
};

class ServerNavigation
{
public:
	bool Init();
	bool IsReady() const { return !_heightMap.empty(); }
	MoveValidationResult ValidateMove(const ServerPosition& current, const ServerPosition& requested,
		float elapsedSeconds, const std::vector<NavigationObstacle>& obstacles) const;
	float GetTerrainHeight(float worldX, float worldZ) const;
	static constexpr float GetPlayerRadius() { return PlayerRadius; }

private:
	bool LoadHeightMap(const std::string& path);
	bool IsFinite(const ServerPosition& position) const;
	bool IsInsideWorld(float x, float z) const;
	bool IsBlocked(float x, float z, const std::vector<NavigationObstacle>& obstacles) const;
	bool IsPathBlocked(const ServerPosition& from, const ServerPosition& to,
		const std::vector<NavigationObstacle>& obstacles) const;
	ServerPosition ResolveCollision(const ServerPosition& current, const ServerPosition& requested,
		const std::vector<NavigationObstacle>& obstacles, bool& collided) const;

private:
	static constexpr int32 HeightMapWidth = 513;
	static constexpr int32 HeightMapLength = 513;
	static constexpr float WorldSize = 500.0f;
	static constexpr float WorldOffset = 249.0f;
	static constexpr float HeightScale = 2.0f;
	static constexpr float ScaleX = WorldSize / static_cast<float>(HeightMapWidth);
	static constexpr float ScaleZ = WorldSize / static_cast<float>(HeightMapLength);
	static constexpr float PlayMin = -62.0f;
	static constexpr float PlayMax = 62.0f;
	static constexpr float PlayerRadius = 0.65f;
	static constexpr float MaximumMoveSpeed = 6.0f;
	static constexpr float NetworkTolerance = 0.75f;

	std::vector<uint8_t> _heightMap;
};

extern ServerNavigation GNavigation;
