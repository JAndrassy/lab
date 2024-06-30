/*
 * Copyright (c) 2019 Tobias Jaster
 *
 * Modified by Zoltan Hudak
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef ENC28J60_ETH_DRV_H_
#define ENC28J60_ETH_DRV_H_

#include "mbed.h"

#include "enc28j60_reg.h"
#include "enc28j60_emac_config.h"

#define ENC28J60_READWRITE  1

/**
 * \brief Error code definitions
 *
 */
typedef struct
{
    uint8_t*    buf;
    uint16_t    len;
} payload_t;

typedef struct
{
    uint16_t    addr;
    payload_t   payload;
} packet_t;

/**
 * \brief Error code definitions
 *
 */
typedef enum
{
    ENC28J60_ERROR_OK               = 0U,   /*!< no error */
    ENC28J60_ERROR_TIMEOUT          = 1U,   /*!< timeout */
    ENC28J60_ERROR_BUSY             = 2U,   /*!< no error */
    ENC28J60_ERROR_PARAM            = 3U,   /*!< invalid parameter */
    ENC28J60_ERROR_INTERNAL         = 4U,   /*!< internal error */
    ENC28J60_ERROR_WRONG_ID         = 5U,   /*!< internal error */
    ENC28J60_ERROR_NOPACKET         = 10U,
    ENC28J60_ERROR_RECEIVE          = 11U,
    ENC28J60_ERROR_LASTPACKET       = 12U,
    ENC28J60_ERROR_POSITIONLENGTH   = 13U,  /*!< internal error */
    ENC28J60_ERROR_SIZE             = 20U,  /*!< internal error */
    ENC28J60_ERROR_FIFOFULL         = 21U,  /*!< internal error */
    ENC28J60_ERROR_NEXTPACKET       = 22U,  /*!< internal error */
} enc28j60_error_t;

/**
 * \brief Interrupt source definitions
 *
 */
typedef enum
{
    ENC28J60_INTERRUPT_ENABLE           = EIE_INTIE,
    ENC28J60_INTERRUPT_RX_PENDING_ENABLE= EIE_PKTIE,
    ENC28J60_INTERRUPT_DMA_ENABLE       = EIE_DMAIE,
    ENC28J60_INTERRUPT_LINK_STATE_ENABLE= EIE_LINKIE,
    ENC28J60_INTERRUPT_TX_ENABLE        = EIE_TXIE,
    ENC28J60_INTERRUPT_TX_ERROR_ENABLE  = EIE_TXERIE,
    ENC28J60_INTERRUPT_RX_ERROR_ENABLE  = EIE_RXERIE
} enc28j60_interrupt_source;

class   ENC28J60
{
public:
    ENC28J60(PinName mosi, PinName miso, PinName sclk, PinName cs);

    ENC28J60(mbed::SPI * spi, PinName cs);

    /**
     * \brief Initializes ENC28J60 Ethernet controller to a known default state:
     *          - device ID is checked
     *          - global interrupt is enabled, but all irq sources are disabled
     *          - Establish link enabled
     *          - Rx enabled
     *          - Tx enabled
     *        Init should be called prior to any other process and
     *        it's the caller's responsibility to follow proper call order.
     *
     * \return error code /ref enc28j60_error_t
     */
    void                init(void);

    /** This returns a unique 6-byte MAC address, based on the device UID
    *  This function overrides hal/common/mbed_interface.c function
    *  @param mac A 6-byte array to write the MAC address
    */
    void                mbed_mac_address(char* mac);
    MBED_WEAK uint8_t   mbed_otp_mac_address(char* mac);
    void                mbed_default_mac_address(char* mac);

    /**
     * \brief Initiates a soft reset, returns failure or success.
     *
     * \return error code /ref enc28j60_error_t
     */
    enc28j60_error_t    softReset(void);

    /**
     * \brief Set maximum transition unit by Rx fifo size.
     *        Note: The MTU will be smaller by 512 bytes,
     *        because the status uses this fixed space.
     *
     * \param[in] val Size of the fifo in kbytes
     */
    void                setRxBufSize(uint32_t val);

    /**
     * \brief Reset PHY
     *
     * \return error code /ref enc28j60_error_t
     */
    enc28j60_error_t    resetPhy(void);

    /**
     * \brief Enable receive
     */
    void                enableMacRecv(void);

    /**
     * \brief Disable receive
     */
    void                disableMacRecv(void);

    /**
     * \brief Read MAC address from EEPROM.
     *
     * \param[in,out] mac array will include the read MAC address in
     *                6 bytes hexadecimal format.
     *                It should be allocated by the caller to 6 bytes.
     *
     * \return error code /ref enc28j60_error_t
     */
    enc28j60_error_t    readMacAddr(char* mac);

    /**
     * \brief Write MAC address to EEPROM.
     *
     * \param[in,out] mac array will include the write MAC address in
     *                6 bytes hexadecimal format.
     *                It should be allocated by the caller to 6 bytes.
     *
     * \return error code /ref enc28j60_error_t
     */
    enc28j60_error_t    writeMacAddr(char* mac);

    /**
     * \brief Check device ID.
     *
     * \return error code /ref enc28j60_error_t
     */
    bool                check_id(void);

    /**
     * \brief Get the data size of the Rx buffer, aka Maximum Transition Unit
     *
     * \return Fifo data size in bytes
     */
    enc28j60_error_t    setWritePrt(uint16_t position, uint16_t offset);
    enc28j60_error_t    startPacketInTxBuffer(uint16_t payloadLen);
    enc28j60_error_t    loadDataInTxBuffer(uint8_t* buf, uint16_t len);
    enc28j60_error_t    transmitPacket(uint16_t payloadLen);

    /**
     * \brief Get the free space of Rx fifo in bytes.
     *
     * \param[in] dev Ethernet device structure \ref enc28j60_eth_dev_t
     *
     * \return Space available to store received data in bytes
     */
    uint32_t            getRxBufFreeSpace(void);

    /**
     * \brief Get the size of next unread packet in Rx buffer, using the peak
     *        register, which is not destructive so can be read asynchronously.
     *        Warning: In case of heavy receiving load, it's possible this register
     *        is not perfectly in sync.
     *
     * \param[in] dev Ethernet device structure \ref enc28j60_eth_dev_t
     *
     * \return Size in bytes of the next packet can be read from Rx fifo, according
     *         to the peek register.
     */
    enc28j60_error_t    setRxBufReadPtr(uint16_t position);
    enc28j60_error_t    getPacketInfo(packet_t* packet);
    void                abortPacketRead(uint16_t addr);
    void                readPacket(packet_t* packet);
    void                freeRxBuffer(void);
    uint16_t            getRecvPointer(void);
    uint16_t            getWritePointer(void);
    void                readBuf(uint8_t* data, uint16_t len);
    void                writeBuf(uint8_t* data, uint16_t len);
    uint8_t             readReg(uint8_t address);
    uint16_t            readRegPair(uint8_t address);
    void                writeReg(uint8_t address, uint8_t data);
    void                writeRegPair(uint8_t address, uint16_t data);
    enc28j60_error_t    phyRead(uint8_t address, uint16_t* data);
    enc28j60_error_t    phyWrite(uint8_t address, uint16_t data);
    bool                linkStatus(void);
    uint8_t             readOp(uint8_t op, uint8_t address);
    void                writeOp(uint8_t op, uint8_t address, uint8_t data);
private:
    void        _setBank(uint8_t address);
    void        _read(uint8_t cmd, uint8_t* buf, uint16_t len, bool blocking);
    void        _write(uint8_t cmd, uint8_t* buf, uint16_t len, bool blocking);
#ifdef ENC28J60_READWRITE
    void        _readwrite(uint8_t cmd, uint8_t* readbuf, uint8_t* writebuf, uint16_t len, bool blocking);
#endif
    mbed::SPI*        _spi;
    rtos::Mutex       _SPIMutex;
    mbed::DigitalOut  _cs;
    uint8_t     _bank;
    bool        _ready;
    uint16_t    _next;
};
#endif /* ENC28J60_ETH_DRV_H_ */
