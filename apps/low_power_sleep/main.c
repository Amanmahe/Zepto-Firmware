/*
 * low_power_sleep for BeagleConnect Zepto (mspm0l1117)
 *
 * Blinks led0 once every 2 seconds. The interesting part isn't the
 * blink -- it's CONFIG_PM=y in prj.conf: whenever this app calls
 * k_msleep() and the CPU is otherwise idle, Zephyr's power management
 * subsystem automatically clock-gates the CPU core via WFI instead of
 * spinning in the default idle loop. No application code changes
 * needed to benefit from it -- compare current draw with CONFIG_PM=n
 * to see the effect on a multimeter.
 *
 * The board overlay disables the deeper STOP/STANDBY power states:
 * this SoC's Zephyr port has no wake-capable timer yet, and those
 * states gate the clock that also feeds the kernel's own tick source,
 * so entering them means the board never wakes back up. Only the
 * RUN-sleep states (which leave that clock running) are safe for now.
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
