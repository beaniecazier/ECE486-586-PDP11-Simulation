#include "Memory.h"
#include "types.h"
#include "PDP11SimController.h"
#include "Register.h"
#include <iostream>

using namespace std;

#pragma region CONSTRUCT_DESTRUCT
///-----------------------------------------------
/// Constructor Destructor Functions
///-----------------------------------------------
PDP11SimController::PDP11SimController()
{	
	totalCount = 0;
	readCount = 0;
	writeCount = 0;
	instructionCount = 0;
	createAddressingModeTable();
	createBranchTable();
	createDoubleOpTable();
	createPSWITable();
	createSingleOpTable();
}

PDP11SimController::~PDP11SimController()
{
}
#pragma endregion

#pragma region TABLE
///-----------------------------------------------
/// Table Creation Functions
///-----------------------------------------------
void PDP11SimController::createSingleOpTable()
{
	SO->add(SWAB_OPCODE, this->SWAB);
	SO->add(JSR_OPCODE, this->JSR);
	SO->add(EMT_OPCODE, this->EMT);
	SO->add(CLR_OPCODE, this->CLR);
	SO->add(COM_OPCODE, this->COM);
	SO->add(INC_OPCODE, this->INC);
	SO->add(DEC_OPCODE, this->DEC);
	SO->add(NEG_OPCODE, this->NEG);
	SO->add(ADC_OPCODE, this->ADC);
	SO->add(SBC_OPCODE, this->SBC);
	SO->add(TST_OPCODE, this->TST);
	SO->add(ROR_OPCODE, this->ROR);
	SO->add(ROL_OPCODE, this->ROL);
	SO->add(ASR_OPCODE, this->ASR);
	SO->add(ASL_OPCODE, this->ASL);
	SO->add(SXT_OPCODE, this->SXT);
}

void PDP11SimController::createDoubleOpTable()
{
	DO->add(MOV_OPCODE, this->MOV);
	DO->add(CMP_OPCODE, this->CMP);
	DO->add(BIT_OPCODE, this->BIT);
	DO->add(BIC_OPCODE, this->BIC);
	DO->add(BIS_OPCODE, this->BIS);
	DO->add(ADD_OPCODE, this->ADD);
	DO->add(SUB_OPCODE, this->SUB);
}

void PDP11SimController::createAddressingModeTable()
{
	AM->add(REGISTER_CODE, this->/*function name no parenthesises*/);
}

void PDP11SimController::createBranchTable()
{
	BI->add(BR_OPCODE, this->BR);
	BI->add(BNE_OPCODE, this->BNE);
	BI->add(BEQ_OPCODE, this->BEQ);
	BI->add(BPL_OPCODE, this->BPL);
	BI->add(BMI_OPCODE, this->BMI);
	BI->add(BVC_OPCODE, this->BVC);
	BI->add(BHIS_OPCODE, this->BHIS);
	BI->add(BCC_OPCODE, this->BCC);
	BI->add(BLO_OPCODE, this->BLO);
	BI->add(BCS_OPCODE, this->BCS);
	BI->add(BGE_OPCODE, this->BGE);
	BI->add(BLT_OPCODE, this->BLT);
	BI->add(BGT_OPCODE, this->BGT);
	BI->add(BLE_OPCODE, this->BLE);
	BI->add(BHI_OPCODE, this->BHI);
	BI->add(BLOS_OPCODE, this->BLOS);
}

void PDP11SimController::createPSWITable()
{
	PSWI->add(CLC_OPCODE, this->CLC);
	PSWI->add(CLV_OPCODE, this->CLV);
	PSWI->add(CLZ_OPCODE, this->CLZ);
	PSWI->add(CLN_OPCODE, this->CLN);
	PSWI->add(SEC_OPCODE, this->SEC);
	PSWI->add(SEV_OPCODE, this->SEV);
	PSWI->add(SEZ_OPCODE, this->SEZ);
	PSWI->add(SEN_OPCODE, this->SEN);
	PSWI->add(CCC_OPCODE, this->CCC);
	PSWI->add(SCC_OPCODE, this->SCC);
}

