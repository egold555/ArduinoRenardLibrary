#ifndef Renard_h
#define Renard_h

#include "Arduino.h"

class Renard
{
private:
    static const int lookingForSync = 1;
    static const int lookingForCommand = 2;
    static const int lookingForBuffer = 3;

    byte _protocolState;
    byte _sawPrefix;
    int _bufferNumber;

    Stream * _stream;
    int _numberOfChannels;
    byte * _channels;
    void (*_callback)(int channel, byte value);

    bool processByte(byte inputByte);

public:
    Renard(Stream & stream, int numberOfChannels);
    byte channelValue(int channelNumber);
    bool processInput();
    void setCallback(void (*callback)(int channel, byte value));
};

#endif