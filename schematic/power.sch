EESchema Schematic File Version 2  date Sat Aug 18 07:43:26 2012
LIBS:power
LIBS:device
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:special
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:atmel
LIBS:contrib
LIBS:valves
LIBS:diode
LIBS:connect
LIBS:stepper_encoder-cache
EELAYER 43  0
EELAYER END
$Descr A4 11700 8267
Sheet 1 1
Title "noname.sch"
Date "18 aug 2012"
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
Text Label 3200 3400 1    60   ~ 0
12V cc stable
$Comp
L CONN_2 P?
U 1 1 502F45EA
P 3350 3100
F 0 "P?" V 3300 3100 40  0000 C CNN
F 1 "CONN_2" V 3400 3100 40  0000 C CNN
	1    3350 3100
	-1   0    0    1   
$EndComp
$Comp
L 1N4004 D?
U 1 1 502F425B
P 4300 3250
F 0 "D?" H 4400 3269 50  0000 L BNN
F 1 "1N4004" H 4400 3159 50  0000 L BNN
F 2 "diode-DO41-10" H 4300 3400 50  0001 C CNN
	1    4300 3250
	0    -1   -1   0   
$EndComp
$Comp
L LM317 U?
U 1 1 502F41FD
P 5100 3150
F 0 "U?" H 5100 3450 60  0000 C CNN
F 1 "LM317" H 5150 2900 60  0000 L CNN
	1    5100 3150
	1    0    0    -1  
$EndComp
$EndSCHEMATC
