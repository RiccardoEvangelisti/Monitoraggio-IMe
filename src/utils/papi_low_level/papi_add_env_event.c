/*
 * Programma che permette di monitorare l'evento specificato nella variabile d'ambiente "PAPI_EVENT"
 * Se la variabile è vuota/errata, viene monitorato un evento di default.
 */

#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include "papi.h" /* This needs to be included anytime you use PAPI */

int PAPI_add_env_event(int * EventSet, int * Event, char * env_variable);

int main() {

  int retval, i;
  float a[1000], b[1000], c[1000];

  int EventSet = PAPI_NULL; // è il set in cui inserire gli eventi da monitorare

  int event_code = PAPI_TOT_CYC; // è il codice o il nome associato all'evento da monitorare

  char event_name[PAPI_MAX_STR_LEN]; // spazio dove inserire il nome chiamando "PAPI_event_code_to_name"

  long long values; // contiene il risultato del conteggio

  /* Inizializzazione della libreria.
  	Effettua un controlllo della versione dell'header file con la versione della libreria.
  	Se non corrispondono il programma potrebbe non funzionare correttamente. 
   */
  if ((retval = PAPI_library_init(PAPI_VER_CURRENT)) != PAPI_VER_CURRENT) {
    PAPI_perror("PAPI_library_init");
    exit(-1);
  }

  /* Create space for the EventSet */
  if ((retval = PAPI_create_eventset( & EventSet)) != PAPI_OK) {
    PAPI_perror("PAPI_create_eventset");
    exit(-1);
  }

  /*  Aggiunge l'evento al set.
   *  Se è stato usato l'evento della variabile d'ambiente, event_code conterrà il CODICE dell'evento.
   */
  if ((retval = PAPI_add_env_event( & EventSet, & event_code, "PAPI_EVENT")) != PAPI_OK) {
    PAPI_perror("PAPI_add_env_event");
    exit(-1);
  }

  /* Inizia a contare */
  if ((retval = PAPI_start(EventSet)) != PAPI_OK) {
    PAPI_perror("PAPI_start");
    exit(-1);
  }

  /* Some work to take up some time, the PAPI_start/PAPI_stop (and/or
   * PAPI_read) should surround what you want to monitor.
   */
  for (i = 0; i < 1000; i++) {
    a[i] = b[i] - c[i];
    c[i] = a[i] * 1.2;
  }

  /* Smetti di contare */
  if ((retval = PAPI_stop(EventSet, & values)) != PAPI_OK) {
    PAPI_perror("PAPI_stop");
    exit(-1);
  }

  /* Preleva il nome dal codice (se necessario) */
  if ((retval = PAPI_event_code_to_name(event_code, event_name)) != PAPI_OK) {
    PAPI_perror("PAPI_event_code_to_name");
    exit(-1);
  }

  /* Stampa il risultato*/
  printf("Ending values for %s: %lld\n", event_name, values);

  /* Importante */
  PAPI_shutdown();

  exit(0);
}

/* 
	EventSet è dove bisogna inserire l'evento da monitorare
	EventCode è il codice/nome dell'evento se env_variable è vuota
	env_variable è la variabile d'ambiente che potrebbe contenere il codice/nome dell'evento.
		For example PAPI_L1_DCM could be defined in the environment variable as
 *       	all of the following:  PAPI_L1_DCM, 0x80000000, or -2147483648
*/
int PAPI_add_env_event(int * EventSet, int * EventCode, char * env_variable) {
  // Di default viene aggiunto al set l'evento in EventCode
  int real_event = * EventCode;
  char * eventname;
  int retval;

  // se env_variable contiene un nome o un codice di un evento valido, viene aggiunto quello
  if (env_variable != NULL) {
    if ((eventname = getenv(env_variable))) {
      // se è il nome (ad es PAPI_L1_TOT)
      if (eventname[0] == 'P') {
        retval = PAPI_event_name_to_code(eventname, & real_event); // modifica real_event
        // se c'è errore ripristina l'evento default
        if (retval != PAPI_OK) real_event = * EventCode;
      }
      // se è il codice (ad es 0x80000054)
      else {
        if (strlen(eventname) > 1 && eventname[1] == 'x')
          sscanf(eventname, "%x", & real_event);
        else
          real_event = atoi(eventname);
      }
    }
  }

  // l'evento viene aggiunto
  if ((retval = PAPI_add_event( * EventSet, real_event)) != PAPI_OK) {
    if (real_event != * EventCode) {
      if ((retval = PAPI_add_event( * EventSet, * EventCode)) == PAPI_OK) {
        real_event = * EventCode;
      }
    }
  }
  /*
   * In EventCode si inserisce il CODICE dell'evento della variabile d'ambiente 
   * o il NOME/CODICE dell'evento di default. 
   * E' utile perché così al di fuori della funzione si sa quale evento è stato aggiunto
   */
  * EventCode = real_event;
  return retval;
}