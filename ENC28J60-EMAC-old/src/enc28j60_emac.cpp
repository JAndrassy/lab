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
#include "enc28j60_emac.h"
#include "mbed_interface.h"
#include "mbed_wait_api.h"
#include "mbed_assert.h"
#include "netsocket/nsapi_types.h"
#include "mbed_shared_queues.h"
#include "EthernetInterface.h"

#include <Arduino.h> // for digitalPinToPinName

#ifndef ENC28J60_SCK
#define ENC28J60_SCK digitalPinToPinName(PIN_SPI_SCK)
#define ENC28J60_MOSI digitalPinToPinName(PIN_SPI_MOSI)
#define ENC28J60_MISO digitalPinToPinName(PIN_SPI_MISO)
#endif
#ifndef ENC28J60_CS
#define ENC28J60_CS digitalPinToPinName(PIN_SPI_SS)
#endif

using namespace mbed;
using namespace rtos;
using namespace std::chrono_literals;

/**
 * @brief
 * @note
 * @param
 * @retval
 */
ENC28J60_EMAC::ENC28J60_EMAC() :
    _enc28j60(new ENC28J60(ENC28J60_MOSI, ENC28J60_MISO, ENC28J60_SCK, ENC28J60_CS)),
    _prev_link_status_up(PHY_STATE_LINK_DOWN),
    _link_status_task_handle(0),
    _receive_task_handle(0),
    _memory_manager(NULL)
{ }

/**
 * @brief
 * @note    Allocates buffer chain from a pool and filles it with incoming packet.
 * @param
 * @retval  Data from incoming packet
 */
emac_mem_buf_t* ENC28J60_EMAC::low_level_input()
{
    emac_mem_buf_t*     chain;
    emac_mem_buf_t*     buf;
    packet_t            packet;

    if (_enc28j60->getPacketInfo(&packet) != ENC28J60_ERROR_OK) {
        return NULL;
    }

    if (packet.payload.len == 0) {
        return NULL;
    }

    // Allocate a buffer chain from the memory pool.
//    chain = _memory_manager->alloc_pool(packet.payload.len, ENC28J60_BUFF_ALIGNMENT);
//    if (chain == NULL) {
      chain = _memory_manager->alloc_heap(packet.payload.len, ENC28J60_BUFF_ALIGNMENT);
//    }
    buf = chain;
    if (buf == NULL) {
      _enc28j60->abortPacketRead(packet.addr);
      return NULL;
    }

    // Iterate through the buffer chain and fill it with packet payload.
    while (buf != NULL) {
        packet.payload.buf = (uint8_t*)_memory_manager->get_ptr(buf);
        packet.payload.len = (uint16_t) (_memory_manager->get_len(buf));

        // Fill the ethernet stack buffer with packet payload received by ENC28J60.
        _enc28j60->readPacket(&packet);

        buf = _memory_manager->get_next(buf);
    }
    _enc28j60->freeRxBuffer();  // make room in ENC28J60 receive buffer for new packets

    // Return the buffer chain filled with packet payload.
    return chain;
}

/**
 * @brief   Receive task.
 * @note    Passes packet payload received by ENC28J60 to the ethernet stack
 * @param
 * @retval
 */
void ENC28J60_EMAC::receive_task()
{
    emac_mem_buf_t*     payload;

    _ethLockMutex.lock();
    payload = low_level_input();
    if (payload != NULL) {
        if (_emac_link_input_cb) {
            _emac_link_input_cb(payload);   // pass packet payload to the ethernet stack
        }
    }
    
    _ethLockMutex.unlock();
}


/**
 * @brief
 * @note    Passes a packet payload from the ethernet stack to ENC28J60 for transmittion
 * @param
 * @retval
 */
bool ENC28J60_EMAC::link_out(emac_mem_buf_t* buf)
{
    emac_mem_buf_t*     chain = buf;
    enc28j60_error_t    error;

    if (buf == NULL) {
        return false;
    }

    _ethLockMutex.lock();

    uint16_t packetLen = _memory_manager->get_total_len(chain);
    error = _enc28j60->startPacketInTxBuffer(packetLen);
    if (error != ENC28J60_ERROR_OK) {
        _memory_manager->free(chain);
        _ethLockMutex.unlock();
        return false;
    }

    // Iterate through the buffer chain and fill the packet with payload.
    while (buf != NULL) {
        uint16_t len = _memory_manager->get_len(buf);
    	  uint8_t* data = (uint8_t *) (_memory_manager->get_ptr(buf));
        error = _enc28j60->loadDataInTxBuffer(data, len);
        if (error != ENC28J60_ERROR_OK) {
            _memory_manager->free(chain);
            _ethLockMutex.unlock();
            return false;
        }
        buf = _memory_manager->get_next(buf);
    }

    _memory_manager->free(chain);

    error = _enc28j60->transmitPacket(packetLen);
    if (error != ENC28J60_ERROR_OK) {
        _ethLockMutex.unlock();
        return false;
    }

    _ethLockMutex.unlock();

    return true;
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
void ENC28J60_EMAC::link_status_task()
{
    uint16_t    phy_basic_status_reg_value = 0;
    bool        current_link_status_up = false;

    /* Get current status */

    _ethLockMutex.lock();
    _enc28j60->phyRead(PHSTAT2, &phy_basic_status_reg_value);

    current_link_status_up = (bool) ((phy_basic_status_reg_value & PHSTAT2_LSTAT) != 0);

    /* Compare with previous state */
    if (current_link_status_up != _prev_link_status_up) {
        if (_emac_link_state_cb) {
            _emac_link_state_cb(current_link_status_up);
        }

        _prev_link_status_up = current_link_status_up;
    }

    _ethLockMutex.unlock();
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
bool ENC28J60_EMAC::power_up()
{
    volatile uint32_t   timeout = 500;
    _enc28j60->writeOp(ENC28J60_BIT_FIELD_CLR, ECON2, ECON2_PWRSV);
    while (_enc28j60->readReg(ESTAT_CLKRDY) == 0) {
        ThisThread::sleep_for(1ms);
        timeout--;
        if (timeout == 0) {
            return false;
        }
    }

    /* Trigger thread to deal with any RX packets that arrived
     * before receiver_thread was started */
    _receive_task_handle = mbed::mbed_event_queue()->call_every
        (
            RECEIVE_TASK_PERIOD_MS,
            mbed::callback(this, &ENC28J60_EMAC::receive_task)
        );

    _prev_link_status_up = PHY_STATE_LINK_DOWN;
    mbed::mbed_event_queue()->call(mbed::callback(this, &ENC28J60_EMAC::link_status_task));

    /* Allow the Link Status task to detect the initial link state */
    ThisThread::sleep_for(10ms);
    _link_status_task_handle = mbed::mbed_event_queue()->call_every
        (
            LINK_STATUS_TASK_PERIOD_MS,
            mbed::callback(this, &ENC28J60_EMAC::link_status_task)
        );

    return true;
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
uint32_t ENC28J60_EMAC::get_mtu_size() const
{
    return ENC28J60_ETH_MTU_SIZE;
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
uint32_t ENC28J60_EMAC::get_align_preference() const
{
    return ENC28J60_BUFF_ALIGNMENT;
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
void ENC28J60_EMAC::get_ifname(char* name, uint8_t size) const
{
    memcpy(name, ENC28J60_ETH_IF_NAME, (size < sizeof(ENC28J60_ETH_IF_NAME)) ? size : sizeof(ENC28J60_ETH_IF_NAME));
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
uint8_t ENC28J60_EMAC::get_hwaddr_size() const
{
    return ENC28J60_HWADDR_SIZE;
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
bool ENC28J60_EMAC::get_hwaddr(uint8_t* addr) const
{
    enc28j60_error_t    error = _enc28j60->readMacAddr((char*)addr);
    if (error == ENC28J60_ERROR_OK) {
        return true;
    }
    else {
        return false;
    }
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
void ENC28J60_EMAC::set_hwaddr(const uint8_t* addr)
{
    if (!addr) {
        return;
    }

    memcpy(_hwaddr, addr, sizeof _hwaddr);
    _ethLockMutex.lock();

    enc28j60_error_t    error = _enc28j60->writeMacAddr((char*)addr);
    _ethLockMutex.unlock();
    if (error) {
        return;
    }
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
void ENC28J60_EMAC::set_link_input_cb(emac_link_input_cb_t input_cb)
{
    _emac_link_input_cb = input_cb;
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
void ENC28J60_EMAC::set_link_state_cb(emac_link_state_change_cb_t state_cb)
{
    _emac_link_state_cb = state_cb;
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
void ENC28J60_EMAC::add_multicast_group(const uint8_t* addr)
{
    // No action for now
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
void ENC28J60_EMAC::remove_multicast_group(const uint8_t* addr)
{
    // No action for now
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
void ENC28J60_EMAC::set_all_multicast(bool all)
{
    // No action for now
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
void ENC28J60_EMAC::power_down()
{
    _enc28j60->disableMacRecv();
    if (_enc28j60->readReg(ESTAT_RXBUSY) != 0) {
        _enc28j60->enableMacRecv();
        return;
    }

    if (_enc28j60->readReg(ECON1_TXRTS) != 0) {
        _enc28j60->enableMacRecv();
        return;
    }

    _enc28j60->writeOp(ENC28J60_BIT_FIELD_SET, ECON2, ECON2_VRPS);
    _enc28j60->writeOp(ENC28J60_BIT_FIELD_SET, ECON2, ECON2_PWRSV);
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
void ENC28J60_EMAC::set_memory_manager(EMACMemoryManager& mem_mngr)
{
    _memory_manager = &mem_mngr;
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
ENC28J60_EMAC& ENC28J60_EMAC::get_instance()
{
    static ENC28J60_EMAC    emac;
    return emac;
}

EMAC& EMAC::get_default_instance()
{
    return ENC28J60_EMAC::get_instance();
}

EthInterface *EthInterface::get_target_default_instance()
{
    static EthernetInterface ethernet;
    return &ethernet;
}
