#include <stdio.h>
#include <iostream>
#include <iomanip>

using namespace std;

int main() {
    std::cout << std::fixed << std::setprecision(2);
    string funcionario; 
    double salario, vendas;

    cin >> funcionario;
    cin >> salario;
    cin >> vendas;
    cout << "TOTAL = R$ " << (salario + (vendas*0.15)) << endl;
    return 0;
}
