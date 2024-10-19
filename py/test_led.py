#!/usr/bin/env python3

import pyvisa
import time
import sys
import time

rm = pyvisa.ResourceManager()
reslist = rm.list_resources("USB?::5824::?*::INSTR")
print(reslist)

if (len(reslist) == 0):
    sys.exit()
	
inst = rm.open_resource(reslist[0]);
inst.timeout = 3000 

print(inst.query("*IDN?"))

inst.write("LED:EN 1")
inst.write("LED:BRIGHT 2")
inst.write("LED:SEC:NUM 1")
inst.write("LED:SEC:SIZE 8")
inst.write("LED:SEC:ORIENT 0")

time.sleep(0.5)
inst.write("LED:EN 0")

time.sleep(0.5)
inst.write("LED:EN 1")

for i in range(10):
    time.sleep(0.5)
    inst.write("LED:BRIGHT "+str(i))
    
for i in range(9,-1,-1):
    time.sleep(0.5)
    inst.write("LED:BRIGHT "+str(i))
    
for i in range(8,0,-1):
    time.sleep(0.5)
    inst.write("LED:SEC:SIZE "+str(i))

for j in range(1,8,1):
    time.sleep(0.5)
    inst.write("LED:SEC:SIZE "+str(j))

    for i in range(0,8,1):
        time.sleep(0.5)
        inst.write("LED:SEC:ORIENT "+str(i))
        
    time.sleep(0.5)
    inst.write("LED:SEC:ORIENT 0")

time.sleep(0.5)
inst.write("LED:SEC:SIZE 1")

time.sleep(0.5)
inst.write("LED:SEC:NUM 2")

for i in range(0,8,1):
    time.sleep(0.5)
    inst.write("LED:SEC:ORIENT "+str(i))
    
time.sleep(0.5)
inst.write("LED:SEC:NUM 4")

for i in range(0,8,1):
    time.sleep(0.5)
    inst.write("LED:SEC:ORIENT "+str(i))
    
#for i in range(1,9):
#    time.sleep(0.5)
#    inst.write("LED:SEC:SIZE "+str(i))

inst.close()
