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
L SN74AVC4T774:SN74AVC4T774RSVR U1
U 1 1 61B8BFB7
P 3600 2650
F 0 "U1" H 4150 2915 50  0000 C CNN
F 1 "SN74AVC4T774RSVR" H 4150 2824 50  0000 C CNN
F 2 "SN74AVC4T774:Texas_Instruments-RSV_R-PUQFN-N16-0-0-0" H 3600 3050 50  0001 L CNN
F 3 "http://www.ti.com/lit/ds/symlink/sn74avc4t774.pdf" H 3600 3150 50  0001 L CNN
F 4 "380Mbps" H 3600 3250 50  0001 L CNN "Data Rate"
F 5 "1.2V ~ 3.6V" H 3600 3350 50  0001 L CNN "Voltage - VCCA"
F 6 "1.2V ~ 3.6V" H 3600 3450 50  0001 L CNN "Voltage - VCCB"
F 7 "IC" H 3600 3550 50  0001 L CNN "category"
F 8 "Voltage Level Translator Bidirectional 1 Circuit 4 Channel 380Mbps 16-UQFN (2.6x1.8)" H 3600 3650 50  0001 L CNN "digikey description"
F 9 "296-24741-1-ND" H 3600 3750 50  0001 L CNN "digikey part number"
F 10 "yes" H 3600 3850 50  0001 L CNN "lead free"
F 11 "e3c229e32d4c5fe7" H 3600 3950 50  0001 L CNN "library id"
F 12 "Texas Instruments" H 3600 4050 50  0001 L CNN "manufacturer"
F 13 "595-SN74AVC4T774RSVR" H 3600 4150 50  0001 L CNN "mouser part number"
F 14 "16-UFQFN" H 3600 4250 50  0001 L CNN "package"
F 15 "yes" H 3600 4350 50  0001 L CNN "rohs"
F 16 "+85°C" H 3600 4450 50  0001 L CNN "temperature range high"
F 17 "-40°C" H 3600 4550 50  0001 L CNN "temperature range low"
	1    3600 2650
	1    0    0    -1  
$EndComp
Wire Wire Line
	3700 2650 3000 2650
Wire Wire Line
	3700 2750 3000 2750
Wire Wire Line
	3700 2950 3000 2950
Wire Wire Line
	3700 3050 3000 3050
Wire Wire Line
	3700 3150 3000 3150
Wire Wire Line
	3700 3250 3000 3250
Wire Wire Line
	3700 3450 3000 3450
Wire Wire Line
	3700 3550 3000 3550
Wire Wire Line
	3700 3650 3000 3650
Wire Wire Line
	3700 3750 3000 3750
Wire Wire Line
	5300 2650 4600 2650
Wire Wire Line
	5300 2750 4600 2750
Wire Wire Line
	5300 2850 4600 2850
Wire Wire Line
	5300 2950 4600 2950
Wire Wire Line
	3700 3950 3000 3950
Text Label 3000 2650 0    50   ~ 0
VCCA
Text Label 3000 2750 0    50   ~ 0
VCCB
Text Label 3000 2950 0    50   ~ 0
A1
Text Label 3000 3050 0    50   ~ 0
A2
Text Label 3000 3150 0    50   ~ 0
A3
Text Label 3000 3250 0    50   ~ 0
A4
Text Label 3000 3450 0    50   ~ 0
DIR1
Text Label 3000 3550 0    50   ~ 0
DIR2
Text Label 3000 3650 0    50   ~ 0
DIR3
Text Label 3000 3750 0    50   ~ 0
DIR4
Text Label 3000 3950 0    50   ~ 0
nOE
Text Label 5300 2650 2    50   ~ 0
B1
Text Label 5300 2750 2    50   ~ 0
B2
Text Label 5300 2850 2    50   ~ 0
B3
Text Label 5300 2950 2    50   ~ 0
B4
$Comp
L Connector:Conn_01x08_Male J1
U 1 1 61B8EE53
P 3150 5300
F 0 "J1" H 3258 5781 50  0000 C CNN
F 1 "Conn_01x08_Male" H 3258 5690 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x08_P2.54mm_Vertical" H 3150 5300 50  0001 C CNN
F 3 "~" H 3150 5300 50  0001 C CNN
	1    3150 5300
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x08_Male J2
U 1 1 61B90C78
P 4300 5300
F 0 "J2" H 4272 5274 50  0000 R CNN
F 1 "Conn_01x08_Male" H 4272 5183 50  0000 R CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x08_P2.54mm_Vertical" H 4300 5300 50  0001 C CNN
F 3 "~" H 4300 5300 50  0001 C CNN
	1    4300 5300
	-1   0    0    -1  
