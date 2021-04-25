---------------------------------------------------------------------------------
APEX Pipeline Simulator
---------------------------------------------------------------------------------
A simple implementation of 7 Stage APEX Pipeline

Swaroop Gowdra Shanthakumar


Notes:
----------------------------------------------------------------------------------
1) This code is a simple implementation of 7 Stage APEX Pipeline. 
	 
	 Fetch -> Decode -> Execute1 -> Execute2 -> Memory1 -> Memory2 -> Writeback

2) All the stages have latency of one cycle. There is a single functional unit in 
	 EX stage which perform all the arithmetic and logic operations.

File-Info
----------------------------------------------------------------------------------
1) Makefile 			- You can edit as needed
2) file_parser.c 	- Contains Functions to parse input file. No need to change this file
3) cpu.c          - Contains Implementation of APEX cpu. You can edit as needed
4) cpu.h          - Contains various data structures declarations needed by 'cpu.c'. You can edit as needed
	 

How to compile and run
----------------------------------------------------------------------------------
1) go to terminal, cd into project directory and type 'make' to compile project
2) Run using ./apex_sim <input file name>
