#
# ECE 521 - Computer Design Techniques, Fall 2014
# Project 2 - Branch Predictor Implementation 
#
# gnuplot script to plot misprediction rate for trace jpeg for different m and
# n values.
#
# Author: Aravindhan Dhanasekaran <adhanas@ncsu.edu>
#

reset

set terminal postscript eps enhanced dashed 
set output 'jpeg_gshare.eps'

set title 'jpeg, gshare'
set grid

set xlabel 'm (# of lower order bits in PC to index predictor table)'
set ylabel 'Misprediction Rate (in %)'

set style line 1 lc rgb '#FF0000' lt 1 lw 2 pt 2 ps 1.5     # cont, red, x pt
set style line 2 lc rgb '#0000FF' lt 1 lw 2 pt 4 ps 1.5     # cont, blue, clear square pt
set style line 3 lc rgb '#000000' lt 1 lw 2 pt 6 ps 1.5     # cont, black, clear circle pt
set style line 4 lc rgb '#FF8C00' lt 1 lw 2 pt 8 ps 1.5     # cont, darkorange, clear 3angle pt
set style line 5 lc rgb '#9400D3' lt 1 lw 2 pt 14 ps 1.5     # cont, darkviolet, clear pentagon pt
set style line 6 lc rgb '#FF00FF' lt 1 lw 2 pt 12 ps 1.5    # cont, magenta, clear diamond pt

plot 'jpeg_gshare.dat' using 1:2 index 0 title 'n = 2' with linespoints ls 1,    \
         '' using 1:2 index 1 title 'n = 4' with linespoints ls 2,     \
         '' using 1:2 index 2 title 'n = 6' with linespoints ls 3,     \
         '' using 1:2 index 3 title 'n = 8' with linespoints ls 4,     \
         '' using 1:2 index 4 title 'n = 10' with linespoints ls 5,    \
         '' using 1:2 index 5 title 'n = 12' with linespoints ls 6

reset

