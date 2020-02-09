/**
 * GPIO on / off. Switch an LED on / off by pressing buttons.
 * 
 **/
#include <gpio.h>
#include <board.h>
#include <device.h>

// GPIO
struct device *gpio_dev;

// GPIO for the buttons
#define PIN_A SW0_GPIO_PIN
#define PIN_B SW1_GPIO_PIN
#define PORT SW0_GPIO_CONTROLLER
#define EDGE (GPIO_INT_EDGE | GPIO_INT_ACTIVE_LOW | GPIO_INT_DEBOUNCE)

static struct gpio_callback gpio_btnA_cb;
static struct gpio_callback gpio_btnB_cb;

#define BUTTON_DEBOUNCE_DELAY_MS 250
#define ONE_WATT_HOUR 3600000
static u32_t time, last_time, flash_time;

// GPIO for the LED connected to pin 0 on the edge connector
// See board.h in zephyr/boards/arm/bbc_microbit
#define LED0 EXT_P0_GPIO_PIN

// for use with k_work_submit which we use to handle button presses in a background thread to avoid holding onto an IRQ for too long
static struct k_work buttonA_work;
static struct k_work buttonB_work;

bool debounce() {
	bool result = false;
	time = k_uptime_get_32();
	if (time < last_time + BUTTON_DEBOUNCE_DELAY_MS) {
		result = true;
	}
	last_time = time;
    return result;
}

u32_t calculatePowerUsage() {
	time = k_uptime_get_32();
	printk("time = %d\n", time);
	u32_t period = time - flash_time;
	printk("period = %d\n", period);
	u32_t power = ONE_WATT_HOUR / period;
	printk("Current power: %dW\n", power);
	return power;
}

void buttonA_work_handler(struct k_work *work)
{
	flash_time = k_uptime_get_32();
	printk("flash_time = %d\n", flash_time);
}

void buttonB_work_handler(struct k_work *work)
{
	calculatePowerUsage();
}

void button_A_pressed(struct device *gpiob, struct gpio_callback *cb,
											u32_t pins)
{
	if (debounce()) {
		return;
	}
	k_work_submit(&buttonA_work);
}

void button_B_pressed(struct device *gpiob, struct gpio_callback *cb,
											u32_t pins)
{
	if (debounce()) {
		return;
	}
	k_work_submit(&buttonB_work);
}

// Connected LDR
// void ldr_init() {
// 	gpio_pin_configure(gpio_dev, LDR0, GPIO_DIR_IN);
// }

// -------------------------------------------------------------------------------------------------------
// Buttons
// -------

void configureButtons(void)
{
	printk("Press button A or button B\n");
	if (!gpio_dev)
	{
		printk("error - no GPIO device\n");
		return;
	}

	// Button A
	k_work_init(&buttonA_work, buttonA_work_handler);
	gpio_pin_configure(gpio_dev, PIN_A, GPIO_DIR_IN | GPIO_INT | EDGE);
	gpio_init_callback(&gpio_btnA_cb, button_A_pressed, BIT(PIN_A));
	gpio_add_callback(gpio_dev, &gpio_btnA_cb);
	gpio_pin_enable_callback(gpio_dev, PIN_A);

	// Button B
	k_work_init(&buttonB_work, buttonB_work_handler);
	gpio_pin_configure(gpio_dev, PIN_B, GPIO_DIR_IN | GPIO_INT | EDGE);
	gpio_init_callback(&gpio_btnB_cb, button_B_pressed, BIT(PIN_B));
	gpio_add_callback(gpio_dev, &gpio_btnB_cb);
	gpio_pin_enable_callback(gpio_dev, PIN_B);
}

void main(void)
{
	printk("Electricity Monitor V1.1\n");

	gpio_dev = device_get_binding(PORT);

	configureButtons();
}
