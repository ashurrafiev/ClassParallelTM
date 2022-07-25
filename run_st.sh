i=0
for t in 0.032 0.064 0.128
do
	for x in {0..4}
	do
		./mnist -step-size 12000 -steps 1 -acc-eval-test 1 -par 1 -t ${t} -log-acc -log-append -save-state st${i}_c%d.csv
		((i++))
	done
done
