/*
 * counter_alarm for BeagleConnect Zepto (mspm0l1117)
 *
 * Uses the hardware timer/counter peripheral (counterg0, alias
 * "counter") to fire a periodic alarm callback every 500 ms, which
 * toggles led0. This runs off the timer's own interrupt, not a
 * k_msleep() loop -- useful once you need other code to keep running
 * on a precise schedule.
 *
 * Already enabled in
 * boards/beagle/beagleconnect_zepto/beagleconnect_zepto.dts.
 * No overlay needed.
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/counter.h>

#define LED0_NODE    DT_ALIAS(led0)
#define COUNTER_NODE DT_ALIAS(counter)

#define ALARM_PERIOD_MS 500

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct device *const counter_dev = DEVICE_DT_GET(COUNTER_NODE);

static struct counter_alarm_cfg alarm_cfg;

static void alarm_handler(const struct device *dev, uint8_t chan_id,
			   uint32_t ticks, void *user_data)
{
	ARG_UNUSED(chan_id);
	ARG_UNUSED(ticks);

	gpio_pin_toggle_dt(&led);

	/* re-arm for the next period (one-shot alarm re-scheduled each time) */
	counter_set_channel_alarm(dev, 0, user_data);
}

int main(void)
{
	int err;
	uint32_t freq;

	if (!device_is_ready(counter_dev)) {
		printk("Counter device not ready\n");
		return 0;
	}

	if (gpio_is_ready_dt(&led)) {
		gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
	}

	freq = counter_get_frequency(counter_dev);
	printk("Counter running at %u Hz\n", freq);

	alarm_cfg.flags = 0;
	alarm_cfg.ticks = counter_us_to_ticks(counter_dev, ALARM_PERIOD_MS * 1000);
	alarm_cfg.callback = alarm_handler;
	alarm_cfg.user_data = &alarm_cfg;

	counter_start(counter_dev);

	err = counter_set_channel_alarm(counter_dev, 0, &alarm_cfg);
	if (err != 0) {
		printk("Failed to set alarm: %d\n", err);
		return 0;
	}

	printk("Alarm armed for every %d ms\n", ALARM_PERIOD_MS);

	while (1) {
		k_sleep(K_FOREVER);
	}

	return 0;
}
