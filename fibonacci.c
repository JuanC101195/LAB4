/*
 * fibonacci.c — Generador de la secuencia de Fibonacci con hilo trabajador
 *
 * Patrón de diseño (memoria compartida):
 *   1. main asigna un arreglo compartido con malloc(N * sizeof(long))
 *   2. main crea un hilo trabajador pasándole un puntero al arreglo y N
 *   3. main llama pthread_join para bloquearse hasta que el trabajador termine
 *   4. main imprime el arreglo (garantizado completo después del join)
 *
 * Secuencia: F(-2)=0, F(-1)=1, F(n)=F(n-1)+F(n-2)  (n>=0)
 *   → 0, 1, 1, 2, 3, 5, 8, 13, 21, 34, ...
 *
 * Uso: ./fibonacci <N>
 *   N: cantidad de términos a generar
 *
 * Compilar: gcc -o fibonacci fibonacci.c -lpthread
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

/* ----------------------------------------------------------
 * FibArgs: estructura de argumentos para el hilo trabajador
 *
 * Campos:
 *   arreglo — puntero al arreglo compartido (asignado por main)
 *   N       — cantidad de términos a calcular
 * ---------------------------------------------------------- */
typedef struct {
    long *arreglo;  /* puntero al espacio de memoria compartida */
    int   N;
} FibArgs;

/* ----------------------------------------------------------
 * hilo_trabajador(arg): calcula la secuencia de Fibonacci
 *
 * Parámetro:  arg (void*) — puntero a FibArgs
 * Retorna:    NULL
 *
 * Escribe directamente en el arreglo compartido.
 * El hilo main NO debe leerlo hasta que pthread_join retorne.
 * ---------------------------------------------------------- */
void *hilo_trabajador(void *arg)
{
    FibArgs *args   = (FibArgs *)arg;
    long    *arr    = args->arreglo;
    int      N      = args->N;

    if (N >= 1) arr[0] = 0;
    if (N >= 2) arr[1] = 1;

    int i;
    for (i = 2; i < N; i++)
    {
        arr[i] = arr[i-1] + arr[i-2];
    }

    pthread_exit(NULL);  /* señaliza al main que terminó */
}

/* ----------------------------------------------------------
 * main: punto de entrada — hilo principal
 *
 * Argumento:
 *   argv[1] — N (cantidad de términos de Fibonacci)
 *
 * Flujo:
 *   malloc → pthread_create → pthread_join → imprimir → free
 * ---------------------------------------------------------- */
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Uso: %s <N>\n", argv[0]);
        fprintf(stderr, "  N: cantidad de terminos (ej: 10)\n");
        return 1;
    }

    int N = atoi(argv[1]);
    if (N <= 0)
    {
        fprintf(stderr, "Error: N debe ser un entero positivo.\n");
        return 1;
    }

    /* PASO 1: main asigna memoria compartida (≈ malloc en C) */
    long *arreglo = malloc(N * sizeof(long));
    if (!arreglo)
    {
        fprintf(stderr, "Error: malloc falló para N=%d\n", N);
        return 1;
    }

    /* PASO 2: empaquetar argumentos para el hilo trabajador */
    FibArgs args;
    args.arreglo = arreglo;
    args.N       = N;

    /* PASO 3: crear el hilo trabajador (≈ pthread_create) */
    pthread_t trabajador;
    if (pthread_create(&trabajador, NULL, hilo_trabajador, &args) != 0)
    {
        fprintf(stderr, "Error: no se pudo crear el hilo trabajador\n");
        free(arreglo);
        return 1;
    }

    /* PASO 4: pthread_join — main se bloquea hasta que el trabajador
     * termine. Garantiza que el arreglo esté completamente lleno
     * antes de que main lo lea. Sin esto habría condición de carrera. */
    pthread_join(trabajador, NULL);

    /* PASO 5: main imprime los resultados (el arreglo está listo) */
    printf("Secuencia de Fibonacci (N=%d):\n", N);
    int i;
    for (i = 0; i < N; i++)
    {
        printf("  F(%d) = %ld\n", i, arreglo[i]);
    }

    free(arreglo);
    return 0;
}
