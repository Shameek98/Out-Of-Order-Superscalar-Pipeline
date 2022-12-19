#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sim_proc.h"
#include <iostream>
#include <math.h>
#include <iomanip>
#include <vector>
#include <algorithm>

using namespace std;

/*  argc holds the number of command line arguments
    argv[] holds the commands themselves

    Example:-
    sim 256 32 4 gcc_trace.txt
    argc = 5
    argv[0] = "sim"
    argv[1] = "256"
    argv[2] = "32"
    ... and so on
*/

int main (int argc, char* argv[])
{
    FILE *FP;               // File handler
    char *trace_file;       // Variable that holds trace file name;
    proc_params params;       // look at sim_bp.h header file for the the definition of struct proc_params
    int op_type, dest, src1, src2;  // Variables are read from trace file
    unsigned long int pc; // Variable holds the pc read from input file
    
    if (argc != 5)
    {
        printf("Error: Wrong number of inputs:%d\n", argc-1);
        exit(EXIT_FAILURE);
    }
    
    params.rob_size     = strtoul(argv[1], NULL, 10);
    params.iq_size      = strtoul(argv[2], NULL, 10);
    params.width        = strtoul(argv[3], NULL, 10);
    trace_file          = argv[4];

    FP = fopen(trace_file, "r");
    if(FP == NULL)
    {
        // Throw error and exit if fopen() failed
        printf("Error: Unable to open file %s\n", trace_file);
        exit(EXIT_FAILURE);
    }
    //Initialization:

    cycle_count = 0;
	PC = 0;
	pipeLine_empty = 0;
    width = params.width;

    ROB.ROB_Size = params.rob_size;
	IQ.IQ_Size = params.iq_size;
    RMT.RMT_Size = 67;

    DE_empty = RN_empty = RR_empty = DI_empty = WB_empty = RT_empty = 1;
    ROB.head = ROB.tail = 3;

    for(int i = 0; i < 67; i++)
	{
		RMT.rmt[i].valid = 0;
		RMT.rmt[i].tag = -1;
	}

    Rebuff rb;
    rb.dst = 0;
    rb.pc = 0;
    rb.ready = 0;

    for(int i = 0; i < params.rob_size; i++){
		ROB.rob.push_back(rb);
    }
    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // The following loop just tests reading the trace and echoing it back to the screen.
    //
    // Replace this loop with the "do { } while (Advance_Cycle());" loop indicated in the Project 3 spec.
    // Note: fscanf() calls -- to obtain a fetch bundle worth of instructions from the trace -- should be
    // inside the Fetch() function.
    //
    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    while(Advance(FP))
	{
		Retire();
		Writeback();
		Execute();
		Issue();
		Dispatch();
		Register_Read();
		Rename();
		Decode();
		Fetch(FP);
		
		if(DE_empty == 1 && RN_empty == 1 && RR_empty == 1 && DI_empty == 1 && IQ.iq.size() == 0 && Exec_q.exec_q.size() == 0 && WB.reg.size() == 0 && ROB.tail == ROB.head && ROB.rob[ROB.tail].pc == 0){
			pipeLine_empty = 1;
        }
		cycle_count++;
	}
    
    fclose(FP);
    //Calculating IPC:

    float IPC = (float)PC/(float)cycle_count;

    //Printing output:

    printf("# === Simulator Command =========\n"); 
    printf("# ./sim %d %d %d %s\n", params.rob_size, params.iq_size, params.width, trace_file);
    printf("# === Processor Configuration ===\n");
    printf("# ROB_SIZE = %d\n", params.rob_size);
    printf("# IQ_SIZE  = %d\n", params.iq_size);
    printf("# width    = %d\n", params.width);
    printf("# === Simulation Results ========\n");
    printf("# Dynamic Instruction Count    = %d\n", PC);
    printf("# Cycles                       = %d\n", cycle_count);
    printf("# Instructions Per Cycle (IPC)  = %0.2f\n", IPC);

    return 0;
}

