/*
 * Piedra Papel Tijeras con FreeRTOS
 * Target: ARM Cortex-M3 (QEMU MPS2-AN385)
 *
 * Arquitectura de tareas:
 *   vInputTask   (prioridad 3) — Lee entrada del jugador via UART
 *   vCPUTask     (prioridad 2) — Genera jugada aleatoria de la CPU
 *   vJudgeTask   (prioridad 2) — Determina el ganador
 *   vDisplayTask (prioridad 1) — Muestra resultado y marcador
 *
 * Comunicacion entre tareas:
 *   xPlayerQueue   : Move (jugador → juez)
 *   xCPUQueue      : Move (cpu    → juez)
 *   xResultQueue   : GameResult (juez → display)
 *   xRoundSem      : Binario (input → cpu) sincroniza inicio de ronda
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#define UART0_ADDRESS    ( 0x40004000UL )
#define UART0_DATA       ( *( ( volatile uint32_t * ) ( UART0_ADDRESS + 0UL ) ) )
#define UART0_STATE      ( *( ( volatile uint32_t * ) ( UART0_ADDRESS + 4UL ) ) )
#define UART0_CTRL       ( *( ( volatile uint32_t * ) ( UART0_ADDRESS + 8UL ) ) )
#define UART0_BAUDDIV    ( *( ( volatile uint32_t * ) ( UART0_ADDRESS + 16UL ) ) )
#define UART_RX_READY_MASK ( 1UL << 1 )

/* ─── Tipos del juego ─── */

typedef enum {
    ROCK     = 0,
    PAPER    = 1,
    SCISSORS = 2
} Move;

typedef enum {
    PLAYER_WINS,
    CPU_WINS,
    DRAW
} Result;

typedef struct {
    Move   playerMove;
    Move   cpuMove;
    Result result;
    uint32_t playerScore;
    uint32_t cpuScore;
    uint32_t draws;
} GameResult;

/* ─── Handles de IPC ─── */

static QueueHandle_t     xPlayerQueue;
static QueueHandle_t     xCPUQueue;
static QueueHandle_t     xResultQueue;
static SemaphoreHandle_t xRoundSem;
static SemaphoreHandle_t xScoreMutex;

/* ─── Marcador compartido ─── */

static volatile uint32_t g_playerScore = 0;
static volatile uint32_t g_cpuScore    = 0;
static volatile uint32_t g_draws       = 0;

/* ─── Cadenas de ayuda ─── */

static const char * const moveStr[]   = { "Piedra", "Papel ", "Tijeras" };
static const char * const resultStr[] = {
    "*** JUGADOR GANA! ***",
    "***   CPU GANA!  ***",
    "***    EMPATE    ***"
};
static const char * const emoji[] = { "🪨", "📄", "✂️ " };

static int prvUARTGetCharBlocking( void )
{
    for( ;; )
    {
        if( ( UART0_STATE & UART_RX_READY_MASK ) != 0U )
        {
            return (int)( UART0_DATA & 0xFFU );
        }

        /* Avoid spinning at highest priority while waiting for input. */
        vTaskDelay( pdMS_TO_TICKS( 1 ) );
    }
}

/* The startup file expects these IRQ handlers in the vector table. */
void TIMER0_Handler( void ) {}
void TIMER1_Handler( void ) {}

/* ══════════════════════════════════════════════════════════
 * TAREA 1 — Entrada del jugador
 * Prioridad alta; bloquea en getchar() hasta que el usuario
 * pulsa 1, 2 o 3. Luego libera el semáforo de ronda para
 * despertar a la CPU y envía su jugada al juez.
 * ══════════════════════════════════════════════════════════ */
