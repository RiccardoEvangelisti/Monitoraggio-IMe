#include "../../libs/SLGEWOS/SLGEWOS.h"
#include "../../libs/MONIT/PWCAP.h"

void controlli_opz(char **argv)
{

    if (argv[1] == NULL)
    {
        printf("\nUsage: SLGEWOS_tests     PARTE     n      ALLOC\n");
        exit(EXIT_FAILURE);
    }

    if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help"))
    {
        printf("\nUsage: SLGEWOS_tests     PARTE     n      ALLOC");

        printf("\n\tPARTE:");
        printf("\n\t\t0: monitora l'intera esecuzione (1+2)");
        printf("\n\t\t1: monitora solo l'inizializzazione della Inibition Table (prima parte)");
        printf("\n\t\t2: monitora solo il calcolo della soluzione (seconda parte)");

        printf("\n\tn:");
        printf("\n\t\tintero (>0) tale che il sistema è a n equazioni e n incognite");

        printf("\n\tALLOC:");
        printf("\n\t\t0: le matrici sono allocate in modo NON contiguo");
        printf("\n\t\t1: le matrici sono allocate in modo contiguo");
        printf("\n\n");

        exit(EXIT_SUCCESS);
    }

    if (argv[2] == NULL || argv[3] == NULL)
    {
        printf("\nUsage: SLGEWOS_tests     PARTE     n      ALLOC\n");
        exit(EXIT_FAILURE);
    }

    if (atoi(argv[1]) != 0 && atoi(argv[1]) != 1 && atoi(argv[1]) != 2)
    {
        printf("\nUsage: SLGEWOS_tests     PARTE     n      ALLOC");
        exit(EXIT_FAILURE);
    }

    if (atoi(argv[2]) <= 0)
    {
        printf("\nUsage: SLGEWOS_tests     PARTE     n      ALLOC");
        exit(EXIT_FAILURE);
    }

    if (atoi(argv[3]) != 0 && atoi(argv[3]) != 1)
    {
        printf("\nUsage: SLGEWOS_tests     PARTE     n      ALLOC");
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Handler associato alla ricezione di SIGUSR1, inviato dal figlio PWCAP_plot quando ha iniziato il monitoraggio
 *
 */
void inizio_monitoraggio()
{
    // figlio PWCAP_plot sta monitorando
    ;
}

int main(int argc, char **argv)
{

    /* MAIN: CONTROLLO ARGOMENTI ------------------------------------------------------------------------------- */
    controlli_opz(argv);

    /* PARTE:
       0: monitora l'intera esecuzione (1+2)
       1: monitora solo l'inizializzazione della Inibition Table (prima parte)
       2: monitora solo il calcolo della soluzione (seconda parte) */
    int PARTE = atoi(argv[1]);

    /* ALLOC:
        0: le matrici sono allocate in modo NON contiguo
        1: le matrici sono allocate in modo contiguo */
    int ALLOC = atoi(argv[3]);

    /* SLEGEWOS: INIZIALIZZAZIONE E ALLOCAZIONE VARIABILI ------------------------------------------------------- */
    int n = atoi(argv[2]); /* n: intero (>0) tale che il sistema è a n equazioni e n incognite */
    double **A;            /* A: è la matrice dei coefficienti, di dimensione n x n */
    double *b;             /* b: è il vettore di termini costanti, di dimensione n */
    double *s;             /* s: è il vettore soluzione, di dimensione n */
    double **K;            /* K: è la seconda parte della Inibition Table, di dimensione n x n */
    double *H;             /*H: è il vettore di Auxiliary Quantities, di dimensione n */
    double *F;             /*F: è un vettore di appoggio in cui vengono inseriti tutti i termini di b, di dimensione n */
    double **X;            /*X: è la prima parte della Inibition Table, di dimensione n x n */

    A = AllocateMatrix2D(n, n, ALLOC);
    b = AllocateVector(n);
    s = AllocateVector(n);
    K = AllocateMatrix2D(n, n, ALLOC);
    H = AllocateVector(n);
    F = AllocateVector(n);

    X = A; // X=AllocateMatrix2D(n,n,CONTIGUOUS);
    FillMatrix2D(A, n, n);
    FillVector(b, n, 1);

    /* Monitoraggio del solo calcolo: l'inizializzazione viene fatta ora  */
    if (PARTE == 2)
    {
        SLGEWOS_init(n, A, s, F, K, b, X);
    }

    /* MAIN: VARIABILI MONITORAGGIO ------------------------------------------------------------------------------- */
    int EventSet = PAPI_NULL;
    long long *values;
    char units[MAX_EVENTS][PAPI_MIN_STR_LEN];
    int data_type[MAX_EVENTS];
    int i;
    long long before_time, after_time;
    double elapsed_time;

    /* Inizializzazione PAPI */
    values = PWCAP_init(argc, argv, &EventSet, event_names, units, data_type);

    /* Start Counting */
    before_time = PAPI_start_AND_time(&EventSet);

    /* SLGEWOS: ESECUZIONE PROGRAMMA ---------------------------------------------------------------------------- */
    /* Monitoraggio dell'intera esecuzione */
    if (PARTE == 0)
    {
        SLGEWOS_init(n, A, s, F, K, b, X);
        SLGEWOS_risoluzione(s, n, K, H, F, X);
    }
    /* Monitoraggio della sola inizializzazione della Inibition Table */
    else if (PARTE == 1)
    {
        SLGEWOS_init(n, A, s, F, K, b, X);
    }
    /* Monitoraggio del solo calcolo della soluzione. L'inizializzazione è stata eseguita prima */
    else if (PARTE == 2)
    {
        SLGEWOS_risoluzione(s, n, K, H, F, X);
    }

    /* MAIN: MONITORAGGIO -------------------------------------------------------------------------------------- */
    /* Stop Counting */
    after_time = PAPI_stop_AND_time(&EventSet, values);

    elapsed_time = ((double)(after_time - before_time)) / 1.0e9;

    /* Stampe */
    if (!TESTS_QUIET)
    {
        printf("\nStopping measurements, took %.3fs, gathering results...\n\n", elapsed_time);

        /* Prelevo solo i 3 registri interessanti */
        for (i = 0; i < MAX_EVENTS; i++)
        {
            if (strstr(event_names[i], "ENERGY_UJ"))
            {
                printf("%-45s%4.6f J\t(Average Power %.6f W)\n",
                       event_names[i],
                       (double)values[i] / 1.0e6,
                       ((double)values[i] / 1.0e6) / elapsed_time);
            }
        }
    }

    /* SLEGEWOS: DEALLOCAZIONE VARIABILI ------------------------------------------------------------------------ */
    DeallocateMatrix2D(A, n, ALLOC);
    DeallocateMatrix2D(K, n, ALLOC);
    DeallocateVector(H);
    DeallocateVector(F);
    DeallocateVector(s);
    DeallocateVector(b);
    // DeallocateMatrix2D(X,n,CONTIGUOUS);

    //
    wait_terminazione();
}