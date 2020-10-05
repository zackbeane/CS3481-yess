//class to perform the Decode Stage
class DecodeStage: public Stage
{
	private:
        void setEInput(E * ereg, uint64_t stat, uint64_t icode, uint64_t ifun, 
                       uint64_t valC, uint64_t valA, uint64_t valB, uint64_t dstE,
                       uint64_t dstM, uint64_t srcA, uint64_t srcB);
        uint64_t getsrcA(uint64_t d_rA, uint64_t d_icode);
        uint64_t getsrcB(uint64_t d_rB, uint64_t d_icode);
        uint64_t getdstE(uint64_t d_icode, uint64_t d_rB);
        uint64_t getdstM(uint64_t d_rA, uint64_t d_icode);
        uint64_t getvalA(uint64_t srcA, D * dreg, M * mreg, W * wreg, uint64_t e_dstE, uint64_t e_valE, uint64_t d_rvalA, uint64_t M_valM);
        uint64_t getvalB(uint64_t srcB, D * dreg, M * mreg, W * wreg, uint64_t e_dstE, uint64_t e_valE, uint64_t d_rvalB, uint64_t M_valM);
		uint64_t d_srcA;
		uint64_t d_srcB;
		bool E_bubble;
		bool setEBubble(uint64_t E_icode, uint64_t E_dstM, uint64_t E_Cnd, uint64_t srcA, uint64_t srcB);
	public:
		bool doClockLow(PipeReg ** pregs, Stage ** stages);
		void doClockHigh(PipeReg ** pregs);
		uint64_t getd_srcA();
		uint64_t getd_srcB();
		bool getE_bubble();
};
