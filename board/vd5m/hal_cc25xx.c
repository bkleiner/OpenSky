/*
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

   author: fishpepper <AT> gmail.com
*/

#include "hal_cc25xx.h"
#include "cc25xx.h"
#include "hal_defines.h"
#include "hal_dma.h"
#include "delay.h"
#include "timeout.h"
#include "debug.h"
#include "frsky.h"
#include <cc2510fx.h>

EXTERNAL_MEMORY volatile uint8_t hal_cc25xx_mode;

void hal_cc25xx_init(void) {
    hal_cc25xx_mode = CC25XX_MODE_RX;
}

void hal_cc25xx_set_gdo_mode(void) {
    //not necessary here IOCFG0 = 0x01
    //not necessary here IOCFG2 = 0x0E
}

void hal_cc25xx_disable_rf_interrupt(void) {
    IEN2 &= ~(IEN2_RFIE);
    RFIM = 0;
}

void hal_cc25xx_enter_rxmode(void) {
    //set up dma for radio--->buffer
    hal_cc25xx_setup_rf_dma(CC25XX_MODE_RX);

    //configure interrupt for every received packet
    IEN2 |= (IEN2_RFIE);

    //set highest prio for ch0 (RF)
    IP0 |= (1<<0);
    IP1 |= (1<<0);

    //mask done irq
    RFIM = (1<<4);
    //interrupts should be enabled globally already..
    //skip this! sei();
}

void hal_cc25xx_enter_txmode(void) {
    //abort ch0
    DMAARM = DMA_ARM_ABORT | DMA_ARM_CH0;
    hal_cc25xx_setup_rf_dma(CC25XX_MODE_TX);
}

void hal_cc25xx_setup_rf_dma(uint8_t mode){
    // CPU has priority over DMA
    // Use 8 bits for transfer count
    // No DMA interrupt when done
    // DMA triggers on radio
    // Single transfer per trigger.
    // One byte is transferred each time.

    hal_dma_config[0].PRIORITY       = DMA_PRI_HIGH;
    hal_dma_config[0].M8             = DMA_M8_USE_8_BITS;
    hal_dma_config[0].IRQMASK        = DMA_IRQMASK_DISABLE;
    hal_dma_config[0].TRIG           = DMA_TRIG_RADIO;
    hal_dma_config[0].TMODE          = DMA_TMODE_SINGLE;
    hal_dma_config[0].WORDSIZE       = DMA_WORDSIZE_BYTE;

    //store mode
    hal_cc25xx_mode = mode;

    if (hal_cc25xx_mode == CC25XX_MODE_TX) {
        // Transmitter specific DMA settings
        // Source: radioPktBuffer
        // Destination: RFD register
        // Use the first byte read + 1
        // Sets the maximum transfer count allowed (length byte + data)
        // Data source address is incremented by 1 byte
        // Destination address is constant
        SET_WORD(hal_dma_config[0].SRCADDRH, hal_dma_config[0].SRCADDRL, frsky_packet_buffer);
        SET_WORD(hal_dma_config[0].DESTADDRH, hal_dma_config[0].DESTADDRL, &X_RFD);
        hal_dma_config[0].VLEN           = DMA_VLEN_FIRST_BYTE_P_1;
        SET_WORD(hal_dma_config[0].LENH, hal_dma_config[0].LENL, (FRSKY_PACKET_LENGTH+1));
        hal_dma_config[0].SRCINC         = DMA_SRCINC_1;
        hal_dma_config[0].DESTINC        = DMA_DESTINC_0;
    }else{
        // Receiver specific DMA settings:
        // Source: RFD register
        // Destination: radioPktBuffer
        // Use the first byte read + 3 (incl. 2 status bytes)
        // Sets maximum transfer count allowed (length byte + data + 2 status bytes)
        // Data source address is constant
        // Destination address is incremented by 1 byte for each write
        SET_WORD(hal_dma_config[0].SRCADDRH, hal_dma_config[0].SRCADDRL, &X_RFD);
        SET_WORD(hal_dma_config[0].DESTADDRH, hal_dma_config[0].DESTADDRL, frsky_packet_buffer);
        hal_dma_config[0].VLEN           = DMA_VLEN_FIRST_BYTE_P_3;
        SET_WORD(hal_dma_config[0].LENH, hal_dma_config[0].LENL, (FRSKY_PACKET_LENGTH+3));
        hal_dma_config[0].SRCINC         = DMA_SRCINC_0;
        hal_dma_config[0].DESTINC        = DMA_DESTINC_1;
    }

    // Save pointer to the DMA configuration struct into DMA-channel 0
    // configuration registers
    SET_WORD(DMA0CFGH, DMA0CFGL, &hal_dma_config[0]);

    frsky_packet_received = 0;
}

void hal_cc25xx_enable_receive(void) {
    //start receiving on dma channel 0
    DMAARM = DMA_ARM_CH0;
    //FIXME: test this
    cc25xx_strobe(RFST_SRX);
}



void hal_cc25xx_rf_interrupt(void) __interrupt RF_VECTOR{
    //clear int flag
    RFIF &= ~(1<<4);

    //clear general statistics reg
    S1CON &= ~0x03;


    if (hal_cc25xx_mode == CC25XX_MODE_RX){
        //mark as received:
        frsky_packet_received = 1;
        //re arm DMA channel 0
        hal_cc25xx_enable_receive();
    }else{
        frsky_packet_sent = 1;
    }
}



void hal_cc25xx_transmit_packet(volatile uint8_t *buffer, uint8_t len) {
    RFST = RFST_STX;

    //start transmitting on dma channel 0
    DMAARM = DMA_ARM_CH0;

    //tricky: this will force an int request and
    //        initiate the actual transmission
    S1CON |= 0x03;


    //wait some time here. packet should be sent within our 9ms
    //frame (actually within 5-6ms). if not print an error...
    frsky_packet_sent = 0;
    while(!frsky_packet_sent){
        if (timeout_timed_out()){
            break;
        }
    }
    if (timeout_timed_out()){
        debug("\nfrsky: ERROR tx timed out\n");
    }

    frsky_packet_sent = 0;
}
