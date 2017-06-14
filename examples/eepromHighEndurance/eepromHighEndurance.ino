//extEEPROM High Endurance
//Paolo Paolucci 13Jun2017

#define EE_PARAM_BUFFER_SIZE  8
#define EE_STATUS_BUFFER_SIZE  EE_PARAM_BUFFER_SIZE

#define EE_PARAM_1  0
#define EE_PARAM_2 (EE_PARAM_1 + EE_PARAM_BUFFER_SIZE + EE_STATUS_BUFFER_SIZE)
//#define EE_PARAM_3 (EE_PARAM_2 + EE_PARAM_BUFFER_SIZE + EE_STATUS_BUFFER_SIZE)  ... and so forth

unsigned int EEBufPtrParam1 = EE_PARAM_1;
unsigned int EEBufPtrParam2 = EE_PARAM_2;

#include <extEEPROM.h>    //https://github.com/PaoloP74/extEEPROM

//One 24LC256 EEPROMs on the bus
extEEPROM eep(kbits_256, 1, 64);         //device size, number of devices, page size

void setup(void)
{
  Serial.begin(115200);
  uint8_t eepStatus = eep.begin(twiClock400kHz);      //go fast!
  if (eepStatus) {
    Serial.print(F("extEEPROM.begin() failed, status = "));
    Serial.println(eepStatus);
    while (1);
  }

  unsigned char counter;
  findCurrentEEpromAddr( &EEBufPtrParam1 );
  findCurrentEEpromAddr( &EEBufPtrParam2 );

  for ( counter = 0 ; counter < 50; counter++)
  {
    EEWriteBuffer( EE_PARAM_1, &EEBufPtrParam1, counter );
    EEWriteBuffer( EE_PARAM_2, &EEBufPtrParam2, counter * 2 );
  }
}

void loop(void)
{
}

void findCurrentEEpromAddr( unsigned int *EEBufPtr )
{
  unsigned char temp;
  unsigned int EEBufEnd;

  *EEBufPtr += EE_PARAM_BUFFER_SIZE;              // Point to the status buffer
  EEBufEnd = *EEBufPtr + EE_STATUS_BUFFER_SIZE;   // The first address outside the buffer

  /* Identify the last written element of the status buffer */
  do {
    temp = EEReadBuffer( *EEBufPtr );
    (*EEBufPtr)++;
    if (*EEBufPtr == EEBufEnd) // Break if end of buffer, so we don't compare out-of-bounds.
      break;
  } while (EEReadBuffer( *EEBufPtr ) == temp + 1);

  *EEBufPtr -= (EE_PARAM_BUFFER_SIZE + 1); // Point to the last used element of the parameter buffer
}

char EEReadBuffer( unsigned int address )
{
  byte data;
  eep.read(address, data, 1);
  return data;
}

void EEWriteBuffer( unsigned char parameter, unsigned int *address, unsigned char data )
{
  unsigned char EEOldStatusValue;

  /* Store the old status value and move pointer to the next element in the buffer */
  EEOldStatusValue = EEReadBuffer( *address + EE_PARAM_BUFFER_SIZE);
  (*address)++;
  if ( *address == parameter + EE_PARAM_BUFFER_SIZE )
  {
    // Wraparound if necessary.
    *address = parameter;
  }

  /* Update the parameter in the EEPROM buffer */
  eep.write(*address, data, 1);

  /* Update the status buffer */
  eep.write(*address + EE_PARAM_BUFFER_SIZE, EEOldStatusValue + 1, 1);
}

