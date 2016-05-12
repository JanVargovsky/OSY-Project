//***************************************************************************
//
// Petr Olivka (c) 03/2013 pro projekt do predmetu OSY.
// Viz. http://poli.cs.vsb.cz/edu/osy
//
// Ukazka kodu pro prijem a odesilani zprav.
//
//***************************************************************************
// Pro vypis informaci
#define M_ERROR       0       // zpravy chybove
#define M_INFO        1       // zpravy informativni a oznamovaci
#define M_APP         2       // zpravy o behu aplikace
#define M_DEBUG       3       // zpravy ladici a pomocne

extern int debug;
extern int p_debug;

void m_print( int msg, const char *form, ... );

//***************************************************************************
// Zpracovani protokolu pro komunikaci mezi serverem a klienty.

#define PROTO_LEN 256

struct msg_io {
  char typ;
  int NN;
  char text[ PROTO_LEN ];
  };

int posli_zpravu( int socket, msg_io *msg );
int cekej_zpravu( int socket, msg_io *wait_msg, int num_msg = 1, msg_io *recv_msg = NULL );

//***************************************************************************
// Tyto funkce neni dovoleno využívat v projektu na straně serveru!
int send_proto_msg( int handle, char type, int NN, const char *text );
int recv_proto_msg( int handle, char *type, int *NN, char *text );


