#include "Led74Hc.h"

/* Table without dot */
const uint8_t Led74Hc_Table[] = {
    0b00111111,  // 0
    0b00000110,  // 1
    0b01011011,  // 2
    0b01001111,  // 3
    0b01100110,  // 4
    0b01101101,  // 5
    0b01111101,  // 6
    0b00000111,  // 7
    0b01111111,  // 8
    0b01101111   // 9
  };

static Led74Hc_Type Led74Hc_PrivateData;

static uint8_t Digits[4] = {0}; // Store individual digits for display

static void Led74Hc_ShiftOutByte(uint8_t Byte);
static void Led74Hc_SendToShiftRegisters(uint8_t *Data, uint8_t DataSize);

void Led74Hc_Init(Led74Hc_Type Config)
{
    Led74Hc_PrivateData.SHCP = Config.SHCP;
    Led74Hc_PrivateData.STCP = Config.STCP;
    Led74Hc_PrivateData.DS = Config.DS;

    pinMode(Led74Hc_PrivateData.SHCP, OUTPUT);
    pinMode(Led74Hc_PrivateData.STCP, OUTPUT);
    pinMode(Led74Hc_PrivateData.DS, OUTPUT);
}

void Led74Hc_ShowValueAt(uint8_t Value, uint8_t Position, bool DotEnabled)
{
    if (Position >= 4)
    {
        Serial.println("Exceed position (must be in range 0-3): " + String(Position));
        return;
    }

    if (Value >= 10)
    {
        Serial.println("Exceed value for single digit (must be in range 0-9): " + String(Value));
        return;
    }

    // Retrieve base segment data from the table
    uint8_t SegmentData = Led74Hc_Table[Value];

    // Enable dot if necessary
    if (DotEnabled)
    {
        SegmentData |= 0b10000000; // Set the MSB to enable the dot
    }

    uint16_t Data = ((SegmentData << 8) & 0xFF00) | (~(1 << Position) & 0x00FF);
    Led74Hc_SendToShiftRegisters((uint8_t *)&Data, 2);
}

void Led74Hc_ShowValue(uint32_t Value, uint8_t DigitCount, uint8_t DotPosition)
{
    // Extract digits from the value (LSB-first)
    for (uint8_t i = 0; i < DigitCount; i++)
    {
        Digits[i] = Value % 10;
        Value /= 10;
    }

    // Loop through each digit and display it
    for (uint8_t i = 0; i < DigitCount; i++)
    {
        // Determine if the dot should be enabled for this digit
        bool DotEnabled = (DotPosition & (1 << i)) ? true : false;

        // Use `Led74Hc_ShowValueAt` with the digit and dot info
        Led74Hc_ShowValueAt(Digits[i], i, DotEnabled);

        delayMicroseconds(500); // Small delay for stability
    }
}



// Function to send a single byte to 74HC595
static void Led74Hc_ShiftOutByte(uint8_t Byte)
{
    for (int i = 0; i < 8; i++)
    {
        digitalWrite(Led74Hc_PrivateData.SHCP, LOW);                                // Prepare for the next bit
        digitalWrite(Led74Hc_PrivateData.DS, (Byte & (1 << (7 - i))) ? HIGH : LOW); // Send bit
        // pr(String((Byte & (1 << (7 - i))) ? HIGH : LOW));
        digitalWrite(Led74Hc_PrivateData.SHCP, HIGH); // Shift bit in
    }
    // pr("\n");
}

static void Led74Hc_SendToShiftRegisters(uint8_t *Data, uint8_t DataSize)
{
    digitalWrite(Led74Hc_PrivateData.STCP, LOW); // Disable output while shifting

    // Send bytes in normal order (low byte first, high byte second)
    for (uint8_t i = 0; i < DataSize; i++)
    {
        Led74Hc_ShiftOutByte(Data[i]);
    }

    digitalWrite(Led74Hc_PrivateData.STCP, HIGH); // Latch data to output
}
