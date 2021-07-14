/*
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "pico/stdlib.h"
#include "usb_audio.h"
#include "hardware/pio.h"
#include "pdm.pio.h"
#include "pico/multicore.h"

PIO pio = pio1;
uint sm;

void core1_worker() {
    uint32_t a=0;
    
    for(int i =0; i< 16;i++){
        a+= 1<< (i*2);
    }
    // Infinte While Loop to wait for interrupt
    while (1){
        if(pio_sm_is_tx_fifo_empty(pio,sm)){
            pio->txf[sm] = a;
            pio->txf[sm] = a;
            pio->txf[sm] = a;
            pio->txf[sm] = a;
            
        }
        if (multicore_fifo_rvalid()){
            uint32_t rec = multicore_fifo_pop_blocking();
            a = pdm_4_os32((int16_t)(rec));
            while(pio_sm_is_tx_fifo_full(pio, sm)){}
            pio->txf[sm] = a;
        }
    }
}

int main(void) {
    gpio_init(23);
    gpio_set_dir(23, GPIO_OUT);
    gpio_put(23, 1);
    
    set_sys_clock_khz(115200, false);
    uint offset = pio_add_program(pio, &pdm_program);
    sm = pio_claim_unused_sm(pio, true);
    pdm_program_init(pio, sm, offset, 14, 115200 * 1000 / (48000.0 * 32));
    multicore_launch_core1(core1_worker);
    sleep_ms(1000);
    usb_sound_card_init();
    
    while (1){};
}
