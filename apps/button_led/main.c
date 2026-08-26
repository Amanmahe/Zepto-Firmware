/*
 * button_led for BeagleConnect Zepto (mspm0l1117)
 *
 * Reads the on-board button (the "sw0" alias, gpioa18) and toggles
 * the on-board LED ("led0", gpioa13) every time it is pressed.
 * Uses a GPIO interrupt + callback, no polling loop -- this is the
 * same pattern as Zephyr's upstream samples/basic/button sample.
 *
 * No devicetree overlay needed: both led0 and sw0 are already wired
 * up in boards/beagle/beagleconnect_zepto/beagleconnect_zepto.dts.
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>

#define LED0_NODE DT_ALIAS(led0)
#define SW0_NODE  DT_ALIAS(sw0)

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios, {0});

static struct gpio_callback button_cb_data;

static void button_pressed(const struct device *dev, struct gpio_callback *cb,
			    uint32_t pins)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(cb);
	ARG_UNUSED(pins);

	gpio_pin_toggle_dt(&led);
	printk("Button pressed -> LED toggled\n");
}

int main(void)
{
	int ret;

	if (!gpio_is_ready_dt(&led) || !gpio_is_ready_dt(&button)) {
		printk("LED or button device not ready\n");
		return 0;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		return 0;
	}

	ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
	if (ret < 0) {
		return 0;
	}

	ret = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret < 0) {
		return 0;
	}

	gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));
	gpio_add_callback(button.port, &button_cb_data);

	printk("Ready. Press the button to toggle the LED.\n");

	while (1) {
		k_sleep(K_FOREVER);
	}

	return 0;
}
