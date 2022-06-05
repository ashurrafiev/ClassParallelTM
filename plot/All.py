from plot import *

page = Page(1)

# accuracy

charts = [
	#('acctrain%s', 'Accuracy (Train)', (False, 0.5, 1.0, 0.1), 'accuracy', '%.2f'),
	('acctest%s', 'Accuracy (Test)', (False, 0.5, 1.0, 0.1), 'accuracy', '%.2f'),
]

data = Data.read(File('../acc.csv'))
addColAvg(data, 'acctrain%s')
addColAvg(data, 'acctest%s')

plotMC(page, data, charts)

# class status charts

charts = [
	('inc', 'Number of inclusions', (False, 0, 12000, 1000), 'literals', '%.0f'),
	('flips', 'Number of decision flips', (False, 0, 60, 10), 'flips', '%.0f'),
	('ratio', 'Feedback by type', (True, 0.25, 4.0, 2), 'T1 / T2 ratio', '%.2f'),
	('avote', 'Absolute sum votes', (False, 0, 25, 5), 'votes', '%.0f'),
	('vdiff', 'Sum votes difference', (False, 0, 25, 5), 'votes', '%.0f'),
]

data = []
for cls in range(numClasses):
	d = Data.read(File('../c%d-status.csv' % cls))
	d.addCol('ratio', ratio(getNum('type1'), getNum('type2')))
	d.addCol('vdiff', diff(getNum('vote1'), getNum('vote0')))
	data.append(d)

data.append(combineDataAvg(data, [hdr for (hdr, _, _, _, _) in charts]))
plotCombSC(page, data, charts)

# finish and print
page.printPage(System.out)
