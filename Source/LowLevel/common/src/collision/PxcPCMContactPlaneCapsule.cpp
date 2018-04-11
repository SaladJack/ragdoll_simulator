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

#include "GuGeometryUnion.h"

#include "PxcNpCache.h"
#include "PxcNpThreadContext.h"
#include "PxcContactMethodImpl.h"

namespace physx
{

bool PxcPCMContactPlaneCapsule(CONTACT_METHOD_ARGS)
{
	return pcmContactPlaneCapsule(shape0, shape1, transform0, transform1, contactDistance, npCache, contactBuffer);
}

}
