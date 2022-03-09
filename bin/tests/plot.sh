#!/bin/bash

# Controllo root
if [ "$EUID" -ne 0 ]
  then echo "Eseguire come utente root o con sudo"
  exit 1
fi

PROGR=$( basename $1 2>/dev/null)      # ad es: SLGEWOS_tests_O3
DIR_PROGR=$( dirname $1 2>/dev/null)   # ad es: O3

# Controllo argomenti. Se il numero è sbagliato viene invocato l'helper del programma O3
if [[ "$#" -lt 5 ]]; then set "" "-h" "" "" ""; DIR_PROGR="O3"; PROGR="SLGEWOS_tests_O3"; fi


# Esecuzione del programma
sudo $DIR_PROGR/$PROGR $2 $3 $4 $5


# Se il programma è andato a buon fine (e quindi esistono i file risultati) analizzo i dati
RESULTS_PACKAGE="./results.powercap:::ENERGY_UJ:ZONE0"
RESULTS_PP0="./results.powercap:::ENERGY_UJ:ZONE0_SUBZONE0"
RESULTS_DRAM="./results.powercap:::ENERGY_UJ:ZONE0_SUBZONE1"

if [[ -f "$RESULTS_PACKAGE" && -f "$RESULTS_PP0" && -f "$RESULTS_DRAM" ]]; then

    # Calcolo la potenza media totale
    > "results.POTENZE_MEDIE"
    for file in {$RESULTS_PACKAGE,$RESULTS_PP0,$RESULTS_DRAM}
    do
        sed -i -e 's/\./,/g' $file
        read POTENZA_TOT COUNT <<< $( awk -F " " '{Total=Total+$2 ; Count=Count+1} END{print Total " " Count}' $file )
        TEMPO_TOT=$( awk 'END{ printf "%.1f", $1 }' $file | sed -e 's/,/\./g' )
        sed -i -e 's/,/\./g' $file
        
        echo "POTENZA_TOTALE_W: $POTENZA_TOT" >> "results.POTENZE_MEDIE"
        echo "$POTENZA_TOT $COUNT" | awk '{printf "POTENZA_MEDIA_W: %.3f\n", $1/$2}' >> "results.POTENZE_MEDIE"
        echo "" >> "results.POTENZE_MEDIE"
    done
    echo "TEMPO_TOTALE_s: $TEMPO_TOT" >> "results.POTENZE_MEDIE"

    # Creo la cartella dove spostare i file results
    DIR_RESULTS="$DIR_PROGR/results/${DIR_PROGR}_PARTE:${2}_n:${3}_ALLOC:${4}_LIMITE:${5}________$( date '+%d_%m_%Y_%H_%M_%S' )"
    mkdir -m a=rwx "$DIR_RESULTS"

    # Generazione grafico, passando il tempo totale registrato
    gnuplot -e "TEMPO_TOT=$TEMPO_TOT" "./gnuplot_files/gnuplot.gp"

    sudo mv results.* "output.png" "$DIR_RESULTS"

fi;