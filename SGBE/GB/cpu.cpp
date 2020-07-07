#include "cpu.h"

CPU::CPU(MMU& i_MMU) : m_MMU(i_MMU), AF(A, F), BC(B, C), DE(D, E), HL(H, L), m_IME(false), m_HALT(false), m_IsCCJump(false) {}

void CPU::Step(uint32_t& o_Cycles)
{
	// read next instruction opcode
	byte OPCode = readNextByte();

	// get opcode data from the opcode data map or the CB opcode data map
	OPCodeData OPCodeData;
	if (OPCode == 0xCB)
	{
		byte cbOPCode = readNextByte();
		OPCodeData = m_CB_OPCodeDataMap[cbOPCode];
	}
	else
	{
		OPCodeData = m_OPCodeDataMap[OPCode];
	}

	// execute
	if (PC.GetValue() - 1 >= 0xC && PC.GetValue() - 1 <= 0x1C) // this is for debug only, to be removed
	{
		LOG_INFO(true, NOP, "Executing " << OPCodeData.Name << " in address 0x" << std::hex << PC.GetValue() - 1);
		(this->*OPCodeData.Operation)();
		dumpRegisters();
		cout << endl;
	}
	else
	{
		(this->*OPCodeData.Operation)();
	}

	// calculate cycles
	if (m_IsCCJump)
	{
		o_Cycles += OPCodeData.ConditionalCycles;
	}
	else
	{
		o_Cycles += OPCodeData.Cycles;
	}

	m_IsCCJump = false; // restore is cc jump flag to false state before next step
}

void CPU::Reset()
{
	A.SetValue(0);
	B.SetValue(0);
	C.SetValue(0);
	D.SetValue(0);
	E.SetValue(0);
	F.SetValue(0);
	H.SetValue(0);
	L.SetValue(0);
	SP.SetValue(0);
	PC.SetValue(0);
	F.SetValue(0);
	m_IME = false;
	m_HALT = false;
	m_IsCCJump = false;
}

byte CPU::readNextByte()
{
	byte nextByte = m_MMU.Read(PC.GetValue());
	PC.Increment();
	return nextByte;
}

word CPU::readNextWord()
{
	byte lowByte = readNextByte();
	byte highByte = readNextByte();
	return static_cast<word>(highByte << 8 | lowByte);
}

sbyte CPU::readNextSignedByte()
{
	sbyte nextSbyte = m_MMU.Read(PC.GetValue());
	PC.Increment();
	return nextSbyte;
}

bool CPU::checkJumpCondition(JumpConditions i_Condition)
{
	switch (i_Condition)
	{
	case CPU::JumpConditions::NZ:
		if (!F.GetZ())
		{
			return true;
		}
		break;
	case CPU::JumpConditions::Z:
		if (F.GetZ())
		{
			return true;
		}
		break;
	case CPU::JumpConditions::NC:
		if (!F.GetC())
		{
			return true;
		}
		break;
	case CPU::JumpConditions::C:
		if (F.GetC())
		{
			return true;
		}
		break;
	}
	return false;
}

/* OPCodes */

/*
	Operation:
	LD nn, n

	Description:
	Put value n (byte from memory) into nn (register).
*/
void CPU::LD_nn_n(IRegister& i_DestRegister)
{
	byte n = readNextByte();
	i_DestRegister.SetValue(n);
}

/*
	Operation:
	LD n, nn

	Description:
	Put value nn (word from memory) into n (16 bit register).
*/
void CPU::LD_n_nn(IRegister& i_DestRegister)
{
	word nn = readNextWord();
	i_DestRegister.SetValue(nn);
}

/*
	Operation:
	LD r1, r2

	Description:
	Put value r2 into r1.

*/
void CPU::LD_r1_r2(IRegister& i_DestRegister, const IRegister& i_SrcRegister)
{
	word val = i_SrcRegister.GetValue();
	i_DestRegister.SetValue(val);
}

void CPU::LD_r1_r2(IRegister& i_DestRegister, const WordAddress& i_SrcMemory)
{
	byte val = m_MMU.Read(i_SrcMemory);
	i_DestRegister.SetValue(val);
}

void CPU::LD_r1_r2(const WordAddress& i_DestMemory, const IRegister& i_SrcRegister)
{
	byte val = static_cast<byte>(i_SrcRegister.GetValue());
	m_MMU.Write(i_DestMemory, val);
}

/*
	Operation:
	LDHL SP,n

	Description:
	Put SP + n effective address into HL
	n = one byte signed immediate value
	Z - Reset
	N - Reset
	H - Set or reset according to operation
	C - Set or reset according to operation
*/
void CPU::LD_HL_SP_n()
{
	sbyte n = readNextSignedByte();
	word res = SP.GetValue() + n;

	word flagCheck = SP.GetValue() ^ n ^ (SP.GetValue() + n);
	F.SetZ(false);
	F.SetN(false);
	(flagCheck & 0x10) == 0x10 ? F.SetH(true) : F.SetH(false);
	(flagCheck & 0x100) == 0x100 ? F.SetC(true) : F.SetC(false);

	HL.SetValue(res);
}

/*
	Operation:
	PUSH nn

	Description:
	Push nn onto stack
	Decrement Stack Pointer (SP) twice
*/
void CPU::PUSH(word i_Value)
{
	SP.Decrement();
	word addr = SP.GetValue();
	byte highVal = static_cast<byte>(i_Value >> 8);
	m_MMU.Write(addr, highVal);

	SP.Decrement();
	addr = SP.GetValue();
	byte lowVal = static_cast<byte>(i_Value);
	m_MMU.Write(addr, lowVal);
}

/*
	Operation:
	POP nn

	Description:
	Pop two bytes off stack into nn
	Increment Stack Pointer (SP) twice
*/
void CPU::POP(word& i_Value)
{
	word addr = SP.GetValue();
	byte lowVal = m_MMU.Read(addr);
	SP.Increment();

	addr = SP.GetValue();
	byte highVal = m_MMU.Read(addr);
	SP.Increment();

	i_Value = static_cast<word>(highVal << 8 | lowVal);
}

/*
	Operation:
	ADD A, n

	Description:
	Add n to A
	Z - Set if result is zero
	N - Reset
	H - Set if carry from bit 3
	C - Set if carry from bit 7
*/
void CPU::ADD(byte i_Value)
{
	byte aVal = static_cast<byte>(A.GetValue());
	byte res = aVal + i_Value;
	A.SetValue(res);

	res == 0x0 ? F.SetZ(true) : F.SetZ(false);
	F.SetN(false);
	((aVal & 0xF) + (i_Value & 0xF)) > 0xF ? F.SetH(true) : F.SetH(false);
	((aVal & 0xFF) + (i_Value & 0xFF)) > 0xFF ? F.SetC(true) : F.SetC(false);
}

/*
	Operation:
	ADD HL, n

	Description:
	Add n to HL
	Z - Not affected
	N - Reset
	H - Set if carry from bit 11
	C - Set if carry from bit 15
*/
void CPU::ADD_HL(word i_Value)
{
	word hlVal = HL.GetValue();
	word res = hlVal + i_Value;
	HL.SetValue(res);

	F.SetN(false);
	((hlVal & 0xFFF) + (i_Value & 0xFFF)) > 0xFFF ? F.SetH(true) : F.SetH(false);
	((hlVal & 0xFFFF) + (i_Value & 0xFFFF)) > 0xFFFF ? F.SetC(true) : F.SetC(false);
}

/*
	Operation:
	ADD SP, n

	Description:
	Add n to SP
	n = one byte signed immediate value (#).
	Z - Reset
	N - Reset
	H - Set or reset according to operation
	C - Set or reset according to operation
*/
void CPU::ADD_SP()
{
	sbyte val = readNextSignedByte();
	word spVal = SP.GetValue();
	int res = static_cast<int>(spVal + val);

	F.SetZ(false);
	F.SetN(false);
	if (val >= 0)
	{
		((spVal & 0xF) + (val & 0xF)) > 0xF ? F.SetH(true) : F.SetH(false);
		((spVal & 0xFF) + (val)) > 0xFF ? F.SetC(true) : F.SetC(false);
	}
	else
	{
		(res & 0xF) <= (spVal & 0xF) ? F.SetH(true) : F.SetH(false);
		(res & 0xFF) <= (spVal & 0xFF) ? F.SetC(true) : F.SetC(false);
	}
}

/*
	Operation:
	ADC A, n

	Description:
	Add n + Carry flag to A
	Z - Set if result is zero
	N - Reset
	H - Set if carry from bit 3
	C - Set if carry from bit 7
*/
void CPU::ADC(byte i_Value)
{
	byte aVal = static_cast<byte>(A.GetValue());
	byte carry = F.GetC();
	byte res = aVal + i_Value + carry;
	A.SetValue(res);

	res == 0x0 ? F.SetZ(true) : F.SetZ(false);
	F.SetN(false);
	((aVal & 0xF) + (i_Value & 0xF) + carry) > 0xF ? F.SetH(true) : F.SetH(false);
	((aVal & 0xFF) + (i_Value & 0xFF) + carry) > 0xFF ? F.SetC(true) : F.SetC(false);
}

/*
	Operation:
	SUB n

	Description:
	Subtract n to A
	Z - Set if result is zero
	N - Set
	H - Set if no borrow from bit 4
	C - Set if no borrow
*/
void CPU::SUB(byte i_Value)
{
	byte aVal = static_cast<byte>(A.GetValue());
	byte res = aVal - i_Value;
	A.SetValue(res);

	res == 0x0 ? F.SetZ(true) : F.SetZ(false);
	F.SetN(true);
	((aVal & 0xF) - (i_Value & 0xF)) < 0x0 ? F.SetH(true) : F.SetH(false);
	aVal < i_Value ? F.SetC(true) : F.SetC(false);
}

/*
	Operation:
	SBC A, n

	Description:
	Subtract n + Carry flag from A
	Z - Set if result is zero
	N - Set
	H - Set if no borrow from bit 4
	C - Set if no borrow
*/
void CPU::SBC(byte i_Value)
{
	byte aVal = static_cast<byte>(A.GetValue());
	byte carry = F.GetC();
	byte res = aVal - carry - i_Value;
	A.SetValue(res);

	res == 0x0 ? F.SetZ(true) : F.SetZ(false);
	F.SetN(true);
	((aVal & 0xF) - (i_Value & 0xF) - carry) < 0x0 ? F.SetH(true) : F.SetH(false);
	aVal < i_Value + carry ? F.SetC(true) : F.SetC(false);
}

/*
	Operation:
	AND n

	Description:
	Logically AND n with A, result in A
	Z - Set if result is zero
	N - Reset
	H - Set
	C - Reset
*/
void CPU::AND(byte i_Value)
{
	byte aVal = static_cast<byte>(A.GetValue());
	byte res = aVal & i_Value;
	A.SetValue(res);

	res == 0x0 ? F.SetZ(true) : F.SetZ(false);
	F.SetN(false);
	F.SetH(true);
	F.SetC(false);
}

/*
	Operation:
	OR n

	Description:
	Logical OR n with A, result in A
	Z - Set if result is zero
	N - Reset
	H - Reset
	C - Reset
*/
void CPU::OR(byte i_Value)
{
	byte aVal = static_cast<byte>(A.GetValue());
	byte res = aVal | i_Value;
	A.SetValue(res);

	res == 0x0 ? F.SetZ(true) : F.SetZ(false);
	F.SetN(false);
	F.SetH(false);
	F.SetC(false);
}

/*
	Operation:
	XOR n

	Description:
	Logical exclusive OR n with register A, result in A
	Z - Set if result is zero
	N - Reset
	H - Reset
	C - Reset
*/
void CPU::XOR(byte i_Value)
{
	byte aVal = static_cast<byte>(A.GetValue());
	byte res = aVal ^ i_Value;
	A.SetValue(res);

	res == 0x0 ? F.SetZ(true) : F.SetZ(false);
	F.SetN(false);
	F.SetH(false);
	F.SetC(false);
}

/*
	Operation:
	CP n

	Description:
	Compare A with n. This is basically an A - n
	subtraction instruction but the results are thrown away
	Z - Set if result is zero
	N - Set
	H - Set if no borrow from bit 4
	C - Set if no borrow
*/
void CPU::CP(byte i_Value)
{
	byte aVal = static_cast<byte>(A.GetValue());
	byte res = aVal - i_Value;

	res == 0x0 ? F.SetZ(true) : F.SetZ(false);
	F.SetN(true);
	((aVal & 0xF) - (i_Value & 0xF)) < 0x0 ? F.SetH(true) : F.SetH(false);
	aVal < i_Value ? F.SetC(true) : F.SetC(false);
}

/*
	Operation:
	INC n

	Description:
	Increment register n
	Z - Set if result is zero
	N - Reset
	H - Set if carry from bit 3
	C - Not affected
*/
void CPU::INC(IRegister& i_DestRegister)
{
	i_DestRegister.Increment();
	word regVal = i_DestRegister.GetValue();

	regVal == 0x0 ? F.SetZ(true) : F.SetZ(false);
	F.SetN(false);
	(regVal & 0xF) == 0x0 ? F.SetH(true) : F.SetH(false);
}

/*
	Operation:
	INC nn

	Description:
	Increment register nn
	nn = BC, DE, HL, SP
	No flags affected
*/
void CPU::INC_no_flags(IRegister& i_DestRegister)
{
	i_DestRegister.Increment();
}

/*
	Operation:
	DEC n

	Description:
	Decrement register n
	Z - Set if result is zero
	N - Reset
	H - Set if carry from bit 3
	C - Not affected
*/
void CPU::DEC(IRegister& i_DestRegister)
{
	i_DestRegister.Decrement();
	word regVal = i_DestRegister.GetValue();

	regVal == 0x0 ? F.SetZ(true) : F.SetZ(false);
	F.SetN(false);
	(regVal & 0xF) == 0x0 ? F.SetH(true) : F.SetH(false);
}

/*
	Operation:
	DEC nn

	Description:
	Decrement register nn
	nn = BC, DE, HL, SP
	No flags affected
*/
void CPU::DEC_no_flags(IRegister& i_DestRegister)
{
	i_DestRegister.Decrement();
}

/*
	Operation:
	DAA

	Description:
	Decimal adjust register A.
	This instruction adjusts register A so that the
	correct representation of Binary Coded Decimal (BCD)
	is obtained.
	Z - Set if register A is zero.
	N - Not affected.
	H - Reset.
	C - Set or reset according to operation.

	a youtube video explaining the operation:
	https://www.youtube.com/watch?v=tvGWNLY3BOE
*/
void CPU::DAA()
{
	byte aVal = static_cast<byte>(A.GetValue());
	int correctionVal = 0x0;

	if (F.GetN())
	{
		if (F.GetH())
		{
			correctionVal -= 0x06;
		}

		if (F.GetC())
		{
			correctionVal -= 0x60;
		}
	}
	else
	{
		// if high nibble is greater than 9 or carry is set, add 0x60 to correction
		if ((((aVal & 0xF0) >> 4) > 0x9) || F.GetC())
		{
			correctionVal += 0x60;
		}

		// if low nibble is greater than 9 or half carry is set, add 0x6 to correction
		if (((aVal & 0xF) > 0x9) || F.GetH())
		{
			correctionVal += 0x6;
		}
	}

	int res = aVal + correctionVal;
	
	(res == 0x0) ? F.SetZ(true) : F.SetZ(false);
	F.SetH(false);
	((aVal & 0xFF) + (correctionVal & 0xFF)) > 0xFF ? F.SetC(true) : F.SetC(false);

	A.SetValue(static_cast<word>(res));
}

