#include <iostream>
#include <iomanip>

using namespace std;

int main(){

	double A, B;
	
	std::cout << std::fixed << std::setprecision(5);
	cin >> A;
	cin >> B;
	cout << "MEDIA = " << (((A*3.5)+(B*7.5))/11) << endl;
	return 0;
}
