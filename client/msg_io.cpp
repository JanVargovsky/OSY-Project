//***************************************************************************
//
// Petr Olivka (c) 03/2013 pro projekt do predmetu OSY.
// Viz. http://poli.cs.vsb.cz/edu/osy
//
// Ukazka kodu pro prijem a odesilani zprav.
// 
// Prijem a odesilani zprav si bude kazdy ve svem projektu implementovat sam!
//
//***************************************************************************
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdarg.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>

#include "msg_io.h"

// debug flag
int debug = 0;

//***************************************************************************
// Vypis informaci

void m_print( int msg, const char *form, ... )
{
    const char *out_fmt[] = { "ERROR: %s%s\n",
                              "INFO:  %s\n",
                              ":)APP: %s\n",
                              "DEBUG: %s\n" };

    char buf[ 1024 ], out[ 1024 ], tmp[ 1024 ];
    va_list arg;

    if ( msg == M_DEBUG && !debug ) return;

    va_start( arg, form );
    vsnprintf( buf, sizeof( buf ), form, arg );
    va_end( arg );

    switch ( msg )
    {
      case M_INFO:
      case M_APP:
      case M_DEBUG:
        snprintf( out, sizeof( out ), out_fmt[ msg ], buf );
        fputs( out, stdout );
        break;

      case M_ERROR:
        if ( errno )
        {
            snprintf( tmp, sizeof( tmp ), out_fmt[ msg ], "(%d-%s) ", buf );
            snprintf( out, sizeof( out ), tmp, errno, strerror( errno ) );
        }
        else
            snprintf( out, sizeof( out ), out_fmt[ msg ], "", buf );
        fputs( out, stderr );
        break;
    }
}

//***************************************************************************
// Odeslani zpravy v pozadovanem formatu

int posli_zpravu( int socket, msg_io *msg )
{
    // nasledujici funkci si pro server implementujte sami! 
    int r = send_proto_msg( socket, msg->typ, msg->NN, msg->text );
    if ( r < 0 ) 
    {
        errno = 0;
        m_print( M_ERROR, "Zprava nebyla odeslana!" );
    }
    else
        m_print( M_DEBUG, "Byla odeslana zprava delky %d bytu.", r );

    return r;
}

//***************************************************************************
// Cekani na pozadovanou zpravu

int cekej_zpravu( int socket, msg_io *wait_msg, int num_msg, msg_io *recv_msg )
{
    while ( 1 )
    {
        msg_io recv;

        // nasledujici funkci si pro server implementujte sami! 
        int r = recv_proto_msg( socket, &recv.typ, &recv.NN, recv.text );
        if ( r < 0 ) 
        {
            if ( errno == EAGAIN )
            {
                m_print( M_INFO, "Byla prijata zprava ve spatnem formatu." );
                continue;
            }
            errno = 0;
            m_print( M_ERROR, "Selhalo spojeni se serverem!" );
            return -1;
        }
        if ( r == 0 )
        {
            errno = 0;
            m_print( M_ERROR, "Spojeni se serverem bylo ukonceno." );
            return 0;
        }

        char tmp[ PROTO_LEN + 1 ] = { recv.typ, 0 };
        if ( recv.NN >= 0 ) sprintf( tmp + 1, "%02d", recv.NN );
        sprintf( tmp + strlen( tmp ), ":%s", recv.text );
        m_print( M_INFO, "Byla prijata zprava:\n\t'%s'", tmp );

        if ( recv_msg ) *recv_msg = recv;

        if ( recv.typ == 'E' ) return -1;

        for ( int n = 0; n < num_msg; n++ )
        {
            // ocekavana shoda v typu a cisle nebo retezci
            // podminku si muze kazdy prizpusobit dle vlastniho uvazeni
            if ( ( recv.typ == wait_msg[ n ].typ ) && 
                 ( ( recv.NN >= 0 && recv.NN == wait_msg[ n ].NN ) || 
                   !strcmp( recv.text, wait_msg[ n ].text ) ) )
            {
                m_print( M_INFO, "Byla prijata pozadovana zprava [%d].", n );
                return 1;
            }
        }
        m_print( M_INFO, "Neocekavana zprava." );
    }
}

//***************************************************************************

