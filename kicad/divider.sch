EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 13 17
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
Text HLabel 1500 2700 2    50   Input ~ 0
DIV+
Text HLabel 1500 3000 2    50   Input ~ 0
DIV-
Text Label 1100 2700 0    50   ~ 0
DIV+
Text Label 1100 3000 0    50   ~ 0
DIV-
Wire Wire Line
	1000 2700 1500 2700
Wire Wire Line
	1000 3000 1500 3000
Wire Wire Line
	1000 3300 1500 3300
Text HLabel 1500 3300 2    50   Input ~ 0
F_REF
Text GLabel 850  1400 2    50   Output ~ 0
DIV1+
Text GLabel 850  1700 2    50   Output ~ 0
DIV1-
Text GLabel 1350 1400 2    50   Output ~ 0
DIV2+
Text GLabel 1350 1700 2    50   Output ~ 0
DIV2-
$Comp
L ff:HMC984LP4E U?
U 1 1 63B7F7FD
P 4500 2300
F 0 "U?" H 4500 3296 60  0000 C CNN
F 1 "HMC984LP4E" H 4500 3186 60  0000 C CNN
F 2 "Housings_DFN_QFN:QFN-24_4x4mm_Pitch0.5mm_NoMask" H 4500 3240 60  0001 C CNN
F 3 "https://www.analog.com/media/en/technical-documentation/data-sheets/hmc984.pdf" H 4500 3186 60  0001 C CNN
	1    4500 2300
	1    0    0    -1  
$EndComp
$EndSCHEMATC
