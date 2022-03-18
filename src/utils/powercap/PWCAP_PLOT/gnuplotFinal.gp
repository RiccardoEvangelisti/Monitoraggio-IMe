reset
set terminal png size 800,600
set output 'output.png'

set autoscale

#set xrange [0:5]
set yrange [0:45]

#set tics font ",18"
#set xlabel "x" font ",18"
#set ylabel "y" font ",18"
#set lmargin at screen 0.5 # set size of left margin
#set rmargin at screen 0.82 # set size of right margin
#set bmargin at screen 0.12 # set size of bottom margin
#set tmargin at screen 0.95 # set size of top margin

set xlabel "Time (seconds)"

set ylabel "Average Power (Watts)"

set title "Power Measurement"

set grid


#using primaColonnaAsseX:secondaColonnaAsseY
plot "results.powercap:::ENERGY_UJ:ZONE0_SUBZONE1" using 1:2 title "DRAM" with linespoints, \
     "results.powercap:::ENERGY_UJ:ZONE0" using 1:2 title "PACKAGE" with linespoints, \
     "results.powercap:::ENERGY_UJ:ZONE0_SUBZONE0" using 1:2 title "PP0" with linespoints
