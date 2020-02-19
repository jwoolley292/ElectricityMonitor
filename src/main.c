/**
 * GPIO on / off. Switch an LED on / off by pressing buttons.
 * 
 **/
#include <gpio.h>
#include <board.h>
#include <device.h>

// GPIO
struct device *gpio_dev;
static struct gpio_callback gpio_trigger_cb;

// GPIO for LDR
#define PORT SW0_GPIO_CONTROLLER
#define PIN_0 EXT_P0_GPIO_PIN

#define DEBOUNCE_DELAY_MS 250
#define ONE_WATT_HOUR 3600000
static u32_t time, last_time, flash_time;

bool debounce() {
	bool result = false;
	time = k_uptime_get_32();
	if (time < last_time + DEBOUNCE_DELAY_MS) {
		result = true;
	}
	last_time = time;
    return result;
}

u32_t calculatePowerUsage() {
	time = k_uptime_get_32();
	u32_t power = ONE_WATT_HOUR / (time - flash_time);
	printk("Current power: %dW\n", power);
	flash_time = time;
	return power;
}

void triggered(struct device *gpiob, struct gpio_callback *cb, u32_t pins)
{
	u32_t val = 0;
	gpio_pin_read(gpio_dev, PIN_0, &val);
	if (val == 0) {
	    calculatePowerUsage();
	}
}

void configureGpio(void)
{
	gpio_dev = device_get_binding(PORT);
	if (!gpio_dev)
	{
		printk("error - no GPIO device\n");
		return;
	}
	printk("pin P0 triggering when low (bright)\n");
	
    gpio_pin_configure(gpio_dev, PIN_0, GPIO_DIR_IN | GPIO_INT | GPIO_INT_EDGE | GPIO_INT_ACTIVE_LOW  );

	gpio_init_callback(&gpio_trigger_cb, triggered, BIT(PIN_0));
	gpio_add_callback(gpio_dev, &gpio_trigger_cb);
	gpio_pin_enable_callback(gpio_dev, PIN_0);
}

void main(void)
{
	printk("Electricity Monitor V1.2\n");

	gpio_dev = device_get_binding(PORT);

	configureGpio();
}
