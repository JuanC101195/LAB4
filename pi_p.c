/*
 * pi_p.c — Cálculo paralelo de π mediante integración numérica + Pthreads
 *
 * Estrategia: Data Parallelism
 *   El rango [0, n) se divide en T sub-rangos, uno por hilo.
 *   Cada hilo calcula su suma parcial en una variable LOCAL
 *   (sin mutex en el bucle) y la retorna al main vía pthread_join.
 *   El main agrega los parciales y multiplica por fH.
 *
 * Uso: ./pi_p <n> <T>
 *   n: número de rectángulos
 *   T: número de hilos
 *
 * Compilar: gcc -o pi_p pi_p.c -lpthread -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <pthread.h>

/* ----------------------------------------------------------
 * get_time(): retorna el tiempo actual en segundos (double)
 * ---------------------------------------------------------- */
double get_time(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

/* ----------------------------------------------------------
 * f(x): función integranda  →  4 / (1 + x²)
 * ---------------------------------------------------------- */
double f(double x)
{
    return 4.0 / (1.0 + x * x);
}

/* ----------------------------------------------------------
 * ThreadArgs: estructura que empaqueta los argumentos de cada hilo
 *
 * Campos:
 *   inicio   — primera iteración del sub-rango (inclusivo)
 *   fin      — última iteración del sub-rango (exclusivo)
 *   n        — número total de rectángulos (para calcular fH)
 *   resultado — suma parcial calculada por el hilo (salida)
 * ---------------------------------------------------------- */
typedef struct {
    int    inicio;
    int    fin;
    int    n;
    double resultado;   /* el hilo escribe aquí su suma parcial */
} ThreadArgs;

/* ----------------------------------------------------------
 * calc_parcial(arg): función ejecutada por cada hilo
 *
 * Parámetro:  arg (void*) — puntero a ThreadArgs del hilo
 * Retorna:    NULL (el resultado se escribe en args->resultado)
 *
 * DISEÑO: cada hilo acumula en fSum_parcial (variable LOCAL),
 * evitando contención. Al terminar, guarda el resultado en la
 * struct compartida y llama pthread_exit(NULL).
 * ---------------------------------------------------------- */
void *calc_parcial(void *arg)
{
    ThreadArgs *args = (ThreadArgs *)arg;
    const double fH  = 1.0 / (double)args->n;
    double fSum_parcial = 0.0;
    double fX;
    int i;

    for (i = args->inicio; i < args->fin; i++)
    {
        fX           = fH * ((double)i + 0.5);
        fSum_parcial += f(fX);
    }

    args->resultado = fSum_parcial;  /* retorno del parcial al main */
    pthread_exit(NULL);
}

/* ----------------------------------------------------------
 * CalcPi(n, T): orquesta T hilos para calcular π en paralelo
 *
 * Parámetros:
 *   n  (int) — número total de rectángulos
 *   T  (int) — número de hilos
 * Retorna:
 *   double — aproximación de π
 *
 * Flujo:
 *   1. Asigna arreglos de hilos y argumentos
 *   2. Divide [0,n) en T sub-rangos y lanza pthread_create
 *   3. Espera a todos con pthread_join y recoge los parciales
 *   4. Suma los parciales y multiplica por fH
 * ---------------------------------------------------------- */
double CalcPi(int n, int T)
{
    pthread_t  *hilos = malloc(T * sizeof(pthread_t));
    ThreadArgs *args  = malloc(T * sizeof(ThreadArgs));

    if (!hilos || !args)
    {
        fprintf(stderr, "Error: malloc falló\n");
        exit(1);
    }

    int chunk = n / T;
    int t;

    /* ── Crear T hilos (≈ pthread_create) ────────────────── */
    for (t = 0; t < T; t++)
    {
        args[t].inicio    = t * chunk;
        args[t].fin       = (t == T - 1) ? n : args[t].inicio + chunk;
        args[t].n         = n;
        args[t].resultado = 0.0;

        if (pthread_create(&hilos[t], NULL, calc_parcial, &args[t]) != 0)
        {
            fprintf(stderr, "Error: no se pudo crear el hilo %d\n", t);
            exit(1);
        }
    }

    /* ── Esperar a todos los hilos y agregar parciales ───── */
    double fSum_total = 0.0;
    for (t = 0; t < T; t++)
    {
        pthread_join(hilos[t], NULL);      /* espera al hilo t */
        fSum_total += args[t].resultado;   /* recoge su parcial */
    }

    free(hilos);
    free(args);

    const double fH = 1.0 / (double)n;
    return fH * fSum_total;
}

/* ----------------------------------------------------------
 * main: punto de entrada del programa
 *
 * Argumentos:
 *   argv[1] — n (número de rectángulos)
 *   argv[2] — T (número de hilos)
 * ---------------------------------------------------------- */
int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Uso: %s <n> <T>\n", argv[0]);
        fprintf(stderr, "  n: numero de rectangulos (ej: 2000000000)\n");
        fprintf(stderr, "  T: numero de hilos       (ej: 4)\n");
        return 1;
    }

    int n = atoi(argv[1]);
    int T = atoi(argv[2]);

    if (n <= 0 || T <= 0)
    {
        fprintf(stderr, "Error: n y T deben ser enteros positivos.\n");
        return 1;
    }

    printf("Calculando pi con n = %d rectangulos, T = %d hilos...\n", n, T);

    double t_inicio = get_time();
    double pi_val   = CalcPi(n, T);
    double Tp       = get_time() - t_inicio;

    printf("  pi calculado : %.10f\n", pi_val);
    printf("  pi real      : %.10f\n", M_PI);
    printf("  error        : %.2e\n",  fabs(pi_val - M_PI));
    printf("  Tp (T=%d)    : %.4f segundos\n", T, Tp);

    return 0;
}
