**CNRF Protocol**


# Connect Request (Wireless > CNRF)
| 0 | 1 | 2 | 3 | 4 | 5  | 6 |  7  |
|---|---|---|---|---|----|---|-----|
|SYN|TY |   |   |SZ |    |   | CHK | 
|fc |5a | 4 | 3 | 2 | ca | 1 | d2  | 

# Connect Reply (CNRF > Wireless)
| 0 | 1 | 2 | 3 | 4 | 5  | 6 |
|---|---|---|---|---|----|---|
|SYN|TY |   |   | SZ|    |CHK| 
|fc |7a | 4 | 3 | 1 | 0  | 7e| 

# Request inital settings (Wireless > CNRF)
| 0 | 1 | 2 | 3 | 4 | 0  | 1 | 2 | 3 | 4 |5  |6  | 7 |8  | 9 | 10|11 |12 |13 |14 |15 |16 |
|---|---|---|---|---|----|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
|SYN|TYP|   |   | SZ|CMD |cf | cf|   |   |   |   |   |   |   |   |   |   |   |   |   |CHK| 
|fc |4c | 4 | 3 |10 | 32 | 3 | ff| 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |69 |
* cf : Configuration?

# Reply inital settings (CNRF > Wireless)
| 0 | 1 | 2 | 3 | 4 | 0  | 1 | 2 | 3 | 4 |5  |6  | 7 |8  | 9 | 10|11 |12 |13 |14 |15 |16 |
|---|---|---|---|---|----|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
|SYN|TYP|   |   | SZ|CMD |Pwr| O | Z1| OM|   |   | S |   |?  |   |?  |   | z2| ? |   |CHK| 
|fc |6c | 4 | 3 |10 | 0  | 1 | 2 | a3| 0 | 0 | 0 | 5 | 0 |21 | 0 |ac | 0 | a4| 2 | 0 |5f |
* Pwr - Power
  * 0 : Standby
  * 1 : On
* z1 : Zone 1 Setpoint
* z2 : Zone 2 Setpoint
* O Operation
  * 0 : Off
  * 1 : Hot Water On
  * 2 : Heating On
  * 3 : Cooling On
  * 5 : Frost Protect
  * 6 : Legionella
* OP - Operation Mode: 
  * 0 : Temperature Mode (heating)
  * 1 : Flow Control Mode (heating)
  * 2 : Compensation Curve Mode
  * 3 : Temperature Mode (cooling)
  * 4 : Flow Control Mode (cooling)
* S - Timer/prohibit setting
  * 0 : no restriction
  * 4 : timer mode heating
  * 5 : prohibit dhw | timer mode heating
  * 6 : prohibit heating

  
* assumes there are holiday mode and forced hot water status in here too

# Normal Request (Wireless > CNRF)
| 0 | 1 | 2 | 3 | 4 | 0  | 1 | 2 | 3 | 4 |5  |6  | 7 |8  | 9 | 10|11 |12 |13 |14 |15 |16 |
|---|---|---|---|---|----|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
|SYN|TYP|   |   | SZ|CMD | c1| c2| c3|c4 |c5 |c6 |c7 |c8 |cf |cf |   |   |   |   |   |CHK|
|fc |48 | 4 | 3 |10 | 27 | a8| a8|ff |ff |ff |ff |ff |ff | 3 |ff | 0 | 0 | 0 | 0 | 0 |2e |
* c1 - c8 : controller 1 current temperature (see hex > dec conversion)
* ff : Where controller not in use
* cf : Configuration?

# Normal Reply (CNRF > Wireless)
| 0 | 1 | 2 | 3 | 4 | 0  | 1 | 2 | 3 | 4 |5  |6  | 7 |8  | 9 | 10|11 |12 |13 |14 |15 |16 |
|---|---|---|---|---|----|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
|SYN|TYP|   |   | SZ|CMD |Pwr| O |z1 | OP| HW |   | ? |   |   |EC |   |z2 |Z2A? |   |   |CHK|
|fc |68 | 4 | 3 |10 | 0  | 1 | 2 | a3| 0 | 0 | 0 | 5 | 0 |21 | 0 |0  | a4| 2 | 0 | 0 | f |    (During normal op state)
|fc |68 | 4 | 3 |10 | 0  | 1 | 0 | a2| 0 | 0 | 0 | 5 | 0 |21 |90 |0  | a2| 2 | 0 | 0 | 84|    (During Error 1 State/J0 on FTC)
|fc |68 | 4 | 3 | 10 | 0 | 1 | 1 | a2| 0 | 1 | 0 | 5 | 0 |21 | 0 |0  | a2| 2 | 0 | 0 |12 |    (During Hot Water Boost)
* Pwr - Power
  * 0 : Standby
  * 1 : On
