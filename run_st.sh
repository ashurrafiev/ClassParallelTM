mkdir cont
i=0
for t in 0.024 0.032 0.064 0.096 0.128
do
	for x in {0..3}
	do
		./mnist -step-size 200 -steps 180 -acc-eval-test 1 -par 1 -t ${t} -log-acc
		#./mnist -step-size 200 -steps 60 -acc-eval-test 1 -par 1 -t ${t} -log-acc -save-state ep1/st_${i}_c%d.csv
		#./mnist -step-size 200 -steps 60 -acc-eval-test 1 -par 1 -t ${t} -log-acc -load-state ep1/st_X_c%d.csv -save-state ep2/st_${i}_c%d.csv
		cp acc.csv cont/acc_${i}.csv
		((i++))
	done
done

#for i in {0..19}; do tail -c 7 cont/acc_${i}.csv; done
