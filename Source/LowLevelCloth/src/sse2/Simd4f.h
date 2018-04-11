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
inline Simd4fFactory<const float&>::operator Simd4f() const
{
	return _mm_set1_ps( v );
}

inline Simd4fFactory<detail::FourTuple>::operator Simd4f() const
{
	return reinterpret_cast<const Simd4f&>(v);
}

template <>
inline Simd4fFactory<detail::IntType<0> >::operator Simd4f() const
{
	return _mm_setzero_ps();
}

template <>
inline Simd4fFactory<detail::IntType<1> >::operator Simd4f() const
{
	return _mm_set1_ps(1.0f);
}

template <>
inline Simd4fFactory<detail::IntType<int(0x80000000)> >::operator Simd4f() const
{
	return _mm_castsi128_ps(_mm_set1_epi32( 0x80000000 ));
}

template <>
inline Simd4fFactory<detail::IntType<int(0xffffffff)> >::operator Simd4f() const
{
	return _mm_castsi128_ps(_mm_set1_epi32( -1 ));
}

template <>
inline Simd4fFactory<const float*>::operator Simd4f() const
{
	return _mm_loadu_ps( v );
}

template <>
inline Simd4fFactory<detail::AlignedPointer<float> >::operator Simd4f() const
{
	return _mm_load_ps( v.ptr );
}

