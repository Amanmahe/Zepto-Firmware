/*
 * entropy_rng for BeagleConnect Zepto (mspm0l1117)
 *
 * Reads bytes from the on-chip hardware TRNG (True Random Number
 * Generator) once a second and prints them as hex. Good building
 * block for generating keys, nonces, or random IDs on-device.
 *
 * The trng node is already marked "zephyr,entropy" and enabled in
 * boards/beagle/beagleconnect_zepto/beagleconnect_zepto.dts.
 * No overlay needed.
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/entropy.h>

static const struct device *const entropy_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_entropy));

int main(void)
{
	if (!device_is_ready(entropy_dev)) {
		printk("Entropy device not ready\n");
		return 0;
	}

	while (1) {
		uint8_t buf[8];
		int ret = entropy_get_entropy(entropy_dev, buf, sizeof(buf));

		if (ret == 0) {
			printk("random: ");
			for (int i = 0; i < sizeof(buf); i++) {
				printk("%02x", buf[i]);
			}
			printk("\n");
		} else {
			printk("entropy_get_entropy failed: %d\n", ret);
		}

		k_sleep(K_SECONDS(1));
	}

	return 0;
}
