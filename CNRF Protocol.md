CNRF Protocol


# Connect Request (Wireless > CNRF)
| 0 | 1 | 2 | 3 | 4 | 5  | 6 |  7  |
|---|---|---|---|---|----|---|-----|
|SYN|TY |   |   |SZ |    |   | CHK | 
|fc |5a | 4 | 3 | 2 | ca | 1 | d2  | 

# Connect Reply (CNRF > Wrieless)
| 0 | 1 | 2 | 3 | 4 | 5  | 6 |
|---|---|---|---|---|----|---|
|SYN|TY |   |   | SZ|    |CHK| 
|fc |7a | 4 | 3 | 1 | 0  | 7e| 

# Request inital settings (Wireless > CNRF)
| 0 | 1 | 2 | 3 | 4 | 0  | 1 | 2 | 3 | 4 |5  |6  | 7 |8  | 9 | 10|11 |12 |13 |14 |15 |16 |
|---|---|---|---|---|----|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
|SYN|TYP|   |   | SZ|CMD |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |CHK| 
|fc |4c | 4 | 3 |10 | 32 | 3 | ff| 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |69 |

# Reply inital settings (CNRF > Wrieless)
| 0 | 1 | 2 | 3 | 4 | 0  | 1 | 2 | 3 | 4 |5  |6  | 7 |8  | 9 | 10|11 |12 |13 |14 |15 |16 |
|---|---|---|---|---|----|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
|SYN|TYP|   |   | SZ|CMD |   |   |Z1 |   |   |   | ? |   |   |   |   |   | z2|   |   |CHK| 
|fc |6c | 4 | 3 |10 | 0  | 1 | 2 | a3| 0 | 0 | 0 | 5 | 0 |21 | 0 |ac | 0 | a4| 2 | 0 |5f |
* z1 : Zone 1 Setpoint
* z2 : Zone 2 Setpoint
* assumes there are holiday mode and forced hot water status in here too

# Normal Request (Wireless > CNRF)
| 0 | 1 | 2 | 3 | 4 | 0  | 1 | 2 | 3 | 4 |5  |6  | 7 |8  | 9 | 10|11 |12 |13 |14 |15 |16 |
|---|---|---|---|---|----|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
|SYN|TYP|   |   | SZ|CMD | c1| c2| c3|c4 |c5 |c6 |c7 |c8 |   |   |   |   |   |   |   |CHK|
|fc |48 | 4 | 3 |10 | 27 | a8| a8|ff |ff |ff |ff |ff |ff | 3 |ff | 0 | 0 | 0 | 0 | 0 |2e |
* c1 - c8 : controller 1 current temperature (see hex > dec conversion)
* ff : Where controller not in use

# Normal Reply (CNRF > Wrieless)
| 0 | 1 | 2 | 3 | 4 | 0  | 1 | 2 | 3 | 4 |5  |6  | 7 |8  | 9 | 10|11 |12 |13 |14 |15 |16 |
|---|---|---|---|---|----|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
|SYN|TYP|   |   | SZ|CMD |   |   |z1 |   |   |   |   |   |   |   |   |z2 |   |   |   |CHK|
|fc |68 | 4 | 3 |10 | 0  | 1 | 2 | a3| 0 | 0 | 0 | 5 | 0 |21 | 0 |0  | a4| 2 | 0 | 0 | f |
* z1 : Zone 1 Setpoint
* z2 : Zone 2 Setpoint

# Set Request (Wireless > CNRF)
| 0 | 1 | 2 | 3 | 4 | 0  | 1 | 2  | 3 | 4 |5  |6  | 7 |8  | 9 | 10|11 |12 |13 |14 |15 |16 |
|---|---|---|---|---|----|---|----|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
|SYN|TYP|   |   | SZ|CMD |c1?|c1/2| c3| c4| c5| c6| c7| c8| c8|   |   |   |   |   |   |CHK|   >> why Offset & Why 3 values of a4
|fc |49 | 4 | 3 |10 | 28 | a4| a4 |a4 |ff |ff |ff |ff |ff | ff| 0 | 3 | ff| 0 | 1 | 0 |8f |

# Set Reply (CNRF > Wrieless)
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