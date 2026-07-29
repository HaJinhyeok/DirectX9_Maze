#include "CombatCollision.h"

#include <cmath>

SphereSweepResult SweepSphereAgainstSphere(
	const MazeWorldPosition& startPosition,
	const MazeWorldPosition& endPosition,
	float movingRadius,
	const CollisionSphere& target) noexcept
{
	SphereSweepResult result;

	const float movementX = endPosition.x - startPosition.x;
	const float movementY = endPosition.y - startPosition.y;
	const float movementZ = endPosition.z - startPosition.z;

	const float offsetX = startPosition.x - target.center.x;
	const float offsetY = startPosition.y - target.center.y;
	const float offsetZ = startPosition.z - target.center.z;

	const float combinedRadius = movingRadius + target.radius;

	const float quadraticC =
		offsetX * offsetX +
		offsetY * offsetY +
		offsetZ * offsetZ -
		combinedRadius * combinedRadius;

	if (quadraticC <= 0.0f)
	{
		result.didHit = true;
		result.hitTime = 0.0f;
		result.hitPosition = startPosition;
		return result;
	}

	const float quadraticA =
		movementX * movementX +
		movementY * movementY +
		movementZ * movementZ;

	constexpr float kMinimumMovementSquared = 1.0e-8f;

	if (quadraticA <= kMinimumMovementSquared)
	{
		return result;
	}

	const float quadraticB = 2.0f * (
		offsetX * movementX +
		offsetY * movementY +
		offsetZ * movementZ);

	const float discriminant = quadraticB * quadraticB - 4.0f * quadraticA * quadraticC;

	if (discriminant < 0.0f)
	{
		return result;
	}

	const float hitTime =
		(-quadraticB - std::sqrt(discriminant)) /
		(2.0f * quadraticA);

	if (hitTime < 0.0f || hitTime > 1.0f)
	{
		return result;
	}

	result.didHit = true;
	result.hitTime = hitTime;
	result.hitPosition =
	{
		startPosition.x + movementX * hitTime,
		startPosition.y + movementY * hitTime,
		startPosition.z + movementZ * hitTime
	};

	return result;
}

bool DoSpheresOverlap(
	const CollisionSphere& first,
	const CollisionSphere& second) noexcept
{
	const float deltaX = first.center.x - second.center.x;
	const float deltaY = first.center.y - second.center.y;
	const float deltaZ = first.center.z - second.center.z;

	const float distanceSquared =
		deltaX * deltaX +
		deltaY * deltaY +
		deltaZ * deltaZ;

	const float combinedRadius = first.radius + second.radius;

	return distanceSquared <= combinedRadius * combinedRadius;
}