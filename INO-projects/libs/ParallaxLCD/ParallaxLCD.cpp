
/* 
	NOTE: you must: #include "NewSoftSerial.h"
	http://arduino.cc/en/Reference/Libraries
	BEFORE including the class header file
				
				For Parallax™ LCD's (27976, 27977, 27979)
				For more information contact http://www.parallax.com	
				
				Douglas Farrell	
				cyberblob59@yahoo.com
			  
			  Used/Modified with permission from allen joslin
			--Based on SparkFunSerLCD.cpp from--
				allen joslin 
				payson productions
				allen@joslin.net
*/

#include "NewSoftSerial.h"
#include "ParallaxLCD.h"

/* ======================================================== */

#define PINOUT      0
#define POSBASE     1
#define BOUNCE      2
#define NUMROWS     3
#define NUMCOLS     4

//--------------------------
ParallaxLCD::ParallaxLCD ( int pin, int numRows, int numCols, int posBase) 
	: NewSoftSerial(pin,pin) {
	_bv[PINOUT]=pin;
	_bv[POSBASE]=posBase;
	_bv[BOUNCE]=10;
	_bv[NUMROWS]=numRows;
	_bv[NUMCOLS]=numCols;
	_ro[0]=0;
	_ro[1]=64;
	_ro[2]=numCols;
	_ro[3]=_ro[1]+numCols;
}

//--------------------------
void ParallaxLCD::setup(boolean startEmpty ) {
	pinMode(_bv[PINOUT], OUTPUT);
	delay(_bv[BOUNCE]);
	begin(19200);
	delay(_bv[BOUNCE]);
	if (startEmpty) {
		empty(); 
	}
	cursorOff();
}

//--------------------------
void ParallaxLCD::on () { 
	print(24,BYTE); delay(_bv[BOUNCE]); 
}

//--------------------------
void ParallaxLCD::off () { 
	print(21,BYTE); delay(_bv[BOUNCE]); 
}

//--------------------------
void ParallaxLCD::scrollLeft () { 
	print(8,BYTE); delay(_bv[BOUNCE]); 
}

//--------------------------
void ParallaxLCD::scrollRight () { 
	print(9,BYTE); delay(_bv[BOUNCE]); 
}

//--------------------------
void ParallaxLCD::empty () { 
	print(12,BYTE); delay(_bv[BOUNCE]*10);
	}

//--------------------------
void ParallaxLCD::cursorUnderline () { 
	print(24,BYTE); delay(_bv[BOUNCE]); 
}

//--------------------------
void ParallaxLCD::cursorBlock () { 
	print(23,BYTE); delay(_bv[BOUNCE]); 
}

//--------------------------
void ParallaxLCD::cursorOff () { 
	print(22,BYTE); delay(_bv[BOUNCE]); 
}

//--------------------------
void ParallaxLCD::cursorOnChrBlink () { 
	print(25,BYTE); delay(_bv[BOUNCE]); 
}

//--------------------------
void ParallaxLCD::cr () { 
	print(13,BYTE); delay(_bv[BOUNCE]); 
}

//--------------------------
void ParallaxLCD::lf () { 
	print(10,BYTE); delay(_bv[BOUNCE]); 
}

//--------------------------
void ParallaxLCD::backLightOn () {
	print(17,BYTE); delay(_bv[BOUNCE]);
}

//--------------------------
void ParallaxLCD::backLightOff () {
	print(18,BYTE); delay(_bv[BOUNCE]);
}

//--------------------------
void ParallaxLCD::pos ( int row, int col ) 
{ 
	if (row == 0)
		{
			print(128 + _ro[(row - _bv[POSBASE])] + (col - _bv[POSBASE]),BYTE); delay(_bv[BOUNCE]);
		}
	else if (row == 1)
		{
			print(148 + _ro[(row - _bv[POSBASE])] + (col - _bv[POSBASE]),BYTE); delay(_bv[BOUNCE]);
		}
	else if (row == 2)
		{
			print(168 + _ro[(row - _bv[POSBASE])] + (col - _bv[POSBASE]),BYTE); delay(_bv[BOUNCE]);			
		}
	else if (row == 3)
		{
			print(188 + _ro[(row - _bv[POSBASE])] + (col - _bv[POSBASE]),BYTE); delay(_bv[BOUNCE]);			
		}
	
}

// shortcuts

void ParallaxLCD::at ( int row, int col, char v )				{ pos(row,col); print(v); }
void ParallaxLCD::at ( int row, int col, const char v[] )	{ pos(row,col); print(v); }
void ParallaxLCD::at ( int row, int col, uint8_t v )			{ pos(row,col); print(v); }
void ParallaxLCD::at ( int row, int col, int v )				{ pos(row,col); print(v); }
void ParallaxLCD::at ( int row, int col, unsigned int v )	{ pos(row,col); print(v); }
void ParallaxLCD::at ( int row, int col, long v )				{ pos(row,col); print(v); }
void ParallaxLCD::at ( int row, int col, unsigned long v )	{ pos(row,col); print(v); }
void ParallaxLCD::at ( int row, int col, long v, int t )		{ pos(row,col); print(v,t); }


/* ======================================================== */
