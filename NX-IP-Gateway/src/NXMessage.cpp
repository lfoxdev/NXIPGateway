#include "NXMessage.h"

uint8_t NXMessage::msgLen(uint8_t msg_num)
{
	switch (msg_num & 0b01111111) {
		case 0x00: return sizeof(msg00_t);
		case 0x01: return sizeof(msg01_t);
		case 0x02: return sizeof(msg02_t);
		case 0x04: return sizeof(msg04_t);
		case 0x05: return sizeof(msg05_t);
		case 0x06: return sizeof(msg06_t);
		case 0x07: return sizeof(msg07_t);
		case 0x08: return sizeof(msg08_t);
		case 0x09: return sizeof(msg09_t);
		case 0x0A: return sizeof(msg0A_t);
		case 0x0B: return sizeof(msg0B_t);
		case 0x0C: return sizeof(msg0C_t);
		case 0x0D: return sizeof(msg0D_t);
		case 0x10: return sizeof(msg10_t);
		case 0x16: return sizeof(msg16_t);
		case 0x18: return sizeof(msg18_t);
		case 0x19: return sizeof(msg19_t);
		case 0x1B: return sizeof(msg1B_t);
		case 0x20: return sizeof(msg20_t);
		case 0x21: return sizeof(msg21_t);
		case 0x22: return sizeof(msg22_t);
		case 0x24: return sizeof(msg24_t);
		case 0x40: return sizeof(msg40_t);
		case 0x41: return sizeof(msg41_t);
		case 0x44: return sizeof(msg44_t);
		case 0x45: return sizeof(msg45_t);
		case 0x50: return sizeof(msg50_t);
		case 0x51: return sizeof(msg51_t);
		case 0x52: return sizeof(msg52_t);
		case 0x53: return sizeof(msg53_t);
		case 0x54: return sizeof(msg54_t);
		case 0x55: return sizeof(msg55_t);
		case 0x5C: return sizeof(msg5C_t);
		case 0x60: return sizeof(msg60_t);
		case 0x61: return sizeof(msg61_t);
		case 0x62: return sizeof(msg62_t);
		case 0x70: return sizeof(msg70_t);
		default: return 0;
	}
}
