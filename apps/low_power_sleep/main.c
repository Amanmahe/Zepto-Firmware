/*
 * low_power_sleep for BeagleConnect Zepto (mspm0l1117)
 *
 * Plain led0 blink, once every 2 seconds.
 *
 * This app originally also set CONFIG_PM=y to let Zephyr's power
 * management subsystem drop the SoC into a low-power state whenever
 * idle. That hangs the board at boot on real hardware, at the Zephyr
 * revision this repo is pinned to -- confirmed on-device (the LED
 * never toggles even once). None of the mspm0 drivers implement
 * device PM actions either, so there would be no benefit even if the
 * hang were fixed. Removed until upstream Zephyr's mspm0 PM support
 * is further along -- see prj.conf.
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>

#define LED0_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

int main(void)
{
	if (!gpio_is_ready_dt(&led)) {
		return 0;
	}

	gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);

	while (1) {
		gpio_pin_toggle_dt(&led);
		k_msleep(2000);
	}

	return 0;
}
