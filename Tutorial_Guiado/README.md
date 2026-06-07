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
# 1. Ir a la carpeta de Tutorial giado
cd Minitaller_FreeRTOS/Tutorial_Guiado

# 2. Descarga el kernel de FreeRTOS
git clone --depth 1 https://github.com/FreeRTOS/FreeRTOS-Kernel.git FreeRTOS-Kernel

# 3. Configura el build
mkdir build && cd build
cmake ..

# 4. Compila y prueba que funciona
make taller
./taller
```

> Si ves el mensaje `FreeRTOS Mini-Taller` en la terminal, el ambiente está listo. El programa no imprimirá más porque los TODOs están vacíos — eso es correcto.


## Instrucciones para las tareas

En el script `taller.c` está indicado donde debes agregar cada parte del tutorial.
Una vez que agregues una parte, córrela con la instrucción #4 anterior y para detener la ejecución presiona `Ctrl + C`
---

## Estructura del repo

```
Tutorial_Guiado/
├── src/
│   └── taller.c           ← tu archivo de trabajo (aquí están los TODOs)
├── FreeRTOSConfig.h        ← configuración del kernel
├── CMakeLists.txt          ← sistema de build
├── README.md
└── FreeRTOS-Kernel/        ← kernel oficial (no modificar)
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
<summary>💡 Pista</summary>

- Para imprimir, usa `printf()` con el mensaje exacto entre comillas.
- Para esperar, llama a `vTaskDelay()` pasando el tiempo en milisegundos convertido con la macro `pdMS_TO_TICKS()`.
- Busca en el ejemplo de `vMiTask` de arriba: el patrón es idéntico.

</details>

---

### TODO 1.2 — Cuerpo de Task B

Haz lo mismo para `vTaskB`, pero con el mensaje `"[ Task B ] corriendo\n"` y **500 ms** de delay.

<details>
<summary>💡 Pista</summary>

- La estructura es exactamente igual a `vTaskA`.
- Solo cambian dos cosas: el texto del mensaje y el número de milisegundos que pasas a `pdMS_TO_TICKS()`.

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
<summary>💡 Pista</summary>

- Necesitas dos llamadas a `xTaskCreate()`, una por cada task.
- El primer argumento es el nombre de la función (sin paréntesis): `vTaskA` y `vTaskB`.
- El segundo es un string literal entre comillas para identificarla en el debugger.
- Para la prioridad, usa las constantes `PRIORIDAD_TASK_A` y `PRIORIDAD_TASK_B` que ya están definidas al inicio del archivo.

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

<details>
<summary>💡 Pista</summary>

- Modifica únicamente los números en los `#define` al inicio del archivo.
- Recuerda que una task con mayor número de prioridad **gana** sobre las de menor número.
- Cuando una task está en `vTaskDelay()` se encuentra en estado **Blocked** y no consume CPU, por eso la otra puede ejecutarse.
- Prueba también poner prioridad `0` en una task y observa qué ocurre.

</details>

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
<summary>💡 Pista</summary>

- El orden de las operaciones importa: primero envía a la queue, luego imprime, luego incrementa, luego espera.
- Para enviar, pasa la dirección de `lContador` (con `&`) como segundo argumento de `xQueueSend()`.
- El `printf` debe mostrar el valor de `lContador` antes de incrementarlo — usa `%d` como formato.
- Para incrementar una variable entera en C: `variable++`.
- El delay es igual al que ya usaste en las tasks anteriores.

</details>

---

### TODO 3.2 — Task Consumidora

Completa `vTaskConsumidora`:
- Recibe de la queue: `xQueueReceive( xQueue, &lValor, portMAX_DELAY )`
- Imprime el valor recibido

<details>
<summary>💡 Pista</summary>

- `xQueueReceive()` bloquea la task automáticamente hasta que haya un dato disponible — no necesitas un delay explícito.
- El segundo argumento es la dirección de la variable donde se guardará el valor recibido (usa `&lValor`).
- Después de recibir, imprime el contenido de `lValor` con `printf`.

</details>

---

### TODO 3.3 — Crear la queue y las tasks en main()

Busca el bloque `TODO 3.3` en `main()` y:

1. Crea la queue: `xQueue = xQueueCreate( 5, sizeof( int32_t ) );`
2. Verifica que no sea NULL
3. Crea las dos tasks (Productora con prioridad 1, Consumidora con prioridad 2)

<details>
<summary>💡 Pista</summary>

- `xQueueCreate()` devuelve `NULL` si no hay memoria suficiente — siempre verifica antes de continuar. Si es `NULL`, imprime un error y retorna `-1`.
- Para crear las tasks, el patrón es el mismo que usaste en el TODO 1.3, pero ahora con `vTaskProductora` y `vTaskConsumidora`.
- La Consumidora tiene prioridad más alta (2) para que el scheduler la despierte inmediatamente cuando llega un dato — esto es intencional.

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
<summary>💡 Pista</summary>

Para los números pares: ¿qué operación matemática te permite saltar de par en par?

Para la segunda Consumidora: la misma función puede registrarse dos veces con `xTaskCreate()`, siempre que cada llamada reciba un parámetro distinto.

Para identificar cada instancia por nombre, dentro de la función puedes recuperar el parámetro así:

```c
/* En main, pasa un string como parámetro */
xTaskCreate( vTaskConsumidora, "Cons-A", 1000, "Consumidora A", 2, NULL );
xTaskCreate( vTaskConsumidora, "Cons-B", 1000, "Consumidora B", 2, NULL );

/* Dentro de la task, léelo así */
const char *pcNombre = ( const char * ) pvParameters;
```

Luego usa `pcNombre` en lugar de un string literal en tu `printf`.

</details>


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
