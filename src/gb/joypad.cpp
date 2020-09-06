#include "joypad.h"

Joypad::Joypad(GBInternals& i_GBInternals) : m_KeysState(0xFF), m_DirectionSelector(false), m_ButtonSelector(false), m_GBInternals(i_GBInternals) {}

void Joypad::Reset()
{
	m_KeysState = 0xFF;
	m_DirectionSelector = false;
	m_ButtonSelector = false;
}

/* used when the player has pressed the button 
 * note - pressed = false, unpressed = true */
void Joypad::KeyPressed(const GBButtons& i_PressedButton)
{
	if (i_PressedButton == GBButtons::None)
	{
		return;
	}

	int key = (int)i_PressedButton;

	// read the current state of the key before alteration
	bool isKeyPreviouslyPressed = !bitwise::IsBitSet(key, m_KeysState) ? true : false;

	
	// clear the bit of the button - (clear = pressed)
	bitwise::ClearBit(key, m_KeysState);

	// check if the selector of the requested key button-group is raised
	// if so we need to request an interrupt
	bool requestInterrupt = false;
	switch (i_PressedButton)
	{
	case GBButtons::Right:
	case GBButtons::Left:
	case GBButtons::Up:
	case GBButtons::Down:
		requestInterrupt = m_DirectionSelector ? true : false;
		break;
	case GBButtons::A:
	case GBButtons::B:
	case GBButtons::Select:
	case GBButtons::Start:
		requestInterrupt = m_ButtonSelector ? true : false;
		break;
	}

	// request an interrupt (but only if the key wasn't already pressed)
	if (requestInterrupt && !isKeyPreviouslyPressed)
	{
		m_GBInternals.GetCPU().RequestInterrupt(CPU::InterruptType::Joypad);
	}
}

/* used when the player has released the button
 * note - pressed = false, unpressed = true */
void Joypad::KeyReleased(const GBButtons& i_ReleasedButton)
{
	if (i_ReleasedButton == GBButtons::None)
	{
		return;
	}
	
	int key = (int)i_ReleasedButton;

	// set the bit of the button - (set = unpressed)
	bitwise::SetBit(key, m_KeysState);
}

/* used when the game request the joypad state
 * note - pressed = false, unpressed = true */
byte Joypad::GetJoypadState() const
{
	byte joypadState = 0xFF;

	// return the joypad state based on the current selector and keys state
	if (m_DirectionSelector)
	{
		joypadState = (m_KeysState) | 0xF0;
		bitwise::ClearBit(JOYPAD_SELECT_DIRECTION_KEYS_BIT, joypadState);

	}
	else if (m_ButtonSelector)
	{
		joypadState = ((m_KeysState >> 4) & 0xF) | 0xF0;
		bitwise::ClearBit(JOYPAD_SELECT_BUTTON_KEYS_BIT, joypadState);

	}

	return joypadState;
}

/* used when the game alters the joypad state 
 * note - pressed = false, unpressed = true */
void Joypad::SetJoypadState(byte i_Value)
{
	// the only settable values are the selectors
	m_DirectionSelector = !bitwise::IsBitSet(JOYPAD_SELECT_DIRECTION_KEYS_BIT, i_Value) ? true : false;
	m_ButtonSelector = !bitwise::IsBitSet(JOYPAD_SELECT_BUTTON_KEYS_BIT, i_Value) ? true : false;
}