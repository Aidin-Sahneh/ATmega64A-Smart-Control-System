/*******************************************************
This program was created by the
CodeWizardAVR V3.14 Advanced
Automatic Program Generator
© Copyright 1998-2014 Pavel Haiduc, HP InfoTech s.r.l.
http://www.hpinfotech.com

Project : K.N.Toosi Digital Lab Final Project
Version : 1.2 - Fully Commented
Date    : 2025-09-11
Author  : Aidin Sahneh
*******************************************************/

#include <mega64a.h>
#include <delay.h>
#include <stdio.h>
#include <string.h>
#include <alcd.h>

// --- Global State Variables ---
bit led1_state = 0, led2_state = 0, led3_state = 0, led4_state = 0;
bit relay1_state = 0, relay2_state = 0, buzzer_state = 0;
bit is_fahrenheit = 0, blinking_mode_active = 0;
bit last_sw1_state = 1, last_sw2_state = 1, last_sw3_state = 1, last_sw4_state = 1;

// Blinking logic variables
unsigned char timer_counter = 0;
unsigned char blinking_speed_ticks = 8; // Default to ~0.5s period

// RGB & Terminal variables
char rgb_buffer[8];
unsigned char rgb_buffer_index = 0;
float temperature;
unsigned int ms_counter = 0;
char received_text[21] = "No text received";
unsigned char received_text_index = 0;
bit send_data_flag = 0;

// --- USART Receiver Buffer ---
#define RX_BUFFER_SIZE0 16
char rx_buffer0[RX_BUFFER_SIZE0];
unsigned char rx_wr_index0=0,rx_rd_index0=0;
unsigned char rx_counter0=0;
bit rx_buffer_overflow0;

// USART0 Receiver interrupt service routine
interrupt [USART0_RXC] void usart0_rx_isr(void)
{
    char status,data;
    status=UCSR0A;
    data=UDR0;
    if ((status & ((1<<FE0) | (1<<UPE0) | (1<<DOR0)))==0)
    {
        rx_buffer0[rx_wr_index0++]=data;
        if (rx_wr_index0 == RX_BUFFER_SIZE0) rx_wr_index0=0;
        if (++rx_counter0 == RX_BUFFER_SIZE0)
        {
            rx_counter0=0;
            rx_buffer_overflow0=1;
        }
    }
}

// Get a character from the USART0 Receiver buffer
char getchar_usart0(void)
{
    char data;
    while (rx_counter0==0);
    data=rx_buffer0[rx_rd_index0++];
    if (rx_rd_index0 == RX_BUFFER_SIZE0) rx_rd_index0=0;
    #asm("cli")
    --rx_counter0;
    #asm("sei")
    return data;
}

// --- Timer 0 Interrupt (approx every 32.768ms) ---
interrupt [TIM0_OVF] void timer0_ovf_isr(void)
{
    // Automatic Data Sending Logic (Step 7)
    ms_counter++;
    if (ms_counter >= 15) // ~500ms
    {
        send_data_flag = 1;
        ms_counter = 0;
    }

    // Blinking LED Logic (Step 5-3)
    if (blinking_mode_active)
    {
        timer_counter++;
        if (timer_counter >= blinking_speed_ticks)
        {
            PORTB.0=!PORTB.0; PORTB.1=!PORTB.1;
            PORTB.2=!PORTB.2; PORTB.3=!PORTB.3;
            timer_counter = 0;
        }
    }
}

// --- ADC Read Function ---
unsigned int read_adc(unsigned char adc_input)
{
    ADMUX=adc_input | (1<<REFS0);
    delay_us(10);
    ADCSRA|=(1<<ADSC);
    while ((ADCSRA & (1<<ADIF))==0);
    ADCSRA|=(1<<ADIF);
    return ADCW;
}

// --- Custom RGB Helper Functions ---
unsigned char hex_char_to_int(char c){
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return 0;
}
void HexToRGB(char *hex, unsigned char *r, unsigned char *g, unsigned char *b){
    *r = (hex_char_to_int(hex[0]) * 16) + hex_char_to_int(hex[1]);
    *g = (hex_char_to_int(hex[2]) * 16) + hex_char_to_int(hex[3]);
    *b = (hex_char_to_int(hex[4]) * 16) + hex_char_to_int(hex[5]);
}

