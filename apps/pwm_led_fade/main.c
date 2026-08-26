/*
 * pwm_led_fade for BeagleConnect Zepto (mspm0l1117)
 *
 * Fades an external LED (or any PWM-driven load) up and down on the
 * "pwm-led0" alias.
 *
 * REQUIRES the overlay in boards/beagleconnect_zepto_mspm0l1117.overlay
 * to be filled in with a real pin first -- see the TODO comments
 * there. Wire the LED (with a series resistor) between that pin and
 * GND, anode to the pin.
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/pwm.h>

#define PWM_LED0_NODE DT_ALIAS(pwm_led0)

static const struct pwm_dt_spec pwm_led = PWM_DT_SPEC_GET(PWM_LED0_NODE);

int main(void)
{
	if (!pwm_is_ready_dt(&pwm_led)) {
		printk("PWM device not ready\n");
		return 0;
	}

	uint32_t period = pwm_led.period;
	int32_t step = period / 50;
	int32_t pulse = 0;
	int8_t dir = 1;

	while (1) {
		pwm_set_pulse_dt(&pwm_led, pulse);

		pulse += dir * step;
		if (pulse < 0) {
			pulse = 0;
			dir = 1;
		} else if ((uint32_t)pulse > period) {
			pulse = period;
			dir = -1;
		}

		k_msleep(20);
	}

	return 0;
}
