/*****************************************************************************
*
* Title         : Microchip ENC28J60 Ethernet Controller Registers
* Author        : Pascal Stang (c)2005
* Modified by   : Tobias Jaster
* Modified by   : Zoltan Hudak
* Copyright     : GPL V2
*
*This driver provides initialization and transmit/receive
*functions for the Microchip ENC28J60 10Mb Ethernet Controller and PHY.
*This chip is novel in that it is a full MAC+PHY interface all in a 28-pin
*chip, using an SPI interface to the host processor.
*
*
*****************************************************************************/
#ifndef ENC28J60_REG_H
#define ENC28J60_REG_H
#include <inttypes.h>
#include "enc28j60_emac_config.h"

// ENC28J60 Control Registers

// Control register definitions are a combination of address,
// bank number, and Ethernet/MAC/PHY indicator bits.
// - Register address       (bits 0-4)
// - Bank number            (bits 5-6)
// - MAC/PHY indicator      (bit 7)
#define ADDR_MASK   0x1F
#define BANK_MASK   0x60
#define SPRD_MASK   0x80
// All-bank registers
#define EIE     0x1B                        // ETHERNET INTERRUPT ENABLE REGISTER
#define EIR     0x1C                        // ETHERNET INTERRUPT REQUEST (FLAG) REGISTER
#define ESTAT   0x1D                        // ETHERNET STATUS REGISTER
#define ECON2   0x1E                        // ETHERNET CONTROL REGISTER 2
#define ECON1   0x1F                        // ETHERNET CONTROL REGISTER 1

/*
 * Bank 0 registers
 */

// Points to a location in receive/transmit buffer to read from
#define ERDPTL  (0x00 | 0x00)               // Ethernet buffer Read Pointer Low Byte (ERDPT<7:0>)
#define ERDPTH  (0x01 | 0x00)               // Ethernet buffer Read Pointer High Byte (ERDPT<12:8>)

// Points to a location in receive/transmit buffer to write to
#define EWRPTL  (0x02 | 0x00)               // Ethernet buffer Write Pointer Low Byte (EWRPT<7:0>)
#define EWRPTH  (0x03 | 0x00)               // Ethernet buffer Write Pointer High Byte (EWRPT<12:8>)

// Pointers to transmit buffer boundaries
#define ETXSTL  (0x04 | 0x00)               // Ethernet Transmit buffer Start Low Byte (ETXST<7:0>)
#define ETXSTH  (0x05 | 0x00)               // Ethernet Transmit buffer Start High Byte (ETXST<12:8>)
#define ETXNDL  (0x06 | 0x00)               // Ethernet Transmit buffer End Low Byte (ETXND<7:0>)
#define ETXNDH  (0x07 | 0x00)               // Ethernet Transmit buffer End High Byte (ETXND<12:8>)

// Pointers to receive buffer boundaries
#define ERXSTL  (0x08 | 0x00)               // Ethernet Receive buffer Start Low Byte (ERXST<7:0>)
#define ERXSTH  (0x09 | 0x00)               // Ethernet Receive buffer Start High Byte (ERXST<12:8>)
#define ERXNDL  (0x0A | 0x00)               // Ethernet Receive buffer End Low Byte (ERXND<7:0>)
#define ERXNDH  (0x0B | 0x00)               // Ethernet Receive buffer End High Byte (ERXND<12:8>)

// Receive pointer. Receive hardware will write data up to, but not including the memory pointed to by ERXRDPT
#define ERXRDPTL    (0x0C | 0x00)           // Ethernet Receive buffer RD Pointer Low Byte (ERXRDPT<7:0>)
#define ERXRDPTH    (0x0D | 0x00)           // Ethernet Receive buffer RD Pointer High Byte (ERXRDPT<12:8>)

// Location within the receive buffer where the hardware will write bytes that it receives.
// The pointer is read-only and is automatically updated by the hardware whenever
// a new packet is successfully received.
#define ERXWRPTL    (0x0E | 0x00)           // Ethernet Receive buffer WR Pointer Low Byte (ERXWRPT<7:0>)   Read-only
#define ERXWRPTH    (0x0F | 0x00)           // Ethernet Receive buffer WR Pointer High Byte (ERXWRPT<12:8>) Read-only

