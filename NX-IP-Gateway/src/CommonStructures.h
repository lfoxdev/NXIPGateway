// CommonStructures.h
// Human-friendly presenting of common data structures used in panel

#include <cstdint>
#include <WString.h>

#pragma once

struct dateTime {
    int year, month, day, hour, minute, weekday, second;
};

struct NXLogRecord {
	uint8_t id;
	uint8_t event;
	bool reporting;
	uint8_t month, day, hour, minute;
	uint8_t partition, item;
	uint8_t cid_qualifier;
	uint16_t cid_code;
	unsigned char sia[2];
	String eventStr();
	String itemStr();
	String toJSON();
};

