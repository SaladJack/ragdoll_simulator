#pragma once
#include "PxPhysicsAPI.h"
using namespace physx;
#define u2p(h,r) SCALE*(h/2-r)
#define SCALE 10.0f
const PxReal HEAD_RADIUS = 2.12391f / SCALE; //'radius': 0.212391
const PxReal HEAD_POS_Y = 16.0f / SCALE; // 'center': (0.0, 0.165338, 0.057292)

const PxReal CHEST_CAPSULE_HALF_LEN = u2p(0.55, 0.193) / SCALE; //'height': 0.55 
const PxReal CHEST_CAPSULE_RAIDUS = 1.93f / SCALE; //  'radius': 0.193
const PxReal CHEST_POS_Y = HEAD_POS_Y - HEAD_RADIUS - CHEST_CAPSULE_RAIDUS;

const PxReal WAIST_CAPSULE_HALF_LEN = u2p(0.49, 0.18) / SCALE; // 'height': 0.49
const PxReal WAIST_CAPSULE_RADIUS = 1.8f / SCALE; //  'radius': 0.18
const PxReal WAIST_POS_Y = CHEST_POS_Y - CHEST_CAPSULE_RAIDUS - WAIST_CAPSULE_RADIUS;

const PxReal STOMACH_CAPSULE_HALF_LEN = u2p(0.437, 0.116) / SCALE; // 'height': 0.437
const PxReal STOMACH_CAPSULE_RADIUS = 1.16f / SCALE; // 'radius': 0.116
const PxReal STOMACH_POS_Y = WAIST_POS_Y - WAIST_CAPSULE_RADIUS;
const PxReal STOMACH_POS_Z = 1.0f / SCALE;

const PxReal HIPS_CAPSULE_HALF_LEN = u2p(0.49, 0.178) / SCALE; // 'height': 0.49
const PxReal HIPS_CAPSULE_RADIUS = 1.78f / SCALE; // 'radius': 0.178
const PxReal HIPS_POS_Y = WAIST_POS_Y - STOMACH_CAPSULE_RADIUS - HIPS_CAPSULE_RADIUS;

const PxReal CROTCH_CAPSULE_HALF_LEN = u2p(0.35, 0.1) / SCALE; //'height': 0.35
const PxReal CROTCH_CAPSULE_RADIUS = 1.0f / SCALE; // 'radius': 0.1
const PxReal CROTCH_POS_Y = HIPS_POS_Y - HIPS_CAPSULE_RADIUS - CROTCH_CAPSULE_RADIUS;
const PxReal CROTCH_POS_Z = 0;// 2.0f / SCALE;

const PxReal ARM_CAPSULE_HALF_LEN = u2p(0.404093,0.06572)/SCALE;//u2p(0.404093, 0.065752) / SCALE; //'height': 0.404093
const PxReal ARM_CAPSULE_RADIUS = 0.65752f / SCALE; // 'radius': 0.065752
const PxReal ARM_POS_Y = CHEST_POS_Y;
const PxReal ARM_POS_X = ARM_CAPSULE_HALF_LEN + ARM_CAPSULE_RADIUS + CHEST_CAPSULE_HALF_LEN + CHEST_CAPSULE_RAIDUS;

const PxReal FOR_ARM_CAPSULE_HALF_LEN = u2p(0.409499, 0.048657) / SCALE / 1.2f; //'height': 0.409499
const PxReal FOR_ARM_CAPSULE_RADIUS = 0.48657f / SCALE; // 'radius': 0.048657
const PxReal FOR_ARM_POS_Y = CHEST_POS_Y;
const PxReal FOR_ARM_POS_X = ARM_POS_X + +ARM_CAPSULE_HALF_LEN + ARM_CAPSULE_RADIUS + FOR_ARM_CAPSULE_HALF_LEN + FOR_ARM_CAPSULE_RADIUS;

const PxReal HAND_RADIUS = 0.07f;//0.4f / SCALE; //'radius': 0.04
const PxReal HAND_POS_Y = CHEST_POS_Y;	// 'center': (-2.268119, 1.30762, 4.757563)
const PxReal HAND_POS_X = FOR_ARM_POS_X + FOR_ARM_CAPSULE_HALF_LEN + FOR_ARM_CAPSULE_RADIUS + HAND_RADIUS;

const PxReal THIGH_HALF_HEIGHT = u2p(0.3, 0.06) / SCALE; // 'height': 0.3
const PxReal THIGH_RADIUS = 0.6f / SCALE;	//  'radius': 0.06
const PxReal THIGH_POS_X = 3.0f / SCALE;
const PxReal THIGH_POS_Y = CROTCH_POS_Y - CROTCH_CAPSULE_RADIUS / 2;

const PxReal LEG_HALF_HEIGHT = u2p(0.15, 0.05) / SCALE;//u2p(0.2, 0.05) / SCALE; //  'height': 0.2,
const PxReal LEG_RADIUS = 0.5f / SCALE;		//  'radius': 0.05
const PxReal LEG_POS_X = THIGH_POS_X;
const PxReal LEG_POS_Y = THIGH_POS_Y - THIGH_HALF_HEIGHT - LEG_HALF_HEIGHT / 2;

const PxReal FOOT_RADIUS = 0.1f / SCALE;
const PxReal FOOT_POS_X = LEG_POS_X;
const PxReal FOOT_POS_Y = LEG_POS_Y - LEG_HALF_HEIGHT - LEG_RADIUS;

const PxReal BALL_RADIUS = 2.0f / SCALE;//0.200000
const PxReal BALL_POS_Y = HIPS_POS_Y - HIPS_CAPSULE_RADIUS;//-1.456972, 0.705614, 4.766696
const PxReal BALL_POS_Z = -0.0f / SCALE;

const PxReal SPRING_HELPER_RADIUS = 0.01f;
const PxReal SPRING_HELPER_POS_Y = BALL_POS_Y + SPRING_HELPER_RADIUS;