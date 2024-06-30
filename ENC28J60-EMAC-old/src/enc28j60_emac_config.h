/*
 * enc28j60_emac_config.h
 *
 *  Created on: 27.08.2019
 *      Author: tobias
 *
 * Modified by Zoltan Hudak
 *
 */

#ifndef ENC28J60_EMAC_CONFIG_H_
#define ENC28J60_EMAC_CONFIG_H_

/*
 *  ENC28J60 receive buffer size in kylobytes
 */
#define ENC28J60_ETH_RXBUF_SIZE_KB           6U
#define ENC28J60_HWADDR_SIZE                 6U
#define ENC28J60_BUFF_ALIGNMENT              4U

/*
 * Maximum Transfer Unit
 * The IEEE 802.3 specification limits the data portion of the 802.3 frame
 * to a minimum of 46 and a maximum of 1522 bytes, this is on L2 level.
 */
#define ENC28J60_ETH_MTU_SIZE                1500U
#define ENC28J60_ETH_IF_NAME                 "enc28j60"

/** \brief Defines for receiver thread */
#define LINK_STATUS_TASK_PERIOD_MS           200ms
#define RECEIVE_TASK_PERIOD_MS               20ms
#define PHY_STATE_LINK_DOWN                  false
#define PHY_STATE_LINK_UP                    true
#define CRC_LENGTH_BYTES                     4U

#endif /* ENC28J60_EMAC_CONFIG_H_ */
