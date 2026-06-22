#ifndef __CONFIG_DAM_H
#define __CONFIG_DMA_H

#include "stm32h7xx.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>


typedef struct
{
 __IO uint32_t  CCR[16];        	        /*!< DMA Multiplexer Channel x Control Register >!*/
 __IO uint32_t  RESERVED1[16];            	/*!< 保留 >!*/
 __IO uint32_t  CSR;      			        /*!< DMA Channel Status Register >!*/
 __IO uint32_t  CFR;      			        /*!< DMA Channel Clear Flag Register >!*/
 __IO uint32_t  RESERVED2[30]; 				/*!< 保留 >!*/
 __IO uint32_t  RGCR[8];        		    /*!< DMA Request Generator x Control Register >!*/
 __IO uint32_t  RGSR;        		        /*!< DMA Request Generator Status Register >!*/
 __IO uint32_t  RGCFR;       		        /*!< DMA Request Generator Clear Flag Register >!*/
}SYS_DMAMUX_TypeDef;

typedef struct
{
  __IO uint32_t ISR[2];       				/*!< DMA low interrupt status register,      Address offset: 0x00 */
  __IO uint32_t IFCR[2];      				/*!< DMA high interrupt flag clear register, Address offset: 0x0C */
  struct
  {
  __IO uint32_t CR;           				/*!< DMA stream x configuration register      */
  __IO uint32_t NDTR;         				/*!< DMA stream x number of data register     */
  __IO uint32_t PAR;          				/*!< DMA stream x peripheral address register */
  __IO uint32_t M0AR;         				/*!< DMA stream x memory 0 address register   */
  __IO uint32_t M1AR;         				/*!< DMA stream x memory 1 address register   */
  __IO uint32_t FCR;          				/*!< DMA stream x FIFO control register       */
  }Stream[8];                 				/*!< 八个通道 >!*/
} SYS_DMA_TypeDef;

typedef struct
{
  __IO uint32_t  GISR0;             		/*!< MDMA Global Interrupt/Status Register 0,          	 Address offset: 0x00 */
  __IO uint32_t  RESERVED0[15]; 			/*!< Reserved, 0x6C  */
  struct 
  {
    __IO uint32_t  CISR;      				/*!< MDMA channel x interrupt/status register,             Address offset: 0x40 */
    __IO uint32_t  CIFCR;    	 			/*!< MDMA channel x interrupt flag clear register,         Address offset: 0x44 */
    __IO uint32_t  CESR;      				/*!< MDMA Channel x error status register,                 Address offset: 0x48 */
    __IO uint32_t  CCR;      		 		/*!< MDMA channel x control register,                      Address offset: 0x4C */
    __IO uint32_t  CTCR;      				/*!< MDMA channel x Transfer Configuration register,       Address offset: 0x50 */
    __IO uint32_t  CBNDTR;    				/*!< MDMA Channel x block number of data register,         Address offset: 0x54 */
    __IO uint32_t  CSAR;      				/*!< MDMA channel x source address register,               Address offset: 0x58 */
    __IO uint32_t  CDAR;      				/*!< MDMA channel x destination address register,          Address offset: 0x5C */
    __IO uint32_t  CBRUR;     				/*!< MDMA channel x Block Repeat address Update register,  Address offset: 0x60 */
    __IO uint32_t  CLAR;      				/*!< MDMA channel x Link Address register,                 Address offset: 0x64 */
    __IO uint32_t  CTBR;      				/*!< MDMA channel x Trigger and Bus selection Register,    Address offset: 0x68 */
    uint32_t       RESERVED1; 				/*!< Reserved, 0x6C                                                             */
    __IO uint32_t  CMAR;      				/*!< MDMA channel x Mask address register,                 Address offset: 0x70 */
    __IO uint32_t  CMDR;      				/*!< MDMA channel x Mask Data register,                    Address offset: 0x74 */
    __IO uint32_t  RESERVED2[2]; 		  	/*!< Reserved, 0x6C  */
  }Channel[16];
}SYS_MDMA_TypeDef;


#define SYS_DMAMUX1               ((SYS_DMAMUX_TypeDef *) DMAMUX1_BASE)    // DMA1 DMA2
#define SYS_DMAMUX2               ((SYS_DMAMUX_TypeDef *) DMAMUX2_BASE)
#define SYS_DMA1                  ((SYS_DMA_TypeDef *) DMA1_BASE)
#define SYS_DMA2                  ((SYS_DMA_TypeDef *) DMA2_BASE)
#define SYS_MDMA                  ((SYS_MDMA_TypeDef *) MDMA_BASE)










#endif