//Functions:

int Advance(FILE* fp)
{
	if(feof(fp) && pipeLine_empty == 1){
		return 0;
    }   
	else{   
		return 1;
    }
}

void Retire()
{
	for(int i = 0; i < width; i++)
	{
		if (ROB.head != ROB.ROB_Size - 1 && ROB.head == ROB.tail && ROB.rob[ROB.head + 1].pc == 0)
		{
			return;
		}
		if (ROB.rob[ROB.head].ready == 1 )
		{
			for (unsigned int j = 0; j < RR.reg.size(); j++)
			{
				if (RR.reg[j].rs1 == ROB.head)
					RR.reg[j].rs1_ready = 1;

				if (RR.reg[j].rs2 == ROB.head)
					RR.reg[j].rs2_ready = 1;
			}
			for (int i = 0; i < RT.reg.size(); i++)
			{
				if (RT.reg[i].pc == ROB.rob[ROB.head].pc)
				{
					RT.reg[i].rt_cycles = (cycle_count + 1) - RT.reg[i].rt_start;
                    printf("%d fu{%d} src{%d,%d} dst{%d} FE{%d,%d} DE{%d,%d} RN{%d,%d} RR{%d,%d} DI{%d,%d} IS{%d,%d} EX{%d,%d} WB{%d,%d} RT{%d,%d}\n", 
                    RT.reg[i].pc, RT.reg[i].type, RT.reg[i].rs1_saved, RT.reg[i].rs2_saved, ROB.rob[ROB.head].dst, RT.reg[i].fe_start, RT.reg[i].fe_cycles,
                    RT.reg[i].de_start, RT.reg[i].de_cycles, RT.reg[i].rn_start, RT.reg[i].rn_cycles, 
                    RT.reg[i].rr_start, RT.reg[i].rr_cycles, RT.reg[i].di_start, RT.reg[i].di_cycles,
                    RT.reg[i].is_start, RT.reg[i].is_cycles, RT.reg[i].ex_start, RT.reg[i].ex_cycles,
                    RT.reg[i].wb_start, RT.reg[i].wb_cycles, RT.reg[i].rt_start, RT.reg[i].rt_cycles);
					break;
				}

			}
			for (int i = 0; i < RMT.RMT_Size; i++)
			{
				if (RMT.rmt[i].tag == ROB.head)
				{
					RMT.rmt[i].tag = 0;
					RMT.rmt[i].valid = 0;
				}
			}
			ROB.rob[ROB.head].dst = 0;
			ROB.rob[ROB.head].pc = 0;
			ROB.rob[ROB.head].ready = 0;

			if (ROB.head != (ROB.ROB_Size - 1))
				ROB.head++;
			else
				ROB.head = 0;
		}
		else
			break;
	}
}

void Writeback()
{
	if (WB.reg.size() != 0)
	{
		for (int i = 0; i < WB.reg.size(); i++)
		{
			ROB.rob[WB.reg[i].dst].ready = 1;
			WB.reg[i].rt_start = cycle_count + 1;
			WB.reg[i].wb_cycles = WB.reg[i].rt_start - WB.reg[i].wb_start; 
			RT.reg.push_back(WB.reg[i]);
		}
		WB.reg.clear();
	}
}

