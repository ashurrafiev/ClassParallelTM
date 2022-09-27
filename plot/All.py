from plot import *

dataPath = '../bak/res0812/3'
#dataPath = '../bak/res0712-JM'

page = Page(1)

# accuracy

charts = [
	#('acctrain%s', 'Accuracy (Train)', (False, 0.5, 1.0, 0.1), 'accuracy', '%.2f'),
	('acctest%s', 'Accuracy (Test)', (False, 0.5, 1.0, 0.1), 'accuracy', NumberFormatter.percent(0)),
]

data = Data.read(File('%s/acc.csv' % dataPath))
#addColAvg(data, 'acctrain%s')
addColAvg(data, 'acctest%s')

plotMC(page, data, charts)

# class status charts

charts = [
	('inc', 'Number of inclusions', (False, 0, 80000, 10000), 'literals', '%.0f'),
	('flips', 'Number of decision flips', (False, 0, 200, 20), 'flips', '%.0f'),
	#('ratio', 'Feedback by type', (True, 1/64.0, 64.0, 4), 'T1 / T2 ratio', '%.2f'),
	#('avote', 'Absolute sum votes', (False, 0, 25, 5), 'votes', '%.0f'),
	#('vdiff', 'Sum vote difference', (False, 0, 25, 5), 'votes', '%.0f'),
	#('v1max', 'Max sum vote, y=1', (False, -50, 50, 10), 'vote sum', '%.0f'),
	#('v1min', 'Min sum vote, y=1', (False, -50, 50, 10), 'vote sum', '%.0f'),
	#('v0max', 'Max sum vote, y=0', (False, -50, 50, 10), 'vote sum', '%.0f'),
	#('v0min', 'Min sum vote, y=0', (False, -50, 50, 10), 'vote sum', '%.0f'),
	#('v1range', 'Sum vote range, y=1', (False, 0, 60, 10), 'votes', '%.0f'),
	#('v0range', 'Sum vote range, y=0', (False, 0, 60, 10), 'votes', '%.0f'),
	#('ccorr', 'Mean clause similarity', (False, 0, 0.1, 0.01), 'votes', '%.2f'),
]

data = []
for cls in range(numClasses):
	d = Data.read(File('%s/c%d-status.csv' % (dataPath, cls)))
	d.addCol('ratio', ratio(getNum('type1'), getNum('type2')))
	d.addCol('vdiff', diff(getNum('vote1'), getNum('vote0')))
	d.addCol('v1range', diff(getNum('v1max'), getNum('v1min')))
	d.addCol('v0range', diff(getNum('v0max'), getNum('v0min')))
	data.append(d)

data.append(combineDataAvg(data, [hdr for (hdr, _, _, _, _) in charts]))
plotCombSC(page, data, charts)

# finish and print
page.printPage(System.out)
