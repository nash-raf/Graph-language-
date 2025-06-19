# Number of iterations
N=10000

for i in $(seq 1 $N)
do
    echo "Run #$i"
    time ./GraphProgram graph_code_test.txt graph_code_test.txt4
    echo "--------------------------"
done