void PDP11SimController::createEDOITable()
{
	EDO->add(MUL_OPCODE, this->MUL);
	EDO->add(DIV_OPCODE, this->DIV);
	EDO->add(ASH_OPCODE, this->ASH);
	EDO->add(ASHC_OPCODE, this->ASHC);
	EDO->add(XOR_OPCODE, this->XOR);
	EDO->add(FLOATING_POINT_OPCODE, this->FPO);
	EDO->add(SYSTEM_NSTRUCTION_OPCODE, this->SYSINSTRUCTION);
	EDO->add(SOB_OPCODE, this->SOB);
}
#pragma endregion

#pragma region DECODE
///-----------------------------------------------
/// Decoding Functions
///-----------------------------------------------
bool PDP11SimController::decode(int octalVA)
{
	OctalWord* ow = new OctalWord(octalVA);

	// check for too long of word
	if (octalVA > MAX_OCTAL_VALUE) { delete ow; return false; }
	// check to see if op is SPL, if true exec op
	if (checkForSPL(ow->octbit[1], ow->octbit[2], ow->octbit[3], ow->octbit[4], ow->octbit[5])) 
	{ 
		SPL(ow->octbit[0]); 
		delete ow; 
		return true; 
	}
	// check to see if op is PSWI, if true exec op
	if (checkForPSW(ow->octbit[3], ow->octbit[4], ow->octbit[5])){ doPSWI(ow->value); delete ow; return true; }
	// check to see if op is Branch Instruction, if true exec Instruction
	if (checkForBranch(ow->value)) { doBranchInstruction(ow->value); delete ow; return true; }
	// check to see if single operand instruction, if true exec instruction
	if (checkForSO(*ow)) { doSingleOpInstruction(*ow); delete ow; return true; }
	// check to see if double operand instruction, if true exec instruction
	if (checkForDO(*ow)) { doDoubleOpInstruction(*ow); delete ow; return true; }
	if (checkUnimplementedDoubleOp(*ow)) { delete ow; return true; }
	delete ow;
	cerr << "un reachable code segment end of bool PDP11SimController::decode(int octalVA) reached\n";
	return false;
}

bool PDP11SimController::checkForSPL(OctalBit b1, OctalBit b2, OctalBit b3, OctalBit b4, OctalBit b5)
{
	if (b5.b == 0 && b4.b == 0 && b3.b == 0 && b2.b == 2 && b1.b == 3) return true;
	return false;
}

bool PDP11SimController::checkForPSW(OctalBit b3, OctalBit b4, OctalBit b5)
{
	if (b3.b == 0 && b4.b == 0 && b5.b == 0) return true;
	return false;
}

bool PDP11SimController::checkForSO(OctalWord w)
{
	if (w.octbit[4].b == 0) return true;
	return false;
}

bool PDP11SimController::checkForDO(OctalWord w)
{
	if (w.octbit[4].b >= 1 && w.octbit[4].b <= 6) return true;
	return false;
}

bool PDP11SimController::checkUnimplementedDoubleOp(OctalWord w)
{
	if (w.octbit[5].b == 7)
	{
		doUnimplementedDoubleOp(w.octbit[4].b);
		return true;
	}
	return false;
}

bool PDP11SimController::checkForBranch(int value)
{
	int opcode = value >> 8;
	
	return (BI->find(opcode)) ? true : false;
}

#pragma endregion

#pragma region INSTRUCTION_EXECUTION
///-----------------------------------------------
/// Instruction Execution Functions
///-----------------------------------------------
void PDP11SimController::doPSWI(int opcode)
{
	//find and exec op by opcode
	(*(PSWI->find(opcode)))();
}

void PDP11SimController::doSingleOpInstruction(OctalWord w)
{
	int regNum, int regAddressMode, int opcode;
}

