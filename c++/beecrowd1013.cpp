#include<iostream>
#include <cmath>

using namespace std;

int main(){

int A, B, C;

cin >> A >> B >> C;

double max_ab = (A + B + fabs(A - B)) / 2.0;
double maior = (max_ab + C + fabs(max_ab - C)) / 2.0;

cout << maior << " eh o maior" << endl;

}
