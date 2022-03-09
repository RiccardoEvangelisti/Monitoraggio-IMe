#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "papi.h"
#include "papi_test.h"

#define MAX_RAPL_EVENTS 64

long long *RAPL_init_AND_start(int argc, char **argv, int *EventSet,
                               char event_names[MAX_RAPL_EVENTS][PAPI_MAX_STR_LEN], int *num_events,
                               char units[MAX_RAPL_EVENTS][PAPI_MIN_STR_LEN], int data_type[MAX_RAPL_EVENTS],
                               long long *before_time)
{
  int retval, cid, numcmp, rapl_cid = -1, code = -1;
  const PAPI_component_info_t *cmpinfo = NULL;
  PAPI_event_info_t evinfo;
  int r;

  printf("Usage: -q[NO OUTPUT]\n\n");

  /* Set TESTS_QUIET variable: se presente opzione -q, TESTS_QUIET=1*/
  tests_quiet(argc, argv);

  /* PAPI Initialization: viene verificata la versione */
  retval = PAPI_library_init(PAPI_VER_CURRENT);
  if (retval != PAPI_VER_CURRENT)
  {
    test_fail(__FILE__, __LINE__, "PAPI_library_init failed\n", retval);
  }

  /* Ricerca del componente RAPL */
  /* Scorre tra tutti i componenti di PAPI per trovare RAPL */
  numcmp = PAPI_num_components();
  for (cid = 0; cid < numcmp; cid++)
  {

    if ((cmpinfo = PAPI_get_component_info(cid)) == NULL)
    {
      test_fail(__FILE__, __LINE__, "PAPI_get_component_info failed\n", 0);
    }

    if (strstr(cmpinfo->name, "rapl"))
    {

      rapl_cid = cid;

      /*if (!TESTS_QUIET)
      {
        printf("Found rapl component at cid %d\n", rapl_cid);
      }*/

      if (cmpinfo->disabled)
      {
        if (!TESTS_QUIET)
        {
          printf("RAPL component disabled: %s\n",
                 cmpinfo->disabled_reason);
        }
        test_skip(__FILE__, __LINE__, "RAPL component disabled", 0);
      }
      break;
    }
  }
  /* Component RAPL not found */
  if (cid == numcmp)
  {
    test_skip(__FILE__, __LINE__, "No rapl component found\n", 0);
  }

  /*if (!TESTS_QUIET)
  {
    printf("Trying all RAPL events\n");
  }*/

  /* Create EventSet */
  retval = PAPI_create_eventset(&(*EventSet));
  if (retval != PAPI_OK)
  {
    test_fail(__FILE__, __LINE__, "PAPI_create_eventset()", retval);
  }

  /* Aggiunge a EventSet all RAPL events disponibili */
  code = PAPI_NATIVE_MASK; // = 0x40000000
  /* (in riferimento al componente rapl identificato da rapl_cid)
      sostituisce a code il primo evento disponibile
  */
  r = PAPI_enum_cmp_event(&code, PAPI_ENUM_FIRST, rapl_cid);
  while (r == PAPI_OK)
  {

    retval = PAPI_event_code_to_name(code, event_names[*num_events]);
    if (retval != PAPI_OK)
    {
      printf("Error translating %#x\n", code);
      test_fail(__FILE__, __LINE__,
                "PAPI_event_code_to_name", retval);
    }

    retval = PAPI_get_event_info(code, &evinfo);
    if (retval != PAPI_OK)
    {
      test_fail(__FILE__, __LINE__,
                "Error getting event info\n", retval);
    }

    strncpy(units[*num_events], evinfo.units, sizeof(units[0]));
    // buffer must be null terminated to safely use strstr operation on it below
    units[*num_events][sizeof(units[0]) - 1] = '\0';

    data_type[*num_events] = evinfo.data_type;

    retval = PAPI_add_event((*EventSet), code);
    if (retval != PAPI_OK)
    {
      break; /* We've hit an event limit */
    }
    (*num_events)++;

    // sostituisce a code il prossimo evento disponibile
    r = PAPI_enum_cmp_event(&code, PAPI_ENUM_EVENTS, rapl_cid);
  }

  /* Prepara il vettore values che conterrà i risultati*/
  long long *values = calloc(*num_events, sizeof(long long));
  if (values == NULL)
  {
    test_fail(__FILE__, __LINE__, "No memory", retval);
  }

  /* Start Counting */
  if (!TESTS_QUIET)
  {
    printf("\nStarting measurements...\n\n");
  }
  *before_time = PAPI_get_real_nsec();
  retval = PAPI_start(*EventSet);
  if (retval != PAPI_OK)
  {
    test_fail(__FILE__, __LINE__, "PAPI_start()", retval);
  }

  return values;
}

void RAPL_stop_AND_print(long long before_time, int *EventSet, long long *values, int num_events, char units[MAX_RAPL_EVENTS][PAPI_MIN_STR_LEN], char event_names[MAX_RAPL_EVENTS][PAPI_MAX_STR_LEN],
                         int data_type[MAX_RAPL_EVENTS])
{

  int retval, i;
  long long after_time;
  double elapsed_time;

  /* Stop Counting */
  after_time = PAPI_get_real_nsec();
  retval = PAPI_stop(*EventSet, values);
  if (retval != PAPI_OK)
  {
    test_fail(__FILE__, __LINE__, "PAPI_stop()", retval);
  }
  elapsed_time = ((double)(after_time - before_time)) / 1.0e9;

  /* Varie print */
  if (!TESTS_QUIET)
  {
    printf("\nStopping measurements, took %.3fs, gathering results...\n\n", elapsed_time);

    printf("Scaled energy measurements:\n");

    for (i = 0; i < num_events; i++)
    {
      if (strstr(units[i], "nJ"))
      {

        printf("%-40s%12.6f J\t(Average Power %.1fW)\n",
               event_names[i],
               (double)values[i] / 1.0e9,
               ((double)values[i] / 1.0e9) / elapsed_time);
      }
    }

    printf("\n");
    printf("Energy measurement counts:\n");

    for (i = 0; i < num_events; i++)
    {
      if (strstr(event_names[i], "ENERGY_CNT"))
      {
        printf("%-40s%12lld\t%#08llx\n", event_names[i], values[i], values[i]);
      }
    }

    printf("\n");
    printf("Scaled Fixed values:\n");

    for (i = 0; i < num_events; i++)
    {
      if (!strstr(event_names[i], "ENERGY"))
      {
        if (data_type[i] == PAPI_DATATYPE_FP64)
        {

          union
          {
            long long ll;
            double fp;
          } result;

          result.ll = values[i];
          printf("%-40s%12.3f %s\n", event_names[i], result.fp, units[i]);
        }
      }
    }

    printf("\n");
    printf("Fixed value counts:\n");

    for (i = 0; i < num_events; i++)
    {
      if (!strstr(event_names[i], "ENERGY"))
      {
        if (data_type[i] == PAPI_DATATYPE_UINT64)
        {
          printf("%-40s%12lld\t%#08llx\n", event_names[i], values[i], values[i]);
        }
      }
    }
  }

  /* Done, clean up */
  retval = PAPI_cleanup_eventset(*EventSet);
  if (retval != PAPI_OK)
  {
    test_fail(__FILE__, __LINE__,
              "PAPI_cleanup_eventset()", retval);
  }

  retval = PAPI_destroy_eventset(&(*EventSet));
  if (retval != PAPI_OK)
  {
    test_fail(__FILE__, __LINE__,
              "PAPI_destroy_eventset()", retval);
  }

  test_pass(__FILE__);
}