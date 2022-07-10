from plot import *

#dataPath = '../bak/res0605'
dataPath = '../bak/res0606-JM'

page = Page(1)

# accuracy

charts = [
	#('acctrain%s', 'Accuracy (Train)', (False, 0.5, 1.0, 0.1), 'accuracy', '%.2f'),
	('acctest%s', 'Accuracy (Test)', (False, 0.5, 1.0, 0.1), 'accuracy', '%.2f'),
]

data = Data.read(File('%s/acc.csv' % dataPath))
addColAvg(data, 'acctrain%s')
addColAvg(data, 'acctest%s')

plotMC(page, data, charts)

# class status charts

charts = [
	('inc', 'Number of inclusions', (False, 0, 12000, 1000), 'literals', '%.0f'),
	('flips', 'Number of decision flips', (False, 0, 60, 10), 'flips', '%.0f'),
	('ratio', 'Feedback by type', (True, 1/64.0, 64.0, 4), 'T1 / T2 ratio', '%.2f'),
	#('avote', 'Absolute sum votes', (False, 0, 25, 5), 'votes', '%.0f'),
	('vdiff', 'Sum votes difference', (False, 0, 25, 5), 'votes', '%.0f'),
]

data = []
for cls in range(numClasses):
	d = Data.read(File('%s/c%d-status.csv' % (dataPath, cls)))
	d.addCol('ratio', ratio(getNum('type1'), getNum('type2')))
	d.addCol('vdiff', diff(getNum('vote1'), getNum('vote0')))
	data.append(d)

data.append(combineDataAvg(data, [hdr for (hdr, _, _, _, _) in charts]))
plotCombSC(page, data, charts)

# finish and print
page.printPage(System.out)
