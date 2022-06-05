import sys 
sys.path.append('JDiagram.jar')
from java.util.function import Function
from java.io import File
from java.lang import System
from java.lang import Math

from com.xrbpowered.jdiagram.data import Data
from com.xrbpowered.jdiagram.data import Formula
from com.xrbpowered.jdiagram.data import Fold
from com.xrbpowered.jdiagram.data import Filter
from com.xrbpowered.jdiagram.data.Formula import *

def fn(func):
	class jf(Function):
		def __init__(self, func):
			self.apply = func
	return Formula.func(jf(func))
