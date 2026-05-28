#include <errno.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>

#define UART0_BASE_ADDR    ( 0x40004000UL )
#define UART0_DATA         ( *( ( volatile uint32_t * ) ( UART0_BASE_ADDR + 0x00UL ) ) )
#define UART0_STATE        ( *( ( volatile uint32_t * ) ( UART0_BASE_ADDR + 0x04UL ) ) )
#define UART_TX_BUFFER_FULL_MASK  ( 1UL << 0 )
#define UART_RX_BUFFER_FULL_MASK  ( 1UL << 1 )

extern char end;

static char *heap_end;

caddr_t _sbrk( int incr )
{
    char *prev_heap_end;

    if( heap_end == 0 )
    {
        heap_end = &end;
    }

    prev_heap_end = heap_end;
    heap_end += incr;

    return ( caddr_t ) prev_heap_end;
}

int _write( int file, const char *ptr, int len )
{
    (void)file;

    for( int i = 0; i < len; i++ )
    {
        while( ( UART0_STATE & UART_TX_BUFFER_FULL_MASK ) != 0U )
        {
        }

        UART0_DATA = ( uint32_t ) ptr[ i ];
    }

    return len;
}

int _read( int file, char *ptr, int len )
{
    (void)file;

    if( len <= 0 )
    {
        return 0;
    }

    while( ( UART0_STATE & UART_RX_BUFFER_FULL_MASK ) == 0U )
    {
    }

    ptr[ 0 ] = ( char ) ( UART0_DATA & 0xFFU );

    return 1;
}

int _close( int file )
{
    (void)file;
    return -1;
}

int _fstat( int file, struct stat *st )
{
    (void)file;
    st->st_mode = S_IFCHR;
    return 0;
}

int _isatty( int file )
{
    (void)file;
    return 1;
}

int _lseek( int file, int ptr, int dir )
{
    (void)file;
    (void)ptr;
    (void)dir;
    return 0;
}

void _exit( int status )
{
    (void)status;
    for( ;; )
    {
    }
}

void _kill( int pid, int sig )
{
    (void)pid;
    (void)sig;
    errno = EINVAL;
}

int _getpid( void )
{
    return 1;
}
