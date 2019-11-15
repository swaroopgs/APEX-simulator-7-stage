/*
 *  cpu.c
 *  Contains APEX cpu pipeline implementation
 *
 *  Author :
 *  Gaurav Kothari (gkothar1@binghamton.edu)
 *  State University of New York, Binghamton
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"

/* Set this flag to 1 to enable debug messages */
#define ENABLE_DEBUG_MESSAGES 1

int bnzcounter = -1;
int zcounter = -1;
int zFlag = -1;
int isComplete = 0;

/*
 * This function creates and initializes APEX cpu.
 *
 * Note : You are free to edit this function according to your
 * 				implementation
 */
APEX_CPU *
APEX_cpu_init(const char *filename)
{
  if (!filename)
  {
    return NULL;
  }

  APEX_CPU *cpu = malloc(sizeof(*cpu));
  if (!cpu)
  {
    return NULL;
  }

  /* Initialize PC, Registers and all pipeline stages */
  cpu->pc = 4000;
  memset(cpu->regs, 0, sizeof(int) * 32);
  memset(cpu->regs_valid, 1, sizeof(int) * 32);
  memset(cpu->stage, 0, sizeof(CPU_Stage) * NUM_STAGES);
  memset(cpu->data_memory, 0, sizeof(int) * 4000);

  /* Parse input file and create code memory */
  cpu->code_memory = create_code_memory(filename, &cpu->code_memory_size);

  if (!cpu->code_memory)
  {
    free(cpu);
    return NULL;
  }

  if (ENABLE_DEBUG_MESSAGES)
  {
    fprintf(stderr,
            "APEX_CPU : Initialized APEX CPU, loaded %d instructions\n",
            cpu->code_memory_size);
    fprintf(stderr, "APEX_CPU : Printing Code Memory\n");
    printf("%-9s %-9s %-9s %-9s %-9s\n", "opcode", "rd", "rs1", "rs2", "imm");

    for (int i = 0; i < cpu->code_memory_size; ++i)
    {
      printf("%-9s %-9d %-9d %-9d %-9d\n",
             cpu->code_memory[i].opcode,
             cpu->code_memory[i].rd,
             cpu->code_memory[i].rs1,
             cpu->code_memory[i].rs2,
             cpu->code_memory[i].imm);
    }
  }

  /* Make all stages busy except Fetch stage, initally to start the pipeline */
  for (int i = 1; i < NUM_STAGES; ++i)
  {
    cpu->stage[i].busy = 1;
  }

  return cpu;
}

/*
 * This function de-allocates APEX cpu.
 *
 * Note : You are free to edit this function according to your
 * 				implementation
 */
void APEX_cpu_stop(APEX_CPU *cpu)
{
  free(cpu->code_memory);
  free(cpu);
}

/* Converts the PC(4000 series) into
 * array index for code memory
 *
 * Note : You are not supposed to edit this function
 *
 */
int get_code_index(int pc)
{
  return (pc - 4000) / 4;
}

static void
print_instruction(CPU_Stage *stage)
{
  // if (stage->stalled == 1 || stage->busy == 1)
  // {
  //   printf("%s ", "No Op");
  //   return;
  // }

  if (strcmp(stage->opcode, "STORE") == 0)
  {
    printf(
        "%s,R%d,R%d,#%d ", stage->opcode, stage->rs1, stage->rs2, stage->imm);
  }

  if (strcmp(stage->opcode, "STR") == 0)
  {
    printf(
        "%s,R%d,R%d,R%d ", stage->opcode, stage->rs1, stage->rs2, stage->rs3);
  }

  if (strcmp(stage->opcode, "ADD") == 0 || strcmp(stage->opcode, "SUB") == 0 || strcmp(stage->opcode, "MUL") == 0 || strcmp(stage->opcode, "AND") == 0 ||
      strcmp(stage->opcode, "OR") == 0 || strcmp(stage->opcode, "XOR") == 0)
  {
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }

  if (strcmp(stage->opcode, "ADDL") == 0 || strcmp(stage->opcode, "SUBL") == 0 || strcmp(stage->opcode, "LOAD") == 0)
  {
    printf("%s,R%d,R%d,#%d ", stage->opcode, stage->rd, stage->rs1, stage->imm);
  }

  if (strcmp(stage->opcode, "MOVC") == 0)
  {
    printf("%s,R%d,#%d ", stage->opcode, stage->rd, stage->imm);
  }

  if (strcmp(stage->opcode, "BZ") == 0 || strcmp(stage->opcode, "BNZ") == 0)
  {
    printf("%s,#%d", stage->opcode, stage->imm);
  }

  if (strcmp(stage->opcode, "JUMP") == 0)
  {
    printf("%s,R%d,#%d", stage->opcode, stage->rs1, stage->imm);
  }

  if (strcmp(stage->opcode, "HALT") == 0)
  {
    printf("%s", stage->opcode);
  }

  if (strcmp(stage->opcode, "") == 0)
  {
    printf("EMPTY");
  }
}

