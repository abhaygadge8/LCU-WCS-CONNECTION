#ifndef MODBUS_FUNCTIONS_H
#define MODBUS_FUNCTIONS_H

#include <stdint.h>

/*===========================================================
 * Function Prototypes
 *===========================================================*/

/**
 * @brief  Initialize UDP socket for Modbus communication
 */
void MODBUS_Init(void);

/**
 * @brief  Close UDP socket and cleanup
 */
void MODBUS_Close(void);

/**
 * @brief  Read Holding Registers  (Function Code 0x03)
 * @param  slave_id   Modbus device address
 * @param  start_addr First register address
 * @param  num_regs   Number of registers to read
 * @param  rx_buf     Pointer to buffer for response
 * @return Number of bytes received or -1 on error
 */
int32_t MODBUS_ReadHolding(uint8_t slave_id, uint16_t start_addr,
                           uint16_t num_regs, uint8_t *rx_buf);

int32_t MODBUS_WriteHolding(uint8_t slave_id,
                            uint16_t start_addr,
                            const uint16_t *values,
                            uint16_t reg_count,
                            uint8_t *rx_buf);
/**
 * @brief  Read Input Registers  (Function Code 0x04)
 */
int32_t MODBUS_ReadInput(uint8_t slave_id, uint16_t start_addr,
                         uint16_t num_regs, uint8_t *rx_buf);

/**
 * @brief  Write Single Register (Function Code 0x06)
 */
int32_t MODBUS_WriteSingle(uint8_t slave_id, uint16_t reg_addr,
                           uint16_t value);

/**
 * @brief  Write Multiple Registers (Function Code 0x10)
 */
int32_t MODBUS_WriteMultiple(uint8_t slave_id, uint16_t start_addr,
                             uint16_t num_regs, const uint16_t *data);

/**
 * @brief Check if drive is reachable
 * @return 0 on success, -1 on failure
 */
int MODBUS_CheckConnection(void);

#endif /* MODBUS_FUNCTIONS_H */