/*
	Operation:
	CPL

	Description:
	Complement A register. (Flip all bits.)
	Z - Not affected
	N - Set
	H - Set
	C - Not affected
*/
void CPU::CPL()
{
	byte aVal = A.GetValue();
	aVal = ~aVal;
	A.SetValue(aVal);

	F.SetN(true);
	F.SetH(true);
}

/*
	Operation:
	CCF

	Description:
	Complement carry flag.
	If C flag is set, then reset it.
	If C flag is reset, then set it.
	Z - Not affected
	N - Reset
	H - Reset
	C - Complemented
*/
void CPU::CCF()
{
	F.GetC() ? F.SetC(false) : F.SetC(true);

	F.SetN(false);
	F.SetH(false);
}

/*
	Operation:
	SCF

	Description:
	Set Carry flag
	Z - Not affected
	N - Reset
	H - Reset
	C - Set
*/
void CPU::SCF()
{
	F.SetC(true);
	
	F.SetN(false);
	F.SetH(false);
}

/*
	Operation:
	NOP

	Description:
	No operation
*/
void CPU::_NOP()
{
	// Do nothing
}

/*
	Operation:
	HALT

	Description:
	Power down CPU until an interrupt occurs. Use this
	when ever possible to reduce energy consumption.
*/
void CPU::HALT()
{
	m_HALT = true;
}

/*
	Operation:
	NOP

	Description:
	Halt CPU & LCD display until button pressed.
*/
void CPU::STOP()
{
	// nothing to do regarding lcd so its just like HALT
	HALT();
}

/*
	Operation:
	DI

	Description:
	This instruction disables interrupts
*/
void CPU::DI()
{
	m_IME = false;
}

/*
	Operation:
	EI

	Description:
	This intruction enables interrupts
*/
void CPU::EI()
{
	m_IME = true;
}

/*
	Operation:
	JP nn

	Description:
	Jump to address nn
	nn = two byte immediate value. (LS byte first.)
*/
void CPU::JP_nn()
{
	word addr = readNextWord();
	PC.SetValue(addr);
}

/*
	Operation:
	JP cc, nn

	Description:
	Jump to address n if following condition is true:
	cc = NZ, Jump if Z flag is reset
	cc = Z, Jump if Z flag is set
	cc = NC, Jump if C flag is reset
	cc = C, Jump if C flag is set
*/
void CPU::JP_cc_nn(JumpConditions i_Condition)
{
	word addr = readNextWord();
	if (checkJumpCondition(i_Condition))
	{
		PC.SetValue(addr);
		m_IsCCJump = true;
	}
}

/*
	Operation:
	JP (HL)

	Description:
	Jump to address contained in HL
*/
void CPU::JP_hl()
{
	word addr = HL.GetValue();
	PC.SetValue(addr);
}

/*
	Operation:
	JR n

	Description:
	Add n to current address and jump to it
*/
void CPU::JR_n()
{
	sbyte val = readNextSignedByte();
	word pcVal = PC.GetValue();
	pcVal += val;
	PC.SetValue(pcVal);
}

/*
	Operation:
	JR cc, n

	Description:
	If following condition is true then add n to current
	address and jump to it:
	n = one byte signed immediate value
	cc = NZ, Jump if Z flag is reset
	cc = Z, Jump if Z flag is set
	cc = NC, Jump if C flag is reset
	cc = C, Jump if C flag is set
*/
void CPU::JR_cc_n(JumpConditions i_Condition)
{
	sbyte val = readNextSignedByte();
	word pcVal = PC.GetValue();
	if (checkJumpCondition(i_Condition))
	{
		pcVal += val;
		PC.SetValue(pcVal);
		m_IsCCJump = true;
	}
}

/*
	Operation:
	CALL nn

	Description:
	Push address of next instruction onto stack and then jump to address nn
*/
void CPU::CALL_nn()
{
	word addr = readNextWord(); // addr = nn, pc now is post instruction CALL nn
	word pcVal = PC.GetValue();
	PUSH(pcVal); // store PC in the stack
	PC.SetValue(addr); // jump to nn
}

/*
	Operation:
	CALL cc nn

	Description:
	Call address nn if following condition is true:
	cc = NZ, Call if Z flag is reset
	cc = Z, Call if Z flag is set
	cc = NC, Call if C flag is reset
	cc = C, Call if C flag is set
*/
void CPU::CALL_cc_nn(JumpConditions i_Condition)
{
	word addr = readNextWord(); // addr = nn, pc now is post instruction CALL cc nn
	word pcVal = PC.GetValue();
	if (checkJumpCondition(i_Condition))
	{
		PUSH(pcVal); // store PC in the stack
		PC.SetValue(addr); // jump to nn
		m_IsCCJump = true;
	}
}

/*
	Operation:
	RST n

	Description:
	Push present address onto stack
	Jump to address $0000 + n
*/
void CPU::RST_n(word i_Value)
{
	word pcVal = PC.GetValue();
	PUSH(pcVal);
	PC.SetValue(i_Value);
}

/*
	Operation:
	RET

	Description:
	Pop two bytes from stack & jump to that address
*/
void CPU::RET()
{
	word val = 0x0;
	POP(val);
	PC.SetValue(val);
}

/*
	Operation:
	RET cc

	Description:
	Return if following condition is true:
	cc = NZ, Return if Z flag is reset
	cc = Z, Return if Z flag is set
	cc = NC, Return if C flag is reset
	cc = C, Return if C flag is set
*/
void CPU::RET_cc(JumpConditions i_Condition)
{
	if (checkJumpCondition(i_Condition))
	{
		m_IsCCJump = true;
		RET();
	}
}

/*
	Operation:
	RETI

	Description:
	Pop two bytes from stack & jump to that address then enable interrupts
*/
void CPU::RETI()
{
	m_IME = true;
	RET();
}

/*
	Operation:
	SWAP n

	Description:
	Swap upper & lower nibles of n
	Z - Set if result is zero
	N - Reset
	H - Reset
	C - Reset
*/
void CPU::SWAP(IRegister& i_DestRegister)
{
	byte val = static_cast<byte>(i_DestRegister.GetValue());
	byte res = (val >> 4) | (val << 4);

	res == 0x0 ? F.SetZ(true) : F.SetZ(false);
	F.SetN(false);
	F.SetH(false);
	F.SetC(false);

	i_DestRegister.SetValue(res);
}

/*
	Operation:
	RLC n

	Description:
	Rotate n left. Old bit 7 to Carry flag
	Z - Set if result is zero
	N - Reset
	H - Reset
	C - Contains old bit 7 data
*/
void CPU::RLC_n(IRegister& i_DestRegister)
{
	byte val = i_DestRegister.GetValue();
	i_DestRegister.GetBit(7) ? F.SetC(true) : F.SetC(false);
	
	val = (val << 1) | F.GetC(); // rotate left and add bit 7 to bit 0
	i_DestRegister.SetValue(val);
	
	val == 0x0 ? F.SetZ(true) : F.SetZ(false);
	F.SetN(false);
	F.SetH(false);
}

/*
	Operation:
	RL n

	Description:
	Rotate n left through Carry flag
	Z - Set if result is zero
	N - Reset
	H - Reset
	C - Contains old bit 7 data
*/
void CPU::RL_n(IRegister& i_DestRegister)
{
	byte val = i_DestRegister.GetValue();
	// save bit 7 from n
	bool bit7 = i_DestRegister.GetBit(7);

	val = (val << 1) | F.GetC(); // rotate left and add carry to bit 0
	i_DestRegister.SetValue(val);

	// set old bit 7 into carry flag
	bit7 ? F.SetC(true) : F.SetC(false);

	val == 0x0 ? F.SetZ(true) : F.SetZ(false);
	F.SetN(false);
	F.SetH(false);
}

/*
	Operation:
	RRC n

	Description:
	Rotate n right. Old bit 0 to Carry flag
	Z - Set if result is zero
	N - Reset
	H - Reset
	C - Contains old bit 0 data
*/
void CPU::RRC_n(IRegister& i_DestRegister)
{
	byte val = i_DestRegister.GetValue();
	i_DestRegister.GetBit(0) ? F.SetC(true) : F.SetC(false);

	val = (val >> 1) | (F.GetC() << 7); // rotate right and add bit 0 to bit 7
	i_DestRegister.SetValue(val);

	val == 0x0 ? F.SetZ(true) : F.SetZ(false);
	F.SetN(false);
	F.SetH(false);
}

/*
	Operation:
	RR n

	Description:
	Rotate n right through Carry flag
	Z - Set if result is zero
	N - Reset
	H - Reset
	C - Contains old bit 0 data
*/
void CPU::RR_n(IRegister& i_DestRegister)
{
	byte val = i_DestRegister.GetValue();
	// save bit 0 from n
	bool bit0 = i_DestRegister.GetBit(0);

	val = (val >> 1) | (F.GetC() << 7); // rotate left and add carry to bit 7
	i_DestRegister.SetValue(val);

	// set old bit 0 into carry flag
	bit0 ? F.SetC(true) : F.SetC(false);

	val == 0x0 ? F.SetZ(true) : F.SetZ(false);
	F.SetN(false);
	F.SetH(false);
}

/*
	Operation:
	SLA n

	Description:
	Shift n left into Carry. LSB of n set to 0.
	Z - Set if result is zero
	N - Reset
	H - Reset
	C - Contains old bit 7 data
*/
void CPU::SLA_n(IRegister& i_DestRegister)
{
	byte val = i_DestRegister.GetValue();

	F.SetC(i_DestRegister.GetBit(7)); // set carry to be bit 7
	val = val << 1; // shift n left (LSB will be 0)
	i_DestRegister.SetValue(val);

	val == 0x0 ? F.SetZ(true) : F.SetZ(false);
	F.SetN(false);
	F.SetH(false);
}

/*
	Operation:
	SRA n

	Description:
	Shift n right into Carry. MSB doesn't change.
	Z - Set if result is zero
	N - Reset
	H - Reset
	C - Contains old bit 0 data
*/
void CPU::SRA_n(IRegister& i_DestRegister)
{
	byte val = i_DestRegister.GetValue();
	bool bit7 = i_DestRegister.GetBit(7);

	F.SetC(i_DestRegister.GetBit(0)); // set carry to be bit 0
	val = val >> 1; // shift n right, MSB is 0 now
	if (bit7) // if MSB was set, restore it
	{
		val |= (1 << 7);
	}
	i_DestRegister.SetValue(val);

	val == 0x0 ? F.SetZ(true) : F.SetZ(false);
	F.SetN(false);
	F.SetH(false);
}

/*
	Operation:
	SRL n

	Description:
	Shift n right into Carry. MSB set to 0
	Z - Set if result is zero
	N - Reset
	H - Reset
	C - Contains old bit 0 data
*/
void CPU::SRL_n(IRegister& i_DestRegister)
{
	byte val = i_DestRegister.GetValue();
	F.SetC(i_DestRegister.GetBit(0)); // set carry to be bit 0
	val = val >> 1; // shift n right, MSB is 0 now
	i_DestRegister.SetValue(val);

	val == 0x0 ? F.SetZ(true) : F.SetZ(false);
	F.SetN(false);
	F.SetH(false);
}

/*
	Operation:
	BIT b, r

	Description:
	Copies the complement of the contents of the specified bit in register r to the Z flag
	Z - Complement of bit b in r
	N - Reset
	H - Set
	C - Not affected
*/
void CPU::BIT_b_r(byte i_BitNumber, byte i_Value)
{
	bool res = !bitwise::GetBit(i_BitNumber, i_Value);
	
	F.SetZ(res);
	F.SetN(false);
	F.SetH(true);
}

/*
	Operation:
	SET b, r

	Description:
	Sets the specified bit in register r
*/
void CPU::SET_b_r(byte i_BitNumber, byte& o_Value)
{
	bitwise::SetBit(i_BitNumber, true, o_Value);
}

/*
	Operation:
	RES b, r

	Description:
	Resets the specified bit in register r
*/
void CPU::RES_b_r(byte i_BitNumber, byte& o_Value)
{
	bitwise::SetBit(i_BitNumber, false, o_Value);
}