// Pointers to DMA boundaries
#define EDMASTL (0x10 | 0x00)               // Ethernet buffer DMA Start Low Byte (EDMAST<7:0>)
#define EDMASTH (0x11 | 0x00)               // Ethernet buffer DMA Start High Byte (EDMAST<12:8>)
#define EDMANDL (0x12 | 0x00)               // Ethernet buffer DMA End Low Byte (EDMAND<7:0>)
#define EDMANDH (0x13 | 0x00)               // Ethernet buffer DMA End High Byte (EDMAND<12:8>)

// Points to the DMA memory copying destination in receive/transmit buffer
#define EDMADSTL    (0x14 | 0x00)           // Ethernet buffer DMA Destination Low Byte (EDMADST<7:0>)
#define EDMADSTH    (0x15 | 0x00)           // Ethernet buffer DMA Destination High Byte (EDMADST<12:8>)

// Points to the DMA check sum calcularion location in receive/transmit buffer
#define EDMACSL (0x16 | 0x00)               // Ethernet buffer DMA Checksum Low Byte (EDMACS<7:0>)
#define EDMACSH (0x17 | 0x00)               // Ethernet buffer DMA Checksum High Byte (EDMACS<15:8>)

/*
 * Bank 1 registers
 */
#define EHT0    (0x00 | 0x20)               //Hash Table Byte 0 (EHT<7:0>)
#define EHT1    (0x01 | 0x20)               //Hash Table Byte 0 (EHT<7:0>)
#define EHT2    (0x02 | 0x20)               //Hash Table Byte 0 (EHT<7:0>)
#define EHT3    (0x03 | 0x20)               //Hash Table Byte 3 (EHT<31:24>)
#define EHT4    (0x04 | 0x20)               //Hash Table Byte 3 (EHT<31:24>)
#define EHT5    (0x05 | 0x20)               //Hash Table Byte 3 (EHT<31:24>)
#define EHT6    (0x06 | 0x20)               //Hash Table Byte 3 (EHT<31:24>)
#define EHT7    (0x07 | 0x20)               //Hash Table Byte 3 (EHT<31:24>)
#define EPMM0   (0x08 | 0x20)               //Pattern Match Mask Byte 0 (EPMM<7:0>)
#define EPMM1   (0x09 | 0x20)               //Pattern Match Mask Byte 1 (EPMM<15:8>)
#define EPMM2   (0x0A | 0x20)               //Pattern Match Mask Byte 2 (EPMM<23:16>)
#define EPMM3   (0x0B | 0x20)               //Pattern Match Mask Byte 3 (EPMM<31:24>)
#define EPMM4   (0x0C | 0x20)               //Pattern Match Mask Byte 4 (EPMM<39:32>)
#define EPMM5   (0x0D | 0x20)               //Pattern Match Mask Byte 5 (EPMM<47:40>)
#define EPMM6   (0x0E | 0x20)               //Pattern Match Mask Byte 6 (EPMM<55:48>)
#define EPMM7   (0x0F | 0x20)               //Pattern Match Mask Byte 7 (EPMM<63:56>)
#define EPMCSL  (0x10 | 0x20)               //Pattern Match Checksum Low Byte (EPMCS<7:0>)
#define EPMCSH  (0x11 | 0x20)               //Pattern Match Checksum High Byte (EPMCS<15:0>)
#define EPMOL   (0x14 | 0x20)               //Pattern Match Offset Low Byte (EPMO<7:0>)
#define EPMOH   (0x15 | 0x20)               //Pattern Match Offset High Byte (EPMO<12:8>)
#define EWOLIE  (0x16 | 0x20)               //Reserved
#define EWOLIR  (0x17 | 0x20)               //Reserved
#define ERXFCON (0x18 | 0x20)               //ETHERNET RECEIVE FILTER CONTROL REGISTER
#define EPKTCNT (0x19 | 0x20)               //Ethernet Packet Count

/*
 * Bank 2 registers
 */
