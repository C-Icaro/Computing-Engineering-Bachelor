#include <stdio.h>
#include <iostream>
#include <iomanip>

using namespace std;

int main() {
    double r, pi=3.14159;
    std::cout << std::fixed << std::setprecision(4);
    cin >> r;
    cout << "A=" << (pi*r*r) << endl;
    return 0;
}
