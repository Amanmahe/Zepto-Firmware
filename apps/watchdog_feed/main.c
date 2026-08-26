/*
 * watchdog_feed for BeagleConnect Zepto (mspm0l1117)
 *
 * Installs a 2-second watchdog timeout and feeds it every 1 second so
 * the board runs normally and blinks led0. Change FEED_WDT to 0 to
 * see the board reset itself when the watchdog fires (nothing feeds
 * it -> reset after ~2s).
 *
 * Uses wdt0 (alias "watchdog0"), already enabled in
 * boards/beagle/beagleconnect_zepto/beagleconnect_zepto.dts.
 * No overlay needed.
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/watchdog.h>

#define LED0_NODE DT_ALIAS(led0)
#define WDT_NODE  DT_ALIAS(watchdog0)

#define WDT_TIMEOUT_MS  2000
#define FEED_PERIOD_MS  1000

/* Set to 0 to intentionally stop feeding and watch the board reset */
#define FEED_WDT 1

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct device *const wdt = DEVICE_DT_GET(WDT_NODE);

int main(void)
{
	int wdt_channel_id;
	struct wdt_timeout_cfg wdt_config = {
		.flags = WDT_FLAG_RESET_SOC,
		.window.min = 0,
		.window.max = WDT_TIMEOUT_MS,
	};

	if (!device_is_ready(wdt)) {
		printk("Watchdog device not ready\n");
		return 0;
	}

	if (gpio_is_ready_dt(&led)) {
		gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	}

	wdt_channel_id = wdt_install_timeout(wdt, &wdt_config);
	if (wdt_channel_id < 0) {
		printk("Watchdog install error: %d\n", wdt_channel_id);
		return 0;
	}

	if (wdt_setup(wdt, 0) < 0) {
		printk("Watchdog setup error\n");
		return 0;
	}

	printk("Watchdog armed for %d ms, feeding every %d ms\n",
	       WDT_TIMEOUT_MS, FEED_PERIOD_MS);

	while (1) {
		gpio_pin_toggle_dt(&led);

#if FEED_WDT
		wdt_feed(wdt, wdt_channel_id);
#else
		printk("(not feeding the watchdog on purpose...)\n");
#endif
		k_msleep(FEED_PERIOD_MS);
	}

	return 0;
}
