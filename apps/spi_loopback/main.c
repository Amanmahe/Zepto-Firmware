/*
 * spi_loopback for BeagleConnect Zepto (mspm0l1117)
 *
 * Sends a known byte pattern out over spi0 and reads back whatever
 * comes in on POCI at the same time. With PICO jumpered directly to
 * POCI on the header, the received buffer should exactly match what
 * was sent -- a quick way to confirm the SPI peripheral, pinmux, and
 * wiring are all correct before trying a real SPI sensor/device.
 *
 * REQUIRES the overlay in boards/beagleconnect_zepto_mspm0l1117.overlay
 * to be filled in with real pins first -- see the TODO comments there.
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/spi.h>

#define SPI_NODE DT_NODELABEL(spi0)

static const struct device *const spi_dev = DEVICE_DT_GET(SPI_NODE);

static const struct spi_config spi_cfg = {
	.frequency = 1000000,
	.operation = SPI_WORD_SET(8) | SPI_TRANSFER_MSB,
	.slave = 0,
};

int main(void)
{
	uint8_t tx_data[4] = { 0xDE, 0xAD, 0xBE, 0xEF };
	uint8_t rx_data[4] = { 0 };

	const struct spi_buf tx_buf = { .buf = tx_data, .len = sizeof(tx_data) };
	const struct spi_buf_set tx = { .buffers = &tx_buf, .count = 1 };

	struct spi_buf rx_buf = { .buf = rx_data, .len = sizeof(rx_data) };
	const struct spi_buf_set rx = { .buffers = &rx_buf, .count = 1 };

	if (!device_is_ready(spi_dev)) {
		printk("SPI device not ready\n");
		return 0;
	}

	while (1) {
		int err = spi_transceive(spi_dev, &spi_cfg, &tx, &rx);

		if (err < 0) {
			printk("spi_transceive failed: %d\n", err);
		} else {
			printk("sent: %02x %02x %02x %02x  received: %02x %02x %02x %02x\n",
			       tx_data[0], tx_data[1], tx_data[2], tx_data[3],
			       rx_data[0], rx_data[1], rx_data[2], rx_data[3]);
		}

		k_sleep(K_SECONDS(1));
	}

	return 0;
}
