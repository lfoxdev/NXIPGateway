#include "NX584Message.h"

uint8_t NX584Message::msgLen(uint8_t msg_num)
{
	switch (msg_num & 0b00111111) {
	case 0x01: return sizeof(nx584msg01_t);
	case 0x03: return sizeof(nx584msg03_t);
	case 0x04: return sizeof(nx584msg04_t);
	case 0x05: return sizeof(nx584msg05_t);
	case 0x06: return sizeof(nx584msg06_t);
	case 0x07: return sizeof(nx584msg07_t);
	case 0x08: return sizeof(nx584msg08_t);
	//case 0x09: return sizeof(nx584msg09_t);
	case 0x0A: return sizeof(nx584msg0A_t);
	//case 0x0B: return sizeof(nx584msg0B_t);
	case 0x10: return sizeof(nx584msg10_t);
	case 0x12: return sizeof(nx584msg12_t);
	case 0x1C: return sizeof(nx584msg1C_t);
	case 0x1D: return sizeof(nx584msg1D_t);
	case 0x1E: return sizeof(nx584msg1E_t);
	case 0x1F: return sizeof(nx584msg1F_t);
	case 0x21: return sizeof(nx584msg21_t);
	case 0x23: return sizeof(nx584msg23_t);
	case 0x24: return sizeof(nx584msg24_t);
	case 0x25: return sizeof(nx584msg25_t);
	case 0x26: return sizeof(nx584msg26_t);
	case 0x27: return sizeof(nx584msg27_t);
	case 0x28: return sizeof(nx584msg28_t);
	//case 0x29: return sizeof(nx584msg29_t);
	case 0x2A: return sizeof(nx584msg2A_t);
	//case 0x2B: return sizeof(nx584msg2B_t);
	//case 0x2C: return sizeof(nx584msg2C_t);
	case 0x30: return sizeof(nx584msg30_t);
	case 0x31: return sizeof(nx584msg31_t);
	case 0x32: return sizeof(nx584msg32_t);
	case 0x33: return sizeof(nx584msg33_t);
	case 0x34: return sizeof(nx584msg34_t);
	case 0x35: return sizeof(nx584msg35_t);
	case 0x36: return sizeof(nx584msg36_t);
	case 0x37: return sizeof(nx584msg37_t);
	//case 0x3A: return sizeof(nx584msg3A_t);
	case 0x3B: return sizeof(nx584msg3B_t);
	case 0x3C: return sizeof(nx584msg3C_t);
	case 0x3D: return sizeof(nx584msg3D_t);
	case 0x3E: return sizeof(nx584msg3E_t);
	case 0x3F: return sizeof(nx584msg3F_t);
	default: return 0;
	}
}
