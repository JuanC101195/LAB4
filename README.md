# Laboratorio de Sistemas Operativos — Práctica No. 4
## API de Hilos (Thread API)

| | |
|---|---|
| **Facultad** | Ingeniería — Ingeniería de Sistemas |
| **Asignatura** | Laboratorio de Sistemas Operativos |
| **Fecha de entrega** | Jun. 07 de 2026 (23:59) |

---

## Integrantes

| Nombre completo | Correo | Documento |
|---|---|---|
| Juan Esteban Cardozo Rivera | juan.cardozor@udea.edu.co | 1036955040 |
| Joseph Roldan Ramirez | joseph.roldan@udea.edu.co | 1115091119 |

---

## Estructura del Repositorio

```
LAB4/
├── pi.c            # Cálculo serial de π
├── pi_p.c          # Cálculo paralelo de π con Pthreads
├── fibonacci.c     # Generador de Fibonacci con hilo trabajador
├── analisis.ipynb  # Notebook de análisis de resultados
└── README.md
```

---

## Parte 1 — Paralelización del Cálculo de π

### Descripción

Se aproxima π mediante la integral definida:

$$\int_0^1 \frac{4}{1+x^2}\,dx = \pi$$

El algoritmo divide `[0,1]` en **n rectángulos** (método del punto medio) y suma sus áreas.

### Documentación de funciones

#### `pi.c` — Versión serial

| Función | Descripción |
|---|---|
| `double get_time(void)` | Retorna el tiempo actual en segundos usando `clock_gettime(CLOCK_MONOTONIC)`. Se usa para medir exclusivamente el tiempo de `CalcPi`. |
| `double f(double x)` | Evalúa la función integranda `4 / (1 + x²)` en el punto `x`. |
| `double CalcPi(int n)` | Aproxima π dividiendo `[0,1]` en `n` rectángulos y sumando sus áreas. Retorna la aproximación de π. |
| `int main(int argc, char *argv[])` | Recibe `n` por línea de comandos, mide el tiempo de `CalcPi` e imprime el resultado y el tiempo `Ts`. |

#### `pi_p.c` — Versión paralela

| Función | Descripción |
|---|---|
| `double get_time(void)` | Igual que en `pi.c`. |
| `double f(double x)` | Igual que en `pi.c`. |
| `void *calc_parcial(void *arg)` | Función ejecutada por cada hilo. Recibe un `ThreadArgs*` con su sub-rango `[inicio, fin)`, calcula la suma parcial en una variable local (sin mutex) y la almacena en `args->resultado`. |
| `double CalcPi(int n, int T)` | Divide `[0,n)` en `T` sub-rangos, lanza `T` hilos con `pthread_create`, espera su finalización con `pthread_join`, agrega las sumas parciales y retorna `fH * suma_total`. |
| `int main(int argc, char *argv[])` | Recibe `n` y `T` por línea de comandos, mide el tiempo de `CalcPi` e imprime el resultado y el tiempo `Tp`. |

**`ThreadArgs` — struct de argumentos por hilo:**

| Campo | Tipo | Descripción |
|---|---|---|
| `inicio` | `int` | Primera iteración del sub-rango (inclusivo) |
| `fin` | `int` | Última iteración del sub-rango (exclusivo) |
| `n` | `int` | Total de rectángulos (para calcular `fH`) |
| `resultado` | `double` | Suma parcial calculada por el hilo (salida) |

---

## Parte 2 — Generador de Secuencia de Fibonacci

### Descripción

Se genera la secuencia de Fibonacci en un **hilo trabajador** que escribe sobre un arreglo compartido asignado por `main`. El patrón implementado es:

```
main: malloc → pthread_create → pthread_join → imprime arreglo
trabajador:    ──► calcula Fibonacci ──► pthread_exit
```

### Documentación de funciones

#### `fibonacci.c`

| Función | Descripción |
|---|---|
| `void *hilo_trabajador(void *arg)` | Recibe un `FibArgs*` con el puntero al arreglo compartido y `N`. Calcula los `N` términos de la secuencia y los escribe en el arreglo. Llama `pthread_exit(NULL)` al terminar. |
| `int main(int argc, char *argv[])` | Recibe `N` por línea de comandos. Asigna memoria con `malloc`, lanza el hilo trabajador, espera con `pthread_join` y finalmente imprime la secuencia. |

**`FibArgs` — struct de argumentos:**

| Campo | Tipo | Descripción |
|---|---|---|
| `arreglo` | `long *` | Puntero al arreglo compartido asignado por `main` |
| `N` | `int` | Cantidad de términos a generar |

