#pragma once
// Enum
enum Side
{
	Left,
	Right
};

enum Pose
{
	Bent,
	Forward,
	Straight,
	Behind
};

// 32 = 8(ActorID) - 24(bodypart)
// ActorID 全是0代表普通的通用物体
#define ACTOR_ID_SHIFT 24
#define ACTOR_FILTER_SHIFT 0xff000000
struct FilterGroup
{
	enum Enum {
		eHead = (1 << 0),
		eChest = (1 << 1),
		eWaist = (1 << 2),
		eStomach = (1 << 3),
		eHips = (1 << 4),
		eCrotch = (1 << 5),
		eBall = (1 << 6),

		eRightArm = (1 << 7),
		eRightForArm = (1 << 8),
		eRightHand = (1 << 9),
		eRightThigh = (1 << 10),
		eRightLeg = (1 << 11),

		eLeftArm = (1 << 12),
		eLeftForArm = (1 << 13),
		eLeftHand = (1 << 14),
		eLeftThigh = (1 << 15),
		eLeftLeg = (1 << 16),

		eGround = (1 << 17),
		eLeftFoot = (1 << 18),
		eRightFoot = (1 << 19),

		eSpringHelper = (1 << 20),
		eObstacales = (1 << 21)
	};
};

enum ActorState
{
	Dead = 0,
	Unconscious,
	Stand,
	Run,
	Jump,
	Fall,
	Climb
};
