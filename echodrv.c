#include <qdos.h>
#include <string.h>
#include "types.h"
#include "chan_ops.h"
#include "heap.h"

// Total size of channel block
#define CHAN_BLOCK_SIZE 0x100

// QDOS IO Sub System operation codes 
#define IO_FBYTE 1
#define IO_FLINE 2
#define IO_FSTRG 3
#define IO_SSTRG 7
#define SD_CHENQ 0x0b

// Print a msg that is a QLSTR_t to channel 0
#define PRINT0( msg ) (void)io_sstrg( (chanid_t)0, (timeout_t)0, msg.qs_str, msg.qs_strlen )

// Prefix for channel open string for this driver
static const char DRIVER_NAME[] = "echo_";

// This driver's QDOS driver linkage block
QLD_LINK_t linkblk;

static long ch_open() {
    register QLSTR_t *name asm( "a0" );
    QLSTR_t *name_store = name;
    char *channel_block;

    if( strnicmp( DRIVER_NAME, name_store->qs_str, (size_t)sizeof( DRIVER_NAME ) - 1 )) {
        // a0 must be returned unmodified if open is unsuccesful
        asm( " move.l %0,a0" : : "m" (name_store));

        // Not Found == Name does not match this driver
        return ERR_NF;
    }
    // $18 bytes for header + whatever is required by implementation
    // http://www.qdosmsq.dunbar-it.co.uk/doku.php?id=qdosmsq:sbinternal:chandef&s[]=channel&s[]=definition&s[]=block
    if( !(channel_block = sv_memalloc( CHAN_BLOCK_SIZE ))) {
        // Out of Memory
        return ERR_OM;
    }
    // Initialize pointers that control reading back from echo buffer
    *(char **)(channel_block + READ_PTR) = channel_block + BUF_START;
    *(char **)(channel_block + END_PTR) = channel_block + BUF_START;
    // Address of channel definition block must be returned in A0
    asm(" move.l %0,a0" : : "m" (channel_block));
    return ERR_OK;
}

static long ch_close() {
    register char *chan_blk asm( "a0" );
    char *chan_blk_store = chan_blk;
    sv_memfree( chan_blk_store );
    return ERR_OK;
}

long ch_io() {
    register unsigned char in_optype asm( "d0" );
    register unsigned long in_param1 asm( "d1" );
    register unsigned long in_param2 asm( "d2" );
    register unsigned long in_timeout asm( "d3" );
    register char *in_chanblk asm( "a0" );
    register char *in_addr1 asm( "a1" );
    unsigned char optype = in_optype;
    unsigned long param1 = in_param1;
    unsigned long param2 = in_param2;
    unsigned long timeout = in_timeout;
    char *chanblk = in_chanblk;
    char *addr1 = in_addr1;


    switch( optype ) {
        case IO_FBYTE:
            {
                char c;
                int status = ERR_OK;

                c = fbyte( chanblk, &status );
                asm( " move.l %0,a0
                       move.b %1,d1
                     " : : "m" (chanblk),
                           "m" (c) );
                return status;
            }
            break;
        case IO_SSTRG:
            {
                int bytes_sent;

                /*
                param2 = number of bytes to write, size == word
                addr1 = pointer to start of sequence of bytes to write
                        passed as a handle so that implementation can update addr1
                        in accordance with bytes written, see return parameters below
                */
                bytes_sent = sstrg( chanblk, timeout, (int)( param2 & 0x0000FFFF ), &addr1 );
                /*
                The following must be returned:
                a0 = pointer to driver linkage block passed in when this function called
                a1 = pointer to one byte past the last byte that was written
                d1 = number of bytes written
                */
                asm( " move.l %0,a0
                       move.l %1,a1
                       move.l %2,d1
                     " : : "m" (chanblk),
                           "m" (addr1),
                           "m" (bytes_sent) );
                return ERR_OK;  
            }
            break;
        case IO_FLINE:
            {
                uint16 bytes_read = 0;
                int error_code = ERR_OK;

                bytes_read = fline( chanblk, timeout, (uint16)( param2 & 0x0000FFFF ), &addr1, &error_code );
                asm( " move.l %0,a0
                       move.l %1,a1
                       move.l %2,d1
                     " : : "m" (chanblk),
                           "m" (addr1),
                           "m" (bytes_read) );
                return error_code;  

            }
        case SD_CHENQ:
            {
                asm( " move.l %0,a0" : : "m" (chanblk) );
                return ERR_BP;
            }
    }
    asm( " move.l %0,a0" : : "m" (chanblk) );
    return ERR_OK;
}

static void link_driver() {
    // linkblk.ld_next will be populate by QDOS when driver is linked
    linkblk.ld_io = &ch_io;
    linkblk.ld_open = &ch_open;
    linkblk.ld_close = &ch_close;
    mt_liod( &linkblk );
}

int main( int ac, char **av ) {
    QLSTR_INIT( msg_init, "Initializing echo driver..." );
    QLSTR_INIT( msg_done, "done. Ready for your echoing needs!\n" );

    PRINT0( msg_init );
    link_driver();
    PRINT0( msg_done );
}