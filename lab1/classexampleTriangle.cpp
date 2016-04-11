//*****triangle.h header file****************************

#ifndef TRIANGLE_H
#define TRIANGLE_H

class Triangle
{ 
	private:
		//no private variables
	protected:
		uint8_t base;
		uint8_t height;
		emstream* ptr_to_serial;

	 public:  
		Triangle(uint8_t myBase, uint8_t myHeight, emstream* p_serial_port); //constructor  
		
		//setters and getters:
		uint8_t getBase(void){return base;}
		void setBase(uint8_t myBase){base = myBase;}
		uint8_t getHeight(){return height;}
		void setHeight(uint8_t myHeight){height = myHeight;}
		
		//calculates area:
		float calculateArea();
};
emstream& operator << (emstream&, Triangle&);
#endif


//*****triangle .CPP file*************************************************

#include "triangle.h"

Triangle::Triangle(uint8_t myBase, uint8_t myHeight, emstream* p_serial_port)
{ 
	base = myBase;
	height = myHeight;
	
	ptr_to_serial = p_serial_port;		//stores local copy of the pointer
	
	*ptr_to_serial << "Triangle Constructor OK" << endl;	//prints a helpful message
}

float Triangle::calculateArea(void)
{ 
	return (.5*base*height);
}

emstream& operator << (emstream& serpt, Triangle& thisTriangle )	// defines operator
{
serpt << "Triangle info:" << endl
		<< "base =" << thisTriangle.base << endl
		<< "height =" << thisTriangle.height << endl
		<< "area =" << thisTriangle.calculateArea(void) <<endl;
		return serpt;
}




//*****main.cpp file*********************************************************
#include "triangle.h"
#include "emstream.h" //for printing out messages

int main()
{  
	rs232 ser_port (9600, 1);								//Creates Serial Port
	
	Triangle* ptrTriangle1 = new Triangle(2,4, &ser_port);			//Defines Triangle1
	Triangle* ptrTriangle2 = new Triangle(0,0, &ser_port); 			//Defines Triangle2
	
	//to set the base and height for triangle 2
	ptrTriangle2->setBase(5);		//or (*ptrTriangle1).setBase(5);  does the same thing
 	ptrTriangle2->setHeight(6);		//or (*ptrTriangle1).setHeight(6);  does the same thing
	
	//at this point:
	//ptrTriangle1 points to a triangle of base 2 and height 4
	//ptrTriangle2 points to a triangle of base 5 and height 6 

	//prints bases, heights, and areas of the triangles
	ser_port << *ptrTriangle1;
	ser_port << *ptrTriangle2;
	
	while (1){}						//infinite loop so the program doesn't end
}