const std::vector<CPU::OPCodeData> CPU::m_OPCodeDataMap
{
	{ &OPCode_00, "NOP", 4, 4 },
	{ &OPCode_01, "LD BC, nn", 12, 12 },
	{ &OPCode_02, "LD (BC), A", 8, 8 },
	{ &OPCode_03, "INC BC", 8, 8 },
	{ &OPCode_04, "INC B", 4, 4 },
	{ &OPCode_05, "DEC B", 4, 4 },
	{ &OPCode_06, "LD B, n", 8, 8 },
	{ &OPCode_07, "RLCA", 4, 4 },
	{ &OPCode_08, "LD (nn), SP", 20, 20 },
	{ &OPCode_09, "ADD HL, BC", 8, 8 },
	{ &OPCode_0A, "LD A, (BC)", 8, 8 },
	{ &OPCode_0B, "DEC BC", 8, 8 },
	{ &OPCode_0C, "INC C", 4, 4 },
	{ &OPCode_0D, "DEC C", 4, 4 },
	{ &OPCode_0E, "LD C, n", 8, 8 },
	{ &OPCode_0F, "RRCA", 4, 4 },
	{ &OPCode_10, "STOP", 4, 4 },
	{ &OPCode_11, "LD DE, nn", 12, 12 },
	{ &OPCode_12, "LD (DE), A", 8, 8 },
	{ &OPCode_13, "INC DE", 8, 8 },
	{ &OPCode_14, "INC D", 4, 4 },
	{ &OPCode_15, "DEC D", 4, 4 },
	{ &OPCode_16, "LD D, n", 8, 8 },
	{ &OPCode_17, "RLA", 4, 4 },
	{ &OPCode_18, "JR n", 12, 12 },
	{ &OPCode_19, "ADD HL, DE", 8, 8 },
	{ &OPCode_1A, "LD A, (DE)", 8, 8 },
	{ &OPCode_1B, "DEC DE", 8, 8 },
	{ &OPCode_1C, "INC E", 4, 4 },
	{ &OPCode_1D, "DEC E", 4, 4 },
	{ &OPCode_1E, "LD E, n", 8, 8 },
	{ &OPCode_1F, "RRA", 4, 4 },
	{ &OPCode_20, "JR NZ, n", 8, 12 },
	{ &OPCode_21, "LD HL, nn", 12, 12 },
	{ &OPCode_22, "LDI (HL), A", 8, 8 },
	{ &OPCode_23, "INC HL", 8, 8 },
	{ &OPCode_24, "INC H", 4, 4 },
	{ &OPCode_25, "DEC H", 4, 4 },
	{ &OPCode_26, "LD H, n", 8, 8 },
	{ &OPCode_27, "DAA", 4, 4 },
	{ &OPCode_28, "JR Z, n", 8, 12 },
	{ &OPCode_29, "ADD HL, HL", 8, 8 },
	{ &OPCode_2A, "LDI A, (HL)", 8, 8 },
	{ &OPCode_2B, "DEC HL", 8, 8 },
	{ &OPCode_2C, "INC L", 4, 4 },
	{ &OPCode_2D, "DEC L", 4, 4 },
	{ &OPCode_2E, "LD L, n", 8, 8 },
	{ &OPCode_2F, "CPL", 4, 4 },
	{ &OPCode_30, "JR NC, n", 8, 12 },
	{ &OPCode_31, "LD SP, nn", 12, 12 },
	{ &OPCode_32, "LDD (HL), A", 8, 8 },
	{ &OPCode_33, "INC SP", 8, 8 },
	{ &OPCode_34, "INC (HL)", 12, 12 },
	{ &OPCode_35, "DEC (HL)", 12, 12 },
	{ &OPCode_36, "LD (HL), n", 12, 12 },
	{ &OPCode_37, "SCF", 4, 4 },
	{ &OPCode_38, "JR C, n", 8, 12 },
	{ &OPCode_39, "ADD HL, SP", 8, 8 },
	{ &OPCode_3A, "LDD A, (HL)", 8, 8 },
	{ &OPCode_3B, "DEC SP", 8, 8 },
	{ &OPCode_3C, "INC A", 4, 4 },
	{ &OPCode_3D, "DEC A", 4, 4 },
	{ &OPCode_3E, "LD A, n", 8, 8 },
	{ &OPCode_3F, "CCF", 4, 4 },
	{ &OPCode_40, "LD B, B", 4, 4 },
	{ &OPCode_41, "LD B, C", 4, 4 },
	{ &OPCode_42, "LD B, D", 4, 4 },
	{ &OPCode_43, "LD B, E", 4, 4 },
	{ &OPCode_44, "LD B, H", 4, 4 },
	{ &OPCode_45, "LD B, L", 4, 4 },
	{ &OPCode_46, "LD B, (HL)", 8, 8 },
	{ &OPCode_47, "LD B, A", 4, 4 },
	{ &OPCode_48, "LD C, B", 4, 4 },
	{ &OPCode_49, "LD C, C", 4, 4 },
	{ &OPCode_4A, "LD C, D", 4, 4 },
	{ &OPCode_4B, "LD C, E", 4, 4 },
	{ &OPCode_4C, "LD C, H", 4, 4 },
	{ &OPCode_4D, "LD C, L", 4, 4 },
	{ &OPCode_4E, "LD C, (HL)", 8, 8 },
	{ &OPCode_4F, "LD C, A", 4, 4 },
	{ &OPCode_50, "LD D, B", 4, 4 },
	{ &OPCode_51, "LD D, C", 4, 4 },
	{ &OPCode_52, "LD D, D", 4, 4 },
	{ &OPCode_53, "LD D, E", 4, 4 },
	{ &OPCode_54, "LD D, H", 4, 4 },
	{ &OPCode_55, "LD D, L", 4, 4 },
	{ &OPCode_56, "LD D, (HL)", 8, 8 },
	{ &OPCode_57, "LD D, A", 4, 4 },
	{ &OPCode_58, "LD E, B", 4, 4 },
	{ &OPCode_59, "LD E, C", 4, 4 },
	{ &OPCode_5A, "LD E, D", 4, 4 },
	{ &OPCode_5B, "LD E, E", 4, 4 },
	{ &OPCode_5C, "LD E, H", 4, 4 },
	{ &OPCode_5D, "LD E, L", 4, 4 },
	{ &OPCode_5E, "LD E, (HL)", 8, 8 },
	{ &OPCode_5F, "LD E, A", 4, 4 },
	{ &OPCode_60, "LD H, B", 4, 4 },
	{ &OPCode_61, "LD H, C", 4, 4 },
	{ &OPCode_62, "LD H, D", 4, 4 },
	{ &OPCode_63, "LD H, E", 4, 4 },
	{ &OPCode_64, "LD H, H", 4, 4 },
	{ &OPCode_65, "LD H, L", 4, 4 },
	{ &OPCode_66, "LD H, (HL)", 8, 8 },
	{ &OPCode_67, "LD H, A", 4, 4 },
	{ &OPCode_68, "LD L, B", 4, 4 },
	{ &OPCode_69, "LD L, C", 4, 4 },
	{ &OPCode_6A, "LD L, D", 4, 4 },
	{ &OPCode_6B, "LD L, E", 4, 4 },
	{ &OPCode_6C, "LD L, H", 4, 4 },
	{ &OPCode_6D, "LD L, L", 4, 4 },
	{ &OPCode_6E, "LD L, (HL)", 8, 8 },
	{ &OPCode_6F, "LD L, A", 4, 4 },
	{ &OPCode_70, "LD (HL), B", 8, 8 },
	{ &OPCode_71, "LD (HL), C", 8, 8 },
	{ &OPCode_72, "LD (HL), D", 8, 8 },
	{ &OPCode_73, "LD (HL), E", 8, 8 },
	{ &OPCode_74, "LD (HL), H", 8, 8 },
	{ &OPCode_75, "LD (HL), L", 8, 8 },
	{ &OPCode_76, "HALT", 4, 4 },
	{ &OPCode_77, "LD (HL), A", 8, 8 },
	{ &OPCode_78, "LD A, B", 4, 4 },
	{ &OPCode_79, "LD A, C", 4, 4 },
	{ &OPCode_7A, "LD A, D", 4, 4 },
	{ &OPCode_7B, "LD A, E", 4, 4 },
	{ &OPCode_7C, "LD A, H", 4, 4 },
	{ &OPCode_7D, "LD A, L", 4, 4 },
	{ &OPCode_7E, "LD A, (HL)", 8, 8 },
	{ &OPCode_7F, "LD A, A", 4, 4 },
	{ &OPCode_80, "ADD A, B", 4, 4 },
	{ &OPCode_81, "ADD A, C", 4, 4 },
	{ &OPCode_82, "ADD A, D", 4, 4 },
	{ &OPCode_83, "ADD A, E", 4, 4 },
	{ &OPCode_84, "ADD A, H", 4, 4 },
	{ &OPCode_85, "ADD A, L", 4, 4 },
	{ &OPCode_86, "ADD A, (HL)", 8, 8 },
	{ &OPCode_87, "ADD A, A", 4, 4 },
	{ &OPCode_88, "ADC A, B", 4, 4 },
	{ &OPCode_89, "ADC A, C", 4, 4 },
	{ &OPCode_8A, "ADC A, D", 4, 4 },
	{ &OPCode_8B, "ADC A, E", 4, 4 },
	{ &OPCode_8C, "ADC A, H", 4, 4 },
	{ &OPCode_8D, "ADC A, L", 4, 4 },
	{ &OPCode_8E, "ADC A, (HL)", 8, 8 },
	{ &OPCode_8F, "ADC A, A", 4, 4 },
	{ &OPCode_90, "SUB B", 4, 4 },
	{ &OPCode_91, "SUB C", 4, 4 },
	{ &OPCode_92, "SUB D", 4, 4 },
	{ &OPCode_93, "SUB E", 4, 4 },
	{ &OPCode_94, "SUB H", 4, 4 },
	{ &OPCode_95, "SUB L", 4, 4 },
	{ &OPCode_96, "SUB (HL)", 8, 8 },
	{ &OPCode_97, "SUB A", 4, 4 },
	{ &OPCode_98, "SBC A, B", 4, 4 },
	{ &OPCode_99, "SBC A, C", 4, 4 },
	{ &OPCode_9A, "SBC A, D", 4, 4 },
	{ &OPCode_9B, "SBC A, E", 4, 4 },
	{ &OPCode_9C, "SBC A, H", 4, 4 },
	{ &OPCode_9D, "SBC A, L", 4, 4 },
	{ &OPCode_9E, "SBC A, (HL)", 8, 8 },
	{ &OPCode_9F, "SBC A, A", 4, 4 },
	{ &OPCode_A0, "AND B", 4, 4 },
	{ &OPCode_A1, "AND C", 4, 4 },
	{ &OPCode_A2, "AND D", 4, 4 },
	{ &OPCode_A3, "AND E", 4, 4 },
	{ &OPCode_A4, "AND H", 4, 4 },
	{ &OPCode_A5, "AND L", 4, 4 },
	{ &OPCode_A6, "AND (HL)", 8, 8 },
	{ &OPCode_A7, "AND A", 4, 4 },
	{ &OPCode_A8, "XOR B", 4, 4 },
	{ &OPCode_A9, "XOR C", 4, 4 },
	{ &OPCode_AA, "XOR D", 4, 4 },
	{ &OPCode_AB, "XOR E", 4, 4 },
	{ &OPCode_AC, "XOR H", 4, 4 },
	{ &OPCode_AD, "XOR L", 4, 4 },
	{ &OPCode_AE, "XOR (HL)", 8, 8 },
	{ &OPCode_AF, "XOR A", 4, 4 },
	{ &OPCode_B0, "OR B", 4, 4 },
	{ &OPCode_B1, "OR C", 4, 4 },
	{ &OPCode_B2, "OR D", 4, 4 },
	{ &OPCode_B3, "OR E", 4, 4 },
	{ &OPCode_B4, "OR H", 4, 4 },
	{ &OPCode_B5, "OR L", 4, 4 },
	{ &OPCode_B6, "OR (HL)", 8, 8 },
	{ &OPCode_B7, "OR A", 4, 4 },
	{ &OPCode_B8, "CP B", 4, 4 },
	{ &OPCode_B9, "CP C", 4, 4 },
	{ &OPCode_BA, "CP D", 4, 4 },
	{ &OPCode_BB, "CP E", 4, 4 },
	{ &OPCode_BC, "CP H", 4, 4 },
	{ &OPCode_BD, "CP L", 4, 4 },
	{ &OPCode_BE, "CP (HL)", 8, 8 },
	{ &OPCode_BF, "CP A", 4, 4 },
	{ &OPCode_C0, "RET NZ", 8, 20 },
	{ &OPCode_C1, "POP BC", 12, 12 },
	{ &OPCode_C2, "JP NZ, nn", 12, 16 },
	{ &OPCode_C3, "JP nn", 16, 16 },
	{ &OPCode_C4, "CALL NZ, nn", 12, 24 },
	{ &OPCode_C5, "PUSH BC", 16, 16 },
	{ &OPCode_C6, "ADD A, n", 8, 8 },
	{ &OPCode_C7, "RST 00H", 16, 16 },
	{ &OPCode_C8, "RET Z", 8, 20 },
	{ &OPCode_C9, "RET", 16, 16 },
	{ &OPCode_CA, "JP Z, nn", 12, 16 },
	{ &_NOP, "", 0, 0 }, // CB - Extended OPcodes
	{ &OPCode_CC, "CALL Z, nn", 12, 24 },
	{ &OPCode_CD, "CALL nn", 24, 24 },
	{ &OPCode_CE, "ADC A, n", 8, 8 },
	{ &OPCode_CF, "RST 08H", 16, 16 },
	{ &OPCode_D0, "RET NC", 8, 20 },
	{ &OPCode_D1, "POP DE", 12, 12 },
	{ &OPCode_D2, "JP NC, nn", 12, 16 },
	{ &_NOP, "", 0, 0 }, /* D3 is undefined in the gb cpu */
	{ &OPCode_D4, "CALL NC, nn", 12, 24 },
	{ &OPCode_D5, "PUSH DE", 16, 16 },
	{ &OPCode_D6, "SUB n", 8, 8 },
	{ &OPCode_D7, "RST 10H", 16, 16 },
	{ &OPCode_D8, "RET C", 8, 20 },
	{ &OPCode_D9, "RETI", 16, 16 },
	{ &OPCode_DA, "JP C, nn", 12, 16 },
	{ &_NOP, "", 0, 0 }, /* DB is undefined in the gb cpu */
	{ &OPCode_DC, "CALL C, nn", 12, 24 },
	{ &_NOP, "", 0, 0 }, /* DD is undefined in the gb cpu */
	{ &OPCode_DE, "SBC A, n", 8, 8 },
	{ &OPCode_DF, "RST 18H", 16, 16 },
	{ &OPCode_E0, "LDH (n), A", 12, 12 },
	{ &OPCode_E1, "POP HL", 12, 12 },
	{ &OPCode_E2, "LD (C), A", 8, 8 },
	{ &_NOP, "", 0, 0 }, /* E3 is undefined in the gb cpu */
	{ &_NOP, "", 0, 0 }, /* E4 is undefined in the gb cpu */
	{ &OPCode_E5, "PUSH HL", 16, 16 },
	{ &OPCode_E6, "AND n", 8, 8 },
	{ &OPCode_E7, "RST 20H", 16, 16 },
	{ &OPCode_E8, "ADD SP, #", 16, 16 },
	{ &OPCode_E9, "JP (HL)", 4, 4 },
	{ &OPCode_EA, "LD (nn), A", 16, 16 },
	{ &_NOP, "", 0, 0 }, /* EB is undefined in the gb cpu */
	{ &_NOP, "", 0, 0 }, /* EC is undefined in the gb cpu */
	{ &_NOP, "", 0, 0 }, /* ED is undefined in the gb cpu */
	{ &OPCode_EE, "XOR n", 8, 8 },
	{ &OPCode_EF, "RST 28H", 16, 16 },
	{ &OPCode_F0, "LDH A, (n)", 12, 12 },
	{ &OPCode_F1, "POP AF", 12, 12 },
	{ &OPCode_F2, "LD A, (C)", 8, 8 },
	{ &OPCode_F3, "DI", 4, 4 },
	{ &_NOP, "", 0, 0 }, /* F4 is undefined in the gb cpu */
	{ &OPCode_F5, "PUSH AF", 16, 16 },
	{ &OPCode_F6, "OR n", 8, 8 },
	{ &OPCode_F7, "RST 30H", 16, 16 },
	{ &OPCode_F8, "LDHL SP, n", 12, 12 },
	{ &OPCode_F9, "LD SP, HL", 8, 8 },
	{ &OPCode_FA, "LD A, (nn)", 16, 16 },
	{ &OPCode_FB, "EI", 4, 4 },
	{ &_NOP, "", 0, 0 }, /* FC is undefined in the gb cpu */
	{ &_NOP, "", 0, 0 }, /* FD is undefined in the gb cpu */
	{ &OPCode_FE, "CP n", 8, 8 },
	{ &OPCode_FF, "RST 38H", 16, 16 },
};

