#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/input.h>
#include <linux/irq.h>

#define GPIO_KEY_UP     17  // BCM2837에서 사용할 GPIO 핀 (예: GPIO 17)
#define GPIO_KEY_DOWN   27  // BCM2837에서 사용할 GPIO 핀 (예: GPIO 27)

static struct input_dev *key_input_dev;
static int irq_key_up;
static int irq_key_down;

// GPIO 인터럽트 핸들러 (키 눌림)
static irqreturn_t key_isr(int irq, void *dev_id)
{
    int key_value;

    if (irq == irq_key_up) {
        key_value = gpio_get_value(GPIO_KEY_UP);
        printk("Up\n");
        input_report_key(key_input_dev, KEY_UP, !key_value);
    }
    else if (irq == irq_key_down) {
        key_value = gpio_get_value(GPIO_KEY_DOWN);
        printk("Down\n");
        input_report_key(key_input_dev, KEY_DOWN, !key_value);
    }
    
    input_sync(key_input_dev);
    return IRQ_HANDLED;
}

// 모듈 초기화
static int __init gpio_key_init(void)
{
    int ret;

    // input device 생성
    key_input_dev = input_allocate_device();
    if (!key_input_dev) {
        pr_err("Failed to allocate input device\n");
        return -ENOMEM;
    }

    key_input_dev->name = "bcm2837-gpio-keys";
    key_input_dev->phys = "bcm2837/input0";
    key_input_dev->id.bustype = BUS_HOST;
    key_input_dev->evbit[0] = BIT_MASK(EV_KEY);
    input_set_capability(key_input_dev, EV_KEY, KEY_UP);
    input_set_capability(key_input_dev, EV_KEY, KEY_DOWN);

    ret = input_register_device(key_input_dev);
    if (ret) {
        pr_err("Failed to register input device\n");
        goto err_free_dev;
    }

    // GPIO 설정
    if (!gpio_is_valid(GPIO_KEY_UP) || !gpio_is_valid(GPIO_KEY_DOWN)) {
        pr_err("Invalid GPIO pin\n");
        ret = -EINVAL;
        goto err_unregister_dev;
    }

    gpio_request(GPIO_KEY_UP, "GPIO_KEY_UP");
    gpio_direction_input(GPIO_KEY_UP);
    gpio_request(GPIO_KEY_DOWN, "GPIO_KEY_DOWN");
    gpio_direction_input(GPIO_KEY_DOWN);

    // 인터럽트 요청
    irq_key_up = gpio_to_irq(GPIO_KEY_UP);
    irq_key_down = gpio_to_irq(GPIO_KEY_DOWN);
    
    ret = request_irq(irq_key_up, key_isr, IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING, "gpio_key_up", NULL);
    if (ret) {
        pr_err("Failed to request IRQ for GPIO_KEY_UP\n");
        goto err_free_gpio;
    }

    ret = request_irq(irq_key_down, key_isr, IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING, "gpio_key_down", NULL);
    if (ret) {
        pr_err("Failed to request IRQ for GPIO_KEY_DOWN\n");
        goto err_free_irq_up;
    }

    pr_info("GPIO Key module loaded\n");
    return 0;

// 오류 처리
err_free_irq_up:
    free_irq(irq_key_up, NULL);
err_free_gpio:
    gpio_free(GPIO_KEY_UP);
    gpio_free(GPIO_KEY_DOWN);
err_unregister_dev:
    input_unregister_device(key_input_dev);
err_free_dev:
    input_free_device(key_input_dev);
    return ret;
}

// 모듈 제거
static void __exit gpio_key_exit(void)
{
    free_irq(irq_key_up, NULL);
    free_irq(irq_key_down, NULL);
    gpio_free(GPIO_KEY_UP);
    gpio_free(GPIO_KEY_DOWN);
    input_unregister_device(key_input_dev);
    pr_info("GPIO Key module unloaded\n");
}

module_init(gpio_key_init);
module_exit(gpio_key_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ChatGPT");
MODULE_DESCRIPTION("BCM2837 GPIO Input Device");
