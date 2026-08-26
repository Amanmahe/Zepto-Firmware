/*
 * low_power_sleep for BeagleConnect Zepto (mspm0l1117)
 *
 * Blinks led0 once every 2 seconds. The interesting part isn't the
 * blink -- it's CONFIG_PM=y in prj.conf: whenever this app calls
 * k_msleep() and the CPU is otherwise idle, Zephyr's power management
 * subsystem automatically drops the SoC into one of the low-power
 * run/stop/standby states declared for this chip family (see
 * dts/arm/ti/mspm0/l/mspm0l.dtsi), instead of just spinning in the
 * default idle loop. No application code changes needed to benefit
 * from it -- compare current draw with CONFIG_PM=n to see the effect
 * on a multimeter.
 *
 * No overlay needed.
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