template <>
inline Simd4fFactory<detail::OffsetPointer<float> >::operator Simd4f() const
{
	return _mm_load_ps( reinterpret_cast<const float*>(
		reinterpret_cast<const char*>(v.ptr) + v.offset) );
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// expression template
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

template <>
inline ComplementExpr<Simd4f>::operator Simd4f() const
{
	return _mm_andnot_ps(v, _mm_castsi128_ps(_mm_set1_epi32( -1 )));
}

Simd4f operator&( const ComplementExpr<Simd4f>& complement, const Simd4f& v)
{
	return _mm_andnot_ps(complement.v, v);
}

Simd4f operator&( const Simd4f& v, const ComplementExpr<Simd4f>& complement)
{
	return _mm_andnot_ps(complement.v, v);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// operator implementations
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

Simd4f operator==( const Simd4f& v0, const Simd4f& v1 )
{
	return _mm_cmpeq_ps( v0 , v1 );
}

Simd4f operator<( const Simd4f& v0, const Simd4f& v1 )
{
	return _mm_cmplt_ps( v0, v1 );
}

Simd4f operator<=( const Simd4f& v0, const Simd4f& v1 )
{
	return _mm_cmple_ps( v0, v1 );
}

Simd4f operator>( const Simd4f& v0, const Simd4f& v1 )
{
	return _mm_cmpgt_ps( v0, v1 );
}

Simd4f operator>=( const Simd4f& v0, const Simd4f& v1 )
{
	return _mm_cmpge_ps( v0, v1 );
}

ComplementExpr<Simd4f> operator~( const Simd4f& v )
{
	return ComplementExpr<Simd4f>(v);
}

Simd4f operator&( const Simd4f& v0, const Simd4f& v1 )
{
	return _mm_and_ps( v0, v1 );
}

Simd4f operator|( const Simd4f& v0, const Simd4f& v1 )
{
	return _mm_or_ps( v0, v1 );
}

Simd4f operator^( const Simd4f& v0, const Simd4f& v1 )
{
	return _mm_xor_ps( v0, v1 );
}

Simd4f operator<<( const Simd4f& v, int shift )
{
	return _mm_castsi128_ps(_mm_slli_epi32(
		_mm_castps_si128(v), shift));
}

Simd4f operator>>( const Simd4f& v, int shift )
{
	return _mm_castsi128_ps(_mm_srli_epi32(
		_mm_castps_si128(v), shift));
}

Simd4f operator+( const Simd4f& v )
{
	return v;
}

Simd4f operator+( const Simd4f& v0, const Simd4f& v1 )
{
	return _mm_add_ps( v0, v1 );
}

Simd4f operator-( const Simd4f& v )
{
	return _mm_sub_ps( _mm_setzero_ps(), v );
}

Simd4f operator-( const Simd4f& v0, const Simd4f& v1 )
{
	return _mm_sub_ps( v0, v1 );
}

Simd4f operator*( const Simd4f& v0, const Simd4f& v1 )
{
	return _mm_mul_ps(v0, v1);
}

Simd4f operator/( const Simd4f& v0, const Simd4f& v1 )
{
	return _mm_div_ps( v0, v1 );
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// function implementations
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

Simd4f simd4f( const Simd4i& v )
{
	return _mm_castsi128_ps(v);
}

float (&array( Simd4f& v ))[4]
{
	return reinterpret_cast<float(&)[4]>(v);
}

const float (&array( const Simd4f& v ))[4]
{
	return reinterpret_cast<const float(&)[4]>(v);
}

void store( float* ptr, Simd4f const& v )
{
	_mm_storeu_ps( ptr, v );
}

void storeAligned( float* ptr, Simd4f const& v )
{
	_mm_store_ps( ptr, v );
}

void storeAligned( float* ptr, unsigned int offset, Simd4f const& v )
{
	_mm_store_ps( reinterpret_cast<float*>(
		reinterpret_cast<char*>(ptr) + offset), v );
}

template <size_t i>
Simd4f splat( Simd4f const& v )
{
	return _mm_shuffle_ps(v, v, _MM_SHUFFLE(i,i,i,i));
}

Simd4f select( Simd4f const& mask, Simd4f const& v0, Simd4f const& v1 )
{
	return _mm_xor_ps( v1, _mm_and_ps( mask, _mm_xor_ps( v1, v0 ) ) );
}

Simd4f abs( const Simd4f& v )
{
	return _mm_andnot_ps(_mm_castsi128_ps(_mm_set1_epi32( 0x80000000 )), v);
}

Simd4f floor( const Simd4f& v )
{
	// SSE 4.1: return _mm_floor_ps(v);
	Simd4i i = _mm_cvttps_epi32(v);
	return _mm_cvtepi32_ps(_mm_sub_epi32(i, _mm_srli_epi32(i, 31)));
}

Simd4f max(const Simd4f& v0, const Simd4f& v1)
{
	return _mm_max_ps(v0, v1);
}

Simd4f min(const Simd4f& v0, const Simd4f& v1)
{
	return _mm_min_ps(v0, v1);
}

Simd4f recip( const Simd4f& v )
{
	return _mm_rcp_ps( v );
}

template <int n>
Simd4f recipT( const Simd4f& v )
{
	Simd4f two = simd4f(2.0f);
	Simd4f recipV = recip(v);
	for(int i=0; i<n; ++i)
		recipV = recipV * (two - v * recipV);
	return recipV;
}

Simd4f sqrt( const Simd4f& v )
{
	return _mm_sqrt_ps( v );
}

Simd4f rsqrt( const Simd4f& v )
{
	return _mm_rsqrt_ps( v );
}

template <int n>
Simd4f rsqrtT( const Simd4f& v )
{
	Simd4f halfV = v * simd4f(0.5f);
	Simd4f threeHalf = simd4f(1.5f);
	Simd4f rsqrtV = rsqrt(v);
	for(int i=0; i<n; ++i)
		rsqrtV = rsqrtV * (threeHalf - halfV * rsqrtV * rsqrtV);
	return rsqrtV;
}

Simd4f exp2( const Simd4f& v )
{
	// http://www.netlib.org/cephes/

	Simd4f limit = simd4f(127.4999f);
	Simd4f x = min(max(-limit, v), limit);

	// separate into integer and fractional part

	Simd4f fx = x + simd4f(0.5f);
	Simd4i ix = _mm_sub_epi32(_mm_cvttps_epi32(fx),
		_mm_srli_epi32(_mm_castps_si128(fx), 31));
	fx = x - Simd4f(_mm_cvtepi32_ps(ix));

	// exp2(fx) ~ 1 + 2*P(fx) / (Q(fx) - P(fx))

	Simd4f fx2 = fx * fx;

	Simd4f px = fx * (simd4f(1.51390680115615096133e+3f) +
		fx2 * (simd4f(2.02020656693165307700e+1f) +
		fx2 * simd4f(2.30933477057345225087e-2f)));
	Simd4f qx = simd4f(4.36821166879210612817e+3f) +
		fx2 * (simd4f(2.33184211722314911771e+2f) + fx2);
		
	Simd4f exp2fx = px * recip(qx - px);
	exp2fx = simd4f(_1) + exp2fx + exp2fx;

	// exp2(ix)

	Simd4f exp2ix = _mm_castsi128_ps(_mm_slli_epi32(
		_mm_add_epi32(ix, _mm_set1_epi32(0x7f)), 23));

	return exp2fx * exp2ix;
}

Simd4f log2( const Simd4f& v )
{
	// todo: fast approximate implementation like exp2
	Simd4f scale = simd4f(1.44269504088896341f); // 1/ln(2)
	const float* ptr = array(v);
	return simd4f(::logf(ptr[0]), ::logf(ptr[1]), 
		::logf(ptr[2]), ::logf(ptr[3])) * scale;
}

Simd4f dot3( const Simd4f& v0, const Simd4f& v1 )
{
	Simd4f tmp = v0 * v1;
	return splat<0>(tmp) + splat<1>(tmp) + splat<2>(tmp);
}

Simd4f cross3( const Simd4f& v0, const Simd4f& v1 )
{
	Simd4f t0 = _mm_shuffle_ps(v0, v0, 0xc9); // w z y x -> w x z y
	Simd4f t1 = _mm_shuffle_ps(v1, v1, 0xc9); 
	Simd4f tmp = v0 * t1 - t0 * v1;
	return _mm_shuffle_ps(tmp, tmp, 0xc9); 
}

void transpose(Simd4f& x, Simd4f& y, Simd4f& z, Simd4f& w)
{
	_MM_TRANSPOSE4_PS(x, y, z, w);
}

int allEqual( const Simd4f& v0, const Simd4f& v1 )
{
	return allTrue(v0 == v1);
}

int allEqual( const Simd4f& v0, const Simd4f& v1, Simd4f& outMask )
{
	return allTrue(outMask = v0 == v1);
}

int anyEqual( const Simd4f& v0, const Simd4f& v1 )
{
	return anyTrue(v0 == v1);
}

int anyEqual( const Simd4f& v0, const Simd4f& v1, Simd4f& outMask )
{
	return anyTrue(outMask = v0 == v1);
}

int allGreater( const Simd4f& v0, const Simd4f& v1 )
{
	return allTrue(v0 > v1);
}

int allGreater( const Simd4f& v0, const Simd4f& v1, Simd4f& outMask )
{
	return allTrue(outMask = v0 > v1);
}

int anyGreater( const Simd4f& v0, const Simd4f& v1 )
{
	return anyTrue(v0 > v1);
}

int anyGreater( const Simd4f& v0, const Simd4f& v1, Simd4f& outMask )
{
	return anyTrue(outMask = v0 > v1);
}

int allGreaterEqual( const Simd4f& v0, const Simd4f& v1 )
{
	return allTrue(v0 >= v1);
}

int allGreaterEqual( const Simd4f& v0, const Simd4f& v1, Simd4f& outMask )
{
	return allTrue(outMask = v0 >= v1);
}

int anyGreaterEqual( const Simd4f& v0, const Simd4f& v1 )
{
	return anyTrue(v0 >= v1);
}

int anyGreaterEqual( const Simd4f& v0, const Simd4f& v1, Simd4f& outMask )
{
	return anyTrue(outMask = v0 >= v1);
}

int allTrue( const Simd4f& v )
{
	return _mm_movemask_ps(v) == 0xf;
}

int anyTrue( const Simd4f& v )
{
	return _mm_movemask_ps(v);
}
