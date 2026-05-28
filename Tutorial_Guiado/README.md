# 🔧 FreeRTOS Mini-Taller

Tutorial guiado para aprender los fundamentos del kernel FreeRTOS en **30–40 minutos**.

---

## Qué vas a construir

Al completar el taller tendrás corriendo un programa con **cuatro tasks** simultáneas:

```
[ Task A ]      imprime cada 1 s     (Parte 1)
[ Task B ]      imprime cada 500 ms  (Parte 1 / Parte 2)
[ Productora ]  envía números a una queue cada 500 ms  (Parte 3)
[ Consumidora ] recibe y muestra cada número           (Parte 3)
```

---

## Requisitos

| Herramienta | Versión |
|-------------|---------|
| GCC         | ≥ 7     |
| CMake       | ≥ 3.15  |
| Git         | cualquiera |
| OS          | Linux o macOS |

---

## Setup (hazlo antes del taller)

```bash
# 1. Clona el repo con el submodule de FreeRTOS-Kernel
git clone --recurse-submodules https://github.com/<tu-usuario>/freertos-taller.git
cd freertos-taller

# Si ya clonaste sin --recurse-submodules:
git submodule update --init

# 2. Configura el build
mkdir build && cd build
cmake ..

# 3. Compila y prueba que funciona
make taller
./taller
```

> Si ves el mensaje `FreeRTOS Mini-Taller` en la terminal, el ambiente está listo. El programa no imprimirá más porque los TODOs están vacíos — eso es correcto.

---

## Estructura del repo

```
freertos-taller/
├── src/
│   ├── taller.c           ← tu archivo de trabajo (aquí están los TODOs)
│   └── taller_solucion.c  ← solución de referencia (¡no la abras todavía!)
├── FreeRTOSConfig.h        ← configuración del kernel
├── CMakeLists.txt          ← sistema de build
└── FreeRTOS-Kernel/        ← kernel oficial (submodule, no modificar)
```

**Abre `src/taller.c` y trabaja de arriba hacia abajo.**

---

## Parte 1 — Tu primera Task *(~10 min)*

### Concepto

Una **task** en FreeRTOS es una función C que corre de forma concurrente con otras tasks. Siempre tiene este patrón:

```c
void vMiTask( void *pvParameters )
{
    /* código de inicialización (se ejecuta una sola vez) */

    for( ;; )   /* loop infinito — la task NUNCA retorna */
    {
        /* trabajo de la task */
        vTaskDelay( pdMS_TO_TICKS( 1000 ) );  /* cede el CPU por 1 segundo */
    }
}
```

`vTaskDelay()` pone la task en estado **Blocked**: libera el CPU para que otras tasks corran. No es un `sleep` que bloquea todo el sistema.

---

### TODO 1.1 — Cuerpo de Task A

En `taller.c`, busca la función `vTaskA` y completa el loop:

```c
void vTaskA( void *pvParameters )
{
    ( void ) pvParameters;
    for( ;; )
    {
        // ► Imprime "[ Task A ] corriendo\n"
        // ► Espera 1000 ms
    }
}
```

<details>
<summary>💡 Ver solución</summary>

```c
printf( "[ Task A ] corriendo\n" );
vTaskDelay( pdMS_TO_TICKS( 1000 ) );
```
</details>

---

### TODO 1.2 — Cuerpo de Task B

Haz lo mismo para `vTaskB`, pero con el mensaje `"[ Task B ] corriendo\n"` y **500 ms** de delay.

<details>
<summary>💡 Ver solución</summary>

```c
printf( "[ Task B ] corriendo\n" );
vTaskDelay( pdMS_TO_TICKS( 500 ) );
```
</details>

---

### TODO 1.3 — Crear las tasks en main()

Busca el bloque `TODO 1.3` en `main()` y crea ambas tasks con `xTaskCreate()`:

```
xTaskCreate( funcion,     ← puntero a la función
             "Nombre",    ← nombre para debug
             1000,        ← tamaño del stack en words
             NULL,        ← parámetro que recibe pvParameters
             prioridad,   ← usa PRIORIDAD_TASK_A o PRIORIDAD_TASK_B
             NULL );      ← handle (no lo necesitamos)
```

<details>
<summary>💡 Ver solución</summary>

```c
xTaskCreate( vTaskA, "TaskA", 1000, NULL, PRIORIDAD_TASK_A, NULL );
xTaskCreate( vTaskB, "TaskB", 1000, NULL, PRIORIDAD_TASK_B, NULL );
```
</details>

---

### Compila y ejecuta

```bash
# (desde build/)
make taller && ./taller
```

**Salida esperada:**
```
[ Task A ] corriendo
[ Task B ] corriendo
[ Task B ] corriendo
[ Task A ] corriendo
[ Task B ] corriendo
...
```

> Task B aparece con el doble de frecuencia porque su delay es la mitad.

---

## Parte 2 — Prioridades *(~10 min)*

### Concepto

El scheduler de FreeRTOS siempre ejecuta la task de **mayor prioridad** que esté en estado Ready. Si dos tasks tienen igual prioridad, alterna entre ellas (time slicing).

```
Prioridad 3 │████████░░░░░░░░░░░░  ← gana siempre mientras esté Ready
Prioridad 1 │░░░░░░░░████████░░░░  ← solo corre cuando P3 está Blocked
```

---

### TODO 2 — Experimenta con los valores

Al inicio de `taller.c` encontrarás estas dos líneas:

```c
#define PRIORIDAD_TASK_A    1
#define PRIORIDAD_TASK_B    1
```

Prueba estos escenarios, compila y observa el output cada vez:

| Experimento | PRIORIDAD_TASK_A | PRIORIDAD_TASK_B | ¿Qué esperas ver? |
|-------------|-----------------|-----------------|-------------------|
| Base        | 1               | 1               | Se alternan |
| B gana      | 1               | 3               | B domina, A solo aparece cuando B está bloqueada |
| A gana      | 3               | 1               | A domina |

```bash
# Recompila y ejecuta después de cada cambio
make taller && ./taller
```

> **Pregunta:** ¿qué pasa si le pones prioridad 0 a una task? (Pruébalo.)

---

## Parte 3 — Comunicación con Queues *(~15 min)*

### Concepto

Las **queues** son el mecanismo principal de comunicación entre tasks. Son buffers FIFO thread-safe:

```
┌─────────────┐  xQueueSend()  ┌───────────────────┐  xQueueReceive()  ┌──────────────┐
│  Productora │ ─────────────► │  Queue [4][3][2]  │ ────────────────► │  Consumidora │
└─────────────┘                └───────────────────┘                   └──────────────┘
```

Si la queue está **vacía**, `xQueueReceive()` bloquea la Consumidora automáticamente (sin quemar CPU) hasta que llegue un dato.

---

### TODO 3.1 — Task Productora

Completa `vTaskProductora` en `taller.c`:
- Envía `lContador` a la queue con `xQueueSend( xQueue, &lContador, portMAX_DELAY )`
- Imprime qué envió
- Incrementa el contador
- Espera 500 ms

<details>
<summary>💡 Ver solución</summary>

```c
xQueueSend( xQueue, &lContador, portMAX_DELAY );
printf( "[ Productora ] envió: %d\n", lContador );
lContador++;
vTaskDelay( pdMS_TO_TICKS( 500 ) );
```
</details>

---

### TODO 3.2 — Task Consumidora

Completa `vTaskConsumidora`:
- Recibe de la queue: `xQueueReceive( xQueue, &lValor, portMAX_DELAY )`
- Imprime el valor recibido

<details>
<summary>💡 Ver solución</summary>

```c
xQueueReceive( xQueue, &lValor, portMAX_DELAY );
printf( "[ Consumidora ] recibió: %d\n", lValor );
```
</details>

---

### TODO 3.3 — Crear la queue y las tasks en main()

Busca el bloque `TODO 3.3` en `main()` y:

1. Crea la queue: `xQueue = xQueueCreate( 5, sizeof( int32_t ) );`
2. Verifica que no sea NULL
3. Crea las dos tasks (Productora con prioridad 1, Consumidora con prioridad 2)

<details>
<summary>💡 Ver solución</summary>

```c
xQueue = xQueueCreate( 5, sizeof( int32_t ) );
if( xQueue == NULL )
{
    printf( "ERROR: no se pudo crear la queue.\n" );
    return -1;
}
xTaskCreate( vTaskProductora,  "Productora",  1000, NULL, 1, NULL );
xTaskCreate( vTaskConsumidora, "Consumidora", 1000, NULL, 2, NULL );
```
</details>

---

### Compila y ejecuta

```bash
make taller && ./taller
```

**Salida esperada:**
```
[ Task A ] corriendo
[ Task B ] corriendo
[ Consumidora ] recibió: 0
[ Productora ] envió: 0
[ Task B ] corriendo
[ Consumidora ] recibió: 1
[ Productora ] envió: 1
[ Task B ] corriendo
[ Task A ] corriendo
...
```
> para detener la ejecucion presiona ``Ctrl + C``


> **¿Por qué Consumidora aparece ANTES que Productora si fue Productora quien envió el dato?**
> Esto no es un bug — es el scheduler funcionando correctamente. Cuando Productora llama
> a `xQueueSend()`, el dato entra a la queue y Consumidora (prioridad 2) pasa a estado
> Ready. El scheduler la **preempta inmediatamente** a Productora (prioridad 1) antes de
> que pueda llegar a su `printf`. Consumidora imprime primero, y recién entonces Productora
> retoma. Este es un ejemplo real de **preempción por prioridad**: el scheduler siempre
> favorece la task más importante.

---

## 🏁 Reto Final *(opcional)*

Si terminaste antes de tiempo, modifica `taller.c` para:

1. Que la Productora envíe solo **números pares**
2. Agregar una **segunda Consumidora** que reciba de la misma queue
3. Que cada Consumidora se identifique por nombre usando `pvParameters`

<details>
<summary>💡 Pista para el parámetro</summary>

```c
/* En main, pasa un string como parámetro */
xTaskCreate( vTaskConsumidora, "Cons-A", 1000, "Consumidora A", 2, NULL );
xTaskCreate( vTaskConsumidora, "Cons-B", 1000, "Consumidora B", 2, NULL );

/* Dentro de la task, léelo así */
const char *pcNombre = ( const char * ) pvParameters;
printf( "%s recibió: %ld\n", pcNombre, lValor );
```
</details>

---

## Verificar tu solución

```bash
# Compila y corre la solución de referencia
make solucion && ./solucion
```

---

## Resumen de APIs

| API | Qué hace |
|-----|----------|
| `xTaskCreate()` | Crea una nueva task |
| `vTaskDelay( pdMS_TO_TICKS(ms) )` | Bloquea la task por `ms` milisegundos |
| `vTaskStartScheduler()` | Arranca el kernel |
| `xQueueCreate( n, size )` | Crea una queue con capacidad para `n` elementos |
| `xQueueSend( q, &val, timeout )` | Envía un elemento a la queue |
| `xQueueReceive( q, &var, timeout )` | Recibe un elemento (bloquea si está vacía) |

---

## Referencias

- [Documentación oficial FreeRTOS](https://www.freertos.org/Documentation/RTOS_book.html)
- [API Reference](https://www.freertos.org/a00106.html)
- [Repo original de tutoriales](https://github.com/FreeRTOS/Lab-Project-FreeRTOS-Tutorials)