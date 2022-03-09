#!/bin/bash

# Controllo root
if [ "$EUID" -ne 0 ]
  then echo "Eseguire come utente root o con sudo"
  exit 1
fi

PROG_O3="O3/SLGEWOS_tests_O3"
PROG_O2="O2/SLGEWOS_tests_O2"
PROG_O1="O1/SLGEWOS_tests_O1"
PROG_O0="O0/SLGEWOS_tests_O0"

PARTE_1="1"
PARTE_2="2"

ALLOC_NONCONT="0"
ALLOC_CONT="1"

n_1000="1000"
n_3000="3000"
n_5000="5000"

NOLIMIT="-1"
LIMIT_10W="10"

for PROG in {$PROG_O3,$PROG_O2,$PROG_O1,$PROG_O0}; do

  read -p "Procedere con $PROG?"

  for PARTE in {$PARTE_1,$PARTE_2}; do

    for n in {$n_1000,$n_3000,$n_5000}; do

      for ALLOC in {$ALLOC_CONT,$ALLOC_NONCONT}; do

        for LIMIT in {$NOLIMIT,$LIMIT_10W}; do

          sudo ./plot.sh $PROG $PARTE $n $ALLOC $LIMIT
          
        done
      done
    done
  done
done