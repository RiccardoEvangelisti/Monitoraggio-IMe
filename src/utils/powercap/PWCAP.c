#include <stdio.h>

#include <stdlib.h>

#include <unistd.h>

#include <string.h>

#include "papi.h"

#include "papi_test.h"

#define MAX_EVENTS 40

#define MATRIX_SIZE 1024
static double a[MATRIX_SIZE][MATRIX_SIZE];
static double b[MATRIX_SIZE][MATRIX_SIZE];
static double c[MATRIX_SIZE][MATRIX_SIZE];

/* Naive matrix multiply */
void run_test(int quiet)
{

  double s;
  int i, j, k;

  if (!quiet)
  {
    printf("Doing a naive %dx%d MMM...\n", MATRIX_SIZE, MATRIX_SIZE);
  }

  for (i = 0; i < MATRIX_SIZE; i++)
  {
    for (j = 0; j < MATRIX_SIZE; j++)
    {
      a[i][j] = (double)i * (double)j;
      b[i][j] = (double)i / (double)(j + 5);
    }
  }

  for (j = 0; j < MATRIX_SIZE; j++)
  {
    for (i = 0; i < MATRIX_SIZE; i++)
    {
      s = 0;
      for (k = 0; k < MATRIX_SIZE; k++)
      {
        s += a[i][k] * b[k][j];
      }
      c[i][j] = s;
    }
  }

  s = 0.0;
  for (i = 0; i < MATRIX_SIZE; i++)
  {
    for (j = 0; j < MATRIX_SIZE; j++)
    {
      s += c[i][j];
    }
  }

  if (!quiet)
    printf("Matrix multiply sum: s=%lf\n", s);
}