/* Debug function which dumps the cpu stage
 * content
 *
 * Note : You are not supposed to edit this function
 *
 */
static void
print_stage_content(char *name, CPU_Stage *stage)
{
  // if (stage->stalled == 1 || stage->busy == 1)
  // {
  //   printf("%-15s ", "EMPTY");
  // }
  // else
  // {
  printf("%-15s: pc(%d) ", name, stage->pc);
  // }

  print_instruction(stage);
  printf("\n");
}

/*
 *  Fetch Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int fetch(APEX_CPU *cpu)
{
  CPU_Stage *stage = &cpu->stage[F];

  if (stage->flush)
  {

    cpu->stage[DRF] = cpu->stage[F];

    if (ENABLE_DEBUG_MESSAGES)
    {
      print_stage_content("Fetch", stage);
    }
  }
  else if (!stage->busy && !stage->stalled)
  {
    /* Store current PC in fetch latch */
    stage->pc = cpu->pc;
    //printf("HERE\n");

    /* Index into code memory using this pc and copy all instruction fields into
     * fetch latch
     */
    APEX_Instruction *current_ins = &cpu->code_memory[get_code_index(cpu->pc)];

    //printf("%s\n", stage->opcode );
    strcpy(stage->opcode, current_ins->opcode);
    stage->rd = current_ins->rd;
    stage->rs1 = current_ins->rs1;
    stage->rs2 = current_ins->rs2;
    stage->imm = current_ins->imm;
    stage->rd = current_ins->rd;

    /* Update PC for next instruction */
    if (stage->pc < ((cpu->code_memory_size * 4) + 4000))
    {
      cpu->pc += 4;
    }

    /* Copy data from fetch latch to decode latch*/
    cpu->stage[DRF] = cpu->stage[F];

    if (ENABLE_DEBUG_MESSAGES)
    {
      print_stage_content("Fetch", stage);
    }
  }
  else
  {
    /* Store current PC in fetch latch */
    stage->pc = cpu->pc;

    /* Index into code memory using this pc and copy all instruction fields into
     * fetch latch
     */
    APEX_Instruction *current_ins = &cpu->code_memory[get_code_index(cpu->pc)];
    strcpy(stage->opcode, current_ins->opcode);
    stage->rd = current_ins->rd;
    stage->rs1 = current_ins->rs1;
    stage->rs2 = current_ins->rs2;
    stage->imm = current_ins->imm;
    stage->rd = current_ins->rd;

    if (ENABLE_DEBUG_MESSAGES)
    {
      print_stage_content("Fetch", stage);
    }
  }

  return 0;
}

/*
 *  Decode Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */

