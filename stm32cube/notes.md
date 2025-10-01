2024-09-02

Checking DAC: bits to voltage

DACIN1 - [SetDAC(0, 65000);]  -> 2.955 V
DACIN1 - [SetDAC(0, 30000);]  -> 1.35 V
DACIN2 - [SetDAC(1, 25000);]  -> 1.125 V
DACIN3 - [SetDAC(2, 20000);]  -> 0.882 V
DACIN4 - [SetDAC(3, 10000);]  -> 0.425 V

Checking time factor
1st test (from Mehrdad program)
10ms in program ->  300ms

Test - transistors
val set in DAC table -> current measured by multimeter  (another board)

L1:
start: -0.1A
0.0V -> 0.1A
0.1V -> -0.11A (-0.42A)
1.0V -> -0.18A (-0.94A)
2.0V -> -0.38A
-1.0V -> 0.2A
-2.0V -> 0.4A (1.89A)

L2:
start: -0.38A
0.0V -> 0.38A
0.1V -> -0.41A (-0.4A)
1.0V -> -0.93A (-0.92A)
2.0V -> -1.85
-1.0V -> 0.93A
-2.0V -> 1.86A (1.85A)

L3:
start: -0.36A
0.0V -> -0.36A
0.1V -> -0.39A (-0.39A)
1.0V -> -0.9A (-0.89A)
2.0V -> -1.8A
-1.0V -> 0.9A 
-2.0V -> 1.8A (1.79A)

2024-09-03
cur1, cur2 and cur3 = 5mV if no current applied (just after starting uC and without load and high power supply)

L1: DAC_set=0V -> DACIN1_mes=0.48V
