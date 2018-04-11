/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  



#include "SnippetCamera.h"
#include <ctype.h>
#include "foundation/PxMat33.h"
using namespace physx;

namespace Snippets
{

	Camera::Camera(const PxVec3 &eye, const PxVec3& dir)
	{
		mEye = eye;
		mDir = dir.getNormalized();
		mMouseX = 0;
		mMouseY = 0;
	}

	void Camera::handleMouse(int button, int state, int x, int y)
	{
		PX_UNUSED(state);
		PX_UNUSED(button);
		mMouseX = x;
		mMouseY = y;
	}

	bool Camera::handleKey(unsigned char key, int x, int y, float speed)
	{
		PX_UNUSED(x);
		PX_UNUSED(y);

		PxVec3 viewY = mDir.cross(PxVec3(0, 1, 0)).getNormalized();
		switch (toupper(key))
		{
		case '8':	mEye += mDir*2.0f*speed;		break;
		case '5':	mEye -= mDir*2.0f*speed;		break;
		case '4':	mEye -= viewY*2.0f*speed;		break;
		case '6':	mEye += viewY*2.0f*speed;		break;
		default:	return false;
		}
		return true;
	}

	void Camera::handleAnalogMove(float x, float y)
	{
		PxVec3 viewY = mDir.cross(PxVec3(0, 1, 0)).getNormalized();
		mEye += mDir*y;
		mEye += viewY*x;
	}

	void Camera::handleMotion(int x, int y)
	{
		int dx = mMouseX - x;
		int dy = mMouseY - y;

		PxVec3 viewY = mDir.cross(PxVec3(0, 1, 0)).getNormalized();

		PxQuat qx(PxPi * dx / 180.0f, PxVec3(0, 1, 0));
		mDir = qx.rotate(mDir);
		PxQuat qy(PxPi * dy / 180.0f, viewY);
		mDir = qy.rotate(mDir);

		mDir.normalize();

		mMouseX = x;
		mMouseY = y;
	}

	PxTransform Camera::getTransform() const
	{
		PxVec3 viewY = mDir.cross(PxVec3(0, 1, 0));

		if (viewY.normalize() < 1e-6f)
			return PxTransform(mEye);

		//PxMat33 m(mDir.cross(viewY), viewY, -mDir);
		PxMat33 m(mDir.cross(viewY), viewY, mDir);
		return PxTransform(mEye, PxQuat(m));
	}

	PxVec3 Camera::getEye() const
	{
		return mEye;
	}

	PxVec3 Camera::getDir() const
	{
		return mDir;
	}

}

