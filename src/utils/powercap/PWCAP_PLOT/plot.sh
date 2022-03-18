#!/bin/bash

# Rimozione i precedenti risultati
sudo rm results.*

# Esecuzione del programma di monitoraggio
sudo ./PWCAP_PLOT &

# Esecuzione del programma da monitorare
sleep 1

/home/riccardo/Desktop/IMe/src/main/SLGEWOS 1000 2 0

sleep 1

# Interruzione del programma di monitoraggio
sudo pkill PWCAP_PLOT


gnuplot "gnuplotFinal.gp"