int decode(APEX_CPU *cpu)
{
  CPU_Stage *stage = &cpu->stage[DRF];

  if (!stage->busy && !stage->stalled)
  {

    /* Read data from register file for store */
    if (strcmp(stage->opcode, "STORE") == 0)
    {
      if (cpu->regs_valid[stage->rs1] == 16843009 && cpu->regs_valid[stage->rs2] == 16843009)
      {
        stage->rs1_value = cpu->regs[stage->rs1];
        stage->rs2_value = cpu->regs[stage->rs2];
        cpu->stage[F].stalled = 0;
        cpu->stage[DRF].stalled = 0;
      }
      else
      {
        cpu->stage[F].stalled = 1;
        cpu->stage[DRF].stalled = 1;
      }
    }

    if (strcmp(stage->opcode, "STR") == 0)
    {
      if (cpu->regs_valid[stage->rs1] == 16843009 && cpu->regs_valid[stage->rs2] == 16843009 && cpu->regs_valid[stage->rs3] == 16843009)
      {
        stage->rs1_value = cpu->regs[stage->rs1];
        stage->rs2_value = cpu->regs[stage->rs2];
        stage->rs3_value = cpu->regs[stage->rs3];
        cpu->stage[F].stalled = 0;
        cpu->stage[DRF].stalled = 0;
      }
      else
      {
        cpu->stage[F].stalled = 1;
        cpu->stage[DRF].stalled = 1;
      }
    }

    /* No Register file read needed for MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0)
    {
      stage->buffer = stage->imm;
      cpu->regs_valid[stage->rd] = 0;
    }

    else if (strcmp(stage->opcode, "ADD") == 0 || strcmp(stage->opcode, "SUB") == 0 || strcmp(stage->opcode, "MUL") == 0 || strcmp(stage->opcode, "AND") == 0 ||
             strcmp(stage->opcode, "OR") == 0 || strcmp(stage->opcode, "XOR") == 0 || strcmp(stage->opcode, "LOAD") == 0)
    {
      if (cpu->regs_valid[stage->rs1] == 16843009 && cpu->regs_valid[stage->rs2] == 16843009)
      {
        stage->rs1_value = cpu->regs[stage->rs1];
        stage->rs2_value = cpu->regs[stage->rs2];
        cpu->stage[F].stalled = 0;
        cpu->stage[DRF].stalled = 0;
        cpu->regs_valid[stage->rd] = 0;
      }
      else
      {
        cpu->stage[F].stalled = 1;
        cpu->stage[DRF].stalled = 1;
      }
    }

    else if (strcmp(stage->opcode, "BZ") == 0)
    {
      if (strcmp(cpu->stage[EX1].opcode, "ADD") == 0 || strcmp(cpu->stage[EX1].opcode, "SUB") == 0 || strcmp(cpu->stage[EX1].opcode, "MUL") == 0 || strcmp(cpu->stage[EX1].opcode, "ADDL") == 0 || strcmp(cpu->stage[EX1].opcode, "SUBL") == 0)
      {
        cpu->stage[F].stalled = 1;
        cpu->stage[DRF].stalled = 1;
        zcounter = 4;
      }
      else if (strcmp(cpu->stage[EX2].opcode, "ADD") == 0 || strcmp(cpu->stage[EX2].opcode, "SUB") == 0 || strcmp(cpu->stage[EX2].opcode, "MUL") == 0 || strcmp(cpu->stage[EX2].opcode, "ADDL") == 0 || strcmp(cpu->stage[EX2].opcode, "SUBL") == 0)
      {
        cpu->stage[F].stalled = 1;
        cpu->stage[DRF].stalled = 1;
        zcounter = 3;
      }
      else if (strcmp(cpu->stage[MEM1].opcode, "ADD") == 0 || strcmp(cpu->stage[MEM1].opcode, "SUB") == 0 || strcmp(cpu->stage[MEM1].opcode, "MUL") == 0 || strcmp(cpu->stage[MEM1].opcode, "ADDL") == 0 || strcmp(cpu->stage[MEM1].opcode, "SUBL") == 0)
      {
        cpu->stage[F].stalled = 1;
        cpu->stage[DRF].stalled = 1;
        zcounter = 2;
      }
      else if (strcmp(cpu->stage[MEM2].opcode, "ADD") == 0 || strcmp(cpu->stage[MEM2].opcode, "SUB") == 0 || strcmp(cpu->stage[MEM2].opcode, "MUL") == 0 || strcmp(cpu->stage[MEM2].opcode, "ADDL") == 0 || strcmp(cpu->stage[MEM2].opcode, "SUBL") == 0)
      {
        cpu->stage[F].stalled = 1;
        cpu->stage[DRF].stalled = 1;
        zcounter = 1;
      }
    }

    else if (strcmp(stage->opcode, "BNZ") == 0)
    {
      if (strcmp(cpu->stage[EX1].opcode, "ADD") == 0 || strcmp(cpu->stage[EX1].opcode, "SUB") == 0 || strcmp(cpu->stage[EX1].opcode, "MUL") == 0 || strcmp(cpu->stage[EX1].opcode, "ADDL") == 0 || strcmp(cpu->stage[EX1].opcode, "SUBL") == 0)
      {
        cpu->stage[F].stalled = 1;
        cpu->stage[DRF].stalled = 1;
        bnzcounter = 4;
      }
      else if (strcmp(cpu->stage[EX2].opcode, "ADD") == 0 || strcmp(cpu->stage[EX2].opcode, "SUB") == 0 || strcmp(cpu->stage[EX2].opcode, "MUL") == 0 || strcmp(cpu->stage[EX2].opcode, "ADDL") == 0 || strcmp(cpu->stage[EX2].opcode, "SUBL") == 0)
      {
        cpu->stage[F].stalled = 1;
        cpu->stage[DRF].stalled = 1;
        bnzcounter = 3;
      }
      else if (strcmp(cpu->stage[MEM1].opcode, "ADD") == 0 || strcmp(cpu->stage[MEM1].opcode, "SUB") == 0 || strcmp(cpu->stage[MEM1].opcode, "MUL") == 0 || strcmp(cpu->stage[MEM1].opcode, "ADDL") == 0 || strcmp(cpu->stage[MEM1].opcode, "SUBL") == 0)
      {
        cpu->stage[F].stalled = 1;
        cpu->stage[DRF].stalled = 1;
        bnzcounter = 2;
      }
      else if (strcmp(cpu->stage[MEM2].opcode, "ADD") == 0 || strcmp(cpu->stage[MEM2].opcode, "SUB") == 0 || strcmp(cpu->stage[MEM2].opcode, "MUL") == 0 || strcmp(cpu->stage[MEM2].opcode, "ADDL") == 0 || strcmp(cpu->stage[MEM2].opcode, "SUBL") == 0)
      {
        cpu->stage[F].stalled = 1;
        cpu->stage[DRF].stalled = 1;
        bnzcounter = 1;
      }
    }

    if (strcmp(stage->opcode, "JUMP") == 0)
    {
      if (cpu->regs_valid[stage->rs1] == 16843009)
      {
        stage->rs1_value = cpu->regs[stage->rs1];
        cpu->stage[F].stalled = 0;
        cpu->stage[DRF].stalled = 0;
      }
      else
      {
        cpu->stage[F].stalled = 1;
        cpu->stage[DRF].stalled = 1;
      }
    }

    if (strcmp(stage->opcode, "HALT") == 0)
    {
      CPU_Stage *fstage = &cpu->stage[F];
      memset(fstage, 0, sizeof(CPU_Stage));
      cpu->stage[F].flush = 1;
      cpu->stage[F].pc = 0;
    }

    /* Copy data from decode latch to execute latch*/
    cpu->stage[EX1] = cpu->stage[DRF];
  }

  if (ENABLE_DEBUG_MESSAGES)
  {
    print_stage_content("Decode/RF", stage);
  }

  return 0;
}