static void vInputTask( void *pvParameters )
{
    (void)pvParameters;
    Move playerMove;

    for( ;; )
    {
        printf( "\n╔══════════════════════════════╗\n" );
        printf( "║   PIEDRA  •  PAPEL  •  TIJERAS  ║\n" );
        printf( "╚══════════════════════════════╝\n" );
        printf( "  Elige tu jugada:\n" );
        printf( "    [1] 🪨  Piedra\n" );
        printf( "    [2] 📄  Papel\n" );
        printf( "    [3] ✂️   Tijeras\n" );
        printf( "> " );
        fflush( stdout );

        /* Esperar una tecla valida y hacer eco para que se vea en la consola. */
        int c = 0;
        while( c < '1' || c > '3' )
        {
            c = prvUARTGetCharBlocking();

            if( c == '\r' || c == '\n' )
            {
                putchar( '\n' );
                fflush( stdout );
                continue;
            }

            putchar( c );
            putchar( '\n' );
            fflush( stdout );
        }
        playerMove = (Move)( c - '1' );

        printf( "\nElegiste: %s %s\n", emoji[playerMove], moveStr[playerMove] );

        /* 1. Señalar a la CPU que puede generar su jugada */
        xSemaphoreGive( xRoundSem );

        /* 2. Enviar jugada al juez */
        xQueueSend( xPlayerQueue, &playerMove, portMAX_DELAY );

        /* Pequeña pausa antes de la siguiente ronda */
        vTaskDelay( pdMS_TO_TICKS( 2500 ) );
    }
}

/* ══════════════════════════════════════════════════════════
 * TAREA 2 — IA de la CPU
 * Espera el semáforo de ronda (para que la CPU no decida
 * antes de que el jugador elija). Usa un generador LCG como
 * fuente de aleatoriedad determinista (suficiente para QEMU).
 * ══════════════════════════════════════════════════════════ */
static void vCPUTask( void *pvParameters )
{
    (void)pvParameters;
    Move     cpuMove;
    uint32_t seed = 0xDEADBEEF;

    for( ;; )
    {
        /* Esperar señal del jugador */
        xSemaphoreTake( xRoundSem, portMAX_DELAY );

        /* Simular "pensamiento" de la CPU */
        vTaskDelay( pdMS_TO_TICKS( 600 ) );

        /* LCG — Numerical Recipes */
        seed = seed * 1664525UL + 1013904223UL;
        /* Mezclar con tick count para mayor variedad */
        seed ^= (uint32_t)xTaskGetTickCount();
        cpuMove = (Move)( seed % 3 );

        printf( "CPU elige:  %s %s\n", emoji[cpuMove], moveStr[cpuMove] );

        /* Enviar al juez */
        xQueueSend( xCPUQueue, &cpuMove, portMAX_DELAY );
    }
}

/* ══════════════════════════════════════════════════════════
 * TAREA 3 — Árbitro / Juez
 * Recibe las dos jugadas, aplica las reglas y actualiza el
 * marcador protegido por un mutex. Luego envía el resultado
 * completo a la tarea de display.
 * ══════════════════════════════════════════════════════════ */
static void vJudgeTask( void *pvParameters )
{
    (void)pvParameters;
    Move       playerMove, cpuMove;
    GameResult gr;

    for( ;; )
    {
        /* Esperar ambas jugadas */
        xQueueReceive( xPlayerQueue, &playerMove, portMAX_DELAY );
        xQueueReceive( xCPUQueue,    &cpuMove,    portMAX_DELAY );

        gr.playerMove = playerMove;
        gr.cpuMove    = cpuMove;

        /* Reglas: Piedra > Tijeras, Papel > Piedra, Tijeras > Papel */
        if( playerMove == cpuMove )
        {
            gr.result = DRAW;
        }
        else if( ( playerMove == ROCK     && cpuMove == SCISSORS ) ||
                 ( playerMove == PAPER    && cpuMove == ROCK     ) ||
                 ( playerMove == SCISSORS && cpuMove == PAPER    ) )
        {
            gr.result = PLAYER_WINS;
        }
        else
        {
            gr.result = CPU_WINS;
        }

        /* Actualizar marcador con mutex */
        xSemaphoreTake( xScoreMutex, portMAX_DELAY );
        {
            switch( gr.result )
            {
                case PLAYER_WINS: g_playerScore++; break;
                case CPU_WINS:    g_cpuScore++;    break;
                case DRAW:        g_draws++;        break;
            }
            gr.playerScore = g_playerScore;
            gr.cpuScore    = g_cpuScore;
            gr.draws       = g_draws;
        }
        xSemaphoreGive( xScoreMutex );

        xQueueSend( xResultQueue, &gr, portMAX_DELAY );
    }
}

