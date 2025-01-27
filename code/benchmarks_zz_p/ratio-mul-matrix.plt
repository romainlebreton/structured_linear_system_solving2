set xlabel "n" offset 33, 1.5
set ylabel "ratio naive / BJMS16"
set terminal pdf
set rmargin 6

set out outfile
set style line 1 lt 1 lw 8 lc 4
set style line 2 lt 1 lw 4 lc 7
set style line 3 lt 1 lw 3 lc 4
plot infile1 using 1:2 with lines ls 1 title "p=65537",\
     infile2 using 1:2 with lines ls 2 title "p=882705526964617217"