void Execute()
{
	if (Exec_q.exec_q.size() != 0)
	{
		for (int i = 0; i < Exec_q.exec_q.size(); i++){
			Exec_q.exec_q[i].count--;
		}
		int flag = 1;
		
		while (flag != 0)
		{
			flag = 0;

			for (int i = 0; i < Exec_q.exec_q.size(); i++)
			{
				if (Exec_q.exec_q[i].count == 0)
				{
					Exec_q.exec_q[i].wb_start = cycle_count + 1;
					Exec_q.exec_q[i].ex_cycles = Exec_q.exec_q[i].wb_start - Exec_q.exec_q[i].ex_start;

					WB.reg.push_back(Exec_q.exec_q[i]);

					for (int j = 0; j < IQ.iq.size(); j++)										//Wake up dependent instructions in the IQ
					{
						if (IQ.iq[j].rs1 == Exec_q.exec_q[i].dst)
							IQ.iq[j].rs1_ready = 1;

						if (IQ.iq[j].rs2 == Exec_q.exec_q[i].dst)
							IQ.iq[j].rs2_ready = 1;
					}

					for (int k = 0; k < DI.reg.size(); k++)									//Wake up dependent instructions in the DI
					{
						if (DI.reg[k].rs1 == Exec_q.exec_q[i].dst)
							DI.reg[k].rs1_ready = 1;

						if (DI.reg[k].rs2 == Exec_q.exec_q[i].dst)
							DI.reg[k].rs2_ready = 1;
					}

					for (int l = 0; l < RR.reg.size(); l++)									//Wake up dependent instructions in the RR
					{
						if (RR.reg[l].rs1 == Exec_q.exec_q[i].dst)
							RR.reg[l].rs1_ready = 1;

						if (RR.reg[l].rs2 == Exec_q.exec_q[i].dst)
							RR.reg[l].rs2_ready = 1;
					}
					Exec_q.exec_q.erase(Exec_q.exec_q.begin() + i);
					flag = 1;
					break;
				}
			}
		}	
	}
}

void Issue()
{
	if (IQ.iq.size() != 0)
	{
		sort(IQ.iq.begin(), IQ.iq.end());

		int i = 0;

		int flag = 1;
		while (flag != 0 && i < width)
		{
			flag = 0;
			for (int j = 0; j < IQ.iq.size(); j++)
			{
				if (IQ.iq[j].rs1_ready && IQ.iq[j].rs2_ready)
				{
					IQ.iq[j].ex_start = cycle_count + 1;
					IQ.iq[j].is_cycles = IQ.iq[j].ex_start - IQ.iq[j].is_start;
					Exec_q.exec_q.push_back(IQ.iq[j]);
					IQ.iq.erase(IQ.iq.begin() + j);
					i++;
					flag = 1;
					break;
				}
			}
		}
	}
}

void Dispatch()
{
	int iq_free = IQ.IQ_Size - IQ.iq.size();
	if (DI_empty == 0 && iq_free >= DI.reg.size())
	{
			for (int i = 0; i < DI.reg.size(); i++)
			{
				DI.reg[i].is_start = cycle_count + 1;
				DI.reg[i].di_cycles = DI.reg[i].is_start - DI.reg[i].di_start;
				IQ.iq.push_back(DI.reg[i]);
			}
			DI.reg.clear();
			DI_empty = 1;
	}
}

void Register_Read()
{
	if (RR_empty == 0 && DI_empty == 1)
	{
		for (int i = 0; i < RR.reg.size(); i++)
		{
			if (RR.reg[i].rs1_ROB == 1)
			{
				if (ROB.rob[RR.reg[i].rs1].ready == 1)
					RR.reg[i].rs1_ready = 1;
			}
			else
				RR.reg[i].rs1_ready = 1;

			if (RR.reg[i].rs2_ROB == 1)
			{
				if (ROB.rob[RR.reg[i].rs2].ready == 1)
					RR.reg[i].rs2_ready = 1;
			}
			else
				RR.reg[i].rs2_ready = 1;

			RR.reg[i].di_start = cycle_count + 1;
			RR.reg[i].rr_cycles = RR.reg[i].di_start - RR.reg[i].rr_start;
		}
		DI.reg = RR.reg; 
		RR.reg.clear();
		RR_empty = 1;
		DI_empty = 0;
	}
}


