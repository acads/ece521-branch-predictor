#
# ECE 521 - Computer Design Techniques, Fall 2014
# Project 2 - Branch Predictor Implementation 
#
# gnuplot script to plot 'm' size vs. 'miss-predictions' for perl_trace.txt. 
#
# Author: Aravindhan Dhanasekaran <adhanas@ncsu.edu>
#

reset

set terminal postscript eps enhanced dashed 
set output 'perl_bimodal.eps'

set title 'perl, bimodal'
set grid

set ylabel 'Misprediction Rate (in %)'
set xlabel 'm (# of lower order bits in PC to index predictor table)'

set style line 1 lc rgb '#FF0000' lt 1 lw 2 pt 2 ps 1.5     # cont-red, x pt
plot 'perl_bimodal.dat' using 1:2 with linespoints ls 1 title 'misprediction rate for perl'

reset

