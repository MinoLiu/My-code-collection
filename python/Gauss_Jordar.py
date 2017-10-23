#!/usr/bin/python3
# -*- coding: utf8 -*-
import re
from fractions import Fraction

stars = lambda n: "*" * n

class GaussJordar:
    def __init__(self):
        self.Linears = []
        self._vars = None
        self._times = None
        try:
            times = int(input("How many Linears? :"))
            for x in range(times):
                linear = input("linear {} :".format(x+1))
                if linear == "":
                    raise TypeError("請不要輸入空值 輸入數字OK?")
                linear = re.split(" +",linear)
                self.Linears.append([ Fraction(x) for x in linear if x != ''])
            self._vars = len(self.Linears[0])
            self._times = times
        except ValueError:
            raise "請輸入整數"
        #Check input correction
        for x in self.Linears:
            if len(x) != self._vars:
                raise ValueError(
                    "線性方程式變數輸入不正確"
                    )


    def Gauss_Jordar(self):
        self.set_to_one()
        for i,x in enumerate(self.Linears):
            for sub in range(self._times):
                if (sub == i) is False:
                    self.Linears[sub] = [ s - temp*(self.Linears[sub][i]) for temp,s in zip(x,self.Linears[sub]) ]
                self.set_to_one()

    def set_to_one(self):
        for i,x in enumerate(self.Linears):
            if (x[i] == 1) is False: 
                self.Linears[i] = [ num/x[i] if x[i] !=0 else x[i] for num in x ]
            

    def __str__(self):
        self.Gauss_Jordar()
        result = '1'
        for list_linear in self.Linears:
            if not (Fraction(1) in list_linear[:-1]):
                result = ('N' if list_linear[-1] == Fraction(0) else '0')
                break
        temp = []
        temp.append("\n".join((stars(5)+'{}'.format("ANS")+stars(5),result)))
        for i,list_linear in enumerate(self.Linears):
            temp.append("\n".join(((stars(5) + "x{}".format(i+1) + stars(5)),str(list_linear[-1]))))
        return "\n".join( x for x in temp)


if __name__ == "__main__":
    gauss = GaussJordar()
    print(gauss)
