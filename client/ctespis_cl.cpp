//***************************************************************************
//
// Petr Olivka (c) 03/2013 pro projekt do predmetu OSY.
// Viz. http://poli.cs.vsb.cz/edu/osy
//
// Priklad klienta pro IPC problem 'Ctenari a spisovatele'.
// 
// Zpravy mezi klientem a serverem jsou zasilany v pozadovanem formatu 
// popsanem v zadani projektu, viz vyse.
// 
// Format: "?[NN]:[text zpravy]\n", kde ? je znak {CAEWI}. 
//
// !!!!!!
// Na strane serveru neni v projektu dovoleno vyuzivat funkce definovane
// v souboru "msg_io.h".
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
#include "ctespis.h"

//***************************************************************************
// Implementace klientske casti dle sekvencniho diagramu

int ctenar( int socket )
{
    m_print( M_APP, "Ctenar vstupuje ..." );

    msg_io newmsg, msg = { 'C', CI_PrichaziC, CS_PrichaziC };
    if ( posli_zpravu( socket, &msg ) < 0 ) return -1;

    m_print( M_APP, "Ctenar ceka na povoleni ..." );

    msg = ( msg_io ) { 'A', AI_Cti, AS_Cti };
    if ( cekej_zpravu( socket, &msg ) <= 0 ) return -1;

    int start = rand() % 100;
    int stop = start + rand() % 100;
    m_print( M_APP, "Ctenar chce cist od %d do %d.", start, stop );
    msg = ( msg_io ) { 'C', CI_Cist };
    sprintf( msg.text, CS_Cist, start, stop );
    if ( posli_zpravu( socket, &msg ) < 0 ) return -1;

    m_print( M_APP, "Ctenar ceka na data..." );

    msg = ( msg_io ) { 'A', AI_Data, AS_Data };
    if ( cekej_zpravu( socket, &msg ) <= 0 ) return -1;

    m_print( M_APP, "Ctenar obdrzel data a odpojuje se..." );

    msg = ( msg_io ) { 'C', CI_Konec, CS_Konec };
    if ( posli_zpravu( socket, &msg ) < 0 ) return -1;

    m_print( M_APP, "Ctenar ceka na potvrzeni odpojeni..." );

    msg = ( msg_io ) { 'A', AI_Nashledanou, AS_Nashledanou };
    if ( cekej_zpravu( socket, &msg ) <= 0 ) return -1;

    m_print( M_APP, "Ctenar zpracovava data..." );
    sleep( rand() % 3 + 1 );
    m_print( M_APP, "Ctenar je hotov..." );

    return 0;
}

int spisovatel( int socket )
{
    m_print( M_APP, "Spisovatel vstupuje ..." );

    msg_io newmsg, msg = { 'C', CI_PrichaziS, CS_PrichaziS };
    if ( posli_zpravu( socket, &msg ) < 0 ) return -1;

    m_print( M_APP, "Spisovatel ceka na povoleni ..." );

    msg = ( msg_io ) { 'A', AI_Pis, AS_Pis };
    if ( cekej_zpravu( socket, &msg ) <= 0 ) return -1;

    int kam = rand() % 100;
    m_print( M_APP, "Spisovatel chce psat na %d.", kam );
    msg = ( msg_io ) { 'C', CI_Psat };
    sprintf( msg.text, CS_Psat, kam );
    if ( posli_zpravu( socket, &msg ) < 0 ) return -1;

    m_print( M_APP, "Spisovatel ceka na zapsani dat..." );

    msg = ( msg_io ) { 'A', AI_Zapsano, AS_Zapsano };
    if ( cekej_zpravu( socket, &msg ) <= 0 ) return -1;

    m_print( M_APP, "Spisovatel zapsal data a odpojuje se..." );

    msg = ( msg_io ) { 'C', CI_Konec, CS_Konec };
    if ( posli_zpravu( socket, &msg ) < 0 ) return -1;

    m_print( M_APP, "Spisovatel ceka na potvrzeni odpojeni..." );

    msg = ( msg_io ) { 'A', AI_Nashledanou, AS_Nashledanou };
    if ( cekej_zpravu( socket, &msg ) <= 0 ) return -1;

    m_print( M_APP, "Spisovatel je hotov..." );

    return 0;
}

