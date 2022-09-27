from jdiag import *

from com.xrbpowered.jdiagram.data import NumberFormatter
from com.xrbpowered.jdiagram.data import ExpFormatter

from com.xrbpowered.jdiagram.chart import Page
from com.xrbpowered.jdiagram.chart import ScatterChart
from com.xrbpowered.jdiagram.chart.ScatterChart import Population
from com.xrbpowered.jdiagram.chart import Anchor

chartWidth = 1200
chartHeight = 200
totalSteps = 300
gridSteps = 10
numClasses = 10

classes = [
	('0', 'per-class', 'stroke-width:1;stroke:#aaa'),
	('1', None, 'stroke-width:1;stroke:#aaa'),
	('2', None, 'stroke-width:1;stroke:#aaa'),
	('3', None, 'stroke-width:1;stroke:#aaa'),
	('4', None, 'stroke-width:1;stroke:#aaa'),
	('5', None, 'stroke-width:1;stroke:#aaa'),
	('6', None, 'stroke-width:1;stroke:#aaa'),
	('7', None, 'stroke-width:1;stroke:#aaa'),
	('8', None, 'stroke-width:1;stroke:#aaa'),
	('9', None, 'stroke-width:1;stroke:#aaa'),
	('A', 'all-class average', 'stroke-width:2.5;stroke:#009')
]

class TimeFmt(NumberFormatter):
	def format(py, x):
		t = int(x-1)
		return '%d:%dk' % (t/60, t%60)

def addColAvg(data, hdr):
	cols =list(map(lambda x:hdr%x, range(numClasses)))
	data.addCol(hdr % 'A', average(cols))

def combineDataAvg(data, cols):
	for cls in range(len(data)):
		if(cls==0):
			avgData = data[cls].copy()
			avgData.addCol('cls', val(cls))
		else:
			avgData.appendMarked(data[cls], 'cls', str(cls))
	folds = [Fold.avg(hdr) for hdr in cols]
	return avgData.groupBy('t', cols, folds)

def createChart(title, yaxis, ylabel, yfmt):
	chart = ScatterChart().setSize(chartWidth, chartHeight).setTitle(title)
	chart.setMargins(50, 20, 30, 80)
	# countLegend = len(classes)
	countLegend = reduce(lambda s, x: s + (0 if x[1]==None else 1), classes, 0)
	chart.legend.setCols(countLegend).posBottom(-40).setItemSize(80, 20)
	chart.clipChart = True
	chart.axisx.setRange(1, totalSteps, gridSteps) \
			.setAnchor(Anchor.bottom).setLabel('time (epochs:inputs)').setNumberFormatter(TimeFmt())
	if(type(yfmt) is str):
		yfmt = NumberFormatter.simple(yfmt)
	chart.axisy.setRange(yaxis[0], yaxis[1], yaxis[2], yaxis[3]) \
			.setAnchor(Anchor.left).setLabel(ylabel, Anchor.left.offset(-35)).setNumberFormatter(yfmt)
	return chart

def plotCombSC(page, data, charts):
	for (hdr, title, yaxis, ylabel, yfmt) in charts:
		chart = createChart(title, yaxis, ylabel, yfmt)
		for cls in range(numClasses+1):
			(_, legend, stroke) = classes[cls]
			pop = Population(data[cls], 't', hdr, 'fill:none;%s' % stroke)
			if(legend==None):
				chart.addPop(pop)
			else:
				chart.addPopLegend(legend, pop)
		page.add(chart)

def plotMC(page, data, charts):
	for (hdr, title, yaxis, ylabel, yfmt) in charts:
		chart = createChart(title, yaxis, ylabel, yfmt)
		for (cls, legend, stroke) in classes:
			pop = Population(data, 't', hdr % cls, 'fill:none;%s' % stroke)
			if(legend==None):
				chart.addPop(pop)
			else:
				chart.addPopLegend(legend, pop)
		page.add(chart)