/*
 *  Execute Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int execute1(APEX_CPU *cpu)
{
  CPU_Stage *stage = &cpu->stage[EX1];

  if (!stage->busy && !stage->stalled)
  {

    /* Store */
    if (strcmp(stage->opcode, "STORE") == 0)
    {
    }

    else if (strcmp(stage->opcode, "LOAD") == 0)
    {
      stage->buffer = stage->rs1_value + stage->imm;
    }

    /* MOVC */
    else if (strcmp(stage->opcode, "MOVC") == 0)
    {
    }

    /* ADD */
    else if (strcmp(stage->opcode, "ADD") == 0)
    {
      stage->buffer = stage->rs1_value + stage->rs2_value;
    }

    else if (strcmp(stage->opcode, "ADDL") == 0)
    {
      stage->buffer = stage->rs1_value + stage->imm;
    }

    else if (strcmp(stage->opcode, "SUB") == 0)
    {
      stage->buffer = stage->rs1_value - stage->rs2_value;
    }

    else if (strcmp(stage->opcode, "SUBL") == 0)
    {
      stage->buffer = stage->rs1_value - stage->imm;
    }

    else if (strcmp(stage->opcode, "MUL") == 0)
    {
      stage->buffer = stage->rs1_value * stage->imm;
    }

    else if (strcmp(stage->opcode, "AND") == 0)
    {
      stage->buffer = stage->rs1_value & stage->imm;
    }

    else if (strcmp(stage->opcode, "OR") == 0)
    {
      stage->buffer = stage->rs1_value | stage->imm;
    }

    else if (strcmp(stage->opcode, "XOR") == 0)
    {
      stage->buffer = stage->rs1_value ^ stage->imm;
    }

    if (strcmp(stage->opcode, "JUMP") == 0)
    {
      stage->buffer = stage->rs1_value + stage->imm;
    }

    /* Copy data from Execute latch to Memory latch*/
    cpu->stage[EX2] = cpu->stage[EX1];

    if (ENABLE_DEBUG_MESSAGES)
    {
      print_stage_content("Execute1", stage);
    }
  }
  else
  {
    cpu->stage[EX2] = cpu->stage[EX1];
    if (ENABLE_DEBUG_MESSAGES)
    {
      printf("Execute1 : no Operation\n");
      // print_stage_content("Execute1", stage);
    }
  }

  return 0;
}

