/*
 *  main.c
 *
 *  Author :
 *  Gaurav Kothari (gkothar1@binghamton.edu)
 *  State University of New York, Binghamton
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"

int
main(int argc, char const* argv[])
{

  int isSimulate = 0;
  int cycles = 0;

  if (argc != 4) {
    fprintf(stderr, "APEX_Help : Usage %s <input_file>\n", argv[0]);
    exit(1);
  }

  if(strcmp(argv[2], "simulate") == 0) {
    isSimulate = 1;
  } else {
    isSimulate = 0;
  }

  cycles = atoi(argv[3]);

  APEX_CPU* cpu = APEX_cpu_init(argv[1], isSimulate, cycles);
  if (!cpu) {
    fprintf(stderr, "APEX_Error : Unable to initialize CPU\n");
    exit(1);
  }

  APEX_cpu_run(cpu);
  APEX_cpu_stop(cpu);
  return 0;
}