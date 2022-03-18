#!/bin/bash

echo "Usage: sudo -E ./start_papi_program.sh  nome_sorgente.c "; 

if [ "$EUID" -ne 0 ]
  then echo "Eseguire come utente root o con sudo"
  exit
fi

if [[ $1 == '-h' ]]
    then echo "Usage: sudo -E ./start_papi_program.sh  nome_sorgente.c "; 
    exit
fi

read -p "--> PAPI_OUTPUT_DIRECTORY = " -e -i $( dirname $1 ) PAPI_OUTPUT_DIRECTORY
export PAPI_OUTPUT_DIRECTORY=$PAPI_OUTPUT_DIRECTORY

NOME_OUTPUT_FILE="$( dirname $1 )/$( basename $1 .c )"

# se non esiste il file lo compilo:
if [[ ! -f "$NOME_OUTPUT_FILE" ]]
then 
  gcc -g -Wextra -Wall -O3 ./$1 -o $NOME_OUTPUT_FILE -I$PAPI_DIR/include -I$PAPI_DIR/share/papi/testlib -L$PAPI_DIR/share/papi/testlib -ltestlib -ldo_loops $PAPI_DIR/lib/libpapi.a
  # per qualche motivo scrivere: -L$PAPI_DIR/lib -lpapi non fa funzionare i contatori appio (risultano tutti 0)
fi

# se è stato compilato
if [[ -f $NOME_OUTPUT_FILE ]]; then
  echo "--------------------------------------------------------------------------"
  echo "Inizio esecuzione \"./$NOME_OUTPUT_FILE\""
  echo "--------------------------------------------------------------------------"
  echo ""
  echo ""

  ./$NOME_OUTPUT_FILE

  echo ""
  echo ""
  echo "--------------------------------------------------------------------------"
  echo "Fine esecuzione di \"./$NOME_OUTPUT_FILE\""
  echo "--------------------------------------------------------------------------"

  # se è un monitoraggio HL
  if [[ -d $PAPI_OUTPUT_DIRECTORY/papi_hl_output ]]; then
    # modifico i permessi dell'output HL
    chmod o=rwx $PAPI_OUTPUT_DIRECTORY/papi_hl_output


    # formattare il testo dell'output
    read -p "--> Formattare il risultato? " -e -i "yes" RESULT
    if [[ $RESULT == "yes" ]]; then 
      # esegue l'ultima cartella di risultati creata
      sudo python3 $( dirname $0)/papi_hl_output_writer.py --source $PAPI_OUTPUT_DIRECTORY/papi_hl_output --notation=derived --type=summary
      if [[ -f "papi.json" ]]; then sudo mv papi.json $PAPI_OUTPUT_DIRECTORY/papi_hl_output/ ; fi;
      if [[ -f "papi_sum.json" ]]; then sudo mv papi_sum.json $PAPI_OUTPUT_DIRECTORY/papi_hl_output/ ; fi;
    fi
  fi; 
fi;