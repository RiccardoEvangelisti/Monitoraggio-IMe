#include "../../libs/SLGEWOS/SLGEWOS.h"
#include "../../libs/MONIT/PWCAP.h"

void controlli_opz(char **argv)
{

    if (argv[1] == NULL || !strcmp(argv[1], "-h") || !strcmp(argv[1], "--help"))
    {
        printf("\nUsage: O*/SLGEWOS_tests_*     PARTE     n      ALLOC      LIMITE");

        printf("\n\tPARTE:");
        printf("\n\t\t0: monitora l'intera esecuzione (1+2)");
        printf("\n\t\t1: monitora solo l'inizializzazione della Inibition Table (prima parte)");
        printf("\n\t\t2: monitora solo il calcolo della soluzione (seconda parte)");

        printf("\n\tn:");
        printf("\n\t\tintero (>0): tale che il sistema è a n equazioni e n incognite");

        printf("\n\tALLOC:");
        printf("\n\t\t0: le matrici sono allocate in modo NON contiguo");
        printf("\n\t\t1: le matrici sono allocate in modo contiguo");

        printf("\n\tLIMITE:");
        printf("\n\t\tintero (>0 e <255): tale che i due Package Power Limit verranno settati a LIMITE");
        printf("\n\t\tintero negativo: non vengono settati limiti");

        printf("\n\n");

        exit(EXIT_SUCCESS);
    }

    if (argv[2] == NULL || argv[3] == NULL || argv[4] == NULL)
    {
        printf("\nUsage: O*/SLGEWOS_tests_*     PARTE     n      ALLOC      LIMITE");
        printf("\n\n");
        exit(EXIT_FAILURE);
    }

    if (atoi(argv[1]) != 0 && atoi(argv[1]) != 1 && atoi(argv[1]) != 2)
    {
        printf("\nUsage: O*/SLGEWOS_tests_*     PARTE     n      ALLOC      LIMITE");
        printf("\n\n");
        exit(EXIT_FAILURE);
    }

    if (atoi(argv[2]) <= 0)
    {
        printf("\nUsage: O*/SLGEWOS_tests_*     PARTE     n      ALLOC      LIMITE");
        printf("\n\n");
        exit(EXIT_FAILURE);
    }

    if (atoi(argv[3]) != 0 && atoi(argv[3]) != 1)
    {
        printf("\nUsage: O*/SLGEWOS_tests_*     PARTE     n      ALLOC      LIMITE");
        printf("\n\n");
        exit(EXIT_FAILURE);
    }

    if (atoi(argv[4]) > 255)
    {
        printf("\nUsage: O*/SLGEWOS_tests_*     PARTE     n      ALLOC      LIMITE");
        printf("\n\n");
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

    /* MAIN: GESTIONE SEGNALI ----------------------------------------------------------------------------------- */
    int pid_PWCAP_plot;
    /* Il figlio PWCAP_plot invia SIGUSR1 quando ha iniziato il monitoraggio */
    if (signal(SIGUSR1, inizio_monitoraggio) == SIG_ERR)
    {
        perror("\nSignal non riuscita\n");
        exit(EXIT_FAILURE);
    }
    switch ((pid_PWCAP_plot = fork()))
    {
    case 0: // Figlio
        fork_PWCAP_plot(argc, argv);
        break;
    case -1: // Errore
        perror("\nERRORE: Creazione figlio\n");
        exit(EXIT_FAILURE);
        break;
    default: // Padre
        break;
    }
    /* Attende che PWCAP_plot invii SIGUSR1 appena ha iniziato il monitoraggio*/
    pause();

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

    /* MAIN: GESTIONE SEGNALI ----------------------------------------------------------------------------------- */
    /* Fermo il monitoraggio del figlio PWCAP_plot */
    kill(pid_PWCAP_plot, SIGUSR1);

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