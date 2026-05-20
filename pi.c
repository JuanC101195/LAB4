/*
 * pi.c — Cálculo serial de π mediante integración numérica
 *
 * Método: regla del punto medio sobre la integral
 *         ∫₀¹ 4/(1+x²) dx = π
 *
 * Uso: ./pi_s <n>
 *   n: número de rectángulos (mayor n = mayor precisión)
 *
 * Compilar: gcc -o pi_s pi.c -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

/* ----------------------------------------------------------
 * get_time(): retorna el tiempo actual en segundos (double)
 * Usa CLOCK_MONOTONIC para evitar saltos del reloj del sistema.
 * ---------------------------------------------------------- */
double get_time(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

/* ----------------------------------------------------------
 * f(x): función integranda  →  4 / (1 + x²)
 * Parámetro:  x  (double) — punto de evaluación
 * Retorna:    double       — valor de la función en x
 * ---------------------------------------------------------- */
double f(double x)
{
    return 4.0 / (1.0 + x * x);
}

/* ----------------------------------------------------------
 * CalcPi(n): aproxima π usando integración numérica serial
 *
 * Parámetro:  n  (int) — número de rectángulos
 * Retorna:    double   — aproximación de π
 *
 * Algoritmo: divide [0,1] en n rectángulos de ancho fH = 1/n,
 * evalúa f en el punto medio de cada rectángulo y suma las áreas.
 * ---------------------------------------------------------- */
double CalcPi(int n)
{
    const double fH = 1.0 / (double)n;
    double fSum = 0.0;
    double fX;
    int i;

    for (i = 0; i < n; i++)
    {
        fX    = fH * ((double)i + 0.5);
        fSum += f(fX);
    }

    return fH * fSum;
}

/* ----------------------------------------------------------
 * main: punto de entrada del programa
 *
 * Argumentos de línea de comandos:
 *   argv[1] — n (número de rectángulos)
 *
 * Salida: valor de π calculado y tiempo de ejecución (Ts)
 * ---------------------------------------------------------- */
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Uso: %s <n>\n", argv[0]);
        fprintf(stderr, "  n: numero de rectangulos (ej: 2000000000)\n");
        return 1;
    }

    int n = atoi(argv[1]);
    if (n <= 0)
    {
        fprintf(stderr, "Error: n debe ser un entero positivo.\n");
        return 1;
    }

    printf("Calculando pi con n = %d rectangulos (serial)...\n", n);

    double t_inicio = get_time();
    double pi_val   = CalcPi(n);
    double Ts       = get_time() - t_inicio;

    printf("  pi calculado : %.10f\n", pi_val);
    printf("  pi real      : %.10f\n", M_PI);
    printf("  error        : %.2e\n",  fabs(pi_val - M_PI));
    printf("  Ts (serial)  : %.4f segundos\n", Ts);

    return 0;
}
