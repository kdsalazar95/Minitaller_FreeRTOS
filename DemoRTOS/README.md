# 🪨📄✂️ Piedra Papel Tijeras — FreeRTOS + QEMU

Juego multitarea de Piedra Papel Tijeras corriendo sobre **FreeRTOS**
en un ARM Cortex-M3 emulado por **QEMU** (placa MPS2-AN385).

---

## Arquitectura de tareas

```
┌─────────────────────────────────────────────────────────────────┐
│                    FreeRTOS Scheduler                           │
│                                                                 │
│  ┌────────────┐  xRoundSem   ┌────────────┐                    │
│  │ vInputTask │─────────────▶│ vCPUTask   │                    │
│  │ (prio: 3)  │              │ (prio: 2)  │                    │
│  └─────┬──────┘              └─────┬──────┘                    │
│        │ xPlayerQueue              │ xCPUQueue                  │
│        │         ┌─────────────────┘                           │
│        ▼         ▼                                             │
│  ┌─────────────────────┐  xResultQueue  ┌──────────────────┐  │
│  │    vJudgeTask       │───────────────▶│  vDisplayTask    │  │
│  │    (prio: 2)        │                │  (prio: 1)       │  │
│  └─────────────────────┘                └──────────────────┘  │
└─────────────────────────────────────────────────────────────────┘
```

| Tarea        | Prioridad | Función                              | IPC utilizada           |
|--------------|-----------|--------------------------------------|-------------------------|
| `vInputTask` | 3 (alta)  | Lee UART, espera tecla 1/2/3         | → xPlayerQueue, xRoundSem |
| `vCPUTask`   | 2         | LCG + tick XOR para jugada aleatoria | xRoundSem ←, → xCPUQueue |
| `vJudgeTask` | 2         | Aplica reglas, actualiza marcador    | ← ambas queues, → xResultQueue |
| `vDisplayTask`| 1 (baja) | Formatea e imprime resultado         | xResultQueue ←            |

**Primitivas FreeRTOS usadas:**
- `xQueueCreate / xQueueSend / xQueueReceive` — paso de mensajes
- `xSemaphoreCreateBinary` — sincronización de inicio de ronda
- `xSemaphoreCreateMutex` — protección del marcador compartido

---

## Requisitos

```bash
# Ubuntu / Debian
sudo apt update
sudo apt install gcc-arm-none-eabi qemu-system-arm cmake git

# macOS (Homebrew)
brew install --cask gcc-arm-embedded
brew install qemu cmake git
```

Versiones mínimas:
- `arm-none-eabi-gcc` ≥ 10
- `qemu-system-arm` ≥ 7.0
- `cmake` ≥ 3.16

---

## Compilar y ejecutar

```bash
# 1. Clonar FreeRTOS-Kernel y compilar (hace todo)
chmod +x setup.sh && ./setup.sh

# 2. Lanzar en QEMU
qemu-system-arm \
  -M mps2-an385 \
  -cpu cortex-m3 \
  -kernel build/freertos_rps.elf \
  -serial stdio \
  -nographic \
  -semihosting \
  -semihosting-config enable=on,target=native

# Salir de QEMU: Ctrl+A, luego X
```

---

## Ejecutar en Docker

```bash
# La imagen compila el proyecto durante docker build.
docker build -t freertos-rps .

# Al ejecutar el contenedor, inicia el juego automaticamente.
docker run --rm -it freertos-rps
```

### Alternativa con Docker Compose

```bash
# Construir imagen (incluye compilacion)
docker compose build

# Inicia directamente QEMU con el juego
docker compose run --rm freertos-rps
```

---

## Ejemplo de sesión

```
╔═════════════════════════════════════╗
║  FreeRTOS — Piedra Papel Tijeras   ║
║  QEMU ARM Cortex-M3 (MPS2-AN385)  ║
╚═════════════════════════════════════╝

╔══════════════════════════════╗
║   PIEDRA  •  PAPEL  •  TIJERAS  ║
╚══════════════════════════════╝
  Elige tu jugada:
    [1] 🪨  Piedra
    [2] 📄  Papel
    [3] ✂️   Tijeras
> 1

Elegiste: 🪨 Piedra
CPU elige:  ✂️  Tijeras

─────────────────────────────
  Tú   : 🪨 Piedra
  CPU  : ✂️  Tijeras

  *** JUGADOR GANA! ***

  Marcador — Tú: 1 | CPU: 0 | Empates: 0
─────────────────────────────
```

---

## Estructura del proyecto

```
freertos-rps/
├── main.c              ← Lógica del juego + definición de tareas
├── FreeRTOSConfig.h    ← Configuración del kernel
├── CMakeLists.txt      ← Build system
├── setup.sh            ← Script de configuración y compilación
├── README.md           ← Este archivo
├── FreeRTOS-Kernel/    ← Clonado por setup.sh
├── FreeRTOS-Demo/      ← Demo MPS2 (startup, linker script, UART)
└── demo_support/       ← Archivos copiados del demo MPS2
```

---

## Notas de diseño

- **heap_4** — esquema de alocación con coalescencia; suficiente para
  colas y semáforos del juego sin fragmentar.
- **configCHECK_FOR_STACK_OVERFLOW 2** — detecta desbordamiento tanto
  en cambio de contexto como pintando el final del stack con un patrón.
- **Aleatoriedad** — `seed ^= xTaskGetTickCount()` garantiza que dos
  partidas consecutivas no generen la misma secuencia aunque el LCG
  arranque con la misma semilla.
- **Prioridades** — `vInputTask` bloquea en `getchar()` la mayor parte
  del tiempo, cediendo CPU sin necesidad de busy-wait.
