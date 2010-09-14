/**
 *   @file  cpmacdrv.c
 *
 *   @brief   
 *      The driver is written for the CPMAC Ethernet controller. It follows
 *      the Network Boot Module Ethernet Driver Specifications.
 *
 *      If there are modifications to this driver required for it to work
 *      on the specific device then device authors are recommended to 
 *      create a copy of this file in the RBL directory and do the 
 *      necessary modifications. Please pass on the appropriate information
 *      to the ROM Boot Loader Framework Development Team.
 *
 *  \par
 *  NOTE:
 *      (C) Copyright 2008, Texas Instruments, Inc.
 *  \par
 */
#include "types.h"
#include "iblloc.h"
#include "device.h"
#include "net.h"
#include "cpmac_regs.h"
#include <string.h>


/**********************************************************************
 ************************** Local Definitions *************************
 **********************************************************************/

// Packet Flags for TX and RX
#define EMAC_DSC_FLAG_SOP                       0x80000000u
#define EMAC_DSC_FLAG_EOP                       0x40000000u
#define EMAC_DSC_FLAG_OWNER                     0x20000000u
#define EMAC_DSC_FLAG_EOQ                       0x10000000u
#define EMAC_DSC_FLAG_TDOWNCMPLT                0x08000000u
#define EMAC_DSC_FLAG_PASSCRC                   0x04000000u

// The following flags are RX only
#define EMAC_DSC_FLAG_JABBER                    0x02000000u
#define EMAC_DSC_FLAG_OVERSIZE                  0x01000000u
#define EMAC_DSC_FLAG_FRAGMENT                  0x00800000u
#define EMAC_DSC_FLAG_UNDERSIZED                0x00400000u
#define EMAC_DSC_FLAG_CONTROL                   0x00200000u
#define EMAC_DSC_FLAG_OVERRUN                   0x00100000u
#define EMAC_DSC_FLAG_CODEERROR                 0x00080000u
#define EMAC_DSC_FLAG_ALIGNERROR                0x00040000u
#define EMAC_DSC_FLAG_CRCERROR                  0x00020000u
#define EMAC_DSC_FLAG_NOMATCH                   0x00010000u

/**********************************************************************
 ************************** Local Structures **************************
 **********************************************************************/

/**
 * @brief 
 *  The structure describes the EMAC Descriptor.
 *
 * @details
 *  Ethernet drivers receives and transmits data through the descriptor
 *  object described here.
 */
typedef struct _EMAC_Desc 
{
    /**
     * @brief   Pointer to next descriptor in chain
     */
    struct _EMAC_Desc* pNext;

    /**
     * @brief   Pointer to the data buffer.
     */
    Uint8*             pBuffer;

    /**
     * @brief   Buffer Offset(MSW) and Length(LSW)
     */
    Uint32             BufOffLen;

    /**
     * @brief   Packet Flags(MSW) and Length(LSW)
     */
    volatile Uint32    PktFlgLen;
}EMAC_Desc;

/**
 * @brief 
 *  The structure describes the EMAC Master Control Block.
 *
 * @details
 *  The structure stores information required by the EMAC Driver;
 *  which includes the Receive and Transmit Buffer Descriptors.
 */
typedef struct EMAC_MCB
{
    /**
     * @brief   Pointer to the Receive Buffer Descriptor
     */
    EMAC_Desc*      rx_bd;

    /**
     * @brief   Pointer to the Transmit Buffer Descriptor
     */
    EMAC_Desc*      tx_bd;

    /**
     * @brief   Buffer for receiving data linked with the receive
     * buffer descriptor.
     */
    Uint8           rx_buffer[NET_MAX_MTU];
}EMAC_MCB;

/**********************************************************************
 ************************** Global Variables **************************
 **********************************************************************/

/**
 * @brief   Global Driver structure which keeps track of all the Ethernet
 * Driver related information.
 */
EMAC_MCB   emacMCB;

/**
 * @brief   EMAC Registers.
 */
CPMAC_REGS*  ptr_EMACRegs = (CPMAC_REGS *)EMAC_BASE_ADDRESS;

/**********************************************************************
 ************************** ETHDRV Functions **************************
 **********************************************************************/

/**
 *  @b Description
 *  @n  
 *      This is the Network Open API which is registered with the 
 *      NET Boot module to initialize the Ethernet device.
 *
 *  @param[in]  ptr_device
 *      Pointer to the NET Device structure which is being opened.
 *
 *  @retval
 *      Success -   0
 *  @retval
 *      Error   -   <0
 */
