# -*- coding: iso-8859-1 -*-
#
# Copyright (C) 2009  Rene Liebscher
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free 
# Software Foundation; either version 3 of the License, or (at your option) any
# later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT 
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public License along with 
# this program; if not, see <http://www.gnu.org/licenses/>. 
#
""" 
    Base class for any kind of parametric fuzzy norm.
"""

__revision__ = "$Id: ParametricNorm.py,v 1.8 2009/10/07 21:08:14 rliebscher Exp $"

from fuzzy.norm.Norm import Norm
from fuzzy.utils import prop

class ParametricNorm(Norm):
    """Abstract base class for any parametric fuzzy norm
    
    @ivar p: parameter for norm
    @type p: float
    """
    _range = None

    def __init__(self,type,p):
        """Initialize type and parameter
        
        @param p: parameter for norm
        @type p: float
        """
        super(ParametricNorm,self).__init__(type)
        self.p = p

    @prop
    def p():
        """x
        @type: float"""
        def fget(self):
            return self._p
        def fset(self,value):
            self._checkParam(value)
            self._p = value
        return locals()

    @prop
    def p_range():
        """range(s) of valid values for p"""
        def fget(self):
            return self._range
        return locals()

    def _checkParam(self,value):
        """check parameter if allowed for paramter p
        @param value: the value to be checked 
        @type value: float"""
        from fuzzy.utils import checkRange
        if not checkRange(value,self._range):
            raise Exception("Parameter value %s is not allowed" % str(value))
