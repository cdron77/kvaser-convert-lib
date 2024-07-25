/**
 * Dummy driver for devices which don't use SPI flash through the pciefd driver
 *
 * All functions will return "happy" status without doing anything
 */

/***************************** Include Files *********************************/
#include "util.h"
#include "spi_flash.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/**
* Initialize and prepare handle
*
* @param spif       Ignored
* @param base_addr  Ignored
* @return int       SPI_FLASH_STATUS_SUCCESS
*/
int SPI_FLASH_dummy_init(struct spi_flash *spif, void *base_addr)
{
    NOT_USED(spif);
    NOT_USED(base_addr);
    return SPI_FLASH_STATUS_SUCCESS;
}

/**
* Deinitialize
*
* @param spif       Ignored
*/
void SPI_FLASH_dummy_deinit(struct spi_flash *spif)
{
    NOT_USED(spif);
}


/**
* Start the SPI driver so that interrupts and the device are enabled.
*
* @param spif   Ignored
* @return int   SPI_FLASH_STATUS_SUCCESS
*/
int SPI_FLASH_dummy_start(struct spi_flash *spif)
{
    NOT_USED(spif);
    return SPI_FLASH_STATUS_SUCCESS;
}

/**
* Shut down
*
* @param spif    Ignored
* @return int    SPI_FLASH_STATUS_SUCCESS
*/
int SPI_FLASH_dummy_stop(struct spi_flash *spif)
{
    NOT_USED(spif);
    return SPI_FLASH_STATUS_SUCCESS;
}

/**
* This function reads the status register of the Flash.
*
* @param spif   Ignored
* @param result Ignored
* @return int   SPI_FLASH_STATUS_SUCCESS
*/
int SPI_FLASH_dummy_get_status(struct spi_flash *spif, u8 *result)
{
    NOT_USED(spif);
    NOT_USED(result);
    return SPI_FLASH_STATUS_SUCCESS;
}

/**
* Wait until the serial flash is ready to accept next command.
*
* @param spif       Ignored
* @param timeout_ms Ignored
* @return int       SPI_FLASH_STATUS_SUCCESS
*
* @note This function reads the status register and waits
*       until the WIP bit of the status register becomes 0.
*/
int SPI_FLASH_dummy_wait_ready(struct spi_flash *spif, u32 timeout_ms)
{
    NOT_USED(spif);
    NOT_USED(timeout_ms);
    return SPI_FLASH_STATUS_SUCCESS;
}

/**
* Enable writes to the serial flash memory.
*
* @param spif   Ignored
* @return int   SPI_FLASH_STATUS_SUCCESS
*/
int SPI_FLASH_dummy_write_enable(struct spi_flash *spif)
{
    NOT_USED(spif);
    return SPI_FLASH_STATUS_SUCCESS;
}

/**
* Disable writes to the serial flash memory.
*
* @param spif   Ignored
* @return int   SPI_FLASH_STATUS_SUCCESS
*/
int SPI_FLASH_dummy_write_disable(struct spi_flash *spif)
{
    NOT_USED(spif);
    return SPI_FLASH_STATUS_SUCCESS;
}

/**
 * Get JEDEC id
 *
 * @param spif  Ignored
 * @param jedec Ignored
 * @return int  SPI_FLASH_STATUS_SUCCESS
 */
int SPI_FLASH_dummy_get_jedec(struct spi_flash *spif, u32 *jedec)
{
    NOT_USED(spif);
    NOT_USED(jedec);
    return SPI_FLASH_STATUS_SUCCESS;
}

/**
 * Verify flash JEDEC id
 *
 * @param spif  Ignored
 * @return bool True
 */
bool SPI_FLASH_dummy_verify_jedec(struct spi_flash *spif)
{
    NOT_USED(spif);
    return true;
}

/**
* Erase a 64K block
*
* @param spif       Ignored
* @param addr       Ignored
* @param timeout_ms Ignored
* @return int       SPI_FLASH_STATUS_SUCCESS
*
* @note 64K erase is the smallest working erase command.
*/
int SPI_FLASH_dummy_erase_64K(struct spi_flash *spif, u32 addr, u32 timeout_ms)
{
    NOT_USED(spif);
    NOT_USED(addr);
    NOT_USED(timeout_ms);
    return SPI_FLASH_STATUS_SUCCESS;
}

