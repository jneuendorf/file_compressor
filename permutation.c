#include "permutation.h"

/*
 * Next lexicographical permutation algorithm (C)
 * By Nayuki Minase, 2014. Public domain.
 * http://nayuki.eigenstate.org/page/next-lexicographical-permutation-algorithm
 */
/*
 * Computes the next lexicographical permutation of the specified array of integers in place,
 * returning a Boolean to indicate whether a next permutation existed.
 * (Returns false when the argument is already the last possible permutation.)
 */
// int next_permutation(int *array, size_t length) {
// 	size_t i;
// 	size_t j;
// 	int temp;
//
// 	if(length == 0) {
// 		return 0;
// 	}
//
// 	// Find non-increasing suffix
// 	i = length - 1;
// 	while(i > 0 && array[i - 1] >= array[i]) {
// 		i--;
// 	}
//
// 	if(i == 0) {
// 		return 0;
// 	}
//
// 	// Find successor to pivot
// 	j = length - 1;
// 	while(array[j] <= array[i - 1]) {
// 		j--;
// 	}
//
// 	temp = array[i - 1];
// 	array[i - 1] = array[j];
// 	array[j] = temp;
//
// 	// Reverse suffix
// 	j = length - 1;
// 	while(i < j) {
// 		temp = array[i];
// 		array[i] = array[j];
// 		array[j] = temp;
// 		i++;
// 		j--;
// 	}
// 	return 1;
// }

byte get_bit_at(number *n, size_t idx) {
	return (*n & (1 << idx)) > 0;
}

number* set_bit_at(number *n, size_t idx, byte val) {
	// clear bit
	if(val == 0) {
		*n &= ~(1 << idx);
	}
	// set bit
	else {
		*n |= 1 << idx;
	}
	return n;
}

// bitwise permutation of a number
int next_permutation_bitwise(number *n, size_t length) {
	size_t i;
	size_t j;
	byte temp;
	byte successor;

	if(length == 0) {
		return 0;
	}

	// Find non-increasing suffix
	i = 0;
	while(i < length && get_bit_at(n, i + 1) >= get_bit_at(n, i) /*array[i - 1] >= array[i]*/) {
		i++;
	}

	// suffix is the entire sequence => no next permutation
	if(i == length - 1) {
		return 0;
	}

	// Find rightmost successor to pivot in suffix
	j = 0;
	// cache pivot element
	temp = get_bit_at(n, i + 1);
	while((successor = get_bit_at(n, j)) <= temp) {
		j++;
	}

	// swap pivot and successor:
	// write successor to pivot bit
	set_bit_at(n, i + 1, successor); // array[i - 1] = array[j];
	// write pivot to successor bit
	set_bit_at(n, j, temp); // array[j] = temp;

	// printf("(%zu, %zu)\n", i, j);

	// Reverse suffix: swap left and right bit and move inward
	j = 0;
	while(i > j) {
		// swap outermost bits
		temp = get_bit_at(n, i);
		set_bit_at(n, i, get_bit_at(n, j));
		set_bit_at(n, j, temp);
		// move indices inward
		i--;
		j++;
	}

	return 1;
}

// previous bitwise permutation of a number
int prev_permutation_bitwise(number *n, size_t length) {
	size_t i;
	size_t j;
	size_t k;
	byte temp;
	byte successor;

	if(length == 0) {
		return 0;
	}

	// normal algo:
	// 1. find suffix thats non-increasing
	// 2. pivot = right in in front of suffix
	// 3. find element E in suffix that greater than pivot
	// 4. swap E and pivot
	// 5. reverse suffix

	// 1. reverse suffix. for that we need to reidentify it first!
	// 1.1. find (weakly) increasing suffix
	i = 0;
	while(i < length && get_bit_at(n, i + 1) <= get_bit_at(n, i)) {
		i++;
	}

	// save suffix index for later...swapping uses and modifies index i
	k = i;

	// suffix is the entire sequence => no previous permutation
	if(i >= length - 1) {
		return 0;
	}

	// 2. reverse suffix
	j = 0;
	while(i > j) {
		// swap outermost bits
		temp = get_bit_at(n, i);
		set_bit_at(n, i, get_bit_at(n, j));
		set_bit_at(n, j, temp);
		// move indices inward
		i--;
		j++;
	}

	// printf("k: %zu\n", k);
	// printf("after reverse: %llu\n", *n);

	// 3. swap E and pivot
	// get pivot value
	temp = get_bit_at(n, k + 1);
	// E is the first bit from the left that is: greatest first elem (from the left) in suffix thats less than pivot
	j = 0;
	while(j <= k && get_bit_at(n, j) < temp) {
		// printf("in while loop: successor = %d, j = %zu\n", get_bit_at(n, j), j);
		j++;
	}
	// j--;
	successor = get_bit_at(n, --j);

	// printf("j: %zu\n", j);

	// printf("pivot: %hhu\n", temp);
	// printf("indices %zu <-> %zu\n", k+1, j);

	// swap pivot and successor:
	// write successor to pivot bit
	set_bit_at(n, k + 1, successor); // array[i - 1] = array[j];
	// write pivot to successor bit
	set_bit_at(n, j, temp); // array[j] = temp;

	return 1;
}


// int main (int argc, char const *argv[]) {
// 	int arr[] = {0, 1, 3, 5, 3, 3, 0};
// 	// next_permutation = 0 1 5 0 3 3 3
// 	// swap -> 015|3330 -> reverse -> 015|0333
//
//
// 	// int arr[] = {0,0,1,1};
// 	number n = 3;
//
// 	// next_permutation_bitwise(&n, 4);
//
// 	// printf("> %llu\n", n); // 9 yey!!
//
// 	while(next_permutation_bitwise(&n, 4)) {
// 		printf("%llu\n", n);
// 	}
//
// 	printf("-----\n");
//
// 	while(prev_permutation_bitwise(&n, 4)) {
// 		printf("%llu\n", n);
// 	}
//
// 	// while(next_permutation(arr, 7)) {
// 	// 	for(size_t i = 0; i < 7; i++) {
// 	// 		printf("%d ", arr[i]);
// 	// 	}
// 	// 	printf("\n");
// 	// }
//
// 	return 0;
// }
