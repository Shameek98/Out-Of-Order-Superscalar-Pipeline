# Out-Of-Order Superscalar Pipeline

Constructed a pipeline simulator for an out-of-order superscalar processor that fetches N instructions per cycle.
Modelled a 9-stage superscalar pipeline and evaluated various configurations of the Issue Queue, ROB, and their effect on the amount of instructions processed per cycle.

The simulator first outputs the timing information for each dynamic instruction in program order, followed by final outputs.

The simulator outputs the timing information for each dynamic instruction in the trace, in program order (i.e., in the same order that instructions appear in the trace). The per-instruction timing information is output in the following format:
<seq_no> fu{\<op_type\>} src{\<src1\>,\<src2\>} dst{\<dst\>} FE{\<begin-cycle\>,\<duration\>} DE{…} RN{…} RR{…} DI{…} IS{…} EX{…} WB{…} RT{…} (\<seq_no\> is the line number in trace).

Command-line arguments to the simulator:
- sim \<ROB_SIZE\> \<IQ_SIZE\> \<WIDTH\> \<tracefile\> (where ROB_SIZE is the size of the ROB in bytes, IQ_SIZE is the size of the Issue Queue in bytes, WIDTH is the superscalar width of the processor i.e. the amount of instrcutions the processor can fetch in one cycle).
