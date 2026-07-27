/*
 * blinky for BeagleConnect Zepto (mspm0l1117)
 *
 * This is the visible, editable source for the LED-blink app.
 * It reads the LED pin from the board's devicetree (the "led0" alias),
 * so if you change boards or wiring, you edit the devicetree overlay
 * in boards/, not this file.
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>

/* 1000 ms between toggles */
#define SLEEP_TIME_MS   1000

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

int main(void)
{
	int ret;

	if (!gpio_is_ready_dt(&led)) {
		/* LED device isn't ready — check the devicetree/board overlay */
		return 0;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return 0;
	}

	while (1) {
		ret = gpio_pin_toggle_dt(&led);
		if (ret < 0) {
			return 0;
		}
		k_msleep(SLEEP_TIME_MS);
	}

	return 0;
}
