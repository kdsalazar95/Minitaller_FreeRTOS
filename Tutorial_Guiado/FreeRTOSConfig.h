/*
 * FreeRTOSConfig.h — Configuración para el puerto POSIX (Linux / macOS)
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include <stdio.h>
#include <pthread.h>

/* ── Scheduler ───────────────────────────────────────────────────────────── */
#define configUSE_PREEMPTION                        1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION     0
#define configUSE_TICKLESS_IDLE                     0
#define configTICK_RATE_HZ                          ( ( TickType_t ) 1000 )
#define configMAX_PRIORITIES                        ( 7 )
#define configMINIMAL_STACK_SIZE                    ( ( unsigned short ) 70 )
#define configTOTAL_HEAP_SIZE                       ( ( size_t ) ( 65 * 1024 ) )
#define configMAX_TASK_NAME_LEN                     ( 16 )
#define configUSE_16_BIT_TICKS                      0
#define configIDLE_SHOULD_YIELD                     1
#define configUSE_TIME_SLICING                      1

/* ── Hooks (desactivados para simplificar) ───────────────────────────────── */
#define configUSE_IDLE_HOOK                         0
#define configUSE_TICK_HOOK                         0
#define configUSE_MALLOC_FAILED_HOOK                0
#define configCHECK_FOR_STACK_OVERFLOW              0
#define configUSE_DAEMON_TASK_STARTUP_HOOK          0

/* ── Primitivas de sincronización ────────────────────────────────────────── */
#define configUSE_MUTEXES                           1
#define configUSE_RECURSIVE_MUTEXES                 1
#define configUSE_COUNTING_SEMAPHORES               1
#define configUSE_TASK_NOTIFICATIONS                1
#define configTASK_NOTIFICATION_ARRAY_ENTRIES       3
#define configQUEUE_REGISTRY_SIZE                   20
#define configUSE_QUEUE_SETS                        1

/* ── Software timers ─────────────────────────────────────────────────────── */
#define configUSE_TIMERS                            1
#define configTIMER_TASK_PRIORITY                   ( configMAX_PRIORITIES - 1 )
#define configTIMER_QUEUE_LENGTH                    20
#define configTIMER_TASK_STACK_DEPTH                ( configMINIMAL_STACK_SIZE * 2 )

/* ── Asignación de memoria ───────────────────────────────────────────────── */
#define configSUPPORT_STATIC_ALLOCATION             0
#define configSUPPORT_DYNAMIC_ALLOCATION            1

/* ── Estadísticas (desactivadas) ─────────────────────────────────────────── */
#define configGENERATE_RUN_TIME_STATS               0
#define configUSE_TRACE_FACILITY                    1
#define configUSE_STATS_FORMATTING_FUNCTIONS        1

/* ── Event groups ────────────────────────────────────────────────────────── */
#define configUSE_EVENT_GROUPS                      1

/* ── Tipos ───────────────────────────────────────────────────────────────── */
#define configSTACK_DEPTH_TYPE                      uint32_t
#define configMESSAGE_BUFFER_LENGTH_TYPE            size_t

/* ── APIs habilitadas ────────────────────────────────────────────────────── */
#define INCLUDE_vTaskPrioritySet                    1
#define INCLUDE_uxTaskPriorityGet                   1
#define INCLUDE_vTaskDelete                         1
#define INCLUDE_vTaskSuspend                        1
#define INCLUDE_vTaskDelayUntil                     1
#define INCLUDE_vTaskDelay                          1
#define INCLUDE_xTaskGetSchedulerState              1
#define INCLUDE_xTaskGetCurrentTaskHandle           1
#define INCLUDE_uxTaskGetStackHighWaterMark         1
#define INCLUDE_xTaskGetIdleTaskHandle              1
#define INCLUDE_eTaskGetState                       1
#define INCLUDE_xTimerPendFunctionCall              1
#define INCLUDE_xTaskAbortDelay                     1
#define INCLUDE_xTaskGetHandle                      1
#define INCLUDE_xQueueGetMutexHolder                1

/* ── Assert ──────────────────────────────────────────────────────────────── */
#define configASSERT( x )                                       \
    if( ( x ) == 0 )                                            \
    {                                                           \
        fprintf( stderr, "ASSERT en %s:%d\n",                  \
                 __FILE__, __LINE__ );                          \
        fflush( stderr );                                       \
        for( ;; );                                              \
    }

/* ── Compatibilidad con POSIX port ───────────────────────────────────────── */
#define configUSE_POSIX_ERRNO                       1

#endif /* FREERTOS_CONFIG_H */