void PDP11SimController::doDoubleOpInstruction(OctalWord w)
{
	int destNum, int destAddressMode, int srcNum, int srcAddressMode, int opcode;
}

void PDP11SimController::doBranchInstruction(int value)
{
}

void PDP11SimController::doUnimplementedDoubleOp(int opnum)
{
	switch (opnum)
	{
	case 0:
		(*(EDO->find(MUL_OPCODE)))();
		break;
	case 1:
		(*(EDO->find(DIV_OPCODE)))();
		break;
	case 2:
		(*(EDO->find(ASH_OPCODE)))();
		break;
	case 3:
		(*(EDO->find(ASHC_OPCODE)))();
		break;
	case 4:
		(*(EDO->find(XOR_OPCODE)))();
		break;
	case 5:
		(*(EDO->find(FLOATING_POINT_OPCODE)))();
		break;
	case 6:
		(*(EDO->find(SYSTEM_NSTRUCTION_OPCODE)))();
		break;
	case 7:
		(*(EDO->find(SOB_OPCODE)))();
		break;
	default:
		break;
	}
}
#pragma endregion

#pragma region GETTERS
///-----------------------------------------------
/// Getters
///-----------------------------------------------
int PDP11SimController::getTotalCount()
{
	return totalCount;
}

int PDP11SimController::getReadCount()
{
	return readCount;
}

int PDP11SimController::getWriteCount()
{
	return writeCount;
}

int PDP11SimController::getInstructionCount()
{
	return instructionCount;
}
#pragma endregion

#pragma region NULL_FUNCTIONS
///-----------------------------------------------
/// NULL Functions
///-----------------------------------------------
void PDP11SimController::NULLFUNC()
{
}

void PDP11SimController::NULLFUNC(int src)
{
}

void PDP11SimController::NULLFUNC(int dest, int src)
{
}

#pragma endregion

#pragma region PROCESSOR_STATUS_WORD_INSTRUCTIONS
///-----------------------------------------------
/// Processor Status Word Instruction Functions
///-----------------------------------------------
void PDP11SimController::SPL(OctalBit bit)
{
	status.I = bit.b;
}

void PDP11SimController::CLC()
{
	status.C = 0;
}

void PDP11SimController::CLV()
{
	status.V = 0;
}

void PDP11SimController::CLZ()
{
	status.Z = 0;
}

void PDP11SimController::CLN()
{
	status.N = 0;
}

void PDP11SimController::SEC()
{
	status.C = 1;
}

void PDP11SimController::SEV()
{
	status.V = 1;
}

void PDP11SimController::SEZ()
{
	status.Z = 1;
}

void PDP11SimController::SEN()
{
	status.N = 1;
}

void PDP11SimController::CCC()
{
	status.C = 0;
	status.V = 0;
	status.Z = 0;
	status.N = 0;
}

void PDP11SimController::SCC()
{
	status.C = 1;
	status.V = 1;
	status.Z = 1;
	status.N = 1;
}
#pragma endregion

#pragma region ADDRESSING_MODES
///-----------------------------------------------
/// Addressing Mode Functions
///----------------------------------------------
///
/// The following are the defined constatns from types.h
/// that are used in this section:
///
///#define NUM_ADDRESSING_MODES			18
///#define REGISTER_CODE				00
///#define REGISTER_DEFERRED_CODE		01
///#define AUTOINC_CODE					02
///#define AUTOINC_DEFERRED_CODE		03
///#define AUTODEC_CODE					04
///#define AUTODEC_DEFERRED_CODE		05
///#define INDEX_CODE					06
///#define INDEX_DEFFERRED_CODE			07
///#define PC_IMMEDIATE_CODE			027
///#define PC_ABSOLUTE_CODE				037
///#define PC_RELATIVE_CODE				067
///#define PC_RELATIVE_DEFERRED_CODE	077
///#define SP_DEFERRED_CODE				016
///#define SP_AUTOINC_CODE				026
///#define SP_AUTOINC_DEFERRED_CODE		036
///#define SP_AUTODEC_CODE				046
///#define SP_INDEX_CODE				066
///#define Sp_INDEX_DEFFERRED_CODE		076



