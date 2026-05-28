/*
 * ============================================================================
 *  FREERTOS MINI-TALLER  —  archivo de trabajo
 * ============================================================================
 *
 *  Busca todos los bloques marcados con TODO y completa el código.
 *  Trabaja de arriba hacia abajo: Parte 1 → Parte 2 → Parte 3.
 *
 *  Compilar y ejecutar (desde la raíz del repo):
 *      mkdir build && cd build
 *      cmake ..
 *      make
 *      ./taller
 *
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* ============================================================================
 *  PARTE 2 — Prioridades
 *  Después de completar la Parte 1, experimenta cambiando estos valores,
 *  recompila y observa cómo cambia el orden de ejecución.
 * ============================================================================ */
#define PRIORIDAD_TASK_A    1   /* <-- TODO 2.A: prueba con 2, luego con 3 */
#define PRIORIDAD_TASK_B    1   /* <-- TODO 2.B: prueba con 2, luego con 1 */

/* ============================================================================
 *  PARTE 3 — Queue
 *  Handle global que comparten la Productora y la Consumidora.
 * ============================================================================ */
static QueueHandle_t xQueue = NULL;


/* ============================================================================
 *  PARTE 1 — Tasks básicas
 * ============================================================================
 *
 *  En FreeRTOS una task es una función C con este prototipo:
 *
 *      void vMiTask( void *pvParameters );
 *
 *  Siempre corre dentro de un loop infinito y NUNCA retorna.
 *  Para "pausar" la task sin bloquear el CPU usa:
 *
 *      vTaskDelay( pdMS_TO_TICKS( milisegundos ) );
 *
 * ============================================================================ */

/* ── TODO 1.1 ────────────────────────────────────────────────────────────────
 *
 *  Implementa el cuerpo de vTaskA:
 *    1. Imprime "[ Task A ] corriendo\n"
 *    2. Espera 1000 ms con vTaskDelay
 *
 * ─────────────────────────────────────────────────────────────────────────── */
void vTaskA( void *pvParameters )
{
    ( void ) pvParameters; /* evita warning de parámetro sin usar */

    for( ;; )
    {
        /* escribe tu código aquí */


    }
}


/* ── TODO 1.2 ────────────────────────────────────────────────────────────────
 *
 *  Implementa el cuerpo de vTaskB:
 *    1. Imprime "[ Task B ] corriendo\n"
 *    2. Espera 500 ms con vTaskDelay
 *
 * ─────────────────────────────────────────────────────────────────────────── */
void vTaskB( void *pvParameters )
{
    ( void ) pvParameters;

    for( ;; )
    {
        /* escribe tu código aquí */


    }
}


/* ============================================================================
 *  PARTE 3 — Productora / Consumidora
 * ============================================================================
 *
 *  Una queue es un buffer FIFO thread-safe entre tasks:
 *
 *      Productora ──xQueueSend()──► [ queue ] ──xQueueReceive()──► Consumidora
 *
 *  Si la queue está vacía, xQueueReceive() bloquea la task automáticamente
 *  (sin desperdiciar CPU) hasta que llegue un dato.
 *
 *  APIs que necesitarás:
 *      xQueueSend   ( xQueue, &valor,   portMAX_DELAY );
 *      xQueueReceive( xQueue, &variable, portMAX_DELAY );
 *
 * ============================================================================ */

/* ── TODO 3.1 ────────────────────────────────────────────────────────────────
 *
 *  Implementa vTaskProductora:
 *    1. Declara  int32_t lContador = 0;
 *    2. En cada iteración del loop:
 *       a. Envía lContador a xQueue
 *       b. Imprime "[ Productora ] envió: <valor>\n"
 *       c. Incrementa lContador
 *       d. Espera 500 ms
 *
 * ─────────────────────────────────────────────────────────────────────────── */
void vTaskProductora( void *pvParameters )
{
    ( void ) pvParameters;
    int32_t lContador = 0;

    for( ;; )
    {
        /* escribe tu código aquí */


    }
}


/* ── TODO 3.2 ────────────────────────────────────────────────────────────────
 *
 *  Implementa vTaskConsumidora:
 *    1. Declara  int32_t lValor;
 *    2. En cada iteración del loop:
 *       a. Recibe un valor de xQueue (bloqueándose con portMAX_DELAY)
 *       b. Imprime "[ Consumidora ] recibió: <valor>\n"
 *
 * ─────────────────────────────────────────────────────────────────────────── */
void vTaskConsumidora( void *pvParameters )
{
    ( void ) pvParameters;
    int32_t lValor;

    for( ;; )
    {
        /* escribe tu código aquí */


    }
}


/* ============================================================================
 *  MAIN
 * ============================================================================ */
int main( void )
{
    printf( "==============================================\n" );
    printf( "         FreeRTOS Mini-Taller\n" );
    printf( "==============================================\n\n" );

    /* ── TODO 1.3 ──────────────────────────────────────────────────────────
     *
     *  Crea vTaskA y vTaskB usando xTaskCreate().
     *
     *  Firma:
     *      xTaskCreate( funcion,    <- puntero a la función de la task
     *                   "Nombre",   <- nombre para debug
     *                   1000,       <- tamaño del stack en words
     *                   NULL,       <- parámetro (pvParameters)
     *                   prioridad,  <- usa PRIORIDAD_TASK_A / PRIORIDAD_TASK_B
     *                   NULL );     <- handle (no lo necesitamos)
     *
     * ────────────────────────────────────────────────────────────────────── */

    /* escribe tu código aquí */


    /* ── TODO 3.3 ──────────────────────────────────────────────────────────
     *
     *  a) Crea la queue con capacidad para 5 enteros:
     *       xQueue = xQueueCreate( 5, sizeof( int32_t ) );
     *
     *  b) Verifica que xQueue no sea NULL (si lo es, imprime un error y retorna -1)
     *
     *  c) Crea vTaskProductora con prioridad 1
     *     y vTaskConsumidora  con prioridad 2
     *     (la consumidora tiene mayor prioridad para procesar el dato
     *      tan pronto llega, antes de que llegue el siguiente)
     *
     * ────────────────────────────────────────────────────────────────────── */

    /* escribe tu código aquí */


    /* Inicia el scheduler — a partir de aquí el kernel toma el control */
    vTaskStartScheduler();

    /* Si llegamos aquí algo salió mal (memoria insuficiente, etc.) */
    printf( "\nERROR: vTaskStartScheduler() retornó.\n" );
    return -1;
}
