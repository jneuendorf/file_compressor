/*
 * Next lexicographical permutation algorithm (C)
 * By Nayuki Minase, 2014. Public domain.
 * http://nayuki.eigenstate.org/page/next-lexicographical-permutation-algorithm
 */

#include <stddef.h>
#include <stdio.h>


typedef unsigned long long number;
typedef unsigned char byte;

// byte get_bit_at(number *n, size_t idx);

// number* set_bit_at(number *n, size_t idx, byte val);

// bitwise permutation of a number
int next_permutation_bitwise(number *n, size_t length);

// previous bitwise permutation of a number
int prev_permutation_bitwise(number *n, size_t length) ;
