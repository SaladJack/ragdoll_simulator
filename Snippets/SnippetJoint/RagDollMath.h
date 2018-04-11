#pragma once
// Math
inline float Angle(PxVec3 from, PxVec3 to)
{
	return PxAcos(PxClamp(from.getNormalized().dot(to.getNormalized()), -1.0f, 1.0f));
}

inline PxReal Deg2Rad(PxReal rad)
{
	return PxPi * rad / 180;
}

inline void AlignToVector(PxRigidDynamic* part, PxVec3 alignmentVector, PxVec3 targetVector, float stability, float speed)
{
	if (part == NULL)
	{
		return;
	}
	PxQuat r;
	if (part->getAngularVelocity().magnitude() == 0.f)
	{
		r = PxQuat(1.0f);
	}
	else
	{
		r = PxQuat(Deg2Rad(part->getAngularVelocity().magnitude() * 180 / PxPi *  stability / speed), part->getAngularVelocity().getNormalized());
	}
	//PxVec3 a = PxVec3.Cross(Quaternion.AngleAxis(part.angularVelocity.magnitude * 57.29578f * stability / speed, part.angularVelocity) * alignmentVector, targetVector * 10f);
	PxVec3 a = r.rotate(alignmentVector).cross(targetVector * 10.f);
	part->addTorque(a * speed* speed);
}

// angle:Radians
inline bool SurfaceWithinAngle(PxVec3 normal, PxVec3 direction, float angle)
{
	return Angle(normal, direction) <= angle;
}
#include <stdlib.h>
inline float Random(float start, float end)
{
	return start + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (end - start)));
}