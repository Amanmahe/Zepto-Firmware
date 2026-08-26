/*
 * adc_read for BeagleConnect Zepto (mspm0l1117)
 *
 * Reads channel 0 of the on-chip 12-bit ADC once a second and prints
 * the raw value and the millivolt equivalent. Good for reading a
 * potentiometer, an analog sensor, or a battery-voltage divider.
 *
 * REQUIRES the overlay in boards/beagleconnect_zepto_mspm0l1117.overlay
 * to be filled in with a real pin + channel first -- see the TODO
 * comments there.
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/adc.h>

#define ADC_NODE DT_PATH(zephyr_user)

static const struct adc_dt_spec adc_chan = ADC_DT_SPEC_GET_BY_IDX(ADC_NODE, 0);

int main(void)
{
	int err;
	uint16_t buf;
	struct adc_sequence sequence = {
		.buffer = &buf,
		.buffer_size = sizeof(buf),
	};

	if (!adc_is_ready_dt(&adc_chan)) {
		printk("ADC device not ready\n");
		return 0;
	}

	err = adc_channel_setup_dt(&adc_chan);
	if (err < 0) {
		printk("ADC channel setup failed: %d\n", err);
		return 0;
	}

	while (1) {
		int32_t val_mv;

		adc_sequence_init_dt(&adc_chan, &sequence);
		err = adc_read_dt(&adc_chan, &sequence);
		if (err < 0) {
			printk("ADC read failed: %d\n", err);
			k_sleep(K_SECONDS(1));
			continue;
		}

		val_mv = buf;
		err = adc_raw_to_millivolts_dt(&adc_chan, &val_mv);

		if (err == 0) {
			printk("raw: %d  -> %d mV\n", buf, val_mv);
		} else {
			printk("raw: %d  (mv conversion not available: %d)\n", buf, err);
		}

		k_sleep(K_SECONDS(1));
	}

	return 0;
}
