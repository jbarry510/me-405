//in some file have this function

uint16_t combine(uint8_t highbyte,uint8_t lowbyte)
{
	uint16_t r = ((uint16_t) highbyte<<8) | ((uint16_t) lowbyte);	//concatenate bytes
	return r
}

//in the main.cpp file have a "main" funtion

int main(void)			//has to return something so put "int", even though we don't return anything
{
	uint8_t DON = 0b10101010;
	uint8_t JIM = 0b11110000;
	uint16_t HARRY = combine(DON,JIM);
	*ptr_to_serial << PMS("combination is") << bin << HARRY << endl)
}