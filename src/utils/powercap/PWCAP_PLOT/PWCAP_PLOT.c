/**
 * ATTENZIONE: PER QUALCHE MOTIVO NON DEFINITO BISOGNA MISURARE TUTTI I REGISTRI
 *
 * Il programma monitora 3 registri:
 *    powercap:::ENERGY_UJ:ZONE0
 *    powercap:::ENERGY_UJ:ZONE0_SUBZONE0
 *    powercap:::ENERGY_UJ:ZONE0_SUBZONE1
 * dove ciascuno monitora l'energia consumata tra l'invocazione di PAPI_start e PAPI_stop.
 *
 * Ogni 0.1s viene invocata PAPI_start e PAPI_stop, leggendo i 3 registri e
 * scrivendo il risultato su rispettivi file.
 * In ogni file è riportato il tempo (in secondi) tra l'inizio del programma e ogni lettura,
 * e la potenza media (in watt) consumata nell'intervallo, calcolata come la lettura del registro diviso il tempo dell'intervallo.
 *
 * Il formato di ogni file è:
 *  tempo potenza_media
 */

#include <stdio.h>

#include <stdlib.h>

#include <unistd.h>

#include <string.h>

#include "papi.h"

#include "papi_test.h"

#define MAX_EVENTS 24

int main(int argc, char **argv)
{
  int retval;
  int EventSet = PAPI_NULL;
  long long *values;
  int i, code;
  PAPI_event_info_t evinfo;
  long long start_time, before_time, after_time;
  double elapsed_time, total_time;
  char units[MAX_EVENTS][BUFSIZ];
  /* INTILE??
  int data_type[MAX_EVENTS];*/
  char filenames[MAX_EVENTS][BUFSIZ];
  FILE *fff[MAX_EVENTS];
  int num_events = 0;

  /* Set TESTS_QUIET variable */
  tests_quiet(argc, argv);

  /* PAPI Initialization */
  retval = PAPI_library_init(PAPI_VER_CURRENT);
  if (retval != PAPI_VER_CURRENT)
  {
    test_fail(__FILE__, __LINE__, "PAPI_library_init failed\n", retval);
  }

  char event_names[MAX_EVENTS][PAPI_MAX_STR_LEN] = {"powercap:::ENERGY_UJ:ZONE0",
                                                    "powercap:::MAX_ENERGY_RANGE_UJ:ZONE0",
                                                    "powercap:::MAX_POWER_A_UW:ZONE0",
                                                    "powercap:::POWER_LIMIT_A_UW:ZONE0",
                                                    "powercap:::TIME_WINDOW_A_US:ZONE0",
                                                    "powercap:::MAX_POWER_B_UW:ZONE0",
                                                    "powercap:::POWER_LIMIT_B_UW:ZONE0",
                                                    "powercap:::TIME_WINDOW_B:ZONE0",
                                                    "powercap:::ENABLED:ZONE0",
                                                    "powercap:::NAME:ZONE0",
                                                    "powercap:::ENERGY_UJ:ZONE0_SUBZONE0",
                                                    "powercap:::MAX_ENERGY_RANGE_UJ:ZONE0_SUBZONE0",
                                                    "powercap:::MAX_POWER_A_UW:ZONE0_SUBZONE0",
                                                    "powercap:::POWER_LIMIT_A_UW:ZONE0_SUBZONE0",
                                                    "powercap:::TIME_WINDOW_A_US:ZONE0_SUBZONE0",
                                                    "powercap:::ENABLED:ZONE0_SUBZONE0",
                                                    "powercap:::NAME:ZONE0_SUBZONE0",
                                                    "powercap:::ENERGY_UJ:ZONE0_SUBZONE1",
                                                    "powercap:::MAX_ENERGY_RANGE_UJ:ZONE0_SUBZONE1",
                                                    "powercap:::MAX_POWER_A_UW:ZONE0_SUBZONE1",
                                                    "powercap:::POWER_LIMIT_A_UW:ZONE0_SUBZONE1",
                                                    "powercap:::TIME_WINDOW_A_US:ZONE0_SUBZONE1",
                                                    "powercap:::ENABLED:ZONE0_SUBZONE1",
                                                    "powercap:::NAME:ZONE0_SUBZONE1"};

  /*
    I registri che voglio andare a monitorare davvero sono agli indici: 0, 10, 17
  */

  /* Create EventSet */
  retval = PAPI_create_eventset(&EventSet);
  if (retval != PAPI_OK)
  {
    test_fail(__FILE__, __LINE__, "PAPI_create_eventset()", retval);
  }

  for (num_events = 0; num_events < MAX_EVENTS; num_events++)
  {

    retval = PAPI_event_name_to_code(event_names[num_events], &code);
    if (retval != PAPI_OK)
    {
      printf("Error translating %s\n", event_names[num_events]);
      test_fail(__FILE__, __LINE__, "PAPI_event_name_to_code", retval);
    }

    retval = PAPI_get_event_info(code, &evinfo);
    if (retval != PAPI_OK)
    {
      test_fail(__FILE__, __LINE__, "Error getting event info\n", retval);
    }

    strncpy(units[num_events], evinfo.units, sizeof(units[0]));
    // buffer must be null terminated to safely use strstr operation on it below
    units[num_events][sizeof(units[0]) - 1] = '\0';

    /* TOGLIERE??
      data_type[num_events] = evinfo.data_type;*/

    retval = PAPI_add_event(EventSet, code);
    if (retval != PAPI_OK)
    {
      test_fail(__FILE__, __LINE__, "hit an event limit\n", retval);
      break; /* We've hit an event limit */
    }

    sprintf(filenames[num_events], "results.%s", event_names[num_events]);
  }

  values = calloc(num_events, sizeof(long long));
  if (values == NULL)
  {
    test_fail(__FILE__, __LINE__, "No memory", retval);
  }

  /* Open output files */
  for (i = 0; i < num_events; i++)
  {

    // Creo solo i file che mi interessano
    if (i == 0 || i == 10 || i == 17)
    {

      fff[i] = fopen(filenames[i], "w");
      if (fff[i] == NULL)
      {
        test_fail(__FILE__, __LINE__, "Open output files", retval);
      }
    }
  }

  start_time = PAPI_get_real_nsec();

  while (1)
  {

    /* Start Counting */
    before_time = PAPI_get_real_nsec();
    retval = PAPI_start(EventSet);
    if (retval != PAPI_OK)
    {
      test_fail(__FILE__, __LINE__, "PAPI_start()", retval);
    }

    usleep(100000);

    /* Stop Counting */
    after_time = PAPI_get_real_nsec();
    retval = PAPI_stop(EventSet, values);
    if (retval != PAPI_OK)
    {
      test_fail(__FILE__, __LINE__, "PAPI_stop()", retval);
    }

    total_time = ((double)(after_time - start_time)) / 1.0e9;
    elapsed_time = ((double)(after_time - before_time)) / 1.0e9;

    for (i = 0; i < num_events; i++)
    {
      // Scrivo solo nei file creati
      if (i == 0 || i == 10 || i == 17)
      {
        // Stampa del tempo (in secondi) e dell'Average Power in watt
        fprintf(fff[i], "%.4f %.3f \n",
                total_time,
                ((double)values[i] / 1.0e6) / elapsed_time);

        fflush(fff[i]);
      }
    }
  }
  return 0;
}