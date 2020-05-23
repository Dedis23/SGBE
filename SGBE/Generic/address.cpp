#include "address.h"

WordAddress::WordAddress(word i_AddressLocation) : m_Value(i_AddressLocation) {}

WordAddress::WordAddress(const WordRegister& i_WordRegister) : m_Value(i_WordRegister.GetValue()) {}

WordAddress::WordAddress(const Pair8BRegisters& i_Pair8BRegisters) : m_Value(i_Pair8BRegisters.GetValue()) {}

word WordAddress::GetValue() const
{
	return m_Value;
}

bool WordAddress::InRange(const WordAddress& i_Lower, const WordAddress& i_Upper) const
{
	return m_Value >= i_Lower.GetValue() && m_Value <= i_Upper.GetValue();
}