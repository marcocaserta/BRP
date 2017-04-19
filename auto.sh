# max = 2   ---> MAX HEIGHT INCREASE
for H in 5
do
    for W in 6 7 8 9 10
    do
        for n in `seq 1 40`;
        do
            for c in `seq 1 $W`;
            do
                for r in 1 2 3 4 5
                do
                    echo PROBLEM data-$H-$W-$n.dat with $(($H + 2))
                    bin/dyn -f ../v3/data/data$H-$W-$n.dat -t 4 -d $c -n $(($H
                    + 2)) -c 1
                    cat result.dat >> result17-All.dat
                done
            done
        done
    done
done