int execute2(APEX_CPU *cpu)
{
  CPU_Stage *stage = &cpu->stage[EX2];

  if (!stage->busy && !stage->stalled)
  {

    /* Store */
    if (strcmp(stage->opcode, "STORE") == 0)
    {
    }

    /* MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0)
    {
    }

    if (strcmp(stage->opcode, "BZ") == 0)
    {
      if (zFlag)
      {
        CPU_Stage *fstage = &cpu->stage[F];
        CPU_Stage *drfstage = &cpu->stage[DRF];
        CPU_Stage *ex1stage = &cpu->stage[EX1];
        memset(fstage, 0, sizeof(CPU_Stage));
        cpu->regs_valid[fstage->rd] = 16843009;
        memset(&cpu->stage[DRF], 0, sizeof(CPU_Stage));
        cpu->regs_valid[drfstage->rd] = 16843009;
        memset(&cpu->stage[EX1], 0, sizeof(CPU_Stage));
        cpu->regs_valid[ex1stage->rd] = 16843009;
        cpu->pc = stage->pc + stage->imm;
      }*+
    }

    if (strcmp(stage->opcode, "BNZ") == 0)
    {
      if (!zFlag)
      {
        CPU_Stage *fstage = &cpu->stage[F];
        CPU_Stage *drfstage = &cpu->stage[DRF];
        CPU_Stage *ex1stage = &cpu->stage[EX1];
        cpu->regs_valid[fstage->rd] = 16843009;
        memset(fstage, 0, sizeof(CPU_Stage));
        cpu->regs_valid[drfstage->rd] = 16843009;
        memset(drfstage, 0, sizeof(CPU_Stage));
        cpu->regs_valid[ex1stage->rd] = 16843009;
        printf("%d %d\n", cpu->regs_valid[drfstage->rd], drfstage->rd);
        memset(ex1stage, 0, sizeof(CPU_Stage));
        cpu->pc = stage->pc + stage->imm;
      }
    }

    if (strcmp(stage->opcode, "JUMP") == 0)
    {
      if ((stage->buffer < (cpu->code_memory_size * 4)) - 4 && stage->buffer > 4000)
      {
        CPU_Stage *fstage = &cpu->stage[F];
        CPU_Stage *drfstage = &cpu->stage[DRF];
        CPU_Stage *ex1stage = &cpu->stage[EX1];
        memset(fstage, 0, sizeof(CPU_Stage));
        cpu->regs_valid[drfstage->rd] = 16843009;
        memset(drfstage, 0, sizeof(CPU_Stage));
        cpu->regs_valid[ex1stage->rd] = 16843009;
        memset(ex1stage, 0, sizeof(CPU_Stage));
        cpu->pc = stage->buffer;
      }
      else
      {
        isComplete = -1;
      }
    }

    if (strcmp(stage->opcode, "HALT") == 0)
    {
    }

    /* Copy data from Execute latch to Memory latch*/
    cpu->stage[MEM1] = cpu->stage[EX2];

    if (ENABLE_DEBUG_MESSAGES)
    {
      print_stage_content("Execute2", stage);
    }
  }
  else
  {
    cpu->stage[MEM1] = cpu->stage[EX2];
    if (ENABLE_DEBUG_MESSAGES)
    {
      printf("Execute2 : No operation\n");
      // print_stage_content("Execute2", stage);
    }
  }

  return 0;
}

/*
 *  Memory Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int memory1(APEX_CPU *cpu)
{
  CPU_Stage *stage = &cpu->stage[MEM1];

  if (!stage->busy && !stage->stalled)
  {

    /* Store */
    if (strcmp(stage->opcode, "STORE") == 0)
    {
    }

    /* MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0)
    {
    }

    /* Copy data from decode latch to execute latch*/
    cpu->stage[MEM2] = cpu->stage[MEM1];

    if (ENABLE_DEBUG_MESSAGES)
    {
      print_stage_content("Memory1", stage);
    }
  }
  else
  {
    cpu->stage[MEM2] = cpu->stage[MEM1];
    if (ENABLE_DEBUG_MESSAGES)
    {
      printf("Memory1 : No operation\n");
      // print_stage_content("Memory1", stage);
    }
  }

  return 0;
}

