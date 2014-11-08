#
# ECE 521 - Computer Design Techniques, Fall 2014
# Project 2 - Branch Predictor Implementation 
#
# gnuplot script to plot 'm' size vs. 'miss-predictions' for gcc_trace.txt. 
#
# Author: Aravindhan Dhanasekaran <adhanas@ncsu.edu>
#

reset

set terminal postscript eps enhanced dashed 
set output 'gcc_bimodal.eps'

set title 'gcc, bimodal'
set grid

set ylabel 'Misprediction Rate'
set xlabel 'm'
#set yrange [:0.7]
#set y2range [1:14]
#set ytics out 0.01,0.05,0.7 nomirror
#set y2tics out 1,1,13 nomirror

set style line 1 lc rgb '#FF0000' lt 1 lw 2 pt 2 ps 1.5     # cont-red, x pt
set style line 2 lc rgb '#0000FF' lt 1 lw 2 pt 4 ps 1.5     # cont-blue, shaded square pt
set style line 3 lc rgb '#000000' lt 1 lw 2 pt 6 ps 1.5     # cont-black, shaded circle pt
set style line 4 lc rgb '#FF8C00' lt 1 lw 2 pt 8 ps 1.5     # cont-darkorange, shaded 3angle
set style line 5 lc rgb '#FF0000' lt 3 lw 2 pt 3 ps 1.5     # dash-red, * pt
set style line 6 lc rgb '#0000FF' lt 3 lw 2 pt 5 ps 1.5     # dash-blue, square pt
set style line 7 lc rgb '#000000' lt 3 lw 2 pt 7 ps 1.5     # dash-black, circle pt
set style line 8 lc rgb '#FF8C00' lt 3 lw 2 pt 9 ps 1.5     # dash-darkorange, 3angle pt

plot 'gcc_bimodal.dat' using 1:2 index 0 with linespoints ls 1 axes xy

reset