$EndComp
Wire Wire Line
	3350 5000 3600 5000
Wire Wire Line
	3350 5100 3600 5100
Wire Wire Line
	3350 5200 3600 5200
Wire Wire Line
	3350 5300 3600 5300
Wire Wire Line
	3350 5400 3600 5400
Wire Wire Line
	3350 5500 3600 5500
Wire Wire Line
	3350 5600 3600 5600
Wire Wire Line
	3350 5700 3600 5700
Wire Wire Line
	3850 5000 4100 5000
Wire Wire Line
	3850 5100 4100 5100
Wire Wire Line
	3850 5200 4100 5200
Wire Wire Line
	3850 5300 4100 5300
Wire Wire Line
	3850 5400 4100 5400
Wire Wire Line
	3850 5500 4100 5500
Wire Wire Line
	3850 5600 4100 5600
Wire Wire Line
	3850 5700 4100 5700
Text Label 3600 5000 2    50   ~ 0
VCCA
Text Label 3600 5100 2    50   ~ 0
A1
Text Label 3600 5200 2    50   ~ 0
A2
Text Label 3600 5300 2    50   ~ 0
A3
Text Label 3600 5400 2    50   ~ 0
A4
Text Label 3600 5500 2    50   ~ 0
DIR1
Text Label 3600 5600 2    50   ~ 0
DIR2
Text Label 3600 5700 2    50   ~ 0
nOE
Text Label 3850 5000 0    50   ~ 0
VCCB
Text Label 3850 5100 0    50   ~ 0
B1
Text Label 3850 5200 0    50   ~ 0
B2
Text Label 3850 5300 0    50   ~ 0
B3
Text Label 3850 5400 0    50   ~ 0
B4
Text Label 3850 5500 0    50   ~ 0
DIR3
Text Label 3850 5600 0    50   ~ 0
DIR4
$Comp
L Device:C_Small C1
U 1 1 61B9E2DF
P 1100 2350
F 0 "C1" H 1192 2396 50  0000 L CNN
F 1 "0.1uF" H 1192 2305 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.18x1.45mm_HandSolder" H 1100 2350 50  0001 C CNN
F 3 "~" H 1100 2350 50  0001 C CNN
	1    1100 2350
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C2
U 1 1 61B9E689
P 1650 2350
F 0 "C2" H 1742 2396 50  0000 L CNN
F 1 "0.1uF" H 1742 2305 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.18x1.45mm_HandSolder" H 1650 2350 50  0001 C CNN
F 3 "~" H 1650 2350 50  0001 C CNN
	1    1650 2350
	1    0    0    -1  
$EndComp
Text Label 850  2250 0    50   ~ 0
VCCA
Text Label 1400 2250 0    50   ~ 0
VCCB
Wire Wire Line
	1100 2250 850  2250
Wire Wire Line
	1650 2250 1400 2250
$Comp
L power:GND #PWR0101
U 1 1 61BA2A63
P 1100 2550
F 0 "#PWR0101" H 1100 2300 50  0001 C CNN
F 1 "GND" H 1105 2377 50  0000 C CNN
F 2 "" H 1100 2550 50  0001 C CNN
F 3 "" H 1100 2550 50  0001 C CNN
	1    1100 2550
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0102
U 1 1 61BA2CD6
P 1650 2550
F 0 "#PWR0102" H 1650 2300 50  0001 C CNN
F 1 "GND" H 1655 2377 50  0000 C CNN
F 2 "" H 1650 2550 50  0001 C CNN
F 3 "" H 1650 2550 50  0001 C CNN
	1    1650 2550
	1    0    0    -1  
$EndComp
Wire Wire Line
	1100 2450 1100 2550
Wire Wire Line
	1650 2450 1650 2550
$Comp
L power:GND #PWR0103
U 1 1 61BA58DF
P 4700 4050
F 0 "#PWR0103" H 4700 3800 50  0001 C CNN
F 1 "GND" H 4705 3877 50  0000 C CNN
F 2 "" H 4700 4050 50  0001 C CNN
F 3 "" H 4700 4050 50  0001 C CNN
	1    4700 4050
	1    0    0    -1  
$EndComp
Wire Wire Line
	4600 3950 4700 3950
Wire Wire Line
	4700 3950 4700 4050
$Comp
L power:GND #PWR0104
U 1 1 61BA6EDB
P 3850 5700
F 0 "#PWR0104" H 3850 5450 50  0001 C CNN
F 1 "GND" H 3855 5527 50  0000 C CNN
F 2 "" H 3850 5700 50  0001 C CNN
F 3 "" H 3850 5700 50  0001 C CNN
	1    3850 5700
	1    0    0    -1  
$EndComp
$EndSCHEMATC