//***************************************************************************

void help()
{
    printf(
      "\n"
      "  Priklad klienta pro IPC problem 'Ctenari a spisovatele'.\n"
      "\n"
      "  Pouziti: [-d -h] jmeno-nebo-ip cislo-portu\n"
      "\n"
      "    -d  ladici mod\n"
      "    -h  tato napoveda\n"
      "\n" );
    exit( 0 );
}

//***************************************************************************

int main( int argn, char **arg )
{

    if ( argn <= 1 ) help();

    int port = 0;
    char *host = NULL;

    // zpracovani prikazoveho radku
    for ( int i = 1; i < argn; i++ )
    {
        if ( !strcmp( arg[ i ], "-dd" ) )
            p_debug = 1;

        if ( !strcmp( arg[ i ], "-d" ) )
            debug = 1;

        if ( !strcmp( arg[ i ], "-h" ) )
            help();

        if ( *arg[ i ] != '-' )
        {
            if ( !host )
                host = arg[ i ];
            else if ( !port )
                port = atoi( arg[ i ] );
        }
    }

    if ( !host || !port )
    {
        m_print( M_INFO, "Nebyl zadan cil pripojeni a port!" );
        help();
    }

    m_print( M_INFO, "Pokusime se navazat spojeni s '%s':%d.", host, port );

    // vytvoreni socketu
    int sock_server = socket( AF_INET, SOCK_STREAM, 0 );
    if ( sock_server == -1 )
    {
        m_print( M_ERROR, "Nelze vytvorit socket.");
        exit( 1 );
    }

    // preklad DNS jmena na IP adresu
    hostent *hostip = gethostbyname( host );
    if ( !hostip )
    {
       m_print( M_ERROR, "Nelze prelozit jmeno stroje na IP adresu" );
       exit( 1 );
    }

    sockaddr_in cl_addr;
    cl_addr.sin_family = AF_INET;
    cl_addr.sin_port = htons( port );
    cl_addr.sin_addr = * (in_addr * ) hostip->h_addr_list[ 0 ];

    // navazani spojeni se serverem
    if ( connect( sock_server, ( sockaddr * ) &cl_addr, sizeof( cl_addr ) ) < 0 )
    {
        m_print( M_ERROR, "Nelze navazat spojeni se serverem." );
        exit( 1 );
    }

    uint lsa = sizeof( cl_addr );

    // ziskani vlastni identifikace
    getsockname( sock_server, ( sockaddr * ) &cl_addr, &lsa );

    m_print( M_INFO, "Moje IP: '%s'  port: %d",
             inet_ntoa( cl_addr.sin_addr ), ntohs( cl_addr.sin_port ) );

    // ziskani informaci o serveru
    getpeername( sock_server, ( sockaddr * ) &cl_addr, &lsa );

    m_print( M_INFO, "Server IP: '%s'  port: %d",
             inet_ntoa( cl_addr.sin_addr ), ntohs( cl_addr.sin_port ) );

    srand( getpid() );

    // po navazani spojeni zacina proces ctenare nebo spisovatele
    if ( rand() % 2 )
    {
        int ret = ctenar( sock_server );

        if ( ret < 0 ) 
            m_print( M_APP, "Ctenar konci neocekavenane, nebo chybou." );
        else 
            m_print( M_APP, "Ctenar ukoncen korektne." );
    }
    else
    {
        int ret = spisovatel( sock_server );

        if ( ret < 0 ) 
            m_print( M_APP, "Spisovatel konci neocekavenane, nebo chybou." );
        else 
            m_print( M_APP, "Spisovatel ukoncen korektne." );
    }


    // uzavreme socket
    close( sock_server );

    return 0;
}


