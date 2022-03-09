reset
set terminal png size 800,600
set output 'output.png'

set autoscale

#set xrange [0:15]
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

set multiplot layout 2,2

#set xtics add ( TEMPO_TOT TEMPO_TOT )
#set label at TEMPO_TOT, 0 "" point pointtype 7 pointsize 1
#unset ytics
#set ytics add ( STATS_mean_y STATS_mean_y )

array point[1]

stats "results.powercap:::ENERGY_UJ:ZONE0" using 1:2 nooutput
plot "results.powercap:::ENERGY_UJ:ZONE0" using 1:2 title "PACKAGE" with lines, \
     "results.powercap:::ENERGY_UJ:ZONE0" using 1:(STATS_mean_y) with lines lt 2 lw 2 title sprintf("Avg: %.2f W", STATS_mean_y), \
     point us (TEMPO_TOT):(STATS_mean_y) pt 7 lc 3 title sprintf("Time: %.2f s", TEMPO_TOT)

     
stats "results.powercap:::ENERGY_UJ:ZONE0_SUBZONE0" using 1:2 nooutput
plot "results.powercap:::ENERGY_UJ:ZONE0_SUBZONE0" using 1:2 title "PP0" with lines, \
     "results.powercap:::ENERGY_UJ:ZONE0_SUBZONE0" using 1:(STATS_mean_y) with lines lt 2 lw 2 title sprintf("Avg: %.2f W", STATS_mean_y), \
     point us (TEMPO_TOT):(STATS_mean_y) pt 7 lc 3 title sprintf("Time: %.2f s", TEMPO_TOT)


stats "results.powercap:::ENERGY_UJ:ZONE0_SUBZONE1" using 1:2 nooutput
plot "results.powercap:::ENERGY_UJ:ZONE0_SUBZONE1" using 1:2 title "DRAM" with lines, \
     "results.powercap:::ENERGY_UJ:ZONE0_SUBZONE1" using 1:(STATS_mean_y) with lines lt 2 lw 2 title sprintf("Avg: %.2f W", STATS_mean_y), \
     point us (TEMPO_TOT):(STATS_mean_y) pt 7 lc 3 title sprintf("Time: %.2f s", TEMPO_TOT)
     

