/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#ifndef _GPS_H
#define _GPS_H

void MX_GPIO_Init(void);
void MX_DMA_Init(void);
void MX_UART4_Init(void);
char * GET_GEO_LOCATION();
//void GET_GEO_LOCATION(char **result, size_t *result_length);

#endif // _GPS_H