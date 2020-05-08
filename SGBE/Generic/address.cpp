#include "address.h"

WordAddress::WordAddress(uint16_t i_AddressLocation) : m_Value(i_AddressLocation) {}

WordAddress::WordAddress(const WordRegister& i_WordRegister) : m_Value(i_WordRegister.GetValue()) {}

WordAddress::WordAddress(const Pair8BRegisters& i_Pair8BRegisters) : m_Value(i_Pair8BRegisters.GetValue()) {}

uint16_t WordAddress::GetValue() const
{
	return m_Value;
}