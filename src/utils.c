#include "../include/ft_ping.h"

uint16_t calculate_checksum(void *data, int len) {

	uint16_t *ptr = (uint16_t *)data;
	uint32_t sum = 0;

	while (len > 1) {
		sum += *ptr++;
		len -= 2;
	}
	if (len == 1) {
		sum += *(uint8_t *)ptr << 8;
	}
	while (sum >> 16) {
		sum = (sum & 0xFFFF) + (sum >> 16);
	}
	return (~sum);
}