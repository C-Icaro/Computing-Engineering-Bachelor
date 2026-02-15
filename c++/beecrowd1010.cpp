#include<iostream>
#include<iomanip>

using namespace std;

int main(){

	int codigoP1, quantP1, codigoP2, quantP2;
	double valorP1, valorP2;

	cin >> codigoP1 >> quantP1 >> valorP1;
	cin >> codigoP2 >> quantP2 >> valorP2;
	cout << "VALOR A PAGAR: R$ " << setprecision(3) << ((quantP1*valorP1)+(quantP2*valorP2)) << endl; 

}
