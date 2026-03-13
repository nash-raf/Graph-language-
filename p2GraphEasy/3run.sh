valgrind --tool=callgrind ./final_program
callgrind_annotate callgrind.out.17402
gprof2dot -f callgrind callgrind.out.11779 -o graph.dot

dot -Tpng graph.dot -o graph1.png

callgrind_annotate callgrind.out.* > annotate.txt
gprof2dot -f callgrind callgrind.out.11779 -o graph.dot
dot -Tpng graph.dot -o graph1.png