#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>

#include "papi.h"
#include "papi_test.h"

#define MAX_EVENTS 24
#define FREQUENZA_CAMPIONAMENTO 100000 // 0.1 sec

#define PACKAGE_index 0
#define PP0_index 10
#define DRAM_index 17

/**
 * Vettore di eventi da monitorare.
 * ATTENZIONE: per motivi non conosciuti bisogna monitorare TUTTI gli eventi powercap disponibili
 *
 */
static char event_names[MAX_EVENTS][PAPI_MAX_STR_LEN] = {
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

/**
 * @brief PAPI_start con controllo d'errore e restituzione del tempo appena prima dello start.
 * Il tempo è in nano-secondi
 *
 * @param EventSet
 * @return before_time
 */
long long PAPI_start_AND_time(int *EventSet)
{

    long long before_time = PAPI_get_real_nsec();
    int retval;
    retval = PAPI_start(*EventSet);
    if (retval != PAPI_OK)
    {
        test_fail(__FILE__, __LINE__, "PAPI_start()", retval);
    }
    return before_time;
}

/**
 * @brief PAPI_stop con controllo d'errore e restituzione del tempo appena prima dello stop.
 * Il tempo è in nano-secondi
 *
 * @param EventSet
 * @return after_time
 */
long long PAPI_stop_AND_time(int *EventSet, long long *values)
{

    long long after_time = PAPI_get_real_nsec();
    int retval;
    retval = PAPI_stop(*EventSet, values);
    if (retval != PAPI_OK)
    {
        test_fail(__FILE__, __LINE__, "PAPI_start()", retval);
    }
    return after_time;
}

/**
 * @brief Inizializzazione iniziale e creazione di EventSet
 *
 * @param argc
 * @param argv
 * @param EventSet
 * @param event_names
 * @param units
 * @param data_type
 * @return vettore di risultati allocato a 0 (calloc)
 */
long long *PWCAP_init(int argc, char **argv, int *EventSet,
                      char event_names[MAX_EVENTS][PAPI_MAX_STR_LEN],
                      char units[MAX_EVENTS][PAPI_MIN_STR_LEN], int data_type[MAX_EVENTS])
{
    int retval, i;
    long long *values;
    int code = -1;
    PAPI_event_info_t evinfo;

    /* Set TESTS_QUIET variable */
    tests_quiet(argc, argv);

    /* PAPI Initialization */
    retval = PAPI_library_init(PAPI_VER_CURRENT);
    if (retval != PAPI_VER_CURRENT)
    {
        test_fail(__FILE__, __LINE__, "PAPI_library_init failed\n", retval);
    }

    /* Create EventSet */
    retval = PAPI_create_eventset(&(*EventSet));
    if (retval != PAPI_OK)
    {
        test_fail(__FILE__, __LINE__, "PAPI_create_eventset()", retval);
    }

    for (i = 0; i < MAX_EVENTS; i++)
    {
        retval = PAPI_event_name_to_code(event_names[i], &code);
        if (retval != PAPI_OK)
        {
            printf("Error translating %s\n", event_names[i]);
            test_fail(__FILE__, __LINE__, "PAPI_event_name_to_code", retval);
        }

        retval = PAPI_get_event_info(code, &evinfo);
        if (retval != PAPI_OK)
        {
            test_fail(__FILE__, __LINE__, "Error getting event info\n", retval);
        }

        strncpy(units[i], evinfo.units, sizeof(units[0]));
        // buffer must be null terminated to safely use strstr operation on it below
        units[i][sizeof(units[0]) - 1] = '\0';

        data_type[i] = evinfo.data_type;

        retval = PAPI_add_event(*EventSet, code);
        if (retval != PAPI_OK)
        {
            test_fail(__FILE__, __LINE__, "hit an event limit\n", retval);
            break; /* We've hit an event limit */
        }
    }

    values = calloc(i, sizeof(long long));
    if (values == NULL)
    {
        test_fail(__FILE__, __LINE__,
                  "No memory", retval);
    }

    return values;
}

/**
 * @brief Inizializzazione di PAPI, creazione di EventSet, generazione dei 3 file dei soli reigstri dell'energia
 *
 * @param argc
 * @param argv
 * @param EventSet
 * @param event_names
 * @param units
 * @param data_type
 * @param fff
 * @param filenames
 * @return long long*
 */
long long *PWCAP_plot_init(int argc, char **argv, int *EventSet,
                           char event_names[MAX_EVENTS][PAPI_MAX_STR_LEN],
                           char units[MAX_EVENTS][PAPI_MIN_STR_LEN], int data_type[MAX_EVENTS], FILE *fff[MAX_EVENTS], char filenames[MAX_EVENTS][PAPI_MIN_STR_LEN])
{
    int retval, i;
    long long *values;
    int code = -1;
    PAPI_event_info_t evinfo;

    /* Set TESTS_QUIET variable */
    tests_quiet(argc, argv);

    /* PAPI Initialization */
    retval = PAPI_library_init(PAPI_VER_CURRENT);
    if (retval != PAPI_VER_CURRENT)
    {
        test_fail(__FILE__, __LINE__, "PAPI_library_init failed\n", retval);
    }

    /* Create EventSet */
    retval = PAPI_create_eventset(&(*EventSet));
    if (retval != PAPI_OK)
    {
        test_fail(__FILE__, __LINE__, "PAPI_create_eventset()", retval);
    }

    for (i = 0; i < MAX_EVENTS; i++)
    {
        retval = PAPI_event_name_to_code(event_names[i], &code);
        if (retval != PAPI_OK)
        {
            printf("Error translating %s\n", event_names[i]);
            test_fail(__FILE__, __LINE__, "PAPI_event_name_to_code", retval);
        }

        retval = PAPI_get_event_info(code, &evinfo);
        if (retval != PAPI_OK)
        {
            test_fail(__FILE__, __LINE__, "Error getting event info\n", retval);
        }

        strncpy(units[i], evinfo.units, sizeof(units[0]));
        // buffer must be null terminated to safely use strstr operation on it below
        units[i][sizeof(units[0]) - 1] = '\0';

        data_type[i] = evinfo.data_type;

        retval = PAPI_add_event(*EventSet, code);
        if (retval != PAPI_OK)
        {
            test_fail(__FILE__, __LINE__, "hit an event limit\n", retval);
            break; /* We've hit an event limit */
        }

        sprintf(filenames[i], "results.%s", event_names[i]);
    }

    values = calloc(i, sizeof(long long));
    if (values == NULL)
    {
        test_fail(__FILE__, __LINE__,
                  "No memory", retval);
    }

    /* Open output files */
    for (i = 0; i < MAX_EVENTS; i++)
    {
        // Creo solo i file che mi interessano
        if (i == PACKAGE_index || i == PP0_index || i == DRAM_index)
        {

            fff[i] = fopen(filenames[i], "w");
            if (fff[i] == NULL)
            {
                test_fail(__FILE__, __LINE__, "Open output files", retval);
            }
        }
    }

    return values;
}

/**
 * Ripristino dei registri Package Power Limit se vi sono state modifiche precedenti. 
 * Infine, terminazione di PAPI: cleanup e destroy di EventSet
 * 
 * @param EventSet 
 * @param values 
 * @param LIMITE 
 */
void PAPI_term(int *EventSet, long long *values, int LIMITE)
{
    int retval, i;

    if (LIMITE > 0)
    {
        PAPI_start_AND_time(&(*EventSet));

        /* Ripristino i registri Package Power Limit a 255W */
        for (i = 0; i < MAX_EVENTS; i++)
        {
            if (!strcmp(event_names[i], "powercap:::POWER_LIMIT_A_UW:ZONE0") || !strcmp(event_names[i], "powercap:::POWER_LIMIT_B_UW:ZONE0"))
            {
                printf("EVENT: %s\tLIMIT: %0.2lf Watts\n", event_names[i], ((double)values[i] * 1e-6));

                /* Cambio il vettore dei risultati con 255W */
                values[i] = 255 * 1e6;
            }
        }

        /* Applico il limite: scrivo sui registri */
        retval = PAPI_write(*EventSet, values);
        if (retval != PAPI_OK)
        {
            test_fail(__FILE__, __LINE__, "PAPI_write()", retval);
        }

        /* Controllo che la modifica sia avvenuta correttamente */
        retval = PAPI_read(*EventSet, values);
        if (retval != PAPI_OK)
        {
            test_fail(__FILE__, __LINE__, "PAPI_read()", retval);
        }
        for (i = 0; i < MAX_EVENTS; i++)
        {
            if (!strcmp(event_names[i], "powercap:::POWER_LIMIT_A_UW:ZONE0") || !strcmp(event_names[i], "powercap:::POWER_LIMIT_B_UW:ZONE0"))
            {
                printf("EVENT: %s\tLIMIT: %0.2lf Watts\n", event_names[i], ((double)values[i] * 1e-6));

                /* Controllo che entrambi i Package Power Limit valgano 255 W */
                if ((values[i] * 1e-6) != 255)
                {
                    fprintf(stderr, "\nERRORE: %s non è settato a 255W\n", event_names[i]);
                    test_fail(__FILE__, __LINE__, "PAPI_read()", retval);
                }
            }
        }

        PAPI_stop_AND_time(&(*EventSet), values);
    }

    retval = PAPI_cleanup_eventset(*EventSet);
    if (retval != PAPI_OK)
    {
        test_fail(__FILE__, __LINE__, "PAPI_cleanup_eventset()", retval);
    }

    retval = PAPI_destroy_eventset(&(*EventSet));
    if (retval != PAPI_OK)
    {
        test_fail(__FILE__, __LINE__, "PAPI_destroy_eventset()", retval);
    }
}

/**
 * @brief Imposta i nuovi valori ai due Package Power Limit
 * 
 * @param EventSet 
 * @param values 
 * @param limite 
 */
void PAPI_limits(int *EventSet, long long *values, int limite)
{

    int retval, i;

    PAPI_start_AND_time(&(*EventSet));

    /* Leggo i valori predefiniti */
    retval = PAPI_read(*EventSet, values);
    if (retval != PAPI_OK)
    {
        test_fail(__FILE__, __LINE__, "PAPI_read()", retval);
    }

    for (i = 0; i < MAX_EVENTS; i++)
    {
        if (!strcmp(event_names[i], "powercap:::POWER_LIMIT_A_UW:ZONE0") || !strcmp(event_names[i], "powercap:::POWER_LIMIT_B_UW:ZONE0"))
        {
            printf("EVENT: %s\tLIMIT: %0.2lf Watts\n", event_names[i], ((double)values[i] * 1e-6));

            /* Controllo che entrambi i Package Power Limit valgano 255 W */
            if ((values[i] * 1e-6) != 255)
            {
                fprintf(stderr, "\nERRORE: %s non è settato a 255W\n", event_names[i]);
                test_fail(__FILE__, __LINE__, "PAPI_read()", retval);
            }

            /* Cambio il vettore dei risultati con i limiti desiderati */
            values[i] = limite * 1e6;
        }
    }

    /* Applico il limite: scrivo sui registri */
    retval = PAPI_write(*EventSet, values);
    if (retval != PAPI_OK)
    {
        test_fail(__FILE__, __LINE__, "PAPI_write()", retval);
    }

    /* Controllo che la modifica sia avvenuta correttamente */
    retval = PAPI_read(*EventSet, values);
    if (retval != PAPI_OK)
    {
        test_fail(__FILE__, __LINE__, "PAPI_read()", retval);
    }
    for (i = 0; i < MAX_EVENTS; i++)
    {
        if (!strcmp(event_names[i], "powercap:::POWER_LIMIT_A_UW:ZONE0") || !strcmp(event_names[i], "powercap:::POWER_LIMIT_B_UW:ZONE0"))
        {
            printf("EVENT: %s\tLIMIT: %0.2lf Watts\n", event_names[i], ((double)values[i] * 1e-6));

            if ((values[i] * 1e-6) != limite)
            {
                fprintf(stderr, "\nERRORE: %s non è settato il nuovo limite\n", event_names[i]);
                test_fail(__FILE__, __LINE__, "PAPI_read()", retval);
            }
        }
    }

    PAPI_stop_AND_time(&(*EventSet), values);
}

/**
 * @brief Variabile utile in fork_PWCAP_plot() e che viene modificata in fine_monitoraggio()
 *
 */
int term = 1;

/**
 * @brief Handler in risposta alla ricezione di SIGUSR1 inviato dal padre quando bisogna terminare il monitoraggio
 *
 */
void fine_monitoraggio()
{
    /* Faccio uscire dal ciclo while */
    term = 0;
}

/**
 * @brief Monitoraggio dei 3 registri energetici con campionamento ogni "FREQUENZA_CAMPIONAMENTO" secondi
 *
 * @param argc
 * @param argv
 */
void fork_PWCAP_plot(int argc, char **argv)
{
    /* VARIABILI MONITORAGGIO --------------------------------------------------------- */
    int EventSet = PAPI_NULL;
    long long *values;
    long long start_time, before_time, after_time;
    double elapsed_time, total_time;
    char units[MAX_EVENTS][PAPI_MIN_STR_LEN];
    int data_type[MAX_EVENTS];
    char filenames[MAX_EVENTS][PAPI_MIN_STR_LEN];
    FILE *fff[MAX_EVENTS];

    double init_PKG = 0;
    double init_PP0 = 0;
    double init_DRAM = 0;

    /* Inizializzazione PAPI */
    values = PWCAP_plot_init(argc, argv, &EventSet, event_names, units, data_type, fff, filenames);

    /* Settaggio limitazioni alla potenza del PACKAGE, se presenti */
    int LIMITE = atoi(argv[4]);
    if (LIMITE > 0)
    {
        PAPI_limits(&EventSet, values, LIMITE);
    }

#ifndef SITUAZIONE_INIZIALE
    /* MONITORAGGIO SITUAZIONE INIZIALE --------------------------------------------------- */
    /* Leggo per 2 secondi i registri energetici per ottenere la situaizone "di partenza" */
    before_time = PAPI_start_AND_time(&EventSet);
    sleep(2);
    after_time = PAPI_stop_AND_time(&EventSet, values);

    /* Calcolo l'energia al secondo consumata dalla situazione "di partenza" */
    init_PKG = (double)(values[PACKAGE_index] / 1.0e6) / 2;
    init_PP0 = (double)(values[PP0_index] / 1.0e6) / 2;
    init_DRAM = (double)(values[DRAM_index] / 1.0e6) / 2;

    /* Open output files */
    FILE *f = fopen("results.SITUAZIONE_INIZIALE", "w");

    fprintf(f, "init_PKG: %f\ninit_PP0: %f\ninit_DRAM: %f\n", init_PKG, init_PP0, init_DRAM);
    fflush(f);
    fclose(f);

    /*printf("\ninit_PKG: %f\n", init_PKG);
    printf("\ninit_PP0: %f\n", init_PP0);
    printf("\ninit_DRAM: %f\n", init_DRAM);*/

#endif

    /* GESTIONE SEGNALI -------------------------------------------------------------- */
    /* Il padre invia SIGUSR1 quando il programma da monitorare è terminato */
    if (signal(SIGUSR1, fine_monitoraggio) == SIG_ERR)
    {
        perror("\nSignal non riuscita\n");
        exit(EXIT_FAILURE);
    }
    /* Segnalo al padre di far partire il programma */
    kill(getppid(), SIGUSR1);

    /* INIZIO MONITORAGGIO -------------------------------------------------------------- */
    start_time = PAPI_get_real_nsec();

    /* Ripeto finché il padre non segnali di terminare il monitoraggio */
    while (term)
    {

        /* Start Counting */
        before_time = PAPI_start_AND_time(&EventSet);

        /* Campiono ogni 0.1 sec */
        usleep(FREQUENZA_CAMPIONAMENTO);

        /* Stop Counting */
        after_time = PAPI_stop_AND_time(&EventSet, values);

        total_time = ((double)(after_time - start_time)) / 1.0e9;
        elapsed_time = ((double)(after_time - before_time)) / 1.0e9;

        /* Stampe del tempo (in secondi) e dell'Average Power in watt*/
        // sottraggo ad ogni lettura la (energia al secondo * tempo trascorso) ad ogni lettura

        // PACKAGE
        fprintf(fff[PACKAGE_index], "%.4f %.3f \n",
                total_time,
                (((double)values[PACKAGE_index] / 1.0e6) / elapsed_time) - init_PKG);
        fflush(fff[PACKAGE_index]);

        // PP0
        fprintf(fff[PP0_index], "%.4f %.3f \n",
                total_time,
                (((double)values[PP0_index] / 1.0e6) / elapsed_time) - init_PP0);
        fflush(fff[PP0_index]);

        // DRAM
        fprintf(fff[DRAM_index], "%.4f %.3f \n",
                total_time,
                (((double)values[DRAM_index] / 1.0e6) / elapsed_time) - init_DRAM);
        fflush(fff[DRAM_index]);
    }

    /* TERMINAZIONE -------------------------------------------------------------- */
    fclose(fff[PACKAGE_index]);
    fclose(fff[PP0_index]);
    fclose(fff[DRAM_index]);

    PAPI_term(&EventSet, values, LIMITE);
    exit(EXIT_SUCCESS);
}

/**
 * @brief wait con controllo di stato
 *
 */
void wait_terminazione()
{

    int status = 0;
    int terminated_pid = wait(&status);

    if (WIFEXITED(status))
    { // terminazione volontaria figlio
        if (WEXITSTATUS(status) == EXIT_SUCCESS)
        { // successo
            exit(EXIT_SUCCESS);
        }
        else
        { // fallimento

            fprintf(stderr, "\nERRORE: Terminazione volontaria ma fallita (!= EXIT_SUCCESS) del programma con PID = %d\n", terminated_pid);
            exit(EXIT_FAILURE);
        }
    }
    else
    { // terminazione involontaria figlio
        if (WIFSIGNALED(status))
        {
            fprintf(stderr, "\nERRORE: Terminazione involontaria del programma %d per segnale %d\n", terminated_pid, WTERMSIG(status));
            exit(EXIT_FAILURE);
        }
    }
}