#ifndef SIM_PROC_H
#define SIM_PROC_H
#include <vector>

using namespace std;

typedef struct proc_params{
    unsigned long int rob_size;
    unsigned long int iq_size;
    unsigned long int width;
}proc_params;

// Put additional data structures here as per your requirement

//Global declarations:

int cycle_count, PC, width;
int pipeLine_empty;
int FE_empty, DE_empty, RN_empty, RR_empty, DI_empty, IS_empty, EX_empty, WB_empty, RT_empty;

//Structs

typedef struct RMT
{
	int valid;
	int tag;
}Remap;

typedef struct ROB
{
	int dst,pc;
	int ready;
}Rebuff;

typedef struct Pipeline_params
{
	int pc, type, dst, rs1, rs2, count, rs1_saved, rs2_saved, rs1_ready, rs2_ready, rs1_ROB, rs2_ROB;
	int fe_start, fe_cycles, de_start, de_cycles, rn_start, rn_cycles, rr_start, rr_cycles, di_start, di_cycles, is_start, is_cycles, ex_start, ex_cycles, wb_start, wb_cycles, rt_start, rt_cycles;

    // Using struct object as parameter for declaration of vectors
	bool operator < (const Pipeline_params &temp) const
	{
		return (pc < temp.pc);
	}
}pp;

//Classes

class Pipeline
{
public:
	int empty;
	vector <pp> reg;
};

class Issue_q 
{
public:
	int IQ_Size;
	vector <pp> iq;
};

class Reorder_buff
{
public:
	int head, tail;
	int  ROB_Size;

	vector <Rebuff> rob;
};

class Rename_map
{
public:
	int RMT_Size;
	Remap rmt[67];
};

class Exec_instr
{
public:
	vector <pp> exec_q;
};

//Declaring Objects:

Pipeline DE, RN, RR, DI, WB, RT;
Exec_instr Exec_q;
Reorder_buff ROB;
Issue_q IQ;
Rename_map RMT;

int Advance(FILE* fp);
void Retire();
void Writeback();
void Execute();
void Issue();
void Dispatch();
void Register_Read();
void Rename();
void Decode();
void Fetch(FILE* FP);

#endif
