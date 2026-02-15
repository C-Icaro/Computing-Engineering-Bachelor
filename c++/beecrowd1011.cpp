#include<iostream>
#include<iomanip>

using namespace std;

int main(){

double r;

cin >> r;
cout << "VOLUME = " << fixed << setprecision(3) << ((4.0/3.0)*3.14159*(r*r*r)) << endl;

}
