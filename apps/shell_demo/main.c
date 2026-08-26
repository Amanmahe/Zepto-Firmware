/*
 * shell_demo for BeagleConnect Zepto (mspm0l1117)
 *
 * Boots the Zephyr shell on uart0 (the same debug UART used for the
 * console) and registers one custom command: "led toggle". Connect
 * with any serial terminal at 115200 8N1 and press enter to get a
 * prompt, then try:
 *
 *   uart:~$ led toggle
 *   uart:~$ kernel version
 *   uart:~$ device list
 *
 * No devicetree overlay needed.
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/shell/shell.h>

#define LED0_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

static int cmd_led_toggle(const struct shell *sh, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	if (!gpio_is_ready_dt(&led)) {
		shell_error(sh, "LED device not ready");
		return -ENODEV;
	}

	gpio_pin_toggle_dt(&led);
	shell_print(sh, "LED toggled");

	return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(sub_led,
	SHELL_CMD(toggle, NULL, "Toggle the on-board LED", cmd_led_toggle),
	SHELL_SUBCMD_SET_END
);
SHELL_CMD_REGISTER(led, &sub_led, "LED control commands", NULL);

int main(void)
{
	if (gpio_is_ready_dt(&led)) {
		gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
	}

	printk("shell_demo ready. Open a serial terminal and press enter.\n");

	return 0;
}
