# Monitoraggio-IMe
Monitoraggio energetico di algoritmi numerici tramite la libreria PAPI


**PREREQUISITI** (leggere interamente prima di proseguire)

  - Installare il tool Linux "perf"

  - Installare PAPI. Seguire l'installazione guidata [https://bitbucket.org/icl/papi/wiki/Downloading-and-Installing-PAPI]
  - Eseguire nella configurazione di installazione i componenti RAPL e POWERCAP:
      - *./configure --with-components="rapl powercap"*
    - IMPORTANTE: eseguire il comando da dentro la cartella *papi/src* !!!
  - Impostare la variabile d'ambiente *PAPI_DIR* al percorso: *papi/src/install/*
  
  - Installare il tool *gnuplot*, necessario per generare i grafici.
		
  - Assicurarsi con *papi/src/utils/papi_avail* che ci siano eventi disponibili. Se nessun evento tra i preset events è disponibile per l'uso, o qualcosa è andato storto nell'installazione o la CPU non supporta/permette la lettura degli eventi.
  - Eseguire tutte le azioni come root oppure impostare a zero il */proc/sys/kernel/perf_event_paranoid*
  - Assicurarsi che con *papi/src/utils/papi_component_avail* tutti i componenti siano correttamente abilitati
  
  
**UTILIZZO**
  - In *src/main* è contenuto un semplice main che esegue IMe prendendo i dati iniziali da linea di comando
  - In *src/tests* si trovano i programmi di test di IMe, nella verisone a campionamento ogni 0.1 secondi e a campionamento unico.
  
  - Per eseguire il programma di test di IMe, si utilizzi lo script *bin/tests/plot.sh* (eseguirlo a vuoto o con "-h" per vedere le opzioni)
  - Per eseguire TUTTI i test nelle varie combinazini possibili, si utilizzi lo script *bin/tests/ALL_TESTS.sh*
     - ATTENZIONE: i test che impostano limiti di potenza sulla CPU ripristinano le impostazioni di default solo al termine dell'esecuzione. Dunque è importante non terminare a metà tali test. Si consulti l'output generato su stdout per monitorare l'andamento.
  
  - In *src/utils* sono presenti altri programmi tratti da esempi disponibili nella documentazione di PAPI (questi non monitorano IMe ma codice di esempio):
     - In *src/utils/papi_high_level* si mostra come si utilizza l'interfaccia ad alto livello di PAPI
