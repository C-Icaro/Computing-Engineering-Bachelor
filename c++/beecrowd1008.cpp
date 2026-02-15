#include <stdio.h>
#include <iostream>
#include <iomanip>

using namespace std;

int main() {
    std::cout << std::fixed << std::setprecision(2);
    int funcionario, hora_trabalho;
    float valor_hora;
    cin >> funcionario;
    cin >> hora_trabalho;
    cin >> valor_hora;
    cout << "NUMBER = " << funcionario << endl;
    cout << "SALARY = U$ " << (hora_trabalho * valor_hora) << endl;
    return 0;
}
