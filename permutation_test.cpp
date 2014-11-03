#include <iostream>
#include <algorithm>

using namespace std;

int main() {
    //  int myints[] = {1,2,3};
     char sequence[] = "1111111111100000";

    //  std::next_permutation();

    // std::sort (myints,myints+3);

    // std::cout << "The 3! possible permutations with 3 elements:\n";
    do {
        // std::cout << myints[0] << ' ' << myints[1] << ' ' << myints[2] << '\n';
        std::cout << sequence << '\n';
    } while ( std::next_permutation(sequence, sequence + 16) );

    // std::cout << "After loop: " << myints[0] << ' ' << myints[1] << ' ' << myints[2] << '\n';
    std::cout << "After loop: " << sequence << '\n';

    return 0;
}
