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

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// factory implementation
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

template <>
inline Simd4iFactory<const int&>::operator Simd4i() const
{
	return vdupq_n_s32( v );
}

inline Simd4iFactory<detail::FourTuple>::operator Simd4i() const
{
	return reinterpret_cast<const Simd4i&>(v);
}

template <int i>
inline Simd4iFactory<detail::IntType<i> >::operator Simd4i() const
{
	return vdupq_n_u32(i);
}

template <>
inline Simd4iFactory<const int*>::operator Simd4i() const
{
	return vld1q_s32( v );
}

template <>
inline Simd4iFactory<detail::AlignedPointer<int> >::operator Simd4i() const
{
	return vld1q_s32( v.ptr );
}

template <>
inline Simd4iFactory<detail::OffsetPointer<int> >::operator Simd4i() const
{
	return vld1q_s32( reinterpret_cast<const int*>(
		reinterpret_cast<const char*>(v.ptr) + v.offset) );
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// expression template
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

template <>
inline ComplementExpr<Simd4i>::operator Simd4i() const
{
	return vbicq_u32( vdupq_n_u32(0xffffffff), v.u4 );
}

Simd4i operator&( const ComplementExpr<Simd4i>& complement, const Simd4i& v)
{
	return vbicq_u32(v.u4, complement.v.u4);
}

Simd4i operator&( const Simd4i& v, const ComplementExpr<Simd4i>& complement)
{
	return vbicq_u32(v.u4, complement.v.u4);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// operator implementations
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

Simd4i simdi::operator==( const Simd4i& v0, const Simd4i& v1 )
{
	return vceqq_u32( v0.u4, v1.u4 );
}

Simd4i simdi::operator<( const Simd4i& v0, const Simd4i& v1 )
{
	return vcltq_s32( v0.i4, v1.i4 );
}

Simd4i simdi::operator>( const Simd4i& v0, const Simd4i& v1 )
{
	return vcgtq_s32( v0.i4, v1.i4 );
}

ComplementExpr<Simd4i> operator~( const Simd4i& v )
{
	return ComplementExpr<Simd4i>(v);
}

Simd4i operator&( const Simd4i& v0, const Simd4i& v1 )
{
	return vandq_u32( v0.u4, v1.u4 );
}

Simd4i operator|( const Simd4i& v0, const Simd4i& v1 )
{
	return vorrq_u32( v0.u4, v1.u4 );
}

Simd4i operator^( const Simd4i& v0, const Simd4i& v1 )
{
	return veorq_u32( v0.u4, v1.u4 );
}

Simd4i operator<<( const Simd4i& v, int shift )
{
	return vshlq_u32( v.u4, vdupq_n_s32(shift) );
}

Simd4i operator>>( const Simd4i& v, int shift )
{
	return vshlq_u32( v.u4, vdupq_n_s32(-shift) );
}

Simd4i operator<<( const Simd4i& v, const Simd4i& shift )
{
	return vshlq_u32( v.u4, shift.i4 );
}

Simd4i operator>>( const Simd4i& v, const Simd4i& shift )
{
	return vshlq_u32( v.u4, vnegq_s32(shift.i4) );
}

Simd4i simdi::operator+( const Simd4i& v0, const Simd4i& v1 )
{
	return vaddq_u32( v0.u4, v1.u4 );
}

Simd4i simdi::operator-( const Simd4i& v )
{
	return vnegq_s32( v.i4 );
}

Simd4i simdi::operator-( const Simd4i& v0, const Simd4i& v1 )
{
	return vsubq_u32( v0.u4, v1.u4 );
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// function implementations
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

Simd4i simd4i( const Simd4f& v )
{
	return v.u4;
}

int (&simdi::array( Simd4i& v ))[4]
{
	return (int(&)[4])v;
}

const int (&simdi::array( const Simd4i& v ))[4]
{
	return (const int(&)[4])v;
}

void store( int* ptr, const Simd4i& v )
{
	return vst1q_s32( ptr, v.i4 );
}

void storeAligned( int* ptr, const Simd4i& v )
{
	vst1q_s32( ptr, v.i4 );
}

void storeAligned( int* ptr, unsigned int offset, const Simd4i& v )
{
	return storeAligned(reinterpret_cast<int*>(
		reinterpret_cast<char*>(ptr) + offset), v );
}

template <size_t i>
Simd4i splat( Simd4i const& v )
{
	return vdupq_n_s32(simdi::array(v)[i]);
}

Simd4i select( Simd4i const& mask, Simd4i const& v0, Simd4i const& v1 )
{
	return vbslq_u32(mask.u4, v0.u4, v1.u4);
}

int simdi::allEqual( const Simd4i& v0, const Simd4i& v1 )
{
	return allTrue(simdi::operator==(v0, v1));
}

int simdi::allEqual( const Simd4i& v0, const Simd4i& v1, Simd4i& outMask )
{
	return allTrue(outMask = simdi::operator==(v0, v1));
}

int simdi::anyEqual( const Simd4i& v0, const Simd4i& v1 )
{
	return anyTrue(simdi::operator==(v0, v1));
}

int simdi::anyEqual( const Simd4i& v0, const Simd4i& v1, Simd4i& outMask )
{
	return anyTrue(outMask = simdi::operator==(v0, v1));
}

int simdi::allGreater( const Simd4i& v0, const Simd4i& v1 )
{
	return allTrue(simdi::operator>(v0, v1));
}

int simdi::allGreater( const Simd4i& v0, const Simd4i& v1, Simd4i& outMask )
{
	return allTrue(outMask = simdi::operator>(v0, v1));
}

int simdi::anyGreater( const Simd4i& v0, const Simd4i& v1 )
{
	return anyTrue(simdi::operator>(v0, v1));
}

int simdi::anyGreater( const Simd4i& v0, const Simd4i& v1, Simd4i& outMask )
{
	return anyTrue(outMask = simdi::operator>(v0, v1));
}

int allTrue( const Simd4i& v )
{
#if NVMATH_INLINE_ASSEMBLER
	int result;
	asm volatile(
        "vmovq q0, %q1 \n\t"
		"vand.u32 d0, d0, d1 \n\t"
		"vpmin.u32 d0, d0, d0 \n\t"
		"vcmp.f32 s0, #0 \n\t"
		"fmrx %0, fpscr" 
		: "=r" (result) 
		: "w" (v.u4) 
		: "q0" );
	return result >> 28 & 0x1;
#else
	uint16x4_t hi = vget_high_u16(vreinterpretq_u16_u32(v.u4));
	uint16x4_t lo = vmovn_u32(v.u4);
	uint16x8_t combined = vcombine_u16(lo, hi);
	uint32x2_t reduced = vreinterpret_u32_u8(vmovn_u16(combined));
	return vget_lane_u32(reduced, 0) == 0xffffffff;
#endif
}

int anyTrue( const Simd4i& v )
{
#if NVMATH_INLINE_ASSEMBLER
	int result;
	asm volatile(
		"vmovq q0, %q1 \n\t"
        "vorr.u32 d0, d0, d1 \n\t"
		"vpmax.u32 d0, d0, d0 \n\t"
		"vcmp.f32 s0, #0 \n\t"
		"fmrx %0, fpscr" 
		: "=r" (result) 
		: "w" (v.u4) 
		: "q0" );
	return result >> 28 & 0x1;
#else
	uint16x4_t hi = vget_high_u16(vreinterpretq_u16_u32(v.u4));
	uint16x4_t lo = vmovn_u32(v.u4);
	uint16x8_t combined = vcombine_u16(lo, hi);
	uint32x2_t reduced = vreinterpret_u32_u8(vmovn_u16(combined));
	return vget_lane_u32(reduced, 0) != 0x0;
#endif
}
