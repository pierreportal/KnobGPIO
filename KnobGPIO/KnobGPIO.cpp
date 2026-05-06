#include "chugin.h"
#include <stdio.h>
#include <limits.h>
#include <iostream>
#include <pigpio.h>

#define SPI_CHANNEL 0
#define SPI_SPEED 500000

CK_DLL_CTOR(knobgpio_ctor);
CK_DLL_DTOR(knobgpio_dtor);
CK_DLL_MFUN(knobgpio_setChannel);
CK_DLL_MFUN(knobgpio_setUpdateRate);
CK_DLL_TICK(knobgpio_tick);

int read_adc(int channel)
{
    if (channel < 0 || channel > 7)
        return -1;
    char tx[3] = {1, (char)((8 + channel) << 4), 0};
    char rx[3] = {0};

    if (spiXfer(SPI_CHANNEL, tx, rx, 3) < 0)
    {
        std::cerr << "SPI transfer error!" << std::endl;
        return -1;
    }
    return ((rx[1] & 3) << 8) + rx[2];
}

class KnobGPIO
{

public:
    KnobGPIO() : channel(0), initialized(false), last_value(0.0), frame_counter(0), update_rate(441)
    {
        // Initialize GPIO with error checking
        int gpio_status = gpioInitialise();
        if (gpio_status < 0)
        {
            std::cerr << "KnobGPIO: Failed to initialize GPIO (error: " << gpio_status << ")" << std::endl;
            return;
        }

        // Open SPI with error checking
        int spi_status = spiOpen(SPI_CHANNEL, SPI_SPEED, 0);
        if (spi_status < 0)
        {
            std::cerr << "KnobGPIO: Failed to open SPI (error: " << spi_status << ")" << std::endl;
            gpioTerminate();
            return;
        }

        initialized = true;
        std::cout << "KnobGPIO: Initialized successfully on channel " << channel << std::endl;
    }

    ~KnobGPIO()
    {
        if (initialized)
        {
            spiClose(SPI_CHANNEL);
            gpioTerminate();
        }
    }

    void setChannel(int chan)
    {
        // Validate and clamp channel to valid range (0-7)
        if (chan < 0)
        {
            std::cerr << "KnobGPIO: Channel " << chan << " is below minimum (0), clamping to 0" << std::endl;
            channel = 0;
        }
        else if (chan > 7)
        {
            std::cerr << "KnobGPIO: Channel " << chan << " exceeds maximum (7), clamping to 7" << std::endl;
            channel = 7;
        }
        else
        {
            channel = chan;
        }
    }

    int tick(SAMPLE *in, SAMPLE *out, int nframes)
    {
        // If initialization failed, output zeros
        if (!initialized)
        {
            for (t_CKINT i = 0; i < nframes; i++)
            {
                out[i] = 0.0;
            }
            return nframes;
        }

        // Performance optimization: only read ADC periodically, not every frame
        // Default: update every 441 frames (~100Hz at 44.1kHz sample rate)
        for (t_CKINT i = 0; i < nframes; i++)
        {
            if (frame_counter % update_rate == 0)
            {
                int raw_value = read_adc(channel);
                if (raw_value >= 0)
                {
                    last_value = (t_CKFLOAT)raw_value / 1023.0;
                }
                // On error, keep last valid value
            }
            out[i] = last_value;
            frame_counter++;
        }
        return nframes;
    }

    void setUpdateRate(int rate)
    {
        // Set how many frames between ADC reads
        // Lower = more responsive but higher CPU usage
        // Higher = less CPU usage but slower response
        if (rate < 1)
            rate = 1;
        update_rate = rate;
    }

private:
    t_CKINT channel;
    bool initialized;
    t_CKFLOAT last_value;
    t_CKINT frame_counter;
    t_CKINT update_rate; // Read ADC every N frames
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
    QUERY->doc_func(QUERY, "Set the ADC channel (0-7) of the KnobGPIO.");

    QUERY->add_mfun(QUERY, knobgpio_setUpdateRate, "void", "setUpdateRate");
    QUERY->add_arg(QUERY, "int", "rate");
    QUERY->doc_func(QUERY, "Set the ADC update rate (frames between reads). Lower = more responsive, higher = less CPU usage.");

    QUERY->end_class(QUERY);

    return TRUE;
}

t_CKINT knobgpio_data_offset = 0;

CK_DLL_CTOR(knobgpio_ctor)
{
    KnobGPIO *knob = new KnobGPIO();
    OBJ_MEMBER_INT(SELF, knobgpio_data_offset) = (t_CKINT)knob;
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

CK_DLL_MFUN(knobgpio_setUpdateRate)
{
    KnobGPIO *knob = (KnobGPIO *)OBJ_MEMBER_INT(SELF, knobgpio_data_offset);
    knob->setUpdateRate(GET_NEXT_INT(ARGS));
}

CK_DLL_TICK(knobgpio_tick)
{
    KnobGPIO *knob = (KnobGPIO *)OBJ_MEMBER_INT(SELF, knobgpio_data_offset);
    if (!knob)
        return FALSE;

    return knob->tick(in, out, nframes);
}