#ifndef RADAR_NET_CRC16_MODBUS_H
#define RADAR_NET_CRC16_MODBUS_H

#include <stdint.h>

/**
 * @brief CRC16_MODBUS
 * @details
 *  输入数据类型必须为uint8_t*,否则计算错误，经过调试发现，
 *  char*类型的输入数据在Qt中运行时，负数会占用64bit，
 *  0xad显示为0xffffffffffffffad
 * @param updata
 * @param len
 * @return
 */
uint16_t CRC16_MODBUS(const uint8_t *updata, int len);
uint16_t CRC16_MODBUS(const char *updata, int len);
#endif
