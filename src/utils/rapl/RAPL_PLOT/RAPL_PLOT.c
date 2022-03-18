/**
 * Il programma monitora 3 registri:
 *    rapl:::PACKAGE_ENERGY:PACKAGE0
 *    rapl:::DRAM_ENERGY:PACKAGE0
 *    rapl:::PP0_ENERGY:PACKAGE0
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

#include <string.h>

#include <unistd.h>

#include "papi.h"
#include "papi_test.h"

#define MAX_EVENTS 3

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

  char event_names[MAX_EVENTS][PAPI_MAX_STR_LEN] = {/*
      "rapl:::THERMAL_SPEC_CNT:PACKAGE0",
      "rapl:::MINIMUM_POWER_CNT:PACKAGE0",
      "rapl:::MAXIMUM_POWER_CNT:PACKAGE0",
      "rapl:::MAXIMUM_TIME_WINDOW_CNT:PACKAGE0",
      "rapl:::PACKAGE_ENERGY_CNT:PACKAGE0",
      "rapl:::DRAM_ENERGY_CNT:PACKAGE0",
      "rapl:::PSYS_ENERGY_CNT:PACKAGE0",
      "rapl:::PP0_ENERGY_CNT:PACKAGE0",
      "rapl:::THERMAL_SPEC:PACKAGE0",
      "rapl:::MINIMUM_POWER:PACKAGE0",
      "rapl:::MAXIMUM_POWER:PACKAGE0",
      "rapl:::MAXIMUM_TIME_WINDOW:PACKAGE0",
      "rapl:::PSYS_ENERGY:PACKAGE0", */
                                                    "rapl:::PACKAGE_ENERGY:PACKAGE0",
                                                    "rapl:::DRAM_ENERGY:PACKAGE0",
                                                    "rapl:::PP0_ENERGY:PACKAGE0"};

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

    //printf("EVENT NAME: %s\nEVENT CODE: %#x\n", event_names[num_events], code);

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

    fff[i] = fopen(filenames[i], "w");
    if (fff[i] == NULL)
    {
      test_fail(__FILE__, __LINE__, "Open output files", retval);
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
      // Stampa del tempo (in secondi) e dell'Average Power in watt
      fprintf(fff[i], "%.4f %.3f \n",
              total_time,
              ((double)values[i] / 1.0e9) / elapsed_time);

      /*fprintf(fff[i], "%.4f  %.3f %s  %.3f %s\n",
              total_time,
              ((double)values[i] / 1.0e9), "J",
              ((double)values[i] / 1.0e9) / elapsed_time, "W");*/

      fflush(fff[i]);
    }
  }

  return 0;
}