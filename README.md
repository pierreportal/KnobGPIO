
## Prerequisite

### MCP3008

The MCP3008 is a 10-bit Analog-to-Digital Converter (ADC) chip that allows microcontrollers (like the Raspberry Pi) to read analog signals. The Raspberry Pi does not have built-in analog input pins, so if you're using an analog sensor (like a potentiometer, temperature sensor, or light sensor), you need an external ADC like the MCP3008 to convert the analog signal into a digital value that the Pi can process.

| MCP3008 Pin   | Connect to Raspberry Pi Pin                                    |
| ------------- | -------------------------------------------------------------- |
| **VDD (16)**  | **3.3V (Pin 1)**                                               |
| **VREF (15)** | **3.3V (Pin 1)**                                               |
| **AGND (14)** | **GND (Pin 6)**                                                |
| **CLK (13)**  | **GPIO11 (SPI CLK, Pin 23)**                                   |
| **DOUT (12)** | **GPIO9 (SPI MISO, Pin 21)**                                   |
| **DIN (11)**  | **GPIO10 (SPI MOSI, Pin 19)**                                  |
| **CS (10)**   | **GPIO8 (SPI CE0, Pin 24)**                                    |
| **DGND (9)**  | **GND (Pin 6)**                                                |
| **CH0 - CH7** | **Connect to potentiometer middle pin or other analog sensor** |



```
       _______
CH0 --|1  ° 16|-- VDD     -> 3.3v
CH1 --|2    15|-- VREF    -> 3.3v
CH2 --|3    14|-- AGND    -> gnd
CH3 --|4    13|-- CLK     -> GPIO11 (SPI CLK, Pin 23)
CH4 --|5    12|-- DOUT    -> GPIO9 (SPI MISO, Pin 21)
CH5 --|6    11|-- DIN     -> GPIO10 (SPI MISO, Pin 19)
CH6 --|7    10|-- CS/SHDN -> GPIO8 (SPI CE0, Pin 24)
CH7 --|8_____9|-- DGND    -> gnd
```


## Install 

On linux

```sh
git clone https://github.com/pierreportal/KnobGPIO.git

cd KnobGPIO
```

```sh
make linux
```

```sh
sudo make install
```

## Usage

```cpp
KnobGPIO knob;

while (true)
{
    <<< knob.last() >>>;
}
```
