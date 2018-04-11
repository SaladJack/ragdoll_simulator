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

#ifndef PS_WINDOWS_AOS_H
#define PS_WINDOWS_AOS_H

// no includes here! this file should be included from PxcVecMath.h only!!!

#if !COMPILE_VECTOR_INTRINSICS
#error Vector intrinsics should not be included when using scalar implementation.
#endif

typedef __m128 FloatV;
typedef __m128 Vec3V;
typedef __m128 Vec4V;
typedef __m128 BoolV;
typedef __m128 VecU32V;
typedef __m128 VecI32V;
typedef __m128 VecU16V;
typedef __m128 VecI16V;
typedef __m128 VecU8V;
typedef __m128 QuatV; 

#define FloatVArg	FloatV&
#define	Vec3VArg	Vec3V&
#define	Vec4VArg	Vec4V&
#define BoolVArg	BoolV&
#define VecU32VArg	VecU32V&
#define VecI32VArg  VecI32V&
#define VecU16VArg  VecU16V&
#define VecI16VArg  VecI16V&
#define VecU8VArg   VecU8V&
#define QuatVArg	QuatV&

//Optimization for situations in which you cross product multiple vectors with the same vector.
//Avoids 2X shuffles per product
struct VecCrossV
{
	Vec3V mL1;
	Vec3V mR1;
};

struct VecShiftV
{
	VecI32V shift;
};
#define VecShiftVArg VecShiftV&

PX_ALIGN_PREFIX(16)
struct Mat33V
{
	Mat33V(){}
	Mat33V(const Vec3V& c0, const Vec3V& c1, const Vec3V& c2)
		: col0(c0),
		  col1(c1),
		  col2(c2)
	{
	}
	Vec3V PX_ALIGN(16,col0);
	Vec3V PX_ALIGN(16,col1);
	Vec3V PX_ALIGN(16,col2);
}PX_ALIGN_SUFFIX(16);

PX_ALIGN_PREFIX(16)
struct Mat34V
{
	Mat34V(){}
	Mat34V(const Vec3V& c0, const Vec3V& c1, const Vec3V& c2, const Vec3V& c3)
		: col0(c0),
		  col1(c1),
		  col2(c2),
		  col3(c3)
	{
	}
	Vec3V PX_ALIGN(16,col0);
	Vec3V PX_ALIGN(16,col1);
	Vec3V PX_ALIGN(16,col2);
	Vec3V PX_ALIGN(16,col3);
}PX_ALIGN_SUFFIX(16);

PX_ALIGN_PREFIX(16)
struct Mat43V
{
	Mat43V(){}
	Mat43V(const Vec4V& c0, const Vec4V& c1, const Vec4V& c2)
		: col0(c0),
		  col1(c1),
		  col2(c2)
	{
	}
	Vec4V PX_ALIGN(16,col0);
	Vec4V PX_ALIGN(16,col1);
	Vec4V PX_ALIGN(16,col2);
}PX_ALIGN_SUFFIX(16);

PX_ALIGN_PREFIX(16)
struct Mat44V
{
	Mat44V(){}
	Mat44V(const Vec4V& c0, const Vec4V& c1, const Vec4V& c2, const Vec4V& c3)
		: col0(c0),
		  col1(c1),
		  col2(c2),
		  col3(c3)
	{
	}
	Vec4V PX_ALIGN(16,col0);
	Vec4V PX_ALIGN(16,col1);
	Vec4V PX_ALIGN(16,col2);
	Vec4V PX_ALIGN(16,col3);
}PX_ALIGN_SUFFIX(16);


#endif //PS_WINDOWS_AOS_H
