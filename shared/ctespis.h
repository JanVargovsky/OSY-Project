//***************************************************************************
//
// Petr Olivka (c) 03/2013 pro projekt do predmetu OSY.
// Viz. http://poli.cs.vsb.cz/edu/osy
//
// Priklad klienta pro IPC problem 'Ctenari a spisovatele'.
// 
//***************************************************************************

// Prikazy protokolu
#define CI_PrichaziC    11
#define CS_PrichaziC    "Chci cist"
#define CI_PrichaziS    12
#define CS_PrichaziS    "Chci psat"
#define CI_Cist         13
#define CS_Cist         "Cti od %d do %d"
#define CI_Psat         14
#define CS_Psat         "Zapis data na %d"
#define CI_Konec        15
#define CS_Konec        "Konec"

// Odpovedi protokolu
#define AI_Cti          21
#define AS_Cti          "Muzes cist"
#define AI_Pis          22
#define AS_Pis          "Muzes psat"
#define AI_Data         23
#define AS_Data         "Data"
#define AI_Zapsano      24
#define AS_Zapsano      "Hotovo"
#define AI_Nashledanou  25
#define AS_Nashledanou  "Na shledanou"

// Chyby
#define EI_Klient       90
#define ES_Klient       "Obecna chyba klienta"
#define EI_Server       91
#define ES_Server       "Obecna chyba serveru"


