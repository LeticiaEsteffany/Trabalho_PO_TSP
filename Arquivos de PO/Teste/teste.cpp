#include <stdio.h> // print, fgets function
#include <stdlib.h> // exit, atoi function
#include <string.h> //strlen function
#include <math.h> // sqrt function
#include <algorithm>    // std::min
#include <limits.h> // INT_MAX
#include <time.h>
#include <iostream>

float potencia(float base, float expoente);

int main(){

    float e = 2.71828182;
    int t = 1000;

    printf("e = %f\nResultado = %.50f",e, pow(e, 1171.0/(float)t));
    printf("\n\nRESULTADO FUNÇÃO = %f", potencia(e, 1171.0/t));
    return 0;
}
float potencia(float base, float expoente){
    float resultado = 0;

    resultado = pow(base, expoente);

    return resultado;
}
