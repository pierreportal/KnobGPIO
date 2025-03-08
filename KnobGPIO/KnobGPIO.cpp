#include "chugin.h"
#include <stdio.h>
#include <limits.h>
#include <iostream>
// #include <pigpio.h>

#define SPI_CHANNEL 0
#define SPI_SPEED 500000

CK_DLL_CTOR(knobgpio_ctor);
CK_DLL_DTOR(knobgpio_dtor);
CK_DLL_MFUN(knobgpio_setChannel);
CK_DLL_TICK(knobgpio_tick);

int read_adc(int channel)
{
    if (channel < 0 || channel > 7)
        return -1;
    char tx[3] = {1, (char)((8 + channel) << 4), 0};
    char rx[3] = {0};

    // spiXfer(SPI_CHANNEL, tx, rx, 3);
    return ((rx[1] & 3) << 8) + rx[2];
}

class KnobGPIO
{

public:
    KnobGPIO() : channel(0)
    {
        // gpioInitialise();
        // spiOpen(SPI_CHANNEL, SPI_SPEED, 0);
    }

    ~KnobGPIO()
    {
        // spiClose(SPI_CHANNEL);
        // gpioTerminate();
    }

    void setChannel(int chan)
    {
        channel = chan;
    }

    int tick(SAMPLE *in, SAMPLE *out, int nframes)
    {
        std::cout << "KnobGPIO channel set to: " << channel << std::endl;
        for (t_CKINT i = 0; i < nframes; i++)
        {
            int raw_value = read_adc(channel);
            out[i] = (t_CKFLOAT)raw_value / 1023.0;
        }
        return nframes;
    }

private:
    t_CKINT channel;
};

CK_DLL_QUERY(KnobGPIO)
{
    QUERY->setname(QUERY, "KnobGPIO");

    QUERY->begin_class(QUERY, "KnobGPIO", "UGen");

    QUERY->add_ctor(QUERY, knobgpio_ctor);
    QUERY->add_dtor(QUERY, knobgpio_dtor);
    QUERY->doc_class(QUERY, "KnobGPIO is a test UGen.");

    QUERY->add_ugen_func(QUERY, knobgpio_tick, NULL, 1, 16);
    QUERY->doc_func(QUERY, "KnobGPIO tick function.");

    QUERY->add_mfun(QUERY, knobgpio_setChannel, "int", "setChannel");
    QUERY->add_arg(QUERY, "int", "channel");
    QUERY->doc_func(QUERY, "Set the channel of the KnobGPIO.");

    QUERY->end_class(QUERY);

    return TRUE;
}

t_CKINT knobgpio_data_offset = 0;

CK_DLL_CTOR(knobgpio_ctor)
{
    OBJ_MEMBER_INT(SELF, knobgpio_data_offset) = 0;
    KnobGPIO *bcdata = new KnobGPIO();
    OBJ_MEMBER_INT(SELF, knobgpio_data_offset) = (t_CKINT)bcdata;
}

CK_DLL_DTOR(knobgpio_dtor)
{
    KnobGPIO *bcdata = (KnobGPIO *)OBJ_MEMBER_INT(SELF, knobgpio_data_offset);
    if (bcdata)
    {
        delete bcdata;
        OBJ_MEMBER_INT(SELF, knobgpio_data_offset) = 0;
        bcdata = NULL;
    }
}

CK_DLL_MFUN(knobgpio_setChannel)
{
    KnobGPIO *knob = (KnobGPIO *)OBJ_MEMBER_INT(SELF, knobgpio_data_offset);
    knob->setChannel(GET_NEXT_INT(ARGS));
}

CK_DLL_TICK(knobgpio_tick)
{
    KnobGPIO *knob = (KnobGPIO *)OBJ_MEMBER_INT(SELF, 0);
    return knob->tick(NULL, out, 16);
}