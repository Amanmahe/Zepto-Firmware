/*
 * rtc_clock for BeagleConnect Zepto (mspm0l1117)
 *
 * Sets the on-chip RTC to a fixed date/time at boot, then reads it
 * back and prints it once a second. Swap in real time (e.g. from a
 * host over UART) instead of the hardcoded value if you need it.
 *
 * Needs the RTC turned on via boards/beagleconnect_zepto_mspm0l1117.overlay
 * (no external pins involved).
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/rtc.h>

#define RTC_NODE DT_ALIAS(rtc)

static const struct device *const rtc_dev = DEVICE_DT_GET(RTC_NODE);

int main(void)
{
	int ret;
	struct rtc_time tm = {
		.tm_year = 2026 - 1900,  /* years since 1900 */
		.tm_mon  = 8 - 1,        /* 0-11 */
		.tm_mday = 13,
		.tm_hour = 12,
		.tm_min  = 0,
		.tm_sec  = 0,
	};

	if (!device_is_ready(rtc_dev)) {
		printk("RTC device not ready\n");
		return 0;
	}

	ret = rtc_set_time(rtc_dev, &tm);
	if (ret < 0) {
		printk("Failed to set RTC time: %d\n", ret);
		return 0;
	}

	while (1) {
		struct rtc_time now;

		ret = rtc_get_time(rtc_dev, &now);
		if (ret == 0) {
			printk("%04d-%02d-%02d %02d:%02d:%02d\n",
			       now.tm_year + 1900, now.tm_mon + 1, now.tm_mday,
			       now.tm_hour, now.tm_min, now.tm_sec);
		}

		k_sleep(K_SECONDS(1));
	}

	return 0;
}