/**
* Erase one ore more 64K blocks
*
* @param spif       Ignored
* @param addr       Ignored
* @param num_bytes  Ignored
* @param timeout_ms Ignored
* @return int   SPI_FLASH_STATUS_SUCCESS
*
* @note 64K erase is the smallest working erase command.
*/
int SPI_FLASH_dummy_erase_multi_64K(struct spi_flash *spif, u32 addr, u32 num_bytes,
                                     u32 timeout_ms)
{
    NOT_USED(spif);
    NOT_USED(addr);
    NOT_USED(num_bytes);
    NOT_USED(timeout_ms);
    return SPI_FLASH_STATUS_SUCCESS;
}

/**
* Write up to 256 bytes of data to a specified location
* in the serial flash memory.
*
* @param spif       Ignored
* @param addr       Ignored
* @param buf        Ignored
* @param num_bytes  Ignored
* @return int       SPI_FLASH_STATUS_SUCCESS
*/
int SPI_FLASH_dummy_write_page(struct spi_flash *spif, u32 addr, const u8 *buf, u32 num_bytes)
{
    NOT_USED(spif);
    NOT_USED(addr);
    NOT_USED(buf);
    NOT_USED(num_bytes);
    return SPI_FLASH_STATUS_SUCCESS;
}

/**
* Write multiple pages of up to 256 bytes of data to a
* specified location in the serial flash memory.
*
* @param spif       Ignored
* @param addr       Ignored
* @param buf        Ignored
* @param num_bytes  Ignored
* @return int       SPI_FLASH_STATUS_SUCCESS
*/
int SPI_FLASH_dummy_write_multi_page(struct spi_flash *spif, u32 addr, const u8 *buf,
                                      u32 num_bytes)
{
    NOT_USED(spif);
    NOT_USED(addr);
    NOT_USED(buf);
    NOT_USED(num_bytes);
    return SPI_FLASH_STATUS_SUCCESS;
}

/**
* Read multiple pages of up to 256 bytes of data from a
* specified location in the serial flash memory.
*
* @param spif       Ignored
* @param addr       Ignored
* @param buf        Ignored
* @param num_bytes  Ignored
*
* @return int       SPI_FLASH_STATUS_SUCCESS
*/
int SPI_FLASH_dummy_read(struct spi_flash *spif, u32 addr, u8 *buf, u32 num_bytes)
{
    NOT_USED(spif);
    NOT_USED(addr);
    NOT_USED(buf);
    NOT_USED(num_bytes);
    return SPI_FLASH_STATUS_SUCCESS;
}

/**
* Compare the contents of a buffer with the contents of a
* specified location in the serial flash memory.
*
* @param spif       Ignored
* @param addr       Ignored
* @param buf        Ignored
* @param num_bytes  Ignored
*
* @return           SPI_FLASH_STATUS_SUCCESS
*/
int SPI_FLASH_dummy_compare(struct spi_flash *spif, u32 addr, const u8 *buf, u32 num_bytes)
{
    NOT_USED(spif);
    NOT_USED(addr);
    NOT_USED(buf);
    NOT_USED(num_bytes);
    return SPI_FLASH_STATUS_SUCCESS;
}

const struct SPI_FLASH_ops SPI_FLASH_dummy_ops = {
    .init = SPI_FLASH_dummy_init,
    .deinit = SPI_FLASH_dummy_deinit,
    .start = SPI_FLASH_dummy_start,
    .stop = SPI_FLASH_dummy_stop,
    .get_status = SPI_FLASH_dummy_get_status,
    .wait_ready = SPI_FLASH_dummy_wait_ready,
    .write_enable = SPI_FLASH_dummy_write_enable,
    .write_disable = SPI_FLASH_dummy_write_disable,
    .get_jedec = SPI_FLASH_dummy_get_jedec,
    .verify_jedec = SPI_FLASH_dummy_verify_jedec,
    .erase_64K = SPI_FLASH_dummy_erase_64K,
    .erase_multi_64K = SPI_FLASH_dummy_erase_multi_64K,
    .write_page = SPI_FLASH_dummy_write_page,
    .write_multi_page = SPI_FLASH_dummy_write_multi_page,
    .read = SPI_FLASH_dummy_read,
    .compare = SPI_FLASH_dummy_compare,
};