* z1 : Zone 1 Setpoint
* z2 : Zone 2 Setpoint
* O Operation
  * 0 : Off
  * 1 : Hot Water On
  * 2 : Heating On
  * 3 : Cooling On
  * 5 : Frost Protect
  * 6 : Legionella
* OP - Operation Mode: 
  * 0 : Temperature Mode (heating)
  * 1 : Flow Control Mode (heating)
  * 2 : Compensation Curve Mode
  * 3 : Temperature Mode (cooling)
  * 4 : Flow Control Mode (cooling)
* Z1/Z2A : Zone 1/2 Active Input Temperature?
* HW : Hot Water Boost Active
* EC : Error Code?

# Set Request Temperature (Wireless > CNRF)
| 0 | 1 | 2 | 3 | 4 | 0  | 1 | 2  | 3 | 4 |5  |6  | 7 |8  | 9 | 10|11 |12 |13 |14 |15 |16 |
|---|---|---|---|---|----|---|----|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
|SYN|TYP|   |   | SZ|CMD |c1?|c1/2| c3| c4| c5| c6| c7| c8| c8|   |cf |cf |   | ? |   |CHK|   
|fc |49 | 4 | 3 |10 | 28 | a4| a4 |a4 |ff |ff |ff |ff |ff | ff| 0 | 3 | ff| 0 | 1 | 0 |8f |     (Was this a messed up packet?)
* c1/c2 here seems strange/incorrect >> why the Offset & Why 3 values of a4 (temperature)
* cf : Configuration?

# Set Request Hot Water (Wireless > CNRF)
| 0 | 1 | 2 | 3 | 4 | 0  | 1 | 2  | 3 | 4 |5  |6  | 7 |8  | 9 | 10|11 |12 |13 |14 |15 |16 |
|---|---|---|---|---|----|---|----|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
|SYN|TYP|   |   | SZ|CMD |c1 |c2 | c3| c4| c5| c6| c7| c8| HW |   |cf |cf |   | ? |   |CHK|   
|fc |4a | 4 | 3 |10 | 29 | a3| a4 |ff |ff |ff |ff |ff|ff | 1  | 3 | ff | 0| 0 | 0 | 0 |31 |
* c1/c2 here seems strange/incorrect >> why the Offset & Why 3 values of a4 (temperature)
* HW : Hot Water?
* cf : Configuration?


# Reply Request Hot Water (CNRF > Wireless)
| 0 | 1 | 2 | 3 | 4 | 0  | 1 | 2 | 3 | 4 |5  |6  | 7 |8  | 9 | 10|11 |12 |13 |14 |15 |16 |
|---|---|---|---|---|----|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
|SYN|TYP|   |   | SZ|CMD |   |   |Z1 |   | HW |   | ? |   |?  |   |?  |   | z2| ? |   |CHK| 
|fc |6a | 4 | 3 |10 | 0  | 1 | 1 | a2| 0 | 1 | 0 | 5 | 0 |21 | 0 |0  | 0 | a2| 2 | 0 |10 |
* z1 : Zone 1 Setpoint
* z2 : Zone 2 Setpoint
* HW : Hot Water Boost
* assumes there are holiday mode and forced hot water status in here too

# Set Reply (CNRF > Wireless)
| 0 | 1 | 2 | 3 | 4 | 0  | 1 | 2 | 3 | 4 |5  |6  | 7 |8  | 9 | 10|11 |12 |13 |14 |15 |16 |
|---|---|---|---|---|----|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
|SYN|TYP|   |   | SZ|CMD |   |   |z1 |   |   |   | ? |   |   |   |   |z2 |   |   |   |CHK| 
|fc |69 | 4 | 3 |10 | 0  | 1 |2  |a3 |0  |0  |0  | 5 |0  | 21| 0 | 0 | a4| 2 | 0 | 0 | e |
* z1 : Zone 1 Setpoint
* z2 : Zone 2 Setpoint
* assumes there are holiday mode and forced hot water status here too


# Temperatures:
min = 0x94 (10degC)
max = 0xbc (30degC)
(value - 128.0f) / 2;