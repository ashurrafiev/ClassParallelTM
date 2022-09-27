i=0
for s in 5 9 9 3 8 1 3 7 7 3
do
	cp st${s}_c${i}.csv sel_c${i}.csv
	((i++))
done