#pragma endregion

#pragma region DOUBLE_OPERAND_INSTRUCTIONS
///-----------------------------------------------
/// Double Operand Instruction Functions
///-----------------------------------------------
void PDP11SimController::MOV(int dest, int src)
{
}

void PDP11SimController::CMP(int dest, int src)
{
}

void PDP11SimController::BIT(int dest, int src)
{
}

void PDP11SimController::BIC(int dest, int src)
{
}

void PDP11SimController::BIS(int dest, int src)
{
}

void PDP11SimController::ADD(int dest, int src)
{
}

void PDP11SimController::SUB(int dest, int src)
{
}


#pragma endregion

#pragma region SINGLE_OPERAND_INSTRUCTIONS
///-----------------------------------------------
/// Single Operand Instruction Functions
///-----------------------------------------------
void PDP11SimController::SWAB(int src)
{
}

void PDP11SimController::JSR(int src)
{
}

void PDP11SimController::EMT(int src)
{
}

void PDP11SimController::CLR(int src)
{
}

void PDP11SimController::COM(int src)
{
}

void PDP11SimController::INC(int src)
{
}

void PDP11SimController::DEC(int src)
{
}

void PDP11SimController::NEG(int src)
{
}

void PDP11SimController::ADC(int src)
{
}

void PDP11SimController::SBC(int src)
{
}

void PDP11SimController::TST(int src)
{
}

void PDP11SimController::ROR(int src)
{
}

void PDP11SimController::ROL(int src)
{
}

void PDP11SimController::ASR(int src)
{
}

void PDP11SimController::ASL(int src)
{
}

void PDP11SimController::SXT(int src)
{
}

#pragma endregion

#pragma region BRANCH_INSTRUCTIONS
///-----------------------------------------------
/// Branch Instruction Functions
///-----------------------------------------------
void PDP11SimController::BR(int src)
{
}

void PDP11SimController::BNE(int src)
{
}

void PDP11SimController::BEQ(int src)
{
}

void PDP11SimController::BPL(int src)
{
}

void PDP11SimController::BMI(int src)
{
}

void PDP11SimController::BVC(int src)
{
}

void PDP11SimController::BHIS(int src)
{
}

void PDP11SimController::BCC(int src)
{
}

void PDP11SimController::BLO(int src)
{
}

void PDP11SimController::BCS(int src)
{
}

void PDP11SimController::BGE(int src)
{
}

void PDP11SimController::BLT(int src)
{
}

void PDP11SimController::BGT(int src)
{
}

void PDP11SimController::BLE(int src)
{
}

void PDP11SimController::BHI(int src)
{
}

void PDP11SimController::BLOS(int src)
{
}
#pragma endregion

#pragma region EXTENDED_DOUBLE_OPERAND_INSTRUCTIONS
///----------------------------------
/// Extended Double Operand Instruction Functions
///----------------------------------
void PDP11SimController::MUL()
{
	cout << "a MUL instruction was detected\n";
}

void PDP11SimController::DIV()
{
	cout << "a DIV instruction was detected\n";
}

void PDP11SimController::ASH()
{
	cout << "a ASH instruction was detected\n";
}

void PDP11SimController::ASHC()
{
	cout << "a ASHC instruction was detected\n";
}

void PDP11SimController::XOR()
{
	cout << "a XOR instruction was detected\n";
}

void PDP11SimController::FPO()
{
	cout << "a floating point instruction was detected\n";
}

void PDP11SimController::SYSINSTRUCTION()
{
	cout << "a system instruction was detected\n";
}

void PDP11SimController::SOB()
{
	cout << "a SOB instruction was detected\n";
}
#pragma endregion