---

## Compilación

```bash
# Versión serial de π
gcc -O2 -o pi_s pi.c -lm

# Versión paralela de π
gcc -O2 -o pi_p pi_p.c -lpthread -lm

# Fibonacci
gcc -O2 -o fibonacci fibonacci.c -lpthread
```

---

## Pruebas Realizadas

### Correctitud de π

```bash
$ ./pi_s 1000000
  pi calculado : 3.1415926536
  pi real      : 3.1415926536
  error        : 2.89e-14

$ ./pi_p 1000000 4
  pi calculado : 3.1415926536
  error        : 8.26e-14
```

Ambas versiones producen el mismo resultado con error < 1e-13.

### Correctitud de Fibonacci

```bash
$ ./fibonacci 10
  F(0) = 0
  F(1) = 1
  F(2) = 1
  F(3) = 2
  F(4) = 3
  F(5) = 5
  F(6) = 8
  F(7) = 13
  F(8) = 21
  F(9) = 34
```

### Benchmarks de rendimiento (n = 2 000 000 000, AMD Ryzen 7 5825U — 16 núcleos)

```bash
$ ./pi_s 2000000000
  Ts (serial) : 3.7935 segundos

$ ./pi_p 2000000000 1   → Tp = 3.7736 s  | Speedup = 1.01
$ ./pi_p 2000000000 2   → Tp = 1.5957 s  | Speedup = 2.38
$ ./pi_p 2000000000 4   → Tp = 1.0075 s  | Speedup = 3.77
$ ./pi_p 2000000000 8   → Tp = 0.5595 s  | Speedup = 6.78
$ ./pi_p 2000000000 16  → Tp = 0.4673 s  | Speedup = 8.11
$ ./pi_p 2000000000 32  → Tp = 0.4833 s  | Speedup = 7.85
```

---

## Problemas Presentados y Soluciones

| Problema | Solución |
|---|---|
| Retornar un `double` desde un hilo vía `pthread_join` con cast a `void*` es inseguro en algunas arquitecturas. | Se almacena el resultado en el campo `resultado` de la `struct ThreadArgs`, que el hilo `main` lee tras el `pthread_join`. |
| El último hilo debe cubrir el resto de iteraciones cuando `n` no es divisible exactamente por `T`. | El último hilo recibe `fin = n` en lugar de `inicio + chunk`, asegurando cobertura completa del rango. |

---

## Video de Sustentación

> Enlace: *(agregar enlace al video de 10 minutos)*

---

## Manifiesto de Transparencia

Se utilizó IA generativa (Claude) como apoyo en los siguientes puntos:

- Generación del esqueleto inicial de los archivos `.c`.
- Verificación de la correctitud del patrón `pthread_create` / `pthread_join`.
- Estructura y redacción del notebook `analisis.ipynb`.

Todo el código fue revisado, entendido y validado por los integrantes del grupo antes de su entrega.

---

## Conclusiones

1. La paralelización con Pthreads permite reducir el tiempo de cálculo de π de 3.79 s a 0.47 s con 16 hilos (Speedup ≈ 8×) sobre un procesador de 8 núcleos físicos.
2. El overhead de creación y sincronización de hilos es despreciable cuando la carga de trabajo es suficientemente grande (n = 2 000 000 000).
3. La Ley de Amdahl impone un límite práctico: el Speedup no escala linealmente con T debido a la fracción serial del programa.
4. La eficiencia disminuye al aumentar T, especialmente al superar el número de núcleos físicos disponibles.
5. `pthread_join` es la barrera de sincronización esencial que garantiza coherencia de datos en memoria compartida; sin él se producen condiciones de carrera.
6. Fibonacci es intrínsecamente serial (cada término depende del anterior) y no se beneficia de la paralelización; el hilo trabajador ilustra el patrón de memoria compartida.
7. El punto óptimo de rendimiento en este sistema es T = 8 hilos (Eficiencia = 0.85), coincidiendo con el número de núcleos físicos.
8. Superar los núcleos físicos (T = 32) produce un Speedup ligeramente menor por el overhead de scheduling del sistema operativo.

---

## Referencias

- Arpaci-Dusseau, R. H. & Arpaci-Dusseau, A. C. *Operating Systems: Three Easy Pieces* — Cap. 26 y 27.
- The Open Group. *IEEE Std 1003.1 — POSIX Threads (Pthreads) API*.
- Lawrence Livermore National Laboratory. *POSIX Threads Programming Tutorial*.
