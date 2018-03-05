#include "octalword.h"
#include "constants.h"
#include "types.h"

OctalWord::OctalWord()
{
	value = 0; 
	for (int i = 0; i < WORD_OCTAL_LENGTH; i++) 
	{ 
		octbit[i].b = 0; 
	}
}

OctalWord::OctalWord(int val)
{
	value = val;
	for (int i = 0; i < WORD_OCTAL_LENGTH; i++)
	{
		int leftshift = val << ((WORD_OCTAL_LENGTH - i - 1) * 3);
		octbit[i].b = leftshift >> ((WORD_OCTAL_LENGTH - 1) * 3);
	}
}

OctalWord::~OctalWord()
{
}

void OctalWord::updateBits()
{
	for (int i = 0; i < WORD_OCTAL_LENGTH; i++)
	{
		int leftshift = this->value << ((WORD_OCTAL_LENGTH - i - 1) * 3);
		this->octbit[i].b = leftshift >> ((WORD_OCTAL_LENGTH - 1) * 3);
	}
}


#pragma region UNARY_OP_OVERLOAD
// twos compliment
OctalWord OctalWord::operator-()
{
	OctalWord result = OctalWord(-(this->value));
	return result;
}

// negate?
OctalWord OctalWord::operator~()
{
	OctalWord result = OctalWord(~(this->value));
	return result;
}
#pragma endregion

#pragma region BINARY_OP_OVERLOAD
OctalWord OctalWord::operator+(const OctalWord& b)
{
	int newValue = this->value + b.value;
	OctalWord sum = OctalWord(newValue);
	sum.overflow = (newValue > MAX_OCTAL_VALUE);
	return sum;
}

OctalWord OctalWord::operator+(const int b)
{
	return (*this + OctalWord(b));
}

OctalWord OctalWord::operator-(const OctalWord& b)
{
	return (*this + (-b));
}

OctalWord OctalWord::operator-(const int b)
{
	return (*this + ( -( OctalWord(b) ) ) );
}

OctalWord OctalWord::operator|(const OctalWord & rhs)
{
	return OctalWord(this->value | rhs.value);
}

OctalWord OctalWord::operator|(const int rhs)
{
	return OctalWord(this->value | rhs);
}

OctalWord OctalWord::operator&(const OctalWord & rhs)
{
	return OctalWord(this->value & rhs.value);
}

OctalWord OctalWord::operator&(const int rhs)
{
	return OctalWord(this->value & value);
}
#pragma endregion

#pragma region ASSIGNMENT_OP_OVERLOAD
OctalWord& OctalWord::operator=(const OctalWord &newAssignment)
{
	if (this != &newAssignment)
	{
		this->value = newAssignment.value;
		this->updateBits();
		this->updateBinary();
	}
	return *this;
}
#pragma endregion

#pragma region SHIFT_OP_OVERLOAD
OctalWord OctalWord::operator<<(const OctalWord& shiftAmount)
{
	return this* << shiftAmount.value;
}

OctalWord OctalWord::operator<<(const int shiftAmount)
{
	int result = this->value;
	bool overflowflag = false;
	for (int i = 0; i < shiftAmount; i++)
	{
		overflowflag = ( OctalWord(result).octbit[5] > 0) ? true : false;
		result = result << 1;
	}
	OctalWord ret = OctalWord(result);
	ret.overflow = overflowflag;
	return ret;
}

OctalWord OctalWord::operator>>(const OctalWord& shiftAmount)
{
	return this* >> shiftAmount.value;
}

OctalWord OctalWord::operator>>(const int shiftAmount)
{
	int result = this->value;
	bool overflowflag = false;
	for (int i = 0; i < shiftAmount; i++)
	{
		overflowflag = (OctalWord().octbit[0].b % 2 > 0) ? true : false;
		result = result >> 1;
	}
	OctalWord ret = OctalWord(result);
	ret.overflow = overflowflag;
	return ret;
}
#pragma endregion

#pragma region RELATIONAL_OP_OVERLOAD
bool OctalWord::operator<(const OctalWord& oneToCompareTo)
{
	return this->value < oneToCompareTo.value;
}

bool OctalWord::operator<=(const OctalWord& oneToCompareTo)
{
	return this->value <= oneToCompareTo.value;
}

bool OctalWord::operator>(const OctalWord& oneToCompareTo)
{
	return this->value > oneToCompareTo.value;
}

bool OctalWord::operator>=(const OctalWord& oneToCompareTo)
{
	return this->value >= oneToCompareTo.value;
}

bool OctalWord::operator==(const OctalWord& oneToCompareTo)
{
	return this->value == oneToCompareTo.value;
}

bool OctalWord::operator!=(const OctalWord& oneToCompareTo)
{
	return this->value != oneToCompareTo.value;
}
#pragma endregion

#pragma region INC_DEC_OP_OVERLOAD
OctalWord OctalWord::operator++()
{
	return *this + 1;
}

OctalWord OctalWord::operator++(int)
{
	return *this + 1;
}

OctalWord OctalWord::operator--()
{
	return *this - 1;
}

OctalWord OctalWord::operator--(int)
{
	return *this -1 ;
}
#pragma endregion