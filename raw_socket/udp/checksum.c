#include "header_udp.h"

/**
 * Расчёт чексумы 
 *
 * @param *buffer начало порядка байт для которых будет расчитываться чексума
 * @param size размер буфера
 *
 * @retval (uint16_t)(~cksum) расчитанная чексумма
 */
uint16_t checksum(uint16_t *buffer, int size) 
{
	uint32_t cksum = 0;

	while (size > 1) {
		cksum += *buffer++;
		size  -= sizeof(uint16_t);   
	}

	if (size){
		cksum += *(uint16_t*)buffer;   
	}

	cksum = (cksum >> 16) + (cksum & 0xffff);
	cksum += (cksum >> 16);
	
	return (uint16_t)(~cksum);
}