Int32 cpmac_drv_start (NET_DRV_DEVICE* ptr_device)
{
    Uint32           tmpval;
    volatile Uint32* pRegAddr;
    Int32            index;
    EMAC_Desc*       pDesc;
    
    /* Reset EMAC */
    ptr_EMACRegs->SOFTRESET = 0x1;
    while (ptr_EMACRegs->SOFTRESET != 0x0);

    /* Reset MAC Control */
    ptr_EMACRegs->MACCONTROL = 0;

    /* Must manually init HDPs to NULL */
    pRegAddr = &ptr_EMACRegs->TX0HDP;
    for( index=0; index<8; index++ )
        *pRegAddr++ = 0;
    pRegAddr = &ptr_EMACRegs->RX0HDP;
    for( index=0; index<8; index++ )
        *pRegAddr++ = 0;

    /* Initialize the RAM locations */
    for (index = 0; index < 32; index++)
    {
        ptr_EMACRegs->MACINDEX  = index;
        ptr_EMACRegs->MACADDRHI = 0;
        ptr_EMACRegs->MACADDRLO = 0;
    }

    /* Setup device MAC address */
    ptr_EMACRegs->MACINDEX = 0x0;

    /* Configure the MAC Address into the EMAC Controller. */
    tmpval = 0;
    for( index=3; index>=0; index-- )
        tmpval = (tmpval<<8) | *(ptr_device->mac_address+index);
    ptr_EMACRegs->MACADDRHI = tmpval;
    tmpval = *(ptr_device->mac_address+5);
    ptr_EMACRegs->MACADDRLO = CPMAC_MACADDRLO_VALID | CPMAC_MACADDRLO_MATCHFILT | 
                              (tmpval<<8) | *(ptr_device->mac_address+4);

    /* For us buffer offset will always be zero */
    ptr_EMACRegs->RXBUFFEROFFSET = 0;

    /* Reset RX (M)ulticast (B)roadcast (P)romiscuous Enable register */
    ptr_EMACRegs->RXMBPENABLE = 0;
    ptr_EMACRegs->MACHASH1    = 0;
    ptr_EMACRegs->MACHASH2    = 0;

    /* Clear Unicast RX on channel 0-7 */
    ptr_EMACRegs->RXUNICASTCLEAR = 0xFF;

    /* Make sure there are none of the interrupts are enabled. */
    ptr_EMACRegs->RXINTMASKCLEAR = 0xFF;
    ptr_EMACRegs->TXINTMASKCLEAR = 0xFF;
    ptr_EMACRegs->MACINTMASKCLEAR = 0x0;

    /* Setup the receive buffer descriptors. */
    pDesc = (EMAC_Desc *)_EMAC_DSC_BASE_ADDR;

    /* Initialize the receive buffer descriptor. */
    pDesc->pNext     = 0;
    pDesc->pBuffer   = &emacMCB.rx_buffer[0];
    pDesc->BufOffLen = NET_MAX_MTU;
    pDesc->PktFlgLen = EMAC_DSC_FLAG_OWNER;

    /* Store this buffer descriptor in the global EMAC MCB */
    emacMCB.rx_bd = pDesc;

    /* Get the transmit buffer descriptor; it comes right after the receive BD. */
    pDesc = pDesc + 1;

    /* Initialize the transmit buffer descriptor. */
    pDesc->pNext     = 0;
    pDesc->pBuffer   = 0;
    pDesc->BufOffLen = 0;
    pDesc->PktFlgLen = 0;

    /* Store this buffer descriptor in the global EMAC MCB */
    emacMCB.tx_bd = pDesc;

#ifdef EMAC_CACHE_SUPPORT
    /* Writeback the EMAC MCB Information to the PHYSICAL Memory. 
     * This is required because the buffer in the cache will always be
     * invalidated on the RECEIVE; if the buffer is not aligned on the
     * CACHE Line then it will result in the RX & TX BD in the structure
     * to also get invalidated to what resides in the PHYSICAL memory.
     * But if we have written back the structure here then the PHYSICAL
     * and CACHE are one and the same as far as the BD's are concerned. */
    Cache_wbL2((void *)&emacMCB, sizeof(emacMCB));
#endif

    /* Enable the receive and transmit. */
    ptr_EMACRegs->TXCONTROL = 0x1;
    ptr_EMACRegs->RXCONTROL = 0x1;
    
    /* Initialize the MAC Control: We set the Receive Ownership Bit and the Receive
     * Offset Length Word and enable the MII. */
    ptr_EMACRegs->MACCONTROL = CPMAC_MACCONTROL_RXOWNERSHIP | 
                               CPMAC_MACCONTROL_RXOFFLENBLOCK | 
                               CPMAC_MACCONTROL_MIIEN;

    /* Startup RX */
    ptr_EMACRegs->RX0HDP = (Uint32)emacMCB.rx_bd;

    /* Enable receive filters for channel 1 and all broadcast packets. */
    ptr_EMACRegs->RXUNICASTSET = 1;
    ptr_EMACRegs->RXMBPENABLE = CPMAC_RXMBPENABLE_RXBROADEN;

    /* Initialize the Device MDIO layer: The function returns only when 
     * the LINK is UP and RUNNING. */
    if (dev_mdio_open () < 0)
        return -1;        

    /* Debug Message: */
    mprintf ("DEBUG: Ethernet Link is UP \n");

    /* Hardware is up and running. */
    return 0;
}