#define MACON1      (0x00 | 0x40 | 0x80)    //MAC CONTROL REGISTER 1
#define MACON2      (0x01 | 0x40 | 0x80)    //MAC CONTROL REGISTER 2
#define MACON3      (0x02 | 0x40 | 0x80)    //MAC CONTROL REGISTER 3
#define MACON4      (0x03 | 0x40 | 0x80)    //MAC CONTROL REGISTER 4
#define MABBIPG     (0x04 | 0x40 | 0x80)    //Back-to-Back Inter-Packet Gap (BBIPG<6:0>)
#define MAIPGL      (0x06 | 0x40 | 0x80)    //Non-Back-to-Back Inter-Packet Gap Low Byte (MAIPGL<6:0>)
#define MAIPGH      (0x07 | 0x40 | 0x80)    //Non-Back-to-Back Inter-Packet Gap High Byte (MAIPGH<6:0>)
#define MACLCON1    (0x08 | 0x40 | 0x80)    //Retransmission Maximum (RETMAX<3:0>)
#define MACLCON2    (0x09 | 0x40 | 0x80)    //Collision Window (COLWIN<5:0>
#define MAMXFLL     (0x0A | 0x40 | 0x80)    //Maximum Frame Length Low Byte (MAMXFL<7:0>)
#define MAMXFLH     (0x0B | 0x40 | 0x80)    //Maximum Frame Length High Byte (MAMXFL<15:8>)
#define MAPHSUP     (0x0D | 0x40 | 0x80)    //Reserved
#define MICON       (0x11 | 0x40 | 0x80)    //Reserved
#define MICMD       (0x12 | 0x40 | 0x80)    //MII COMMAND REGISTER
#define MIREGADR    (0x14 | 0x40 | 0x80)    //MII Register Address (MIREGADR<4:0>)
#define MIWRL       (0x16 | 0x40 | 0x80)    //MII Write Data Low Byte (MIWR<7:0>)
#define MIWRH       (0x17 | 0x40 | 0x80)    //MII Write Data High Byte (MIWR<15:8>)
#define MIRDL       (0x18 | 0x40 | 0x80)    //MII Read Data Low Byte (MIRD<7:0>)
#define MIRDH       (0x19 | 0x40 | 0x80)    //MII Read Data High Byte(MIRD<15:8>)

/*
 * Bank 3 registers
 */
#define MAADR1  (0x00 | 0x60 | 0x80)        //MAC Address Byte 1 (MAADR<47:40>), OUI Byte 1
#define MAADR0  (0x01 | 0x60 | 0x80)        //MAC Address Byte 2 (MAADR<39:32>), OUI Byte 2
#define MAADR3  (0x02 | 0x60 | 0x80)        //MAC Address Byte 3 (MAADR<31:24>), OUI Byte 3
#define MAADR2  (0x03 | 0x60 | 0x80)        //MAC Address Byte 4 (MAADR<23:16>)
#define MAADR5  (0x04 | 0x60 | 0x80)        //MAC Address Byte 5 (MAADR<15:8>)
#define MAADR4  (0x05 | 0x60 | 0x80)        //MAC Address Byte 4 (MAADR<23:16>)
#define EBSTSD  (0x06 | 0x60)               //Built-in Self-Test Fill Seed (EBSTSD<7:0>)
#define EBSTCON (0x07 | 0x60)               //
#define EBSTCSL (0x08 | 0x60)               //Built-in Self-Test Checksum Low Byte (EBSTCS<7:0>)
#define EBSTCSH (0x09 | 0x60)               //Built-in Self-Test Checksum High Byte (EBSTCS<15:8>)
#define MISTAT  (0x0A | 0x60 | 0x80)        //MII STATUS REGISTER
#define EREVID  (0x12 | 0x60)               //Ethernet Revision ID (EREVID<4:0>)
#define ECOCON  (0x15 | 0x60)               //CLOCK OUTPUT CONTROL REGISTER
#define EFLOCON (0x17 | 0x60)               //ETHERNET FLOW CONTROL REGISTER
#define EPAUSL  (0x18 | 0x60)               //Pause Timer Value Low Byte (EPAUS<7:0>)
#define EPAUSH  (0x19 | 0x60)               //Pause Timer Value High Byte (EPAUS<15:8>)

/*
 * PHY registers
 */
#define PHCON1  0x00                        //PHY CONTROL REGISTER 1
#define PHSTAT1 0x01                        //PHYSICAL LAYER STATUS REGISTER 1
#define PHHID1  0x02                        //
#define PHHID2  0x03                        //
#define PHCON2  0x10                        //PHY CONTROL REGISTER 2
#define PHSTAT2 0x11                        //PHYSICAL LAYER STATUS REGISTER 2
#define PHIE    0x12                        //PHY INTERRUPT ENABLE REGISTER
#define PHIR    0x13                        //PHY INTERRUPT REQUEST (FLAG) REGISTER
#define PHLCON  0x14                        //PHY MODULE LED CONTROL REGISTER

/*
 * ENC28J60 ERXFCON Register Bit Definitions
 */
#define ERXFCON_UCEN    0x80
#define ERXFCON_ANDOR   0x40
#define ERXFCON_CRCEN   0x20
#define ERXFCON_PMEN    0x10
#define ERXFCON_MPEN    0x08
#define ERXFCON_HTEN    0x04
#define ERXFCON_MCEN    0x02
#define ERXFCON_BCEN    0x01
/*
 * ENC28J60 EIE Register Bit Definitions
 */
#define EIE_INTIE   0x80
#define EIE_PKTIE   0x40
#define EIE_DMAIE   0x20
#define EIE_LINKIE  0x10
#define EIE_TXIE    0x08
#define EIE_WOLIE   0x04
#define EIE_TXERIE  0x02
#define EIE_RXERIE  0x01
/*
 * ENC28J60 EIR Register Bit Definitions
 */
#define EIR_PKTIF   0x40                    // Receive Packet Pending Interrupt Flag bit
#define EIR_DMAIF   0x20                    // DMA Interrupt Flag bit
#define EIR_LINKIF  0x10                    // Link Change Interrupt Flag bit
#define EIR_TXIF    0x08                    // Transmit Interrupt Flag bit
#define EIR_WOLIF   0x04
#define EIR_TXERIF  0x02                    // Transmit Error Interrupt Flag bit
#define EIR_RXERIF  0x01                    // Receive Error Interrupt Flag bit

/*
 * ENC28J60 ESTAT Register Bit Definitions
 */
#define ESTAT_INT       0x80                // INT Interrupt Flag bit (INT interrupt is pending)
#define ESTAT_BUFER     0x40                // Ethernet Buffer Error Status bit
#define ESTAT_LATECOL   0x10                // Late Collision Error bit
#define ESTAT_RXBUSY    0x04                // Receive Busy bit
#define ESTAT_TXABRT    0x02                // Transmit Abort Error bit
#define ESTAT_CLKRDY    0x01                // Clock Ready bit

/*
 * ENC28J60 ECON2 Register Bit Definitions
 */
#define ECON2_AUTOINC   0x80
#define ECON2_PKTDEC    0x40
#define ECON2_PWRSV     0x20
#define ECON2_VRPS      0x08
/*
 * ENC28J60 ECON1 Register Bit Definitions
 */
#define ECON1_TXRST     0x80
#define ECON1_RXRST     0x40
#define ECON1_DMAST     0x20
#define ECON1_CSUMEN    0x10
#define ECON1_TXRTS     0x08
#define ECON1_RXEN      0x04
#define ECON1_BSEL1     0x02
#define ECON1_BSEL0     0x01
/*
 * ENC28J60 MACON1 Register Bit Definitions
 */
#define MACON1_LOOPBK   0x10
#define MACON1_TXPAUS   0x08
#define MACON1_RXPAUS   0x04
#define MACON1_PASSALL  0x02
#define MACON1_MARXEN   0x01
/*
 * ENC28J60 MACON2 Register Bit Definitions
 */
#define MACON2_MARST    0x80
#define MACON2_RNDRST   0x40
#define MACON2_MARXRST  0x08
#define MACON2_RFUNRST  0x04
#define MACON2_MATXRST  0x02
#define MACON2_TFUNRST  0x01
/*
 * ENC28J60 MACON3 Register Bit Definitions
 */
#define MACON3_PADCFG2  0x80
#define MACON3_PADCFG1  0x40
#define MACON3_PADCFG0  0x20
#define MACON3_TXCRCEN  0x10
#define MACON3_PHDRLEN  0x08
#define MACON3_HFRMLEN  0x04
#define MACON3_FRMLNEN  0x02
#define MACON3_FULDPX   0x01
/*
 * ENC28J60 MACON4 Register Bit Definitions
 */
#define MACON4_DEFER    0x40
#define MACON4_BPEN     0x20
#define MACON4_NOBKOFF  0x10
/*
 * ENC28J60 MICMD Register Bit Definitions
 */
#define MICMD_MIISCAN   0x02
#define MICMD_MIIRD     0x01
/*
 * ENC28J60 MISTAT Register Bit Definitions
 */
#define MISTAT_NVALID   0x04
#define MISTAT_SCAN     0x02
#define MISTAT_BUSY     0x01
/*
 * ENC28J60 PHY PHCON1 Register Bit Definitions
 */
#define PHCON1_PRST     0x8000
#define PHCON1_PLOOPBK  0x4000
#define PHCON1_PPWRSV   0x0800
#define PHCON1_PDPXMD   0x0100
/*
 * ENC28J60 PHY PHSTAT1 Register Bit Definitions
 */
#define PHSTAT1_PFDPX   0x1000
#define PHSTAT1_PHDPX   0x0800
#define PHSTAT1_LLSTAT  0x0004
#define PHSTAT1_JBSTAT  0x0002
/*
 * ENC28J60 PHY PHSTAT2 Register Bit Definitions
 */
#define PHSTAT2_TXSTAT  0x2000
#define PHSTAT2_RXSTAT  0x1000
#define PHSTAT2_COLSTAT 0x0800
#define PHSTAT2_LSTAT   0x0400
#define PHSTAT2_DPXSTAT 0x0200
#define PHSTAT2_PLRITY  0x0020
/*
 * ENC28J60 PHY PHCON2 Register Bit Definitions
 */
#define PHCON2_FRCLINK  0x4000
#define PHCON2_TXDIS    0x2000
#define PHCON2_JABBER   0x0400
#define PHCON2_HDLDIS   0x0100
/*
 * ENC28J60 PHY PHIE Register Bit Definitions
 */
#define PHIE_PLNKIE 0x0010
#define PHIE_PGEIE  0x0002
/*
 * ENC28J60 Packet Control Byte Bit Definitions
 */
#define PKTCTRL_PHUGEEN     0x08
#define PKTCTRL_PPADEN      0x04
#define PKTCTRL_PCRCEN      0x02
#define PKTCTRL_POVERRIDE   0x01
/*
 * SPI operation codes
 */
#define ENC28J60_READ_CTRL_REG  0x00
#define ENC28J60_READ_BUF_MEM   0x3A
#define ENC28J60_WRITE_CTRL_REG 0x40
#define ENC28J60_WRITE_BUF_MEM  0x7A
#define ENC28J60_BIT_FIELD_SET  0x80
#define ENC28J60_BIT_FIELD_CLR  0xA0
#define ENC28J60_SOFT_RESET     0xFF

// The ERXST_INI should be zero. See Silicon Errata:

// Sometimes, when ERXST or ERXND is written to, the exact value, 0000h, is stored in the Internal
// Receive Write Pointer instead of the ERXST address.
// Workaround:
// Use the lower segment of the buffer memory for the receive buffer, starting at address 0000h.
// Use the range (0000h to n) for the receive buffer, and ((n + 1) to 8191) for the transmit buffer.
#define ERXST_INI   0x0000U

// RX buffer end. Make sure this is an odd value ( See Rev. B1,B4,B5,B7 Silicon Errata 'Memory (Ethernet Buffer)')
#define ERXND_INI   (ENC28J60_ETH_RXBUF_SIZE_KB * 1024 - 1)

// TX buffer start.
#define ETXST_INI   (ERXND_INI + 1)

// TX buffer end at end of ethernet buffer memory.
#define ETXND_INI   0x1FFF

// max frame length which the conroller will accept:
#define MAX_FRAMELEN    ENC28J60_ETH_MTU_SIZE   // (note: maximum ethernet frame length would be 1518)

#define RX_NEXT_LEN 2U  // next packet pointer bytes
#define RX_STAT_LEN 4U  // receive status vector bytes
#define RX_CRC_LEN  4U  // CRC bytes
#define TX_CTRL_LEN 1U  // control byte
#define TX_STAT_LEN 7U  // transmit status vector bytes

#endif //ENC28J60_REG_H
