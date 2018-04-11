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


#ifndef PHYSX_SNIPPET_RENDER_H
#define PHYSX_SNIPPET_RENDER_H

#include "PxPhysicsAPI.h"
#include "foundation/PxPreprocessor.h"

#if defined(PX_WINDOWS)
#include <windows.h>
#pragma warning(disable: 4505)
#include <glut.h>
#elif defined(PX_LINUX)
#include <GL/glut.h>
#elif defined(PX_APPLE) && !defined(PX_APPLE_IOS)
#include <GLUT/glut.h>
#elif defined(PX_PS3)
#include <GL/glut.h>
#else
#error platform not supported.
#endif

namespace Snippets
{
void setupDefaultWindow(const char* name);
void setupDefaultRenderState();

void startRender(const physx::PxVec3& cameraEye, const physx::PxVec3& cameraDir);
void renderActors(physx::PxRigidActor** actors, const physx::PxU32 numActors, bool shadows = false, const physx::PxVec3 & color = physx::PxVec3(1.0, 1.0, 224.f/255.f));
void renderLine(const physx::PxVec3& start, const physx::PxVec3& end, const physx::PxVec3& color);
void finishRender();
}

#define MAX_NUM_ACTOR_SHAPES 128

#endif //PHYSX_SNIPPET_RENDER_H
