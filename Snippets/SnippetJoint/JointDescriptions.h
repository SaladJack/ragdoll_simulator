#pragma once
#include "PxPhysicsAPI.h"
using namespace physx;

struct JointDrive
{
	float positionSpring; ///< The spring used to reach the target. range{ 0, infinity }
	float positionDamper; ///< The damping used to reach the target. range{ 0, infinity }
	float maximumForce; ///< The maximum force the drive can exert to reach the target velocity. range{ 0, infinity }

	JointDrive();
	void SetToDefaults();
	void SetToCharacterJointDefaults();

	physx::PxD6JointDrive ToPx() const;

};

struct SoftJointLimit
{
	float limit;
	float bounciness; ///< When the joint hits the limit. This will determine how bouncy it will be. range{ 0, 1 }
	float contactDistance; ///< Solver hint. Setting this low can cause jittering, but runs fast. Setting high can reduce jittering, but runs the solver more often (0 = use default) [Translation limit: Meters, Rotation limit: Degrees]. range{ 0, infinity }

	SoftJointLimit();
	void SetToDefaults();

};

struct SoftJointLimitSpring
{
	float spring; ///< If greater than zero, the limit is soft. The spring will pull the joint back. range{ 0, infinity }
	float damper; ///< If spring is greater than zero, the limit is soft. This is the damping of spring. range{ 0, infinity }

	SoftJointLimitSpring();
	void SetToDefaults();

};

//-----------------------------------------------------------------------------

inline JointDrive::JointDrive()
{
	SetToDefaults();
}

inline void JointDrive::SetToDefaults()
{
	positionSpring = 0.0F;
	positionDamper = 0.0F;
	maximumForce = PX_MAX_F32;
}

inline void JointDrive::SetToCharacterJointDefaults()
{
	positionSpring = 50.0F;
	positionDamper = 5.0F;
	maximumForce = 20.F;
}

inline physx::PxD6JointDrive JointDrive::ToPx() const
{
	physx::PxD6JointDrive out;
	out.stiffness = positionSpring;
	out.damping = positionDamper;
	out.forceLimit = maximumForce;
	return out;
}

//-----------------------------------------------------------------------------
inline SoftJointLimit::SoftJointLimit()
{
	SetToDefaults();
}

inline void SoftJointLimit::SetToDefaults()
{
	limit = 0.0F;
	bounciness = 0.0F;
	contactDistance = 0.0F;
}
//-----------------------------------------------------------------------------

inline SoftJointLimitSpring::SoftJointLimitSpring()
{
	SetToDefaults();
}

inline void SoftJointLimitSpring::SetToDefaults()
{
	spring = 0.f;
	damper = 0.f;
}

inline float PostprocessJointConstraintDistance(float distance, float bounciness)
{
	if (bounciness > 0.f)
	{
		return 1e-5f;
	}

	if (distance < 0.2f)
		return 0.2f;
	return distance;
}
inline float ChooseContactDistanceForCtor(float contactDistancePropertyVal, float clampedContactDistance)
{
	if (contactDistancePropertyVal > 0.0f)
		return clampedContactDistance;
	return -1.0f;
}
inline float FixLimitConeContactDistance(float yAngle, float zAngle, float restitution, float originalPropertyVal, float clampedContactDistance)
{
	PxJointLimitCone wrap(yAngle, zAngle, ChooseContactDistanceForCtor(originalPropertyVal, clampedContactDistance));
	return PostprocessJointConstraintDistance(wrap.contactDistance, restitution);
}
inline float FixAngularLimitContactDistance(float lower, float upper, float restitution, float originalPropertyVal, float clampedContactDistance)
{
	// only constructor can do proper clamping.
	physx::PxJointAngularLimitPair wrap(lower, upper, ChooseContactDistanceForCtor(originalPropertyVal, clampedContactDistance));
	return PostprocessJointConstraintDistance(wrap.contactDistance, restitution);
}
