/*
 * OLED.h
 *
 * Created: 29.09.2022 14:15:38
 * Author: chrimeh
 */ 


#ifndef OLED_H_
#define OLED_H_

void OLED_Init(void);
void OLED_send_command(char msg);
void OLED_send_data(char msg);
void OLED_reset(void);
void OLED_goto_line(char page);
void OLED_goto_column(char column);
void OLED_home(void);
void OLED_pos(char page, char column);
void OLED_clear_line(char page);
void OLED_print_char(unsigned char msg);
void OLED_print(char msg[], int length);
void OLED_print_arrow(char page, char column);
void OLED_print_heart_empty(void);
void OLED_print_heart_full(void);

void OLED_print_arrow(char page, char column) {
	OLED_pos(page, column);
	OLED_send_data(0b00000000);
	OLED_send_data(0b00011000);
	OLED_send_data(0b00011000);
	OLED_send_data(0b00011000);
	OLED_send_data(0b01111110);
	OLED_send_data(0b00111100);
	OLED_send_data(0b00011000);
	OLED_send_data(0b00000000);
}

void OLED_print_heart_empty(void) {
	OLED_send_data(0b00000000);
	OLED_send_data(0b00001110);
	OLED_send_data(0b00010001);
	OLED_send_data(0b00100001);
	OLED_send_data(0b01000001);
	OLED_send_data(0b10000010);
	OLED_send_data(0b10000010);
	OLED_send_data(0b01000001);
	OLED_send_data(0b00100001);
	OLED_send_data(0b00010001);
	OLED_send_data(0b00001110);
	OLED_send_data(0b00000000);
}

void OLED_print_heart_full(void) {
	OLED_send_data(0b00000000);
	OLED_send_data(0b00001110);
	OLED_send_data(0b00011111);
	OLED_send_data(0b00111111);
	OLED_send_data(0b01111111);
	OLED_send_data(0b11111110);
	OLED_send_data(0b11111110);
	OLED_send_data(0b01111111);
	OLED_send_data(0b00111111);
	OLED_send_data(0b00011111);
	OLED_send_data(0b00001110);
	OLED_send_data(0b00000000);
}

void OLED_print(char msg[], int length) {
	for (int i = 0; i < length; i++) {
		OLED_print_char(msg[i]);
	}
}

void OLED_print_char(unsigned char msg) {
	for (int i = 0; i < 8; i++) {
		char byte = pgm_read_byte(&(font8[msg-32][i]));
		OLED_send_data(byte);
	}
}

void OLED_clear_line(char page) {
	OLED_goto_line(page);
	for (int i = 0; i < 128; i++) 
		OLED_send_data(0);
}

void OLED_pos(char page, char column) {
	OLED_goto_line(page);
	OLED_goto_column(column);
}

void OLED_home(void) {
	OLED_goto_line(0);
	OLED_goto_column(0);
}

void OLED_goto_line(char page) {
	if (0 <= page && page < 8)
		OLED_send_command(0xB0 + page);
}

void OLED_goto_column(char column) {
	if (0 <= column && column < 128) {
		char lower = column & 0b00001111;
		char upper = column >> 4;
		OLED_send_command(0x00 + lower);
		OLED_send_command(0x10 + upper);
	}
}

void OLED_send_command(char msg) {
	volatile char *oled_cmd = 0x1000;
	*oled_cmd = msg;
}

void OLED_send_data(char msg) {
	volatile char *oled_data = 0x1200;
	*oled_data = msg;
}

void OLED_reset(void) {
	for (int j = 0; j < 8; j++) {
		OLED_clear_line(j);
	}
	OLED_home();
}

// Copied directly from data sheet
void OLED_Init(void) {
	OLED_send_command(0xae); // display off
	OLED_send_command(0xa1); // segment remap
	OLED_send_command(0xda); // common pads hardware: alternative
	OLED_send_command(0x12);
	OLED_send_command(0xc8); // common output scan direction:com63~com0
	OLED_send_command(0xa8); // multiplex ration mode:63
	OLED_send_command(0x3f);
	OLED_send_command(0xd5); // display divide ratio/osc. freq. mode
	OLED_send_command(0x80);
	OLED_send_command(0x81); // contrast control
	OLED_send_command(0x50);
	OLED_send_command(0xd9); // set pre-charge period
	OLED_send_command(0x21);
	OLED_send_command(0x20); // Set Memory Addressing Mode
	OLED_send_command(0x02);
	OLED_send_command(0xdb); // VCOM deselect level mode
	OLED_send_command(0x30);
	OLED_send_command(0xad); // master configuration
	OLED_send_command(0x00);
	OLED_send_command(0xa4); // out follows RAM content
	OLED_send_command(0xa6); // set normal display
	OLED_send_command(0xaf); // display on 
}



#endif /* OLED_H_ */