int memory2(APEX_CPU *cpu)
{
  CPU_Stage *stage = &cpu->stage[MEM2];

  if (!stage->busy && !stage->stalled)
  {

    /* Store */
    if (strcmp(stage->opcode, "STORE") == 0)
    {
      int mem_address = stage->rs2_value + stage->imm;
      cpu->data_memory[mem_address] = stage->rs1_value;
    }

    /* Str */
    if (strcmp(stage->opcode, "STR") == 0)
    {
      int mem_address = stage->rs2_value + stage->rs3_value;
      cpu->data_memory[mem_address] = stage->rs1_value;
    }

    /* MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0)
    {
    }

    /* Copy data from decode latch to execute latch*/
    cpu->stage[WB] = cpu->stage[MEM2];
    if (ENABLE_DEBUG_MESSAGES)
    {
      print_stage_content("Memory2", stage);
    }
  }
  else
  {
    cpu->stage[WB] = cpu->stage[MEM2];
    if (ENABLE_DEBUG_MESSAGES)
    {
      printf("Memory2 : No operation\n");
      // print_stage_content("Memory2", stage);
    }
  }

  return 0;
}

/*
 *  Writeback Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int writeback(APEX_CPU *cpu)
{
  CPU_Stage *stage = &cpu->stage[WB];

  if (strcmp(stage->opcode, "BZ") == 0)
  {
    zcounter--;
    if (zcounter == 0)
    {
      cpu->stage[DRF].stalled = 0;
      cpu->stage[F].stalled = 0;
    }
  }

  if (strcmp(stage->opcode, "BNZ") == 0)
  {
    bnzcounter--;
    if (bnzcounter == 0)
    {
      cpu->stage[DRF].stalled = 0;
      cpu->stage[F].stalled = 0;
    }
  }

  if (!stage->busy && !stage->stalled)
  {

    if (strcmp(stage->opcode, "STORE") == 0)
    {
      cpu->stage[DRF].stalled = 0;
      cpu->stage[F].stalled = 0;
    }

    /* Update register file */
    else if (strcmp(stage->opcode, "MOVC") == 0)
    {
      cpu->regs[stage->rd] = stage->buffer;
      cpu->stage[DRF].stalled = 0;
      cpu->stage[F].stalled = 0;
      cpu->regs_valid[stage->rd] = 16843009;
    }
    else if (strcmp(stage->opcode, "ADD") == 0 || strcmp(stage->opcode, "SUB") == 0 || strcmp(stage->opcode, "MUL") == 0)
    {
      //remove
      cpu->regs[stage->rd] = stage->buffer;
      cpu->stage[DRF].stalled = 0;
      cpu->stage[F].stalled = 0;
      cpu->regs_valid[stage->rd] = 16843009;
      if (stage->buffer == 0)
      {
        zFlag = 1;
      }
      else
      {
        zFlag = 0;
      }
    }
    else if (strcmp(stage->opcode, "AND") == 0 || strcmp(stage->opcode, "OR") == 0 || strcmp(stage->opcode, "XOR") == 0 || strcmp(stage->opcode, "LOAD") == 0)
    {
      cpu->regs[stage->rd] = stage->buffer;
      cpu->stage[DRF].stalled = 0;
      cpu->stage[F].stalled = 0;
      cpu->regs_valid[stage->rd] = 16843009;
    }

    cpu->ins_completed++;

    if (stage->pc == ((cpu->code_memory_size * 4) + 4000) - 4 || strcmp(stage->opcode, "HALT") == 0 || strcmp(stage->opcode, "HALT\n") == 0)
    {
      isComplete = 1;
    }

    if (ENABLE_DEBUG_MESSAGES)
    {
      print_stage_content("Writeback", stage);
    }
  }
  else
  {
    if (ENABLE_DEBUG_MESSAGES)
    {
      printf("Writeback : No operation\n");
      // print_stage_content("Writeback", stage);
    }
  }

  return 0;
}

/*
 *  APEX CPU simulation loop
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int APEX_cpu_run(APEX_CPU *cpu)
{
  while (1)
  {

    /* All the instructions committed, so exit */
    if (isComplete)
    {
      printf("(apex) >> Simulation Complete");
      break;
    }

    if (isComplete == -1)
    {
      printf("(apex) >> Invalid Jump");
      break;
    }

    if (ENABLE_DEBUG_MESSAGES)
    {
      printf("--------------------------------\n");
      printf("Clock Cycle #: %d\n", cpu->clock);
      printf("--------------------------------\n");
    }

    writeback(cpu);
    memory2(cpu);
    memory1(cpu);
    execute2(cpu);
    execute1(cpu);
    decode(cpu);
    fetch(cpu);
    cpu->clock++;
  }

  return 0;
}
