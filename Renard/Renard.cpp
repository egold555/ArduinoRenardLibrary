#include "Arduino.h"
#include "Renard.h"

Renard::Renard(Stream & stream, int numberOfChannels)
{
    _stream = &stream;
    _numberOfChannels = numberOfChannels;
    _channels = (byte *) malloc(numberOfChannels);

    for (int i = 0; i < numberOfChannels; ++i) {
        _channels[i] = 0;
    }

    _protocolState = lookingForSync;
    _sawPrefix = 0;
    _bufferNumber = 0;
    _callback = NULL;
}

void Renard::setCallback(void(*callback)(int channel, byte value))
{
    _callback = callback;
}

byte Renard::channelValue(int channelNumber)
{
    if (channelNumber >= 1 && channelNumber <= _numberOfChannels)
        return _channels[channelNumber - 1];
    else
        return 0;
}

bool Renard::processInput()
{
    bool ret = false;
    while (_stream->available() > 0) {
        int b = _stream->read();
        if (b >= 0)
            ret = processByte((byte)b);
    }

    return ret;
}

bool Renard::processByte(byte inputByte)
{
    if (inputByte == 0x7D) {
        _sawPrefix = 0;
        return false;
    }
    if (inputByte == 0x7E) {
        _protocolState = lookingForCommand;
        _sawPrefix = 0;
        return false;
    }
    if (_protocolState == lookingForSync) {
        _sawPrefix = 0;
        return false;
    }
    else if (_protocolState == lookingForCommand) {
        if (inputByte == 0x80) {
            _protocolState = lookingForBuffer;
            _bufferNumber = 0;
            _sawPrefix = 0;
            return false;
        }
        else {
            _protocolState = lookingForSync;
            _sawPrefix = 0;
            return false;
        }
    }
    else if (_protocolState == lookingForBuffer) {
        if (inputByte == 0x7F) {
            _sawPrefix = 1;
            return false;
        }
        else if (_sawPrefix) {
            if (inputByte == 0x2F) {
                inputByte = 0x7D;
            }
            else if (inputByte == 0x30) {
                inputByte = 0x7E;
            }
            else if (inputByte == 0x31) {
                inputByte = 0x7F;
            }
            _sawPrefix = 0;
        }

        if (_bufferNumber < _numberOfChannels)
        {
            if (_channels[_bufferNumber] != inputByte) {
                _channels[_bufferNumber] = inputByte;
                if (_callback) {
                    _callback(_bufferNumber + 1, inputByte);
                }
            }
        }

        _bufferNumber += 1;

        return (_bufferNumber >= _numberOfChannels);
    }
}