const std::vector<CPU::OPCodeData> CPU::m_CB_OPCodeDataMap
{
	{ &OPCode_CB_00, "RLC B", 8, 8 },
	{ &OPCode_CB_01, "RLC C", 8, 8 },
	{ &OPCode_CB_02, "RLC D", 8, 8 },
	{ &OPCode_CB_03, "RLC E", 8, 8 },
	{ &OPCode_CB_04, "RLC H", 8, 8 },
	{ &OPCode_CB_05, "RLC L", 8, 8 },
	{ &OPCode_CB_06, "RLC (HL)", 16, 16 },
	{ &OPCode_CB_07, "RLC A", 8, 8 },
	{ &OPCode_CB_08, "RRC B", 8, 8 },
	{ &OPCode_CB_09, "RRC C", 8, 8 },
	{ &OPCode_CB_0A, "RRC D", 8, 8 },
	{ &OPCode_CB_0B, "RRC E", 8, 8 },
	{ &OPCode_CB_0C, "RRC H", 8, 8 },
	{ &OPCode_CB_0D, "RRC L", 8, 8 },
	{ &OPCode_CB_0E, "RRC (HL)", 16, 16 },
	{ &OPCode_CB_0F, "RRC A", 8, 8 },
	{ &OPCode_CB_10, "RL B", 8, 8 },
	{ &OPCode_CB_11, "RL C", 8, 8 },
	{ &OPCode_CB_12, "RL D", 8, 8 },
	{ &OPCode_CB_13, "RL E", 8, 8 },
	{ &OPCode_CB_14, "RL H", 8, 8 },
	{ &OPCode_CB_15, "RL L", 8, 8 },
	{ &OPCode_CB_16, "RL (HL)", 16, 16 },
	{ &OPCode_CB_17, "RL A", 8, 8 },
	{ &OPCode_CB_18, "RR B", 8, 8 },
	{ &OPCode_CB_19, "RR C", 8, 8 },
	{ &OPCode_CB_1A, "RR D", 8, 8 },
	{ &OPCode_CB_1B, "RR E", 8, 8 },
	{ &OPCode_CB_1C, "RR H", 8, 8 },
	{ &OPCode_CB_1D, "RR L", 8, 8 },
	{ &OPCode_CB_1E, "RR (HL)", 16, 16 },
	{ &OPCode_CB_1F, "RR A", 8, 8 },
	{ &OPCode_CB_20, "SLA B", 8, 8 },
	{ &OPCode_CB_21, "SLA C", 8, 8 },
	{ &OPCode_CB_22, "SLA D", 8, 8 },
	{ &OPCode_CB_23, "SLA E", 8, 8 },
	{ &OPCode_CB_24, "SLA H", 8, 8 },
	{ &OPCode_CB_25, "SLA L", 8, 8 },
	{ &OPCode_CB_26, "SLA (HL)", 16, 16 },
	{ &OPCode_CB_27, "SLA A", 8, 8 },
	{ &OPCode_CB_28, "SRA B", 8, 8 },
	{ &OPCode_CB_29, "SRA C", 8, 8 },
	{ &OPCode_CB_2A, "SRA D", 8, 8 },
	{ &OPCode_CB_2B, "SRA E", 8, 8 },
	{ &OPCode_CB_2C, "SRA H", 8, 8 },
	{ &OPCode_CB_2D, "SRA L", 8, 8 },
	{ &OPCode_CB_2E, "SRA (HL)", 16, 16 },
	{ &OPCode_CB_2F, "SRA A", 8, 8 },
	{ &OPCode_CB_30, "SWAP B", 8, 8 },
	{ &OPCode_CB_31, "SWAP C", 8, 8 },
	{ &OPCode_CB_32, "SWAP D", 8, 8 },
	{ &OPCode_CB_33, "SWAP E", 8, 8 },
	{ &OPCode_CB_34, "SWAP H", 8, 8 },
	{ &OPCode_CB_35, "SWAP L", 8, 8 },
	{ &OPCode_CB_36, "SWAP (HL)", 16, 16 },
	{ &OPCode_CB_37, "SWAP A", 8, 8 },
	{ &OPCode_CB_38, "SRL B", 8, 8 },
	{ &OPCode_CB_39, "SRL C", 8, 8 },
	{ &OPCode_CB_3A, "SRL D", 8, 8 },
	{ &OPCode_CB_3B, "SRL E", 8, 8 },
	{ &OPCode_CB_3C, "SRL H", 8, 8 },
	{ &OPCode_CB_3D, "SRL L", 8, 8 },
	{ &OPCode_CB_3E, "SRL (HL)", 16, 16 },
	{ &OPCode_CB_3F, "SRL A", 8, 8 },
	{ &OPCode_CB_40, "BIT 0, B", 8, 8 },
	{ &OPCode_CB_41, "BIT 0, C", 8, 8 },
	{ &OPCode_CB_42, "BIT 0, D", 8, 8 },
	{ &OPCode_CB_43, "BIT 0, E", 8, 8 },
	{ &OPCode_CB_44, "BIT 0, H", 8, 8 },
	{ &OPCode_CB_45, "BIT 0, L", 8, 8 },
	{ &OPCode_CB_46, "BIT 0, (HL)", 12, 12 },
	{ &OPCode_CB_47, "BIT 0, A", 8, 8 },
	{ &OPCode_CB_48, "BIT 1, B", 8, 8 },
	{ &OPCode_CB_49, "BIT 1, C", 8, 8 },
	{ &OPCode_CB_4A, "BIT 1, D", 8, 8 },
	{ &OPCode_CB_4B, "BIT 1, E", 8, 8 },
	{ &OPCode_CB_4C, "BIT 1, H", 8, 8 },
	{ &OPCode_CB_4D, "BIT 1, L", 8, 8 },
	{ &OPCode_CB_4E, "BIT 1, (HL)", 12, 12 },
	{ &OPCode_CB_4F, "BIT 1, A", 8, 8 },
	{ &OPCode_CB_50, "BIT 2, B", 8, 8 },
	{ &OPCode_CB_51, "BIT 2, C", 8, 8 },
	{ &OPCode_CB_52, "BIT 2, D", 8, 8 },
	{ &OPCode_CB_53, "BIT 2, E", 8, 8 },
	{ &OPCode_CB_54, "BIT 2, H", 8, 8 },
	{ &OPCode_CB_55, "BIT 2, L", 8, 8 },
	{ &OPCode_CB_56, "BIT 2, (HL)", 12, 12 },
	{ &OPCode_CB_57, "BIT 2, A", 8, 8 },
	{ &OPCode_CB_58, "BIT 3, B", 8, 8 },
	{ &OPCode_CB_59, "BIT 3, C", 8, 8 },
	{ &OPCode_CB_5A, "BIT 3, D", 8, 8 },
	{ &OPCode_CB_5B, "BIT 3, E", 8, 8 },
	{ &OPCode_CB_5C, "BIT 3, H", 8, 8 },
	{ &OPCode_CB_5D, "BIT 3, L", 8, 8 },
	{ &OPCode_CB_5E, "BIT 3, (HL)", 12, 12 },
	{ &OPCode_CB_5F, "BIT 3, A", 8, 8 },
	{ &OPCode_CB_60, "BIT 4, B", 8, 8 },
	{ &OPCode_CB_61, "BIT 4, C", 8, 8 },
	{ &OPCode_CB_62, "BIT 4, D", 8, 8 },
	{ &OPCode_CB_63, "BIT 4, E", 8, 8 },
	{ &OPCode_CB_64, "BIT 4, H", 8, 8 },
	{ &OPCode_CB_65, "BIT 4, L", 8, 8 },
	{ &OPCode_CB_66, "BIT 4, (HL)", 12, 12 },
	{ &OPCode_CB_67, "BIT 4, A", 8, 8 },
	{ &OPCode_CB_68, "BIT 5, B", 8, 8 },
	{ &OPCode_CB_69, "BIT 5, C", 8, 8 },
	{ &OPCode_CB_6A, "BIT 5, D", 8, 8 },
	{ &OPCode_CB_6B, "BIT 5, E", 8, 8 },
	{ &OPCode_CB_6C, "BIT 5, H", 8, 8 },
	{ &OPCode_CB_6D, "BIT 5, L", 8, 8 },
	{ &OPCode_CB_6E, "BIT 5, (HL)", 12, 12 },
	{ &OPCode_CB_6F, "BIT 5, A", 8, 8 },
	{ &OPCode_CB_70, "BIT 6, B", 8, 8 },
	{ &OPCode_CB_71, "BIT 6, C", 8, 8 },
	{ &OPCode_CB_72, "BIT 6, D", 8, 8 },
	{ &OPCode_CB_73, "BIT 6, E", 8, 8 },
	{ &OPCode_CB_74, "BIT 6, H", 8, 8 },
	{ &OPCode_CB_75, "BIT 6, L", 8, 8 },
	{ &OPCode_CB_76, "BIT 6, (HL)", 12, 12 },
	{ &OPCode_CB_77, "BIT 6, A", 8, 8 },
	{ &OPCode_CB_78, "BIT 7, B", 8, 8 },
	{ &OPCode_CB_79, "BIT 7, C", 8, 8 },
	{ &OPCode_CB_7A, "BIT 7, D", 8, 8 },
	{ &OPCode_CB_7B, "BIT 7, E", 8, 8 },
	{ &OPCode_CB_7C, "BIT 7, H", 8, 8 },
	{ &OPCode_CB_7D, "BIT 7, L", 8, 8 },
	{ &OPCode_CB_7E, "BIT 7, (HL)", 12, 12 },
	{ &OPCode_CB_7F, "BIT 7, A", 8, 8 },
	{ &OPCode_CB_80, "RES 0, B", 8, 8 },
	{ &OPCode_CB_81, "RES 0, C", 8, 8 },
	{ &OPCode_CB_82, "RES 0, D", 8, 8 },
	{ &OPCode_CB_83, "RES 0, E", 8, 8 },
	{ &OPCode_CB_84, "RES 0, H", 8, 8 },
	{ &OPCode_CB_85, "RES 0, L", 8, 8 },
	{ &OPCode_CB_86, "RES 0, (HL)", 16, 16 },
	{ &OPCode_CB_87, "RES 0, A", 8, 8 },
	{ &OPCode_CB_88, "RES 1, B", 8, 8 },
	{ &OPCode_CB_89, "RES 1, C", 8, 8 },
	{ &OPCode_CB_8A, "RES 1, D", 8, 8 },
	{ &OPCode_CB_8B, "RES 1, E", 8, 8 },
	{ &OPCode_CB_8C, "RES 1, H", 8, 8 },
	{ &OPCode_CB_8D, "RES 1, L", 8, 8 },
	{ &OPCode_CB_8E, "RES 1, (HL)", 16, 16 },
	{ &OPCode_CB_8F, "RES 1, A", 8, 8 },
	{ &OPCode_CB_90, "RES 2, B", 8, 8 },
	{ &OPCode_CB_91, "RES 2, C", 8, 8 },
	{ &OPCode_CB_92, "RES 2, D", 8, 8 },
	{ &OPCode_CB_93, "RES 2, E", 8, 8 },
	{ &OPCode_CB_94, "RES 2, H", 8, 8 },
	{ &OPCode_CB_95, "RES 2, L", 8, 8 },
	{ &OPCode_CB_96, "RES 2, (HL)", 16, 16 },
	{ &OPCode_CB_97, "RES 2, A", 8, 8 },
	{ &OPCode_CB_98, "RES 3, B", 8, 8 },
	{ &OPCode_CB_99, "RES 3, C", 8, 8 },
	{ &OPCode_CB_9A, "RES 3, D", 8, 8 },
	{ &OPCode_CB_9B, "RES 3, E", 8, 8 },
	{ &OPCode_CB_9C, "RES 3, H", 8, 8 },
	{ &OPCode_CB_9D, "RES 3, L", 8, 8 },
	{ &OPCode_CB_9E, "RES 3, (HL)", 16, 16 },
	{ &OPCode_CB_9F, "RES 3, A", 8, 8 },
	{ &OPCode_CB_A0, "RES 4, B", 8, 8 },
	{ &OPCode_CB_A1, "RES 4, C", 8, 8 },
	{ &OPCode_CB_A2, "RES 4, D", 8, 8 },
	{ &OPCode_CB_A3, "RES 4, E", 8, 8 },
	{ &OPCode_CB_A4, "RES 4, H", 8, 8 },
	{ &OPCode_CB_A5, "RES 4, L", 8, 8 },
	{ &OPCode_CB_A6, "RES 4, (HL)", 16, 16 },
	{ &OPCode_CB_A7, "RES 4, A", 8, 8 },
	{ &OPCode_CB_A8, "RES 5, B", 8, 8 },
	{ &OPCode_CB_A9, "RES 5, C", 8, 8 },
	{ &OPCode_CB_AA, "RES 5, D", 8, 8 },
	{ &OPCode_CB_AB, "RES 5, E", 8, 8 },
	{ &OPCode_CB_AC, "RES 5, H", 8, 8 },
	{ &OPCode_CB_AD, "RES 5, L", 8, 8 },
	{ &OPCode_CB_AE, "RES 5, (HL)", 16, 16 },
	{ &OPCode_CB_AF, "RES 5, A", 8, 8 },
	{ &OPCode_CB_B0, "RES 6, B", 8, 8 },
	{ &OPCode_CB_B1, "RES 6, C", 8, 8 },
	{ &OPCode_CB_B2, "RES 6, D", 8, 8 },
	{ &OPCode_CB_B3, "RES 6, E", 8, 8 },
	{ &OPCode_CB_B4, "RES 6, H", 8, 8 },
	{ &OPCode_CB_B5, "RES 6, L", 8, 8 },
	{ &OPCode_CB_B6, "RES 6, (HL)", 16, 16 },
	{ &OPCode_CB_B7, "RES 6, A", 8, 8 },
	{ &OPCode_CB_B8, "RES 7, B", 8, 8 },
	{ &OPCode_CB_B9, "RES 7, C", 8, 8 },
	{ &OPCode_CB_BA, "RES 7, D", 8, 8 },
	{ &OPCode_CB_BB, "RES 7, E", 8, 8 },
	{ &OPCode_CB_BC, "RES 7, H", 8, 8 },
	{ &OPCode_CB_BD, "RES 7, L", 8, 8 },
	{ &OPCode_CB_BE, "RES 7, (HL)", 16, 16 },
	{ &OPCode_CB_BF, "RES 7, A", 8, 8 },
	{ &OPCode_CB_C0, "SET 0, B", 8, 8 },
	{ &OPCode_CB_C1, "SET 0, C", 8, 8 },
	{ &OPCode_CB_C2, "SET 0, D", 8, 8 },
	{ &OPCode_CB_C3, "SET 0, E", 8, 8 },
	{ &OPCode_CB_C4, "SET 0, H", 8, 8 },
	{ &OPCode_CB_C5, "SET 0, L", 8, 8 },
	{ &OPCode_CB_C6, "SET 0, (HL)", 16, 16 },
	{ &OPCode_CB_C7, "SET 0, A", 8, 8 },
	{ &OPCode_CB_C8, "SET 1, B", 8, 8 },
	{ &OPCode_CB_C9, "SET 1, C", 8, 8 },
	{ &OPCode_CB_CA, "SET 1, D", 8, 8 },
	{ &OPCode_CB_CB, "SET 1, E", 8, 8 },
	{ &OPCode_CB_CC, "SET 1, H", 8, 8 },
	{ &OPCode_CB_CD, "SET 1, L", 8, 8 },
	{ &OPCode_CB_CE, "SET 1, (HL)", 16, 16 },
	{ &OPCode_CB_CF, "SET 1, A", 8, 8 },
	{ &OPCode_CB_D0, "SET 2, B", 8, 8 },
	{ &OPCode_CB_D1, "SET 2, C", 8, 8 },
	{ &OPCode_CB_D2, "SET 2, D", 8, 8 },
	{ &OPCode_CB_D3, "SET 2, E", 8, 8 },
	{ &OPCode_CB_D4, "SET 2, H", 8, 8 },
	{ &OPCode_CB_D5, "SET 2, L", 8, 8 },
	{ &OPCode_CB_D6, "SET 2, (HL)", 16, 16 },
	{ &OPCode_CB_D7, "SET 2, A", 8, 8 },
	{ &OPCode_CB_D8, "SET 3, B", 8, 8 },
	{ &OPCode_CB_D9, "SET 3, C", 8, 8 },
	{ &OPCode_CB_DA, "SET 3, D", 8, 8 },
	{ &OPCode_CB_DB, "SET 3, E", 8, 8 },
	{ &OPCode_CB_DC, "SET 3, H", 8, 8 },
	{ &OPCode_CB_DD, "SET 3, L", 8, 8 },
	{ &OPCode_CB_DE, "SET 3, (HL)", 16, 16 },
	{ &OPCode_CB_DF, "SET 3, A", 8, 8 },
	{ &OPCode_CB_E0, "SET 4, B", 8, 8 },
	{ &OPCode_CB_E1, "SET 4, C", 8, 8 },
	{ &OPCode_CB_E2, "SET 4, D", 8, 8 },
	{ &OPCode_CB_E3, "SET 4, E", 8, 8 },
	{ &OPCode_CB_E4, "SET 4, H", 8, 8 },
	{ &OPCode_CB_E5, "SET 4, L", 8, 8 },
	{ &OPCode_CB_E6, "SET 4, (HL)", 16, 16 },
	{ &OPCode_CB_E7, "SET 4, A", 8, 8 },
	{ &OPCode_CB_E8, "SET 5, B", 8, 8 },
	{ &OPCode_CB_E9, "SET 5, C", 8, 8 },
	{ &OPCode_CB_EA, "SET 5, D", 8, 8 },
	{ &OPCode_CB_EB, "SET 5, E", 8, 8 },
	{ &OPCode_CB_EC, "SET 5, H", 8, 8 },
	{ &OPCode_CB_ED, "SET 5, L", 8, 8 },
	{ &OPCode_CB_EE, "SET 5, (HL)", 16, 16 },
	{ &OPCode_CB_EF, "SET 5, A", 8, 8 },
	{ &OPCode_CB_F0, "SET 6, B", 8, 8 },
	{ &OPCode_CB_F1, "SET 6, C", 8, 8 },
	{ &OPCode_CB_F2, "SET 6, D", 8, 8 },
	{ &OPCode_CB_F3, "SET 6, E", 8, 8 },
	{ &OPCode_CB_F4, "SET 6, H", 8, 8 },
	{ &OPCode_CB_F5, "SET 6, L", 8, 8 },
	{ &OPCode_CB_F6, "SET 6, (HL)", 16, 16 },
	{ &OPCode_CB_F7, "SET 6, A", 8, 8 },
	{ &OPCode_CB_F8, "SET 7, B", 8, 8 },
	{ &OPCode_CB_F9, "SET 7, C", 8, 8 },
	{ &OPCode_CB_FA, "SET 7, D", 8, 8 },
	{ &OPCode_CB_FB, "SET 7, E", 8, 8 },
	{ &OPCode_CB_FC, "SET 7, H", 8, 8 },
	{ &OPCode_CB_FD, "SET 7, L", 8, 8 },
	{ &OPCode_CB_FE, "SET 7, (HL)", 16, 16 },
	{ &OPCode_CB_FF, "SET 7, A", 8, 8 },
};

void CPU::OPCode_06()
{
	LD_nn_n(B);
}

void CPU::OPCode_0E()
{
	LD_nn_n(C);
}

void CPU::OPCode_16()
{
	LD_nn_n(D);
}

void CPU::OPCode_1E()
{
	LD_nn_n(E);
}

void CPU::OPCode_26()
{
	LD_nn_n(H);
}

void CPU::OPCode_2E()
{
	LD_nn_n(L);
}

void CPU::OPCode_7F()
{
	LD_r1_r2(A, A);
}

void CPU::OPCode_78()
{
	LD_r1_r2(A, B);
}

void CPU::OPCode_79()
{
	LD_r1_r2(A, C);
}

void CPU::OPCode_7A()
{
	LD_r1_r2(A, D);
}

void CPU::OPCode_7B()
{
	LD_r1_r2(A, E);
}

void CPU::OPCode_7C()
{
	LD_r1_r2(A, H);
}

void CPU::OPCode_7D()
{
	LD_r1_r2(A, L);
}

void CPU::OPCode_7E()
{
	WordAddress addr = HL.GetValue();
	LD_r1_r2(A, addr);
}

void CPU::OPCode_40()
{
	LD_r1_r2(B, B);
}

void CPU::OPCode_41()
{
	LD_r1_r2(B, C);
}

void CPU::OPCode_42()
{
	LD_r1_r2(B, D);
}

void CPU::OPCode_43()
{
	LD_r1_r2(B, E);
}

void CPU::OPCode_44()
{
	LD_r1_r2(B, H);
}

void CPU::OPCode_45()
{
	LD_r1_r2(B, L);
}

void CPU::OPCode_46()
{
	WordAddress addr = HL.GetValue();
	LD_r1_r2(B, addr);
}

void CPU::OPCode_48()
{
	LD_r1_r2(C, B);
}

void CPU::OPCode_49()
{
	LD_r1_r2(C, C);
}

void CPU::OPCode_4A()
{
	LD_r1_r2(C, D);
}

void CPU::OPCode_4B()
{
	LD_r1_r2(C, E);
}

void CPU::OPCode_4C()
{
	LD_r1_r2(C, H);
}

void CPU::OPCode_4D()
{
	LD_r1_r2(C, L);
}

void CPU::OPCode_4E()
{
	WordAddress addr = HL.GetValue();
	LD_r1_r2(C, addr);
}

void CPU::OPCode_50()
{
	LD_r1_r2(D, B);
}

void CPU::OPCode_51()
{
	LD_r1_r2(D, C);
}

void CPU::OPCode_52()
{
	LD_r1_r2(D, D);
}

void CPU::OPCode_53()
{
	LD_r1_r2(D, E);
}

void CPU::OPCode_54()
{
	LD_r1_r2(D, H);
}

void CPU::OPCode_55()
{
	LD_r1_r2(D, L);
}

void CPU::OPCode_56()
{
	WordAddress addr = HL.GetValue();
	LD_r1_r2(D, addr);
}

void CPU::OPCode_58()
{
	LD_r1_r2(E, B);
}

void CPU::OPCode_59()
{
	LD_r1_r2(E, C);
}

void CPU::OPCode_5A()
{
	LD_r1_r2(E, D);
}

void CPU::OPCode_5B()
{
	LD_r1_r2(E, E);
}

void CPU::OPCode_5C()
{
	LD_r1_r2(E, H);
}

void CPU::OPCode_5D()
{
	LD_r1_r2(E, L);
}

void CPU::OPCode_5E()
{
	WordAddress addr = HL.GetValue();
	LD_r1_r2(E, addr);
}

void CPU::OPCode_60()
{
	LD_r1_r2(H, B);
}

void CPU::OPCode_61()
{
	LD_r1_r2(H, C);
}

void CPU::OPCode_62()
{
	LD_r1_r2(H, D);
}

void CPU::OPCode_63()
{
	LD_r1_r2(H, E);
}

void CPU::OPCode_64()
{
	LD_r1_r2(H, H);
}

void CPU::OPCode_65()
{
	LD_r1_r2(H, L);
}

void CPU::OPCode_66()
{
	WordAddress addr = HL.GetValue();
	LD_r1_r2(H, addr);
}

void CPU::OPCode_68()
{
	LD_r1_r2(L, B);
}

void CPU::OPCode_69()
{
	LD_r1_r2(L, C);
}

void CPU::OPCode_6A()
{
	LD_r1_r2(L, D);
}

void CPU::OPCode_6B()
{
	LD_r1_r2(L, E);
}

void CPU::OPCode_6C()
{
	LD_r1_r2(L, H);
}

void CPU::OPCode_6D()
{
	LD_r1_r2(L, L);
}

void CPU::OPCode_6E()
{
	WordAddress addr = HL.GetValue();
	LD_r1_r2(L, addr);
}

void CPU::OPCode_70()
{
	word addr = HL.GetValue();
	LD_r1_r2(addr, B);
}

void CPU::OPCode_71()
{
	word addr = HL.GetValue();
	LD_r1_r2(addr, C);
}

void CPU::OPCode_72()
{
	word addr = HL.GetValue();
	LD_r1_r2(addr, D);
}

void CPU::OPCode_73()
{
	word addr = HL.GetValue();
	LD_r1_r2(addr, E);
}

void CPU::OPCode_74()
{
	word addr = HL.GetValue();
	LD_r1_r2(addr, H);
}

void CPU::OPCode_75()
{
	word addr = HL.GetValue();
	LD_r1_r2(addr, L);
}

void CPU::OPCode_36()
{
	byte n = readNextByte();
	word addr = HL.GetValue();
	LD_r1_r2(addr, (ByteRegister&)n);
}

void CPU::OPCode_0A()
{
	word addr = BC.GetValue();
	LD_r1_r2(A, addr);
}

void CPU::OPCode_1A()
{
	word addr = DE.GetValue();
	LD_r1_r2(A, addr);
}

void CPU::OPCode_FA()
{
	word addr = readNextWord();
	LD_r1_r2(A, addr);
}

void CPU::OPCode_3E()
{
	LD_nn_n(A);
}

void CPU::OPCode_47()
{
	LD_r1_r2(B, A);
}

void CPU::OPCode_4F()
{
	LD_r1_r2(C, A);
}

void CPU::OPCode_57()
{
	LD_r1_r2(D, A);
}

void CPU::OPCode_5F()
{
	LD_r1_r2(E, A);
}

void CPU::OPCode_67()
{
	LD_r1_r2(H, A);
}

void CPU::OPCode_6F()
{
	LD_r1_r2(L, A);
}

void CPU::OPCode_02()
{
	word addr = BC.GetValue();
	LD_r1_r2(addr, A);
}

void CPU::OPCode_12()
{
	word addr = DE.GetValue();
	LD_r1_r2(addr, A);
}

void CPU::OPCode_77()
{
	word addr = HL.GetValue();
	LD_r1_r2(addr, A);
}

void CPU::OPCode_EA()
{
	word addr = readNextWord();
	LD_r1_r2(addr, A);
}

void CPU::OPCode_F2()
{
	word addr = 0xFF00 + C.GetValue();
	LD_r1_r2(A, addr);
}

void CPU::OPCode_E2()
{
	word addr = 0xFF00 + C.GetValue();
	LD_r1_r2(addr, A);
}

void CPU::OPCode_3A()
{
	OPCode_7E();
	HL.Decrement();
}

void CPU::OPCode_32()
{
	OPCode_77();
	HL.Decrement();
}

void CPU::OPCode_2A()
{
	OPCode_7E();
	HL.Increment();
}

void CPU::OPCode_22()
{
	OPCode_77();
	HL.Increment();
}

void CPU::OPCode_E0()
{
	byte val = readNextByte();
	word addr = 0xFF00 + val;
	LD_r1_r2(addr, A);
}

void CPU::OPCode_F0()
{
	byte val = readNextByte();
	word addr = 0xFF00 + val;
	LD_r1_r2(A, addr);
}

void CPU::OPCode_01()
{
	LD_n_nn(BC);
}

void CPU::OPCode_11()
{
	LD_n_nn(DE);
}

void CPU::OPCode_21()
{
	LD_n_nn(HL);
}

void CPU::OPCode_31()
{
	LD_n_nn(SP);
}

void CPU::OPCode_F9()
{
	LD_r1_r2(SP, HL);
}

void CPU::OPCode_F8()
{
	LD_HL_SP_n();
}

void CPU::OPCode_08()
{
	word addr = readNextWord();
	LD_r1_r2(addr, SP);
}

void CPU::OPCode_F5()
{
	word val = AF.GetValue();
	PUSH(val);
}

void CPU::OPCode_C5()
{
	word val = BC.GetValue();
	PUSH(val);
}

void CPU::OPCode_D5()
{
	word val = DE.GetValue();
	PUSH(val);
}

void CPU::OPCode_E5()
{
	word val = HL.GetValue();
	PUSH(val);
}

void CPU::OPCode_F1()
{
	word val = 0x0;
	POP(val);
	AF.SetValue(val);
}

void CPU::OPCode_C1()
{
	word val = 0x0;
	POP(val);
	BC.SetValue(val);
}

void CPU::OPCode_D1()
{
	word val = 0x0;
	POP(val);
	DE.SetValue(val);
}

void CPU::OPCode_E1()
{
	word val = 0x0;
	POP(val);
	HL.SetValue(val);
}

void CPU::OPCode_87()
{
	byte val = static_cast<byte>(A.GetValue());
	ADD(val);
}

void CPU::OPCode_80()
{
	byte val = static_cast<byte>(B.GetValue());
	ADD(val);
}

void CPU::OPCode_81()
{
	byte val = static_cast<byte>(C.GetValue());
	ADD(val);
}

void CPU::OPCode_82()
{
	byte val = static_cast<byte>(D.GetValue());
	ADD(val);
}

void CPU::OPCode_83()
{
	byte val = static_cast<byte>(E.GetValue());
	ADD(val);
}

void CPU::OPCode_84()
{
	byte val = static_cast<byte>(H.GetValue());
	ADD(val);
}

void CPU::OPCode_85()
{
	byte val = static_cast<byte>(L.GetValue());
	ADD(val);
}

void CPU::OPCode_86()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	ADD(val);
}

void CPU::OPCode_C6()
{
	byte val = readNextByte();
	ADD(val);
}

void CPU::OPCode_8F()
{
	byte val = static_cast<byte>(A.GetValue());
	ADC(val);
}

void CPU::OPCode_88()
{
	byte val = static_cast<byte>(B.GetValue());
	ADC(val);
}

void CPU::OPCode_89()
{
	byte val = static_cast<byte>(C.GetValue());
	ADC(val);
}

void CPU::OPCode_8A()
{
	byte val = static_cast<byte>(D.GetValue());
	ADC(val);
}

void CPU::OPCode_8B()
{
	byte val = static_cast<byte>(E.GetValue());
	ADC(val);
}

void CPU::OPCode_8C()
{
	byte val = static_cast<byte>(H.GetValue());
	ADC(val);
}

void CPU::OPCode_8D()
{
	byte val = static_cast<byte>(L.GetValue());
	ADC(val);
}

void CPU::OPCode_8E()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	ADC(val);
}

void CPU::OPCode_CE()
{
	byte val = readNextByte();
	ADC(val);
}

void CPU::OPCode_97()
{
	byte val = static_cast<byte>(A.GetValue());
	SUB(val);
}

void CPU::OPCode_90()
{
	byte val = static_cast<byte>(B.GetValue());
	SUB(val);
}

void CPU::OPCode_91()
{
	byte val = static_cast<byte>(C.GetValue());
	SUB(val);
}

void CPU::OPCode_92()
{
	byte val = static_cast<byte>(D.GetValue());
	SUB(val);
}

void CPU::OPCode_93()
{
	byte val = static_cast<byte>(E.GetValue());
	SUB(val);
}

void CPU::OPCode_94()
{
	byte val = static_cast<byte>(H.GetValue());
	SUB(val);
}

void CPU::OPCode_95()
{
	byte val = static_cast<byte>(L.GetValue());
	SUB(val);
}

void CPU::OPCode_96()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	SUB(val);
}

void CPU::OPCode_D6()
{
	byte val = readNextByte();
	SUB(val);
}

void CPU::OPCode_9F()
{
	byte val = static_cast<byte>(A.GetValue());
	SBC(val);
}

void CPU::OPCode_98()
{
	byte val = static_cast<byte>(B.GetValue());
	SBC(val);
}

void CPU::OPCode_99()
{
	byte val = static_cast<byte>(C.GetValue());
	SBC(val);
}

void CPU::OPCode_9A()
{
	byte val = static_cast<byte>(D.GetValue());
	SBC(val);
}

void CPU::OPCode_9B()
{
	byte val = static_cast<byte>(E.GetValue());
	SBC(val);
}