void Rename()
{
	if (RN_empty == 0 && RR_empty == 1)
	{
		//Calculating free space in the ROB//
		int rob_free;
		if (ROB.tail < ROB.head)
			rob_free = ROB.head - ROB.tail;
		else if (ROB.head < ROB.tail)
			rob_free = ROB.ROB_Size - (ROB.tail - ROB.head);
		else
		{
			if (ROB.tail < (ROB.ROB_Size - 1))
			{
				if (ROB.rob[ROB.tail + 1].dst == 0 && ROB.rob[ROB.tail + 1].pc == 0 && ROB.rob[ROB.tail + 1].ready == 0)
					rob_free = ROB.ROB_Size;
				else
					rob_free = 0;
			}
			else
			{
				if (ROB.rob[ROB.tail - 1].dst == 0 && ROB.rob[ROB.tail - 1].pc == 0 && ROB.rob[ROB.tail - 1].ready == 0)
					rob_free = ROB.ROB_Size;
				else
					rob_free = 0;
			}
		}

		//Processing the bundle//
		if (rob_free <RN.reg.size())
			return;
		else
		{
			for (int i = 0; i < RN.reg.size(); i++)
			{
				if (RN.reg[i].rs1 != -1 && RMT.rmt[RN.reg[i].rs1].valid == 1)
					{
						RN.reg[i].rs1_ROB = 1;	
						RN.reg[i].rs1 = RMT.rmt[RN.reg[i].rs1].tag;															
					}

				if (RN.reg[i].rs2 != -1 && RMT.rmt[RN.reg[i].rs2].valid == 1)
					{
						RN.reg[i].rs2_ROB = 1;	
						RN.reg[i].rs2 = RMT.rmt[RN.reg[i].rs2].tag;															
					}

				ROB.rob[ROB.tail].ready = 0;
				ROB.rob[ROB.tail].dst = RN.reg[i].dst;									
				ROB.rob[ROB.tail].pc = RN.reg[i].pc;																				

				if (RN.reg[i].dst != -1)												
				{
					RMT.rmt[RN.reg[i].dst].tag = ROB.tail;								
					RMT.rmt[RN.reg[i].dst].valid = 1;								
				}

				RN.reg[i].dst = ROB.tail;												

				if (ROB.tail != (ROB.ROB_Size - 1))
					ROB.tail++;
				else
					ROB.tail = 0;

				RN.reg[i].rr_start = cycle_count + 1;
				RN.reg[i].rn_cycles = RN.reg[i].rr_start - RN.reg[i].rn_start;
			}
			RR.reg = RN.reg;
			RN.reg.clear();
			RN_empty = 1;
			RR_empty = 0;
		}
	}
}

void Decode()
{
	if (DE_empty == 0 && RN_empty == 1)
	{
		for (int i = 0; i < DE.reg.size(); i++)
		{
			DE.reg[i].rn_start = cycle_count + 1;
			DE.reg[i].de_cycles = DE.reg[i].rn_start - DE.reg[i].de_start;
		}
		RN.reg = DE.reg;
		DE.reg.clear();
		DE_empty = 1;
		RN_empty = 0;
	}
}

void Fetch(FILE* FP)
{
	pp pipe;
	if(DE_empty == 0)
		return;
	if (DE_empty == 1)
	{
        while(DE.reg.size() < ((unsigned)width)){
			if (feof(FP))
				return;
        	else{
				fscanf(FP,"%x %d %d %d %d\n",&pipe.pc, &pipe.type, &pipe.dst, &pipe.rs1_saved, &pipe.rs2_saved); 
        		pipe.rs1 = pipe.rs1_saved;
        		pipe.rs2 = pipe.rs2_saved;
				pipe.pc = PC;
				pipe.rs1_ready = pipe.rs2_ready = pipe.rs1_ROB = pipe.rs2_ROB = 0;
				pipe.fe_start = cycle_count;
				pipe.fe_cycles = 1;
				pipe.de_start = cycle_count + 1;
				switch(pipe.type){
					case 0:
						pipe.count = 1;
						break;
					case 1:
						pipe.count = 2;
						break;
					case 2:
						pipe.count = 5;
						break;
					default:
						break;
				}
				DE.reg.push_back(pipe);
				PC++;
				DE_empty = 0;
			}
        }
	}
}
