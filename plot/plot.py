from jdiag import *

from com.xrbpowered.jdiagram.data import NumberFormatter
from com.xrbpowered.jdiagram.data import ExpFormatter

from com.xrbpowered.jdiagram.chart import Page
from com.xrbpowered.jdiagram.chart import ScatterChart
from com.xrbpowered.jdiagram.chart.ScatterChart import Population
from com.xrbpowered.jdiagram.chart import Anchor

chartWidth = 1500
chartHeight = 250
totalSteps = 180
gridSteps = 10
numClasses = 10

classes = [
	('0', 'class 0', 'stroke-width:1;stroke:#ccc'),
	('1', 'class 1', 'stroke-width:1;stroke:#ccc'),
	('2', 'class 2', 'stroke-width:1;stroke:#ccc'),
	('3', 'class 3', 'stroke-width:1;stroke:#ccc'),
	('4', 'class 4', 'stroke-width:1;stroke:#ccc'),
	('5', 'class 5', 'stroke-width:1;stroke:#ccc'),
	('6', 'class 6', 'stroke-width:1;stroke:#ccc'),
	('7', 'class 7', 'stroke-width:1;stroke:#ccc'),
	('8', 'class 8', 'stroke-width:1;stroke:#ccc'),
	('9', 'class 9', 'stroke-width:1;stroke:#ccc'),
	('A', 'all-class average', 'stroke-width:2;stroke:#d50')
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
	chart.legend.setCols(len(classes)).posBottom(-40).setItemSize(80, 20)
	chart.clipChart = True
	chart.axisx.setRange(1, totalSteps, gridSteps) \
			.setAnchor(Anchor.bottom).setLabel('time (epochs:inputs)').setNumberFormatter(TimeFmt())
	chart.axisy.setRange(yaxis[0], yaxis[1], yaxis[2], yaxis[3]) \
			.setAnchor(Anchor.left).setLabel(ylabel, Anchor.left.offset(-35)).setNumberFormatter(NumberFormatter.simple(yfmt))
	return chart

def plotCombSC(page, data, charts):
	for (hdr, title, yaxis, ylabel, yfmt) in charts:
		chart = createChart(title, yaxis, ylabel, yfmt)
		for cls in range(numClasses+1):
			(_, legend, stroke) = classes[cls]
			chart.addPopLegend(legend, Population(data[cls], 't', hdr, 'fill:none;%s' % stroke))
		page.add(chart)

def plotMC(page, data, charts):
	for (hdr, title, yaxis, ylabel, yfmt) in charts:
		chart = createChart(title, yaxis, ylabel, yfmt)
		for (cls, legend, stroke) in classes:
			chart.addPopLegend(legend, Population(data, 't', hdr % cls, 'fill:none;%s' % stroke))
		page.add(chart)
