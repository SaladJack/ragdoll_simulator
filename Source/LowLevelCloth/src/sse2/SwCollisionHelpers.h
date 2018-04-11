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

#pragma once

#ifdef PX_GNUC
	#include <xmmintrin.h> // _BitScanForward
#else
	#include <intrin.h> // _BitScanForward
#endif

namespace physx
{
namespace cloth
{

	uint32_t findBitSet(uint32_t mask)
	{
	#if _MSC_VER
		unsigned long result;
		_BitScanForward(&result, unsigned long(mask));
		return result;
	#else
		return __builtin_ffs(mask) - 1;
	#endif
	}

	Simd4i intFloor( const Simd4f& v )
	{
		Simd4i i = _mm_cvttps_epi32(v);
		return simdi::operator-(i, _mm_srli_epi32(simd4i(v), 31));
	}

	Simd4i horizontalOr(Simd4i mask)
	{
		Simd4i tmp = mask | _mm_shuffle_epi32(mask, 0xb1); // w z y x -> z w x y
		return tmp | _mm_shuffle_epi32(tmp, 0x4e); // w z y x -> y x w z
	}

	Gather<Simd4i>::Gather(const Simd4i& index)
	{
		mSelectQ = _mm_srai_epi32(index << 29, 31);
		mSelectD = _mm_srai_epi32(index << 30, 31);
		mSelectW = _mm_srai_epi32(index << 31, 31);
		mOutOfRange = simdi::operator>(index ^ sIntSignBit, sSignedMask);
	}

	Simd4i Gather<Simd4i>::operator()(const Simd4i* ptr) const
	{
		// more efficient with _mm_shuffle_epi8 (SSSE3)
		Simd4i lo = ptr[0], hi = ptr[1];
		Simd4i m01 = select(mSelectW, splat<1>(lo), splat<0>(lo));
		Simd4i m23 = select(mSelectW, splat<3>(lo), splat<2>(lo));
		Simd4i m45 = select(mSelectW, splat<1>(hi), splat<0>(hi));
		Simd4i m67 = select(mSelectW, splat<3>(hi), splat<2>(hi));
		Simd4i m0123 = select(mSelectD, m23, m01);
		Simd4i m4567 = select(mSelectD, m67, m45);
		return select(mSelectQ, m4567, m0123) & ~mOutOfRange;
	}

} // namespace cloth
} // namespace physx