/**
 *  @b Description
 *  @n
 *      This is the Network Send API which is registered with the NET boot module
 *      to send out packets. 
 *
 *  @param[in]   ptr_device
 *      Pointer to the network interface descriptor object.
 *  @param[in]   buffer
 *      Pointer to the packet which is to be transmitted.
 *  @param[in]   num_bytes
 *      Length of the packet which is transmitted.
 *
 *  @retval
 *      Success -   Number of bytes transmitted.
 *  @retval
 *      Error   -   <0
 */
Int32 cpmac_drv_send (NET_DRV_DEVICE* ptr_device, Uint8* buffer, int num_bytes)
{
    volatile EMAC_Desc* pDesc;

    /* Ensure the minimum ethernet size is sent out. */
    if (num_bytes < 64)
        num_bytes = 64;

#ifdef EMAC_CACHE_SUPPORT
    /* Clean the cache for external addesses */
    Cache_wbL2((void *)buffer, num_bytes);
#endif

    /* Get the pointer to the transmit buffer descriptor. */
    pDesc = emacMCB.tx_bd;

    /* Fill out the transmit buffer descriptor */
    pDesc->pNext     = 0;
    pDesc->pBuffer   = buffer;
    pDesc->BufOffLen = num_bytes;
    pDesc->PktFlgLen = EMAC_DSC_FLAG_SOP | EMAC_DSC_FLAG_EOP | num_bytes | EMAC_DSC_FLAG_OWNER;

    /* Send the packet out. */
    ptr_EMACRegs->TX0HDP = (Uint32)pDesc;

    /* Loop around till the transmission is complete. */
    pDesc = (EMAC_Desc *)ptr_EMACRegs->TX0CP;
    while (pDesc->PktFlgLen & EMAC_DSC_FLAG_OWNER);

    /* The packet has been successfully transmitted. */
    return num_bytes;
}

/**
 *  @b Description
 *  @n
 *      This is the Network Receive API which is registered with the NET boot module
 *      to receive packets. 
 *
 *  @param[in]   ptr_device
 *      Pointer to the network interface descriptor object.
 *  @param[out]   buffer
 *      Pointer to the packet which is populated with the received data
 *
 *  @retval
 *      Success -   Number of bytes received.
 *  @retval
 *      Error   -   <0
 */
Int32 cpmac_drv_receive (NET_DRV_DEVICE* ptr_device, Uint8* buffer)
{
    Int32       bytes_received = 0;
    EMAC_Desc*  pDesc;

    /* Initialize the number of bytes received. */
    bytes_received = 0;

    /* Read the completion register. We know for sure if the SOP flag is set then
     * a packet has been received and needs to be picked up from the controller. */
    pDesc = (EMAC_Desc *)ptr_EMACRegs->RX0CP;
    if (pDesc->PktFlgLen & EMAC_DSC_FLAG_SOP)
    {
        /* Acknowledge that the descriptor has been processed. */
        ptr_EMACRegs->RX0CP = (Uint32)pDesc;

#ifdef EMAC_CACHE_SUPPORT
		/* External Memory Support: Invalidate the cache. */
        Cache_invL2((void *)(pDesc->pBuffer), NET_MAX_MTU);
#endif
        /* Remember the number of bytes received. */
        bytes_received = (pDesc->PktFlgLen & 0xFFFF);

        /* The descriptor is now free to receive more data. Set the status accordingly. */
        pDesc->BufOffLen = NET_MAX_MTU;
        pDesc->PktFlgLen = EMAC_DSC_FLAG_OWNER;

        /* Copy the data from the descriptor buffer to the supplied buffer. */
        memcpy((void *)buffer, (void *)pDesc->pBuffer, bytes_received);

        /* Put this descriptor back to the HDP. */
        ptr_EMACRegs->RX0HDP = (Uint32)pDesc;
    }

    /* Return the number of bytes received. */
    return bytes_received;
}

/**
 *  @b Description
 *  @n
 *      This is the Network Close API which is registered with the NET boot module
 *      to close and shutdown the Ethernet controller.
 *
 *  @param[in]   ptr_device
 *      Pointer to the network interface descriptor object.
 *
 *  @retval
 *      Success -   0
 *  @retval
 *      Error   -   <0
 */
Int32 cpmac_drv_stop (NET_DRV_DEVICE* ptr_device)
{
    return 0;
}


