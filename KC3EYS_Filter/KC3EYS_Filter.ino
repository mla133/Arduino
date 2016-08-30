#include <Filters.h>

float testFrequency = 7025000;               // test signal frequency (Hz)
float testAmplitude = 100;                   // test signal amplitude
float testOffset = 100;

float windowLength = 20.0/testFrequency;     // how long to average the signal, for statistist

float testSignalSigma = testAmplitude / sqrt(2.0);         // the RMS amplitude of the test signal
float testSignal3dbSigma = testSignalSigma / sqrt(2.0);    // the RMS amplitude of the test signal, down -3db
float cutoffFrequency = 2000;
float outputValue;
float printPeriod = 0.1;


// return the current time
float time() {
  return float( micros() ) * 1e-6;
}

float startTime = time();
float nextPrintTime = time();
FilterOnePole filterOneLowpass( LOWPASS, testFrequency ); 

void setup() 
{
  Serial.begin( 57600 );    // start the serial port
}

void loop()
{

  float inputValue = testAmplitude + testAmplitude*sin( TWO_PI * testFrequency * time() );
  outputValue = filterOneLowpass.input( inputValue );

  if( time() > nextPrintTime ) 
  {
    // display current values to the screen
    nextPrintTime += printPeriod;   // update the next print time
    
    Serial.print( "time: " ); Serial.print( time() );
    Serial.print( "\tin: " ); Serial.print( inputValue );
    Serial.print( "\tout: " ); Serial.println( outputValue );
  }
}
