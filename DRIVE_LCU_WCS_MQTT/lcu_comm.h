#ifndef LCU_COMM_H
#define LCU_COMM_H

#include <stdint.h>

/* ---------------- INIT ---------------- */
/* Create TCP server, bind, listen, accept WCS */
int LCU_Comm_Init(void);

/* ---------------- RECEIVE ---------------- */
/* Receive command from WCS (blocking TCP recv) */
int LCU_Recv_Command(char *buf, int buf_len);

/* ---------------- DEINIT ---------------- */
void LCU_Comm_Close(void);

#endif /* LCU_COMM_H */
