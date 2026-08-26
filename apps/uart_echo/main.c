/*
 * uart_echo for BeagleConnect Zepto (mspm0l1117)
 *
 * Talks to the UART driver directly (interrupt-driven) instead of
 * going through printk/the console subsystem, and echoes back every
 * byte it receives on uart0. Useful as a starting point for a custom
 * serial protocol instead of the shell.
 *
 * Uses the same uart0 device already enabled as zephyr,console in
 * boards/beagle/beagleconnect_zepto/beagleconnect_zepto.dts -- no
 * overlay needed. Connect over the board's USB-serial / debug UART
 * at 115200 8N1.
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/uart.h>

static const struct device *const uart_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));

static void uart_isr(const struct device *dev, void *user_data)
{
	ARG_UNUSED(user_data);
	uint8_t c;

	if (!uart_irq_update(dev)) {
		return;
	}

	if (!uart_irq_rx_ready(dev)) {
		return;
	}

	while (uart_fifo_read(dev, &c, 1) == 1) {
		/* echo it straight back out */
		uart_poll_out(dev, c);

		if (c == '\r') {
			uart_poll_out(dev, '\n');
		}
	}
}

int main(void)
{
	if (!device_is_ready(uart_dev)) {
		printk("UART device not ready\n");
		return 0;
	}

	uart_irq_callback_set(uart_dev, uart_isr);
	uart_irq_rx_enable(uart_dev);

	printk("uart_echo ready -- type something, it will be echoed back\n");

	while (1) {
		k_sleep(K_FOREVER);
	}

	return 0;
}
