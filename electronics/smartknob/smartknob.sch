EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Sensor_Magnetic:TLV493D U?
U 1 1 616E0E0C
P 5900 2450
F 0 "U?" H 6180 2546 50  0000 L CNN
F 1 "TLV493D" H 6180 2455 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23-6" H 5850 1950 50  0001 C CNN
F 3 "http://www.infineon.com/dgdl/Infineon-TLV493D-A1B6-DS-v01_00-EN.pdf?fileId=5546d462525dbac40152a6b85c760e80" H 5750 2950 50  0001 C CNN
	1    5900 2450
	1    0    0    -1  
$EndComp
$Comp
L lilygo_micro32:T-Micro32_Plus U?
U 1 1 616E21EC
P 2350 2350
F 0 "U?" H 2350 3315 50  0000 C CNN
F 1 "T-Micro32_Plus" H 2350 3224 50  0000 C CNN
F 2 "" H 2350 2350 50  0001 C CNN
F 3 "" H 2350 2350 50  0001 C CNN
	1    2350 2350
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C?
U 1 1 616E37C4
P 6300 2100
F 0 "C?" H 6392 2146 50  0000 L CNN
F 1 "C_Small" H 6392 2055 50  0000 L CNN
F 2 "" H 6300 2100 50  0001 C CNN
F 3 "~" H 6300 2100 50  0001 C CNN
	1    6300 2100
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR?
U 1 1 616E3FA7
P 1000 1050
F 0 "#PWR?" H 1000 900 50  0001 C CNN
F 1 "+3.3V" H 1015 1223 50  0000 C CNN
F 2 "" H 1000 1050 50  0001 C CNN
F 3 "" H 1000 1050 50  0001 C CNN
	1    1000 1050
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR?
U 1 1 616E42FB
P 5550 1650
F 0 "#PWR?" H 5550 1500 50  0001 C CNN
F 1 "+3.3V" H 5565 1823 50  0000 C CNN
F 2 "" H 5550 1650 50  0001 C CNN
F 3 "" H 5550 1650 50  0001 C CNN
	1    5550 1650
	1    0    0    -1  
$EndComp
$EndSCHEMATC
