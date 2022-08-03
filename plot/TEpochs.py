from plot import *

dataPath = '../bak/res0727/ep%d/acc%d_%d.csv'
dataPathCont = '../bak/res0727/cont/acc_%d.csv'

styles = [
	('stroke-width:1;stroke:#f66'),
	('stroke-width:1;stroke:#a33'),
	('stroke-width:1;stroke:#333'),
	('stroke-width:1;stroke:#999'),
	('stroke-width:1;stroke:#ccc'),
]

maxs = [0, 0.858, 0.863]
ts = [18, 25, 50, 75, 100]

page = Page(1)

chart = createChart('Accuracy (Test) - GA', (False, 0.8, 0.9, 0.01), 'accuracy', '%.2f')
chart.legend.setCols(len(ts))

accId = [1, 2, 2]
d = None
for idx in range(19, -1, -1):
	for ep in range(3):
		de = Data.read(File(dataPath % (ep, accId[ep], idx)))
		if ep==0:
			d = de
		else:
			d = Data(d.headers())
			row = d.addRow()
			row.set('t', str(ep*60))
			row.set('avgacctest', maxs[ep])
			d.append(de)
		pop = Population(d, 't', 'avgacctest', 'fill:none;%s' % styles[idx//4])
		if (idx%4)==0 and ep==0:
			chart.addPopLegend('Tlit = %d' % ts[idx//4], pop)
		else:
			chart.addPop(pop)

page.add(chart)

chart = createChart('Accuracy (Test) - Continuous', (False, 0.8, 0.9, 0.01), 'accuracy', '%.2f')
chart.legend.setCols(len(ts))

for idx in range(19, -1, -1):
	d = Data.read(File(dataPathCont % idx))
	pop = Population(d, 't', 'avgacctest', 'fill:none;%s' % styles[idx//4])
	if (idx%4)==0:
		chart.addPopLegend('Tlit = %d' % ts[idx//4], pop)
	else:
		chart.addPop(pop)

page.add(chart)

# finish and print
page.printPage(System.out)