/* ══════════════════════════════════════════════════════════
 * TAREA 4 — Display / Resultados
 * Muestra el resultado de cada ronda y el marcador global.
 * Es la tarea de menor prioridad — solo corre cuando las
 * demás están bloqueadas esperando.
 * ══════════════════════════════════════════════════════════ */
static void vDisplayTask( void *pvParameters )
{
    (void)pvParameters;
    GameResult gr;

    for( ;; )
    {
        xQueueReceive( xResultQueue, &gr, portMAX_DELAY );

        printf( "\n─────────────────────────────\n" );
        printf( "  Tú   : %s %s\n", emoji[gr.playerMove], moveStr[gr.playerMove] );
        printf( "  CPU  : %s %s\n", emoji[gr.cpuMove],    moveStr[gr.cpuMove]    );
        printf( "\n  %s\n", resultStr[gr.result] );
        printf( "\n  Marcador — Tú: %u | CPU: %u | Empates: %u\n",
            (unsigned int)gr.playerScore,
            (unsigned int)gr.cpuScore,
            (unsigned int)gr.draws );
        printf( "─────────────────────────────\n" );
        fflush( stdout );
    }
}

/* ══════════════════════════════════════════════════════════
 * main() — Crea recursos e inicia el scheduler
 * ══════════════════════════════════════════════════════════ */
int main( void )
{
    UART0_BAUDDIV = 16;
    UART0_CTRL = 3;

    printf( "\n" );
    printf( "╔═════════════════════════════════════╗\n" );
    printf( "║  FreeRTOS — Piedra Papel Tijeras    ║\n" );
    printf( "║  QEMU ARM Cortex-M3 (MPS2-AN385)    ║\n" );
    printf( "╚═════════════════════════════════════╝\n\n" );

    /* ── Colas ── */
    xPlayerQueue  = xQueueCreate( 1, sizeof( Move ) );
    xCPUQueue     = xQueueCreate( 1, sizeof( Move ) );
    xResultQueue  = xQueueCreate( 1, sizeof( GameResult ) );

    /* ── Semáforos ── */
    xRoundSem   = xSemaphoreCreateBinary();
    xScoreMutex = xSemaphoreCreateMutex();

    configASSERT( xPlayerQueue  != NULL );
    configASSERT( xCPUQueue     != NULL );
    configASSERT( xResultQueue  != NULL );
    configASSERT( xRoundSem     != NULL );
    configASSERT( xScoreMutex   != NULL );

    /* ── Tareas ── */
    xTaskCreate( vInputTask,   "Input",   configMINIMAL_STACK_SIZE * 4, NULL, 3, NULL );
    xTaskCreate( vCPUTask,     "CPU",     configMINIMAL_STACK_SIZE * 2, NULL, 2, NULL );
    xTaskCreate( vJudgeTask,   "Judge",   configMINIMAL_STACK_SIZE * 2, NULL, 2, NULL );
    xTaskCreate( vDisplayTask, "Display", configMINIMAL_STACK_SIZE * 4, NULL, 1, NULL );

    printf( "Tareas creadas. Iniciando scheduler...\n\n" );

    /* Inicia el scheduler — nunca retorna */
    vTaskStartScheduler();

    /* Solo llega aquí si no hay heap suficiente */
    for( ;; );
    return 0;
}

void vApplicationMallocFailedHook( void )
{
    taskDISABLE_INTERRUPTS();
    for( ;; );
}

void vApplicationStackOverflowHook( TaskHandle_t xTask,
                                    char *pcTaskName )
{
    (void)xTask;
    (void)pcTaskName;
    taskDISABLE_INTERRUPTS();
    for( ;; );
}