// --- Main Program ---
void main(void)
{
    // --- Initialization ---
    DDRA=0xFF; PORTA=0x00;
    DDRB=0b11101111; PORTB=0x00;
    DDRC=0b00000111; PORTC=0x00;
    DDRD=0x00; PORTD=0b00001111;
    TCCR0=0x05; TCNT0=0x00;
    TCCR1A=0xA1; TCCR1B=0x0B; TCNT1H=0x00; TCNT1L=0x00; OCR1AH=0x00; OCR1AL=0x00; OCR1BH=0x00; OCR1BL=0x00;
    TCCR2=0x6B; TCNT2=0x00; OCR2=0x00;
    TIMSK=0x01;
    UCSR0B=0x98; UCSR0C=0x06; UBRR0L=0x33;
    ADMUX=0x40; ADCSRA=0x86;
    lcd_init(20);
    #asm("sei")

    while (1)
    {
        char display_buffer[21];
        bit current_sw1_state, current_sw2_state, current_sw3_state, current_sw4_state;

        // --- Handle Incoming Bluetooth Commands ---
        if (rx_counter0 > 0)
        {
            char command = getchar_usart0();

            // RGB command parsing (e.g., #RRGGBB)
            if (command == '#') {
                rgb_buffer_index = 1;
            } else if (rgb_buffer_index > 0 && rgb_buffer_index < 7) {
                rgb_buffer[rgb_buffer_index - 1] = command;
                rgb_buffer_index++;
                if (rgb_buffer_index == 7) {
                    unsigned char red, green, blue;
                    HexToRGB(rgb_buffer, &red, &green, &blue);
                    OCR1AL = red; OCR1BL = green; OCR2 = blue;
                    rgb_buffer_index = 0;
                }
            // Terminal text parsing
            } else if (command == '\r') {
                received_text[received_text_index] = '\0';
                received_text_index = 0;
            } else if (command >= 32 && command <= 126 && received_text_index < 20) {
                received_text[received_text_index++] = command;
            // Single-character device control
            } else {
                /**************************************************************
                 * REQUIREMENT 3-3: Control both relays from LED/Lamp page.
                 * The app's LED/Lamp page sends a single character for ON/OFF.
                 * We assign 'L' for ON and 'l' for OFF to control both relays.
                 **************************************************************/
                if (command == 'L') {
                    PORTC.1=1; relay1_state=1;
                    PORTC.2=1; relay2_state=1;
                } else if (command == 'l') {
                    PORTC.1=0; relay1_state=0;
                    PORTC.2=0; relay2_state=0;
                }

                /**************************************************************
                 * REQUIREMENT 2-3: Control devices from the Switches page.
                 * The app sends characters like 'A'/'a' for Switch 1, etc.
                 * We use a switch statement to handle these commands.
                 **************************************************************/
                switch(command) {
                    // --- Relay & Buzzer Control ---
                    case 'E': PORTC.1=1; relay1_state=1; break;
                    case 'e': PORTC.1=0; relay1_state=0; break;
                    case 'F': PORTC.2=1; relay2_state=1; break;
                    case 'f': PORTC.2=0; relay2_state=0; break;
                    case 'G': PORTC.0=1; buzzer_state=1; break;
                    case 'g': PORTC.0=0; buzzer_state=0; break;
                }
                
                // --- LED Control (only if not in blinking mode) ---
                if (blinking_mode_active == 0) {
                    switch (command) {
                        case 'A': PORTB.0=1; led1_state=1; break;
                        case 'a': PORTB.0=0; led1_state=0; break;
                        case 'B': PORTB.1=1; led2_state=1; break;
                        case 'b': PORTB.1=0; led2_state=0; break;
                        case 'C': PORTB.2=1; led3_state=1; break;
                        case 'c': PORTB.2=0; led3_state=0; break;
                        case 'D': PORTB.3=1; led4_state=1; break;
                        case 'd': PORTB.3=0; led4_state=0; break;
                    }
                }
            }
        }

        // --- Handle On-board Push-Buttons ---
        current_sw1_state = PIND.0; // Temp Unit Toggle
        if ((last_sw1_state == 1) && (current_sw1_state == 0)) { is_fahrenheit = !is_fahrenheit; delay_ms(50); }
        last_sw1_state = current_sw1_state;

        current_sw2_state = PIND.1; // Blinking Mode Toggle
        if ((last_sw2_state == 1) && (current_sw2_state == 0)) {
            blinking_mode_active = !blinking_mode_active;
            if (blinking_mode_active == 0) { PORTB.0=0; PORTB.1=0; PORTB.2=0; PORTB.3=0; }
            delay_ms(50);
        }
        last_sw2_state = current_sw2_state;

        current_sw3_state = PIND.2; // Blinking Faster
        if ((last_sw3_state == 1) && (current_sw3_state == 0)) { if (blinking_speed_ticks > 2) blinking_speed_ticks--; delay_ms(50); }
        last_sw3_state = current_sw3_state;

        current_sw4_state = PIND.3; // Blinking Slower
        if ((last_sw4_state == 1) && (current_sw4_state == 0)) { if (blinking_speed_ticks < 30) blinking_speed_ticks++; delay_ms(50); }
        last_sw4_state = current_sw4_state;

        // --- Read Sensor ---
        temperature = (read_adc(0) * 500.0) / 1024.0;

        // --- Update LCD Display ---
        #asm("cli")
        lcd_clear();
        if (is_fahrenheit) {
            float fahrenheit = (temperature * 9.0 / 5.0) + 32.0;
            sprintf(display_buffer, "Temp:%.0f F   Bzr:%s", fahrenheit, buzzer_state ? "ON " : "OFF");
        } else {
            sprintf(display_buffer, "Temp:%.0f C   Bzr:%s", temperature, buzzer_state ? "ON " : "OFF");
        }
        lcd_gotoxy(0, 0); lcd_puts(display_buffer);
        sprintf(display_buffer, "L1:%s L2:%s R1:%s", led1_state ? "ON ":"OFF", led2_state ? "ON ":"OFF", relay1_state ? "ON ":"OFF");
        lcd_gotoxy(0, 1); lcd_puts(display_buffer);
        sprintf(display_buffer, "L3:%s L4:%s R2:%s", led3_state ? "ON ":"OFF", led4_state ? "ON ":"OFF", relay2_state ? "ON ":"OFF");
        lcd_gotoxy(0, 2); lcd_puts(display_buffer);
        lcd_gotoxy(0, 3); lcd_puts(received_text);
        #asm("sei")

        // --- Send Automatic Data via Bluetooth ---
        if (send_data_flag)
        {
            char tx_buffer[40];
            sprintf(tx_buffer, "Temp:%.0f%c, R1:%s, R2:%s\r\n",
                is_fahrenheit ? (temperature * 9.0 / 5.0) + 32.0 : temperature,
                is_fahrenheit ? 'F' : 'C',
                relay1_state ? "ON" : "OFF",
                relay2_state ? "ON" : "OFF");
            puts(tx_buffer);
            send_data_flag = 0;
        }

        delay_ms(150);
    }
}
