# Mbed OS Ethernet MAC (EMAC) driver for the ENC28J60 Ethernet controller
  and modules equipped with such chip.

This Arduino library has Mbed ENC28J60-EMAC driver from here: https://os.mbed.com/users/hudakz/code/ENC28J60-EMAC/. Only difference is a major fix of `ENC28J60_EMAC::link_out` and added `using namespace` required for ArduinoMbed-core.

To use it with Mbed Nano boards Ethernet library and SocketWrapper have to be copied from Mbed Portenta boards package. For Giga R1, Ethernet library has to be copied.

Wire the ENC28J60 module to standard SPI pins of the board. On Nano pins 10 to 13, on Portenta pins 7 to 10 and on Giga R1 use the SPI header and pin 10 as CS.

The Robotdyn Nano ENC28J60 Ethernet shield can be used with the 3.3V Nanos but only with [5 V level shifter removed!](https://github.com/JAndrassy/EthernetENC/wiki/Nano-Ethernet-Shield)

The HW-270 ENC28J60 Ethernet shield can be used with Arduino Giga. Other ENC28J60 Uno shields need [modifications](https://github.com/Networking-for-Arduino/EthernetENC/wiki/Shields) for use with the 3.3 V Giga.

To use other pins, create boards.local.txt next to board.txt with a line for your board, to define the SPI pins as Mbed PinName(!). It is possible to define the CS pin only. Then standard SPI pins are then used. Example:
```
giga.build.extra_flags=-DENC28J60_MOSI=PD_7 -DENC28J60_MISO=PG_9 -DENC28J60_SCK=PB_3 -DENC28J60_CS=PK_1
```