void CPU::OPCode_9C()
{
	byte val = static_cast<byte>(H.GetValue());
	SBC(val);
}

void CPU::OPCode_9D()
{
	byte val = static_cast<byte>(L.GetValue());
	SBC(val);
}

void CPU::OPCode_9E()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	SBC(val);
}

void CPU::OPCode_DE()
{
	byte val = readNextByte();
	SBC(val);
}

void CPU::OPCode_A7()
{
	byte val = static_cast<byte>(A.GetValue());
	AND(val);
}

void CPU::OPCode_A0()
{
	byte val = static_cast<byte>(B.GetValue());
	AND(val);
}

void CPU::OPCode_A1()
{
	byte val = static_cast<byte>(C.GetValue());
	AND(val);
}

void CPU::OPCode_A2()
{
	byte val = static_cast<byte>(D.GetValue());
	AND(val);
}

void CPU::OPCode_A3()
{
	byte val = static_cast<byte>(E.GetValue());
	AND(val);
}

void CPU::OPCode_A4()
{
	byte val = static_cast<byte>(H.GetValue());
	AND(val);
}

void CPU::OPCode_A5()
{
	byte val = static_cast<byte>(L.GetValue());
	AND(val);
}

void CPU::OPCode_A6()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	AND(val);
}

void CPU::OPCode_E6()
{
	byte val = readNextByte();
	AND(val);
}

void CPU::OPCode_B7()
{
	byte val = static_cast<byte>(A.GetValue());
	OR(val);
}

void CPU::OPCode_B0()
{
	byte val = static_cast<byte>(B.GetValue());
	OR(val);
}

void CPU::OPCode_B1()
{
	byte val = static_cast<byte>(C.GetValue());
	OR(val);
}

void CPU::OPCode_B2()
{
	byte val = static_cast<byte>(D.GetValue());
	OR(val);
}

void CPU::OPCode_B3()
{
	byte val = static_cast<byte>(E.GetValue());
	OR(val);
}

void CPU::OPCode_B4()
{
	byte val = static_cast<byte>(H.GetValue());
	OR(val);
}

void CPU::OPCode_B5()
{
	byte val = static_cast<byte>(L.GetValue());
	OR(val);
}

void CPU::OPCode_B6()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	OR(val);
}

void CPU::OPCode_F6()
{
	byte val = readNextByte();
	OR(val);
}

void CPU::OPCode_AF()
{
	byte val = static_cast<byte>(A.GetValue());
	XOR(val);
}

void CPU::OPCode_A8()
{
	byte val = static_cast<byte>(B.GetValue());
	XOR(val);
}

void CPU::OPCode_A9()
{
	byte val = static_cast<byte>(C.GetValue());
	XOR(val);
}

void CPU::OPCode_AA()
{
	byte val = static_cast<byte>(D.GetValue());
	XOR(val);
}

void CPU::OPCode_AB()
{
	byte val = static_cast<byte>(E.GetValue());
	XOR(val);
}

void CPU::OPCode_AC()
{
	byte val = static_cast<byte>(H.GetValue());
	XOR(val);
}

void CPU::OPCode_AD()
{
	byte val = static_cast<byte>(L.GetValue());
	XOR(val);
}

void CPU::OPCode_AE()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	XOR(val);
}

void CPU::OPCode_EE()
{
	byte val = readNextByte();
	XOR(val);
}

void CPU::OPCode_BF()
{
	byte val = static_cast<byte>(A.GetValue());
	CP(val);
}

void CPU::OPCode_B8()
{
	byte val = static_cast<byte>(B.GetValue());
	CP(val);
}

void CPU::OPCode_B9()
{
	byte val = static_cast<byte>(C.GetValue());
	CP(val);
}

void CPU::OPCode_BA()
{
	byte val = static_cast<byte>(D.GetValue());
	CP(val);
}

void CPU::OPCode_BB()
{
	byte val = static_cast<byte>(E.GetValue());
	CP(val);
}

void CPU::OPCode_BC()
{
	byte val = static_cast<byte>(H.GetValue());
	CP(val);
}

void CPU::OPCode_BD()
{
	byte val = static_cast<byte>(L.GetValue());
	CP(val);
}

void CPU::OPCode_BE()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	CP(val);
}

void CPU::OPCode_FE()
{
	byte val = readNextByte();
	CP(val);
}

void CPU::OPCode_3C()
{
	INC(A);
}

void CPU::OPCode_04()
{
	INC(B);
}

void CPU::OPCode_0C()
{
	INC(C);
}

void CPU::OPCode_14()
{
	INC(D);
}

void CPU::OPCode_1C()
{
	INC(E);
}

void CPU::OPCode_24()
{
	INC(H);
}

void CPU::OPCode_2C()
{
	INC(L);
}

void CPU::OPCode_34()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	INC((ByteRegister&)val); // the incremented value will be lost as its just a copy, however this call will adjust the cpu flag register
	val += 1;
	m_MMU.Write(addr, val);
}

void CPU::OPCode_3D()
{
	DEC(A);
}

void CPU::OPCode_05()
{
	DEC(B);
}

void CPU::OPCode_0D()
{
	DEC(C);
}

void CPU::OPCode_15()
{
	DEC(D);
}

void CPU::OPCode_1D()
{
	DEC(E);
}

void CPU::OPCode_25()
{
	DEC(H);
}

void CPU::OPCode_2D()
{
	DEC(L);
}

void CPU::OPCode_35()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	DEC((ByteRegister&)val); // the decremented value will be lost as its just a copy, however this call will adjust the cpu flag register
	val -= 1;
	m_MMU.Write(addr, val);
}

void CPU::OPCode_09()
{
	word val = BC.GetValue();
	ADD_HL(val);
}

void CPU::OPCode_19()
{
	word val = DE.GetValue();
	ADD_HL(val);
}

void CPU::OPCode_29()
{
	word val = HL.GetValue();
	ADD_HL(val);
}

void CPU::OPCode_39()
{
	word val = SP.GetValue();
	ADD_HL(val);
}

void CPU::OPCode_E8()
{
	ADD_SP();
}

void CPU::OPCode_03()
{
	INC_no_flags(BC);
}

void CPU::OPCode_13()
{
	INC_no_flags(DE);
}

void CPU::OPCode_23()
{
	INC_no_flags(HL);
}

void CPU::OPCode_33()
{
	INC_no_flags(SP);
}

void CPU::OPCode_0B()
{
	DEC_no_flags(BC);
}

void CPU::OPCode_1B()
{
	DEC_no_flags(DE);
}

void CPU::OPCode_2B()
{
	DEC_no_flags(HL);
}

void CPU::OPCode_3B()
{
	DEC_no_flags(SP);
}

void CPU::OPCode_27()
{
	DAA();
}

void CPU::OPCode_2F()
{
	CPL();
}

void CPU::OPCode_3F()
{
	CCF();
}

void CPU::OPCode_37()
{
	SCF();
}

void CPU::OPCode_00()
{
	_NOP();
}

void CPU::OPCode_76()
{
	HALT();
}

void CPU::OPCode_10()
{
	STOP();
}

void CPU::OPCode_F3()
{
	DI();
}

void CPU::OPCode_FB()
{
	EI();
}

void CPU::OPCode_07()
{
	RLC_n(A);
}

void CPU::OPCode_17()
{
	RL_n(A);
}

void CPU::OPCode_0F()
{
	RRC_n(A);
}

void CPU::OPCode_1F()
{
	RR_n(A);
}

void CPU::OPCode_C3()
{
	JP_nn();
}

void CPU::OPCode_C2()
{
	JP_cc_nn(JumpConditions::NZ);
}

void CPU::OPCode_CA()
{
	word addr = readNextWord();
	JP_cc_nn(JumpConditions::Z);
}

void CPU::OPCode_D2()
{
	word addr = readNextWord();
	JP_cc_nn(JumpConditions::NC);
}

void CPU::OPCode_DA()
{
	word addr = readNextWord();
	JP_cc_nn(JumpConditions::C);
}

void CPU::OPCode_E9()
{
	JP_hl();
}

void CPU::OPCode_18()
{
	JR_n();
}

void CPU::OPCode_20()
{
	JR_cc_n(JumpConditions::NZ);
}

void CPU::OPCode_28()
{
	JR_cc_n(JumpConditions::Z);
}

void CPU::OPCode_30()
{
	JR_cc_n(JumpConditions::NC);
}

void CPU::OPCode_38()
{
	JR_cc_n(JumpConditions::C);
}

void CPU::OPCode_CD()
{
	CALL_nn();
}

void CPU::OPCode_C4()
{
	CALL_cc_nn(JumpConditions::NZ);
}

void CPU::OPCode_CC()
{
	CALL_cc_nn(JumpConditions::Z);
}

void CPU::OPCode_D4()
{
	CALL_cc_nn(JumpConditions::NC);
}

void CPU::OPCode_DC()
{
	CALL_cc_nn(JumpConditions::C);
}

void CPU::OPCode_C7()
{
	RST_n(0x00);
}

void CPU::OPCode_CF()
{
	RST_n(0x08);
}

void CPU::OPCode_D7()
{
	RST_n(0x10);
}

void CPU::OPCode_DF()
{
	RST_n(0x18);
}

void CPU::OPCode_E7()
{
	RST_n(0x20);
}

void CPU::OPCode_EF()
{
	RST_n(0x28);
}

void CPU::OPCode_F7()
{
	RST_n(0x30);
}

void CPU::OPCode_FF()
{
	RST_n(0x38);
}

void CPU::OPCode_C9()
{
	RET();
}

void CPU::OPCode_C0()
{
	RET_cc(JumpConditions::NZ);
}

void CPU::OPCode_C8()
{
	RET_cc(JumpConditions::Z);
}

void CPU::OPCode_D0()
{
	RET_cc(JumpConditions::NC);
}

void CPU::OPCode_D8()
{
	RET_cc(JumpConditions::C);
}

void CPU::OPCode_D9()
{
	RETI();
}

void CPU::OPCode_CB_37()
{
	SWAP(A);
}

void CPU::OPCode_CB_30()
{
	SWAP(B);
}

void CPU::OPCode_CB_31()
{
	SWAP(C);
}

void CPU::OPCode_CB_32()
{
	SWAP(D);
}

void CPU::OPCode_CB_33()
{
	SWAP(E);
}

void CPU::OPCode_CB_34()
{
	SWAP(H);
}

void CPU::OPCode_CB_35()
{
	SWAP(L);
}

void CPU::OPCode_CB_36()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	ByteRegister temp(val);
	SWAP(temp);
	val = static_cast<byte>(temp.GetValue());
	m_MMU.Write(addr, val);
}

void CPU::OPCode_CB_07()
{
	RLC_n(A);
}

void CPU::OPCode_CB_00()
{
	RLC_n(B);
}

void CPU::OPCode_CB_01()
{
	RLC_n(C);
}

void CPU::OPCode_CB_02()
{
	RLC_n(D);
}

void CPU::OPCode_CB_03()
{
	RLC_n(E);
}

void CPU::OPCode_CB_04()
{
	RLC_n(H);
}

void CPU::OPCode_CB_05()
{
	RLC_n(L);
}

void CPU::OPCode_CB_06()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	ByteRegister temp(val);
	RLC_n(temp);
	val = static_cast<byte>(temp.GetValue());
	m_MMU.Write(addr, val);
}

void CPU::OPCode_CB_17()
{
	RL_n(A);
}

void CPU::OPCode_CB_10()
{
	RL_n(B);
}

void CPU::OPCode_CB_11()
{
	RL_n(C);
}

void CPU::OPCode_CB_12()
{
	RL_n(D);
}

void CPU::OPCode_CB_13()
{
	RL_n(E);
}

void CPU::OPCode_CB_14()
{
	RL_n(H);
}

void CPU::OPCode_CB_15()
{
	RL_n(L);
}

void CPU::OPCode_CB_16()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	ByteRegister temp(val);
	RL_n(temp);
	val = static_cast<byte>(temp.GetValue());
	m_MMU.Write(addr, val);
}

void CPU::OPCode_CB_0F()
{
	RRC_n(A);
}

void CPU::OPCode_CB_08()
{
	RRC_n(B);
}

void CPU::OPCode_CB_09()
{
	RRC_n(C);
}

void CPU::OPCode_CB_0A()
{
	RRC_n(D);
}

void CPU::OPCode_CB_0B()
{
	RRC_n(E);
}

void CPU::OPCode_CB_0C()
{
	RRC_n(H);
}

void CPU::OPCode_CB_0D()
{
	RRC_n(L);
}

void CPU::OPCode_CB_0E()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	ByteRegister temp(val);
	RRC_n(temp);
	val = static_cast<byte>(temp.GetValue());
	m_MMU.Write(addr, val);
}

void CPU::OPCode_CB_1F()
{
	RR_n(A);
}

void CPU::OPCode_CB_18()
{
	RR_n(B);
}

void CPU::OPCode_CB_19()
{
	RR_n(C);
}

void CPU::OPCode_CB_1A()
{
	RR_n(D);
}

void CPU::OPCode_CB_1B()
{
	RR_n(E);
}

void CPU::OPCode_CB_1C()
{
	RR_n(H);
}

void CPU::OPCode_CB_1D()
{
	RR_n(L);
}

void CPU::OPCode_CB_1E()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	ByteRegister temp(val);
	RR_n(temp);
	val = static_cast<byte>(temp.GetValue());
	m_MMU.Write(addr, val);
}

void CPU::OPCode_CB_27()
{
	SLA_n(A);
}

void CPU::OPCode_CB_20()
{
	SLA_n(B);
}

void CPU::OPCode_CB_21()
{
	SLA_n(C);
}

void CPU::OPCode_CB_22()
{
	SLA_n(D);
}

void CPU::OPCode_CB_23()
{
	SLA_n(E);
}

void CPU::OPCode_CB_24()
{
	SLA_n(H);
}

void CPU::OPCode_CB_25()
{
	SLA_n(L);
}

void CPU::OPCode_CB_26()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	ByteRegister temp(val);
	SLA_n(temp);
	val = static_cast<byte>(temp.GetValue());
	m_MMU.Write(addr, val);
}

void CPU::OPCode_CB_2F()
{
	SRA_n(A);
}

void CPU::OPCode_CB_28()
{
	SRA_n(B);
}

void CPU::OPCode_CB_29()
{
	SRA_n(C);
}

void CPU::OPCode_CB_2A()
{
	SRA_n(D);
}

void CPU::OPCode_CB_2B()
{
	SRA_n(E);
}

void CPU::OPCode_CB_2C()
{
	SRA_n(H);
}

void CPU::OPCode_CB_2D()
{
	SRA_n(L);
}

void CPU::OPCode_CB_2E()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	ByteRegister temp(val);
	SRA_n(temp);
	val = static_cast<byte>(temp.GetValue());
	m_MMU.Write(addr, val);
}

void CPU::OPCode_CB_3F()
{
	SRL_n(A);
}

void CPU::OPCode_CB_38()
{
	SRL_n(B);
}

void CPU::OPCode_CB_39()
{
	SRL_n(C);
}

void CPU::OPCode_CB_3A()
{
	SRL_n(D);
}

void CPU::OPCode_CB_3B()
{
	SRL_n(E);
}

void CPU::OPCode_CB_3C()
{
	SRL_n(H);
}

void CPU::OPCode_CB_3D()
{
	SRL_n(L);
}

void CPU::OPCode_CB_3E()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	ByteRegister temp(val);
	SRL_n(temp);
	val = static_cast<byte>(temp.GetValue());
	m_MMU.Write(addr, val);
}

void CPU::OPCode_CB_47()
{
	byte val = static_cast<byte>(A.GetValue());
	BIT_b_r(0, val);
}

void CPU::OPCode_CB_40()
{
	byte val = static_cast<byte>(B.GetValue());
	BIT_b_r(0, val);
}

void CPU::OPCode_CB_41()
{
	byte val = static_cast<byte>(C.GetValue());
	BIT_b_r(0, val);
}

void CPU::OPCode_CB_42()
{
	byte val = static_cast<byte>(D.GetValue());
	BIT_b_r(0, val);
}

void CPU::OPCode_CB_43()
{
	byte val = static_cast<byte>(E.GetValue());
	BIT_b_r(0, val);
}

void CPU::OPCode_CB_44()
{
	byte val = static_cast<byte>(H.GetValue());
	BIT_b_r(0, val);
}

void CPU::OPCode_CB_45()
{
	byte val = static_cast<byte>(L.GetValue());
	BIT_b_r(0, val);
}

void CPU::OPCode_CB_46()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	BIT_b_r(0, val);
}

void CPU::OPCode_CB_4F()
{
	byte val = static_cast<byte>(A.GetValue());
	BIT_b_r(1, val);
}

void CPU::OPCode_CB_48()
{
	byte val = static_cast<byte>(B.GetValue());
	BIT_b_r(1, val);
}

void CPU::OPCode_CB_49()
{
	byte val = static_cast<byte>(C.GetValue());
	BIT_b_r(1, val);
}

void CPU::OPCode_CB_4A()
{
	byte val = static_cast<byte>(D.GetValue());
	BIT_b_r(1, val);
}

void CPU::OPCode_CB_4B()
{
	byte val = static_cast<byte>(E.GetValue());
	BIT_b_r(1, val);
}

void CPU::OPCode_CB_4C()
{
	byte val = static_cast<byte>(H.GetValue());
	BIT_b_r(1, val);
}

void CPU::OPCode_CB_4D()
{
	byte val = static_cast<byte>(L.GetValue());
	BIT_b_r(1, val);
}

void CPU::OPCode_CB_4E()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	BIT_b_r(1, val);
}

void CPU::OPCode_CB_57()
{
	byte val = static_cast<byte>(A.GetValue());
	BIT_b_r(2, val);
}

void CPU::OPCode_CB_50()
{
	byte val = static_cast<byte>(B.GetValue());
	BIT_b_r(2, val);
}

void CPU::OPCode_CB_51()
{
	byte val = static_cast<byte>(C.GetValue());
	BIT_b_r(2, val);
}

void CPU::OPCode_CB_52()
{
	byte val = static_cast<byte>(D.GetValue());
	BIT_b_r(2, val);
}

void CPU::OPCode_CB_53()
{
	byte val = static_cast<byte>(E.GetValue());
	BIT_b_r(2, val);
}

void CPU::OPCode_CB_54()
{
	byte val = static_cast<byte>(H.GetValue());
	BIT_b_r(2, val);
}

void CPU::OPCode_CB_55()
{
	byte val = static_cast<byte>(L.GetValue());
	BIT_b_r(2, val);
}

void CPU::OPCode_CB_56()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	BIT_b_r(2, val);
}

void CPU::OPCode_CB_5F()
{
	byte val = static_cast<byte>(A.GetValue());
	BIT_b_r(3, val);
}

void CPU::OPCode_CB_58()
{
	byte val = static_cast<byte>(B.GetValue());
	BIT_b_r(3, val);
}

void CPU::OPCode_CB_59()
{
	byte val = static_cast<byte>(C.GetValue());
	BIT_b_r(3, val);
}

void CPU::OPCode_CB_5A()
{
	byte val = static_cast<byte>(D.GetValue());
	BIT_b_r(3, val);
}

void CPU::OPCode_CB_5B()
{
	byte val = static_cast<byte>(E.GetValue());
	BIT_b_r(3, val);
}

void CPU::OPCode_CB_5C()
{
	byte val = static_cast<byte>(H.GetValue());
	BIT_b_r(3, val);
}

void CPU::OPCode_CB_5D()
{
	byte val = static_cast<byte>(L.GetValue());
	BIT_b_r(3, val);
}

void CPU::OPCode_CB_5E()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	BIT_b_r(3, val);
}

void CPU::OPCode_CB_67()
{
	byte val = static_cast<byte>(A.GetValue());
	BIT_b_r(4, val);
}

void CPU::OPCode_CB_60()
{
	byte val = static_cast<byte>(B.GetValue());
	BIT_b_r(4, val);
}

void CPU::OPCode_CB_61()
{
	byte val = static_cast<byte>(C.GetValue());
	BIT_b_r(4, val);
}

void CPU::OPCode_CB_62()
{
	byte val = static_cast<byte>(D.GetValue());
	BIT_b_r(4, val);
}

void CPU::OPCode_CB_63()
{
	byte val = static_cast<byte>(E.GetValue());
	BIT_b_r(4, val);
}

void CPU::OPCode_CB_64()
{
	byte val = static_cast<byte>(H.GetValue());
	BIT_b_r(4, val);
}

void CPU::OPCode_CB_65()
{
	byte val = static_cast<byte>(L.GetValue());
	BIT_b_r(4, val);
}

void CPU::OPCode_CB_66()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	BIT_b_r(4, val);
}

void CPU::OPCode_CB_6F()
{
	byte val = static_cast<byte>(A.GetValue());
	BIT_b_r(5, val);
}

void CPU::OPCode_CB_68()
{
	byte val = static_cast<byte>(B.GetValue());
	BIT_b_r(5, val);
}

void CPU::OPCode_CB_69()
{
	byte val = static_cast<byte>(C.GetValue());
	BIT_b_r(5, val);
}

void CPU::OPCode_CB_6A()
{
	byte val = static_cast<byte>(D.GetValue());
	BIT_b_r(5, val);
}

void CPU::OPCode_CB_6B()
{
	byte val = static_cast<byte>(E.GetValue());
	BIT_b_r(5, val);
}

void CPU::OPCode_CB_6C()
{
	byte val = static_cast<byte>(H.GetValue());
	BIT_b_r(5, val);
}

void CPU::OPCode_CB_6D()
{
	byte val = static_cast<byte>(L.GetValue());
	BIT_b_r(5, val);
}

void CPU::OPCode_CB_6E()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	BIT_b_r(5, val);
}

void CPU::OPCode_CB_77()
{
	byte val = static_cast<byte>(A.GetValue());
	BIT_b_r(6, val);
}

void CPU::OPCode_CB_70()
{
	byte val = static_cast<byte>(B.GetValue());
	BIT_b_r(6, val);
}

void CPU::OPCode_CB_71()
{
	byte val = static_cast<byte>(C.GetValue());
	BIT_b_r(6, val);
}

void CPU::OPCode_CB_72()
{
	byte val = static_cast<byte>(D.GetValue());
	BIT_b_r(6, val);
}

void CPU::OPCode_CB_73()
{
	byte val = static_cast<byte>(E.GetValue());
	BIT_b_r(6, val);
}

void CPU::OPCode_CB_74()
{
	byte val = static_cast<byte>(H.GetValue());
	BIT_b_r(6, val);
}

void CPU::OPCode_CB_75()
{
	byte val = static_cast<byte>(L.GetValue());
	BIT_b_r(6, val);
}

void CPU::OPCode_CB_76()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	BIT_b_r(6, val);
}

void CPU::OPCode_CB_7F()
{
	byte val = static_cast<byte>(A.GetValue());
	BIT_b_r(7, val);
}

void CPU::OPCode_CB_78()
{
	byte val = static_cast<byte>(B.GetValue());
	BIT_b_r(7, val);
}

void CPU::OPCode_CB_79()
{
	byte val = static_cast<byte>(C.GetValue());
	BIT_b_r(7, val);
}

void CPU::OPCode_CB_7A()
{
	byte val = static_cast<byte>(D.GetValue());
	BIT_b_r(7, val);
}

void CPU::OPCode_CB_7B()
{
	byte val = static_cast<byte>(E.GetValue());
	BIT_b_r(7, val);
}

void CPU::OPCode_CB_7C()
{
	byte val = static_cast<byte>(H.GetValue());
	BIT_b_r(7, val);
}

void CPU::OPCode_CB_7D()
{
	byte val = static_cast<byte>(L.GetValue());
	BIT_b_r(7, val);
}

void CPU::OPCode_CB_7E()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	BIT_b_r(7, val);
}

void CPU::OPCode_CB_C7()
{
	byte val = static_cast<byte>(A.GetValue());
	SET_b_r(0, val);
	A.SetValue(val);
}

void CPU::OPCode_CB_C0()
{
	byte val = static_cast<byte>(B.GetValue());
	SET_b_r(0, val);
	B.SetValue(val);
}

void CPU::OPCode_CB_C1()
{
	byte val = static_cast<byte>(C.GetValue());
	SET_b_r(0, val);
	C.SetValue(val);
}

void CPU::OPCode_CB_C2()
{
	byte val = static_cast<byte>(D.GetValue());
	SET_b_r(0, val);
	D.SetValue(val);
}

void CPU::OPCode_CB_C3()
{
	byte val = static_cast<byte>(E.GetValue());
	SET_b_r(0, val);
	E.SetValue(val);
}

void CPU::OPCode_CB_C4()
{
	byte val = static_cast<byte>(H.GetValue());
	SET_b_r(0, val);
	H.SetValue(val);
}

void CPU::OPCode_CB_C5()
{
	byte val = static_cast<byte>(L.GetValue());
	SET_b_r(0, val);
	L.SetValue(val);
}

void CPU::OPCode_CB_C6()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	SET_b_r(0, val);
	m_MMU.Write(addr, val);
}

void CPU::OPCode_CB_CF()
{
	byte val = static_cast<byte>(A.GetValue());
	SET_b_r(1, val);
	A.SetValue(val);
}

void CPU::OPCode_CB_C8()
{
	byte val = static_cast<byte>(B.GetValue());
	SET_b_r(1, val);
	B.SetValue(val);
}

void CPU::OPCode_CB_C9()
{
	byte val = static_cast<byte>(C.GetValue());
	SET_b_r(1, val);
	C.SetValue(val);
}

void CPU::OPCode_CB_CA()
{
	byte val = static_cast<byte>(D.GetValue());
	SET_b_r(1, val);
	D.SetValue(val);
}

void CPU::OPCode_CB_CB()
{
	byte val = static_cast<byte>(E.GetValue());
	SET_b_r(1, val);
	E.SetValue(val);
}

void CPU::OPCode_CB_CC()
{
	byte val = static_cast<byte>(H.GetValue());
	SET_b_r(1, val);
	H.SetValue(val);
}

void CPU::OPCode_CB_CD()
{
	byte val = static_cast<byte>(L.GetValue());
	SET_b_r(1, val);
	L.SetValue(val);
}

void CPU::OPCode_CB_CE()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	SET_b_r(1, val);
	m_MMU.Write(addr, val);
}

void CPU::OPCode_CB_D7()
{
	byte val = static_cast<byte>(A.GetValue());
	SET_b_r(2, val);
	A.SetValue(val);
}

void CPU::OPCode_CB_D0()
{
	byte val = static_cast<byte>(B.GetValue());
	SET_b_r(2, val);
	B.SetValue(val);
}

void CPU::OPCode_CB_D1()
{
	byte val = static_cast<byte>(C.GetValue());
	SET_b_r(2, val);
	C.SetValue(val);
}

void CPU::OPCode_CB_D2()
{
	byte val = static_cast<byte>(D.GetValue());
	SET_b_r(2, val);
	D.SetValue(val);
}

void CPU::OPCode_CB_D3()
{
	byte val = static_cast<byte>(E.GetValue());
	SET_b_r(2, val);
	E.SetValue(val);
}

void CPU::OPCode_CB_D4()
{
	byte val = static_cast<byte>(H.GetValue());
	SET_b_r(2, val);
	H.SetValue(val);
}

void CPU::OPCode_CB_D5()
{
	byte val = static_cast<byte>(L.GetValue());
	SET_b_r(2, val);
	L.SetValue(val);
}

void CPU::OPCode_CB_D6()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	SET_b_r(2, val);
	m_MMU.Write(addr, val);
}

void CPU::OPCode_CB_DF()
{
	byte val = static_cast<byte>(A.GetValue());
	SET_b_r(3, val);
	A.SetValue(val);
}

void CPU::OPCode_CB_D8()
{
	byte val = static_cast<byte>(B.GetValue());
	SET_b_r(3, val);
	B.SetValue(val);
}

void CPU::OPCode_CB_D9()
{
	byte val = static_cast<byte>(C.GetValue());
	SET_b_r(3, val);
	C.SetValue(val);
}

void CPU::OPCode_CB_DA()
{
	byte val = static_cast<byte>(D.GetValue());
	SET_b_r(3, val);
	D.SetValue(val);
}

void CPU::OPCode_CB_DB()
{
	byte val = static_cast<byte>(E.GetValue());
	SET_b_r(3, val);
	E.SetValue(val);
}

void CPU::OPCode_CB_DC()
{
	byte val = static_cast<byte>(H.GetValue());
	SET_b_r(3, val);
	H.SetValue(val);
}

void CPU::OPCode_CB_DD()
{
	byte val = static_cast<byte>(L.GetValue());
	SET_b_r(3, val);
	L.SetValue(val);
}

void CPU::OPCode_CB_DE()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	SET_b_r(3, val);
	m_MMU.Write(addr, val);
}

void CPU::OPCode_CB_E7()
{
	byte val = static_cast<byte>(A.GetValue());
	SET_b_r(4, val);
	A.SetValue(val);
}

void CPU::OPCode_CB_E0()
{
	byte val = static_cast<byte>(B.GetValue());
	SET_b_r(4, val);
	B.SetValue(val);
}

void CPU::OPCode_CB_E1()
{
	byte val = static_cast<byte>(C.GetValue());
	SET_b_r(4, val);
	C.SetValue(val);
}

void CPU::OPCode_CB_E2()
{
	byte val = static_cast<byte>(D.GetValue());
	SET_b_r(4, val);
	D.SetValue(val);
}

void CPU::OPCode_CB_E3()
{
	byte val = static_cast<byte>(E.GetValue());
	SET_b_r(4, val);
	E.SetValue(val);
}

void CPU::OPCode_CB_E4()
{
	byte val = static_cast<byte>(H.GetValue());
	SET_b_r(4, val);
	H.SetValue(val);
}

void CPU::OPCode_CB_E5()
{
	byte val = static_cast<byte>(L.GetValue());
	SET_b_r(4, val);
	L.SetValue(val);
}

void CPU::OPCode_CB_E6()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	SET_b_r(4, val);
	m_MMU.Write(addr, val);
}

void CPU::OPCode_CB_EF()
{
	byte val = static_cast<byte>(A.GetValue());
	SET_b_r(5, val);
	A.SetValue(val);
}

void CPU::OPCode_CB_E8()
{
	byte val = static_cast<byte>(B.GetValue());
	SET_b_r(5, val);
	B.SetValue(val);
}

void CPU::OPCode_CB_E9()
{
	byte val = static_cast<byte>(C.GetValue());
	SET_b_r(5, val);
	C.SetValue(val);
}

void CPU::OPCode_CB_EA()
{
	byte val = static_cast<byte>(D.GetValue());
	SET_b_r(5, val);
	D.SetValue(val);
}

void CPU::OPCode_CB_EB()
{
	byte val = static_cast<byte>(E.GetValue());
	SET_b_r(5, val);
	E.SetValue(val);
}

void CPU::OPCode_CB_EC()
{
	byte val = static_cast<byte>(H.GetValue());
	SET_b_r(5, val);
	H.SetValue(val);
}

void CPU::OPCode_CB_ED()
{
	byte val = static_cast<byte>(L.GetValue());
	SET_b_r(5, val);
	L.SetValue(val);
}

void CPU::OPCode_CB_EE()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	SET_b_r(5, val);
	m_MMU.Write(addr, val);
}

void CPU::OPCode_CB_F7()
{
	byte val = static_cast<byte>(A.GetValue());
	SET_b_r(6, val);
	A.SetValue(val);
}

void CPU::OPCode_CB_F0()
{
	byte val = static_cast<byte>(B.GetValue());
	SET_b_r(6, val);
	B.SetValue(val);
}

void CPU::OPCode_CB_F1()
{
	byte val = static_cast<byte>(C.GetValue());
	SET_b_r(6, val);
	C.SetValue(val);
}

void CPU::OPCode_CB_F2()
{
	byte val = static_cast<byte>(D.GetValue());
	SET_b_r(6, val);
	D.SetValue(val);
}

void CPU::OPCode_CB_F3()
{
	byte val = static_cast<byte>(E.GetValue());
	SET_b_r(6, val);
	E.SetValue(val);
}

void CPU::OPCode_CB_F4()
{
	byte val = static_cast<byte>(H.GetValue());
	SET_b_r(6, val);
	H.SetValue(val);
}

void CPU::OPCode_CB_F5()
{
	byte val = static_cast<byte>(L.GetValue());
	SET_b_r(6, val);
	L.SetValue(val);
}

void CPU::OPCode_CB_F6()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	SET_b_r(6, val);
	m_MMU.Write(addr, val);
}

void CPU::OPCode_CB_FF()
{
	byte val = static_cast<byte>(A.GetValue());
	SET_b_r(7, val);
	A.SetValue(val);
}

void CPU::OPCode_CB_F8()
{
	byte val = static_cast<byte>(B.GetValue());
	SET_b_r(7, val);
	B.SetValue(val);
}

void CPU::OPCode_CB_F9()
{
	byte val = static_cast<byte>(C.GetValue());
	SET_b_r(7, val);
	C.SetValue(val);
}

void CPU::OPCode_CB_FA()
{
	byte val = static_cast<byte>(D.GetValue());
	SET_b_r(7, val);
	D.SetValue(val);
}

void CPU::OPCode_CB_FB()
{
	byte val = static_cast<byte>(E.GetValue());
	SET_b_r(7, val);
	E.SetValue(val);
}

void CPU::OPCode_CB_FC()
{
	byte val = static_cast<byte>(H.GetValue());
	SET_b_r(7, val);
	H.SetValue(val);
}

void CPU::OPCode_CB_FD()
{
	byte val = static_cast<byte>(L.GetValue());
	SET_b_r(7, val);
	L.SetValue(val);
}

void CPU::OPCode_CB_FE()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	SET_b_r(7, val);
	m_MMU.Write(addr, val);
}

void CPU::OPCode_CB_87()
{
	byte val = static_cast<byte>(A.GetValue());
	RES_b_r(0, val);
	A.SetValue(val);
}

void CPU::OPCode_CB_80()
{
	byte val = static_cast<byte>(B.GetValue());
	RES_b_r(0, val);
	B.SetValue(val);
}

void CPU::OPCode_CB_81()
{
	byte val = static_cast<byte>(C.GetValue());
	RES_b_r(0, val);
	C.SetValue(val);
}

void CPU::OPCode_CB_82()
{
	byte val = static_cast<byte>(D.GetValue());
	RES_b_r(0, val);
	D.SetValue(val);
}

void CPU::OPCode_CB_83()
{
	byte val = static_cast<byte>(E.GetValue());
	RES_b_r(0, val);
	E.SetValue(val);
}

void CPU::OPCode_CB_84()
{
	byte val = static_cast<byte>(H.GetValue());
	RES_b_r(0, val);
	H.SetValue(val);
}

void CPU::OPCode_CB_85()
{
	byte val = static_cast<byte>(L.GetValue());
	RES_b_r(0, val);
	L.SetValue(val);
}

void CPU::OPCode_CB_86()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	RES_b_r(0, val);
	m_MMU.Write(addr, val);
}

void CPU::OPCode_CB_8F()
{
	byte val = static_cast<byte>(A.GetValue());
	RES_b_r(1, val);
	A.SetValue(val);
}

void CPU::OPCode_CB_88()
{
	byte val = static_cast<byte>(B.GetValue());
	RES_b_r(1, val);
	B.SetValue(val);
}

void CPU::OPCode_CB_89()
{
	byte val = static_cast<byte>(C.GetValue());
	RES_b_r(1, val);
	C.SetValue(val);
}

void CPU::OPCode_CB_8A()
{
	byte val = static_cast<byte>(D.GetValue());
	RES_b_r(1, val);
	D.SetValue(val);
}

void CPU::OPCode_CB_8B()
{
	byte val = static_cast<byte>(E.GetValue());
	RES_b_r(1, val);
	E.SetValue(val);
}

void CPU::OPCode_CB_8C()
{
	byte val = static_cast<byte>(H.GetValue());
	RES_b_r(1, val);
	H.SetValue(val);
}

void CPU::OPCode_CB_8D()
{
	byte val = static_cast<byte>(L.GetValue());
	RES_b_r(1, val);
	L.SetValue(val);
}

void CPU::OPCode_CB_8E()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	RES_b_r(1, val);
	m_MMU.Write(addr, val);
}

void CPU::OPCode_CB_97()
{
	byte val = static_cast<byte>(A.GetValue());
	RES_b_r(2, val);
	A.SetValue(val);
}

void CPU::OPCode_CB_90()
{
	byte val = static_cast<byte>(B.GetValue());
	RES_b_r(2, val);
	B.SetValue(val);
}

void CPU::OPCode_CB_91()
{
	byte val = static_cast<byte>(C.GetValue());
	RES_b_r(2, val);
	C.SetValue(val);
}

void CPU::OPCode_CB_92()
{
	byte val = static_cast<byte>(D.GetValue());
	RES_b_r(2, val);
	D.SetValue(val);
}

void CPU::OPCode_CB_93()
{
	byte val = static_cast<byte>(E.GetValue());
	RES_b_r(2, val);
	E.SetValue(val);
}

void CPU::OPCode_CB_94()
{
	byte val = static_cast<byte>(H.GetValue());
	RES_b_r(2, val);
	H.SetValue(val);
}

void CPU::OPCode_CB_95()
{
	byte val = static_cast<byte>(L.GetValue());
	RES_b_r(2, val);
	L.SetValue(val);
}

void CPU::OPCode_CB_96()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	RES_b_r(2, val);
	m_MMU.Write(addr, val);
}

void CPU::OPCode_CB_9F()
{
	byte val = static_cast<byte>(A.GetValue());
	RES_b_r(3, val);
	A.SetValue(val);
}

void CPU::OPCode_CB_98()
{
	byte val = static_cast<byte>(B.GetValue());
	RES_b_r(3, val);
	B.SetValue(val);
}

void CPU::OPCode_CB_99()
{
	byte val = static_cast<byte>(C.GetValue());
	RES_b_r(3, val);
	C.SetValue(val);
}

void CPU::OPCode_CB_9A()
{
	byte val = static_cast<byte>(D.GetValue());
	RES_b_r(3, val);
	D.SetValue(val);
}

void CPU::OPCode_CB_9B()
{
	byte val = static_cast<byte>(E.GetValue());
	RES_b_r(3, val);
	E.SetValue(val);
}

void CPU::OPCode_CB_9C()
{
	byte val = static_cast<byte>(H.GetValue());
	RES_b_r(3, val);
	H.SetValue(val);
}

void CPU::OPCode_CB_9D()
{
	byte val = static_cast<byte>(L.GetValue());
	RES_b_r(3, val);
	L.SetValue(val);
}

void CPU::OPCode_CB_9E()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	RES_b_r(3, val);
	m_MMU.Write(addr, val);
}

void CPU::OPCode_CB_A7()
{
	byte val = static_cast<byte>(A.GetValue());
	RES_b_r(4, val);
	A.SetValue(val);
}

void CPU::OPCode_CB_A0()
{
	byte val = static_cast<byte>(B.GetValue());
	RES_b_r(4, val);
	B.SetValue(val);
}

void CPU::OPCode_CB_A1()
{
	byte val = static_cast<byte>(C.GetValue());
	RES_b_r(4, val);
	C.SetValue(val);
}

void CPU::OPCode_CB_A2()
{
	byte val = static_cast<byte>(D.GetValue());
	RES_b_r(4, val);
	D.SetValue(val);
}

void CPU::OPCode_CB_A3()
{
	byte val = static_cast<byte>(E.GetValue());
	RES_b_r(4, val);
	E.SetValue(val);
}

void CPU::OPCode_CB_A4()
{
	byte val = static_cast<byte>(H.GetValue());
	RES_b_r(4, val);
	H.SetValue(val);
}

void CPU::OPCode_CB_A5()
{
	byte val = static_cast<byte>(L.GetValue());
	RES_b_r(4, val);
	L.SetValue(val);
}

void CPU::OPCode_CB_A6()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	RES_b_r(4, val);
	m_MMU.Write(addr, val);
}

void CPU::OPCode_CB_AF()
{
	byte val = static_cast<byte>(A.GetValue());
	RES_b_r(5, val);
	A.SetValue(val);
}

void CPU::OPCode_CB_A8()
{
	byte val = static_cast<byte>(B.GetValue());
	RES_b_r(5, val);
	B.SetValue(val);
}

void CPU::OPCode_CB_A9()
{
	byte val = static_cast<byte>(C.GetValue());
	RES_b_r(5, val);
	C.SetValue(val);
}

void CPU::OPCode_CB_AA()
{
	byte val = static_cast<byte>(D.GetValue());
	RES_b_r(5, val);
	D.SetValue(val);
}

void CPU::OPCode_CB_AB()
{
	byte val = static_cast<byte>(E.GetValue());
	RES_b_r(5, val);
	E.SetValue(val);
}

void CPU::OPCode_CB_AC()
{
	byte val = static_cast<byte>(H.GetValue());
	RES_b_r(5, val);
	H.SetValue(val);
}

void CPU::OPCode_CB_AD()
{
	byte val = static_cast<byte>(L.GetValue());
	RES_b_r(5, val);
	L.SetValue(val);
}

void CPU::OPCode_CB_AE()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	RES_b_r(5, val);
	m_MMU.Write(addr, val);
}

void CPU::OPCode_CB_B7()
{
	byte val = static_cast<byte>(A.GetValue());
	RES_b_r(6, val);
	A.SetValue(val);
}

void CPU::OPCode_CB_B0()
{
	byte val = static_cast<byte>(B.GetValue());
	RES_b_r(6, val);
	B.SetValue(val);
}

void CPU::OPCode_CB_B1()
{
	byte val = static_cast<byte>(C.GetValue());
	RES_b_r(6, val);
	C.SetValue(val);
}

void CPU::OPCode_CB_B2()
{
	byte val = static_cast<byte>(D.GetValue());
	RES_b_r(6, val);
	D.SetValue(val);
}

void CPU::OPCode_CB_B3()
{
	byte val = static_cast<byte>(E.GetValue());
	RES_b_r(6, val);
	E.SetValue(val);
}

void CPU::OPCode_CB_B4()
{
	byte val = static_cast<byte>(H.GetValue());
	RES_b_r(6, val);
	H.SetValue(val);
}

void CPU::OPCode_CB_B5()
{
	byte val = static_cast<byte>(L.GetValue());
	RES_b_r(6, val);
	L.SetValue(val);
}

void CPU::OPCode_CB_B6()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	RES_b_r(6, val);
	m_MMU.Write(addr, val);
}

void CPU::OPCode_CB_BF()
{
	byte val = static_cast<byte>(A.GetValue());
	RES_b_r(7, val);
	A.SetValue(val);
}

void CPU::OPCode_CB_B8()
{
	byte val = static_cast<byte>(B.GetValue());
	RES_b_r(7, val);
	B.SetValue(val);
}

void CPU::OPCode_CB_B9()
{
	byte val = static_cast<byte>(C.GetValue());
	RES_b_r(7, val);
	C.SetValue(val);
}

void CPU::OPCode_CB_BA()
{
	byte val = static_cast<byte>(D.GetValue());
	RES_b_r(7, val);
	D.SetValue(val);
}

void CPU::OPCode_CB_BB()
{
	byte val = static_cast<byte>(E.GetValue());
	RES_b_r(7, val);
	E.SetValue(val);
}

void CPU::OPCode_CB_BC()
{
	byte val = static_cast<byte>(H.GetValue());
	RES_b_r(7, val);
	H.SetValue(val);
}

void CPU::OPCode_CB_BD()
{
	byte val = static_cast<byte>(L.GetValue());
	RES_b_r(7, val);
	L.SetValue(val);
}

void CPU::OPCode_CB_BE()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	RES_b_r(7, val);
	m_MMU.Write(addr, val);
}


/* debug methods */

/* simple registers dump into stdout */
void CPU::dumpRegisters()
{
	cout << "Registers dump:" << endl;
	cout << "A: 0x" << std::hex << std::setfill('0') << std::setw(2) << A.GetValue() << " | F: 0x" << std::hex << std::setfill('0') << std::setw(2) << F.GetValue() << " | AF: 0x" << std::hex << std::setfill('0') << std::setw(4) << AF.GetValue() << endl;
	cout << "B: 0x" << std::hex << std::setfill('0') << std::setw(2) << B.GetValue() << " | C: 0x" << std::hex << std::setfill('0') << std::setw(2) << C.GetValue() << " | BC: 0x" << std::hex << std::setfill('0') << std::setw(4) << BC.GetValue() << endl;
	cout << "D: 0x" << std::hex << std::setfill('0') << std::setw(2) << D.GetValue() << " | E: 0x" << std::hex << std::setfill('0') << std::setw(2) << E.GetValue() << " | DE: 0x" << std::hex << std::setfill('0') << std::setw(4) << DE.GetValue() << endl;
	cout << "H: 0x" << std::hex << std::setfill('0') << std::setw(2) << H.GetValue() << " | L: 0x" << std::hex << std::setfill('0') << std::setw(2) << L.GetValue() << " | HL: 0x" << std::hex << std::setfill('0') << std::setw(4) << HL.GetValue() << endl;
	cout << "------------------------------" << endl;
	cout << "PC: 0x" << std::hex << std::setfill('0') << std::setw(4) << PC.GetValue() << " | SP: 0x" << std::hex << std::setfill('0') << std::setw(4) << SP.GetValue() << " | (HL): 0x" << std::hex << std::setfill('0') << std::setw(2) << static_cast<word>(m_MMU.Read(HL.GetValue())) << endl;
	cout << "------------------------------" << endl;
	cout << "Z: " << F.GetZ() << " | N: " << F.GetN() << " | H: " << F.GetH() << " | C: " << F.GetC() << endl;
}