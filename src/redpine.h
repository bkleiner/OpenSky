/*
    Copyright 2017 fishpepper <AT> gmail.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http:// www.gnu.org/licenses/>.

   author: fishpepper <AT> gmail.com
*/

#ifndef REDPINE_H_
#define REDPINE_H_

#include <stdint.h>
#include "main.h"
#include "hal_cc25xx.h"

#define REDPINE_HOPTABLE_SIZE 49

#define REDPINE_COUNT_RXSTATS 50


// extern EXTERNAL_MEMORY uint8_t txid[2];
// extern EXTERNAL_MEMORY int8_t redpine_freq_offset;
extern EXTERNAL_MEMORY uint8_t redpine_current_ch_idx;
extern EXTERNAL_MEMORY uint8_t redpine_diversity_count;
// rssi
extern EXTERNAL_MEMORY uint8_t rssi;
extern EXTERNAL_MEMORY uint8_t redpine_link_quality;
// pll calibration
extern EXTERNAL_MEMORY uint8_t redpine_calib_fscal1_table[REDPINE_HOPTABLE_SIZE];
extern EXTERNAL_MEMORY uint8_t redpine_calib_fscal2;
extern EXTERNAL_MEMORY uint8_t redpine_calib_fscal3;
// extern EXTERNAL_MEMORY int16_t redpine_freq_offset_acc;

#define REDPINE_PACKET_SIZE 11
#define REDPINE_PACKET_SIZE_W_ADDONS (REDPINE_PACKET_SIZE + 2)

#define REDPINE_PACKET_LENGTH REDPINE_PACKET_SIZE
#define REDPINE_PACKET_BUFFER_SIZE REDPINE_PACKET_SIZE_W_ADDONS
extern EXTERNAL_MEMORY volatile uint8_t packet[REDPINE_PACKET_BUFFER_SIZE];
extern EXTERNAL_MEMORY volatile uint8_t redpine_packet_received;

void redpine_init(void);
void redpine_show_partinfo(void);
void redpine_configure(void);
uint8_t redpine_bind_jumper_set(void);
void redpine_do_bind(void);
void redpine_configure_address(void);
void redpine_autotune(void);
void redpine_enter_rxmode(uint8_t channel);
void redpine_tune_channel(uint8_t ch);
void redpine_handle_overflows(void);
void redpine_fetch_txid_and_hoptable(void);
void redpine_calib_pll(void);
void redpine_main(void);
uint8_t redpine_extract_rssi(uint8_t rssi_raw);
void redpine_increment_channel(int8_t cnt);
void redpine_set_channel(uint8_t hop_index);
void redpine_send_telemetry(uint8_t telemetry_id);
void redpine_update_ppm(void);


#if 0
void redpine_fetch_txid_and_hoptable(void);
void redpine_configure_address(void);
void redpine_calib_pll(void);
void redpine_rf_interrupt(void) __interrupt RF_VECTOR;
void redpine_handle_overflows(void);
void redpine_setup_rf_dma(uint8_t);
uint8_t redpine_extract_rssi(uint8_t rssi_raw);
void redpine_enter_rxmode(uint8_t ch);
void redpine_frame_sniffer(void);
uint8_t redpine_append_hub_data(uint8_t sensor_id, uint16_t value, uint8_t *buf);

// binding
uint8_t redpine_bind_jumper_set(void);
void redpine_do_bind(void);
void redpine_store_config(void);
#endif  // 0


// packet data example:
// BIND:   [11 03 01 16 68 14 7E BF 15 56 97 00 00 00 00 00 00 0B F8 AF ]
// NORMAL: [11 16 68 ... ]
// TX:                 11 16 68 7A 1B 0B CA CB CF C4 88 85 CB CB CB 92 8B 78 21 AF
// TELEMETRY WITH HUB: 11 16 68 60 64 5B 00 00 5E 3B 09 00 5E 5E 3B 09 00 5E 48 B1
#define REDPINE_VALID_FRAMELENGTH(_b) (_b[0] == 0x11)
#define REDPINE_VALID_TXID(_b) ((_b[1] == storage.txid[0]) && (_b[2] == storage.txid[1]))
#define REDPINE_VALID_PACKET_BIND(_b) ((_b[2] == 0x01))
#define REDPINE_VALID_PACKET(_b)  ( REDPINE_VALID_TXID(_b) )

/*
#define REDPINE_HUB_TELEMETRY_HEADER 0x5E
#define REDPINE_HUB_TELEMETRY_VOLTAGE 0x39 // not really documented, seems to be volt in 0.1V steps...
#define REDPINE_HUB_TELEMETRY_VOLTAGE_BEFORE 0x3A
#define REDPINE_HUB_TELEMETRY_VOLTAGE_AFTER  0x3B
#define REDPINE_HUB_TELEMETRY_CURRENT        0x28
*/

#endif  // REDPINE_H_
