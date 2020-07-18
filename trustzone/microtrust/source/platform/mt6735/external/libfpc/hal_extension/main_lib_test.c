
#include <unistd.h>

#include "fpc_hal_extension.h"

void irq_callback(int val)
{
}

const sensor_env_t sensor_env =
{
	.device_file = "/dev/fpc1020",
	.device_spi  = "/sys/bus/spi/devices/spi4.0",
	.device_irq  ="/sys/bus/platform/devices/fpc_interrupt.21"
};

int main(void)
{
	fpc_hal_extension_enter(&sensor_env, &irq_callback);
	//fpc_hal_extension_enter(&sensor_env, NULL);

	fpc_hal_extension_run();

	while (fpc_hal_extension_check_running())
	{
		sleep(1);
	}

	fpc_hal_extension_exit();

	return 0;
}