int main(int argc, char **argv)
{
  int retval;
  int EventSet = PAPI_NULL;
  long long *values;
  int num_events = 0;
  int code = -1;
  char event_descrs[MAX_EVENTS][PAPI_MAX_STR_LEN];
  char units[MAX_EVENTS][PAPI_MIN_STR_LEN];
  int data_type[MAX_EVENTS];
  int i;

  PAPI_event_info_t evinfo;
  long long before_time, after_time;
  double elapsed_time;

  /* Set TESTS_QUIET variable */
  tests_quiet(argc, argv);

  /* PAPI Initialization */
  retval = PAPI_library_init(PAPI_VER_CURRENT);
  if (retval != PAPI_VER_CURRENT)
  {
    test_fail(__FILE__, __LINE__, "PAPI_library_init failed\n", retval);
  }

  /* Create EventSet */
  retval = PAPI_create_eventset(&EventSet);
  if (retval != PAPI_OK)
  {
    test_fail(__FILE__, __LINE__, "PAPI_create_eventset()", retval);
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
      "rapl:::PACKAGE_ENERGY:PACKAGE0",
      "rapl:::DRAM_ENERGY:PACKAGE0",
      "rapl:::PSYS_ENERGY:PACKAGE0",
      "rapl:::PP0_ENERGY:PACKAGE0",*/
                                                    "powercap:::ENERGY_UJ:ZONE0",
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

  if (!TESTS_QUIET)
  {
    printf("Trying all RAPL POWERCAP events\n");
  }

  for (num_events = 0; num_events < 24; num_events++)
  {
    printf("index: %d\n", num_events);

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

    data_type[num_events] = evinfo.data_type;

    printf("EVENT NAME: %s\nEVENT CODE: %#x\n", event_names[num_events], code);

    retval = PAPI_add_event(EventSet, code);
    if (retval != PAPI_OK)
    {
      test_fail(__FILE__, __LINE__, "hit an event limit\n", retval);
      break; /* We've hit an event limit */
    }
  }
  printf("\nnum_events: %d\n\n", num_events);

  values = calloc(num_events, sizeof(long long));
  if (values == NULL)
  {
    test_fail(__FILE__, __LINE__,
              "No memory", retval);
  }

  if (!TESTS_QUIET)
  {
    printf("\nStarting measurements...\n\n");
  }

  /* Start Counting */
  before_time = PAPI_get_real_nsec();
  retval = PAPI_start(EventSet);
  if (retval != PAPI_OK)
  {
    test_fail(__FILE__, __LINE__, "PAPI_start()", retval);
  }

  /* Run test */
  run_test(TESTS_QUIET);

  /* Stop Counting */
  after_time = PAPI_get_real_nsec();
  retval = PAPI_stop(EventSet, values);
  if (retval != PAPI_OK)
  {
    test_fail(__FILE__, __LINE__, "PAPI_stop()", retval);
  }

  elapsed_time = ((double)(after_time - before_time)) / 1.0e9;

  if (!TESTS_QUIET)
  {
    printf("\nStopping measurements, took %.3fs, gathering results...\n\n", elapsed_time);

    printf("\n");
    for (i = 0; i < num_events; i++)
    {
      if (strstr(event_names[i], "powercap:::NAME"))
      {
        if (data_type[i] == PAPI_DATATYPE_UINT64)
        {
          printf("%-45s%s\n",
                 event_names[i], event_descrs[i]);
        }
      }
    }

    printf("\n");
    for (i = 0; i < num_events; i++)
    {
      if (strstr(event_names[i], "powercap:::ENABLED"))
      {
        if (data_type[i] == PAPI_DATATYPE_UINT64)
        {
          printf("%-45s%-20s%lld \n",
                 event_names[i], event_descrs[i],
                 values[i]);
        }
      }
    }

    printf("\n");
    for (i = 0; i < num_events; i++)
    {
      if (strstr(event_names[i], "powercap:::MAX_ENERGY"))
      {
        if (data_type[i] == PAPI_DATATYPE_UINT64)
        {
          printf("%-45s%-20s%4.6f J\n",
                 event_names[i], event_descrs[i],
                 (double)values[i] / 1.0e6);
        }
      }
    }

    printf("\n");
    for (i = 0; i < num_events; i++)
    {
      if (strstr(event_names[i], "powercap:::MAX_POWER"))
      {
        if (data_type[i] == PAPI_DATATYPE_UINT64)
        {
          printf("%-45s%-20s%4f (watts)\n",
                 event_names[i], event_descrs[i],
                 (double)values[i] / 1.0e6);
        }
      }
    }

    printf("\n");
    printf("scaled energy measurements:\n");
    for (i = 0; i < num_events; i++)
    {
      if (strstr(event_names[i], "ENERGY_UJ"))
      {
        if (data_type[i] == PAPI_DATATYPE_UINT64)
        {
          printf("%-45s%-20s%4.6f J (Average Power %.1fW)\n",
                 event_names[i], event_descrs[i],
                 (double)values[i] / 1.0e6,
                 ((double)values[i] / 1.0e6) / elapsed_time);
        }
      }
    }

    /*printf("\n");
    printf("energy counts:\n");
    for (i = 0; i < num_events; i++)
    {
        if (strstr(event_names[i], "ENERGY_UJ"))
        {
            if (data_type[i] == PAPI_DATATYPE_UINT64)
            {
                printf("%-45s%-20s%12lld\t%#08llx\n", event_names[i],
                       event_descrs[i],
                       values[i], values[i]);
            }
        }
    }*/

    printf("\n");
    printf("long term time window values:\n");
    for (i = 0; i < num_events; i++)
    {
      if (strstr(event_names[i], "TIME_WINDOW_A"))
      {
        if (data_type[i] == PAPI_DATATYPE_UINT64)
        {
          printf("%-45s%-20s%4f (secs)\n",
                 event_names[i], event_descrs[i],
                 (double)values[i] / 1.0e6);
        }
      }
    }

    printf("\n");
    printf("short term time window values:\n");
    for (i = 0; i < num_events; i++)
    {
      if (strstr(event_names[i], "TIME_WINDOW_B"))
      {
        if (data_type[i] == PAPI_DATATYPE_UINT64)
        {
          printf("%-45s%-20s%4f (secs)\n",
                 event_names[i], event_descrs[i],
                 (double)values[i] / 1.0e6);
        }
      }
    }

    printf("\n");
    printf("long term power limit:\n");
    for (i = 0; i < num_events; i++)
    {
      if (strstr(event_names[i], "POWER_LIMIT_A_UW"))
      {
        if (data_type[i] == PAPI_DATATYPE_UINT64)
        {
          printf("%-45s%-20s%4f (watts)\n",
                 event_names[i], event_descrs[i],
                 (double)values[i] / 1.0e6);
        }
      }
    }

    printf("\n");
    printf("short term power limit:\n");
    for (i = 0; i < num_events; i++)
    {
      if (strstr(event_names[i], "POWER_LIMIT_B_UW"))
      {
        if (data_type[i] == PAPI_DATATYPE_UINT64)
        {
          printf("%-45s%-20s%4f (watts)\n",
                 event_names[i], event_descrs[i],
                 (double)values[i] / 1.0e6);
        }
      }
    }
  }

  /* Done, clean up */
  retval = PAPI_cleanup_eventset(EventSet);
  if (retval != PAPI_OK)
  {
    test_fail(__FILE__, __LINE__,
              "PAPI_cleanup_eventset()", retval);
  }

  retval = PAPI_destroy_eventset(&EventSet);
  if (retval != PAPI_OK)
  {
    test_fail(__FILE__, __LINE__,
              "PAPI_destroy_eventset()", retval);
  }

  test_pass(__FILE__);

  return 0;
}