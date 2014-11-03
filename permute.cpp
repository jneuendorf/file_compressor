// #include <iostream>
#include <algorithm>

// using namespace std;
//
// int main() {
//    int myints[] = {1,2,3};
//
//   // std::sort (myints,myints+3);
//
//   std::cout << "The 3! possible permutations with 3 elements:\n";
//   do {
//     std::cout << myints[0] << ' ' << myints[1] << ' ' << myints[2] << '\n';
//   } while ( std::next_permutation(myints,myints+3) );
//
//   std::cout << "After loop: " << myints[0] << ' ' << myints[1] << ' ' << myints[2] << '\n';
//
//   return 0;
// }

extern "C" bool permute(char* sequence, int length) {
    return std::next_permutation(sequence, sequence + length);
}

extern "C" bool unpermute(char* sequence, int length) {
    return std::prev_permutation(sequence, sequence + length);
}
