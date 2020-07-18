#ifndef __LIB_FPC_HAL_EXTENSION_H__
#define __LIB_FPC_HAL_EXTENSION_H__

#ifdef __cplusplus
extern "C"
{
#endif

/* NOTE: driver and HAL must share same definition for SIGNAL and STATE */
enum {
	FPC_IRQ_SIGNAL_TEST          = 1,
	FPC_IRQ_SIGNAL_INTERRUPT_REQ = 2,
	FPC_IRQ_SIGNAL_SUSPEND_REQ   = 3,
	FPC_IRQ_SIGNAL_RESUME_REQ    = 4,
	FPC_IRQ_SIGNAL_SUSPEND_EARLY_REQ   = 5,
	FPC_IRQ_SIGNAL_RESUME_LATE_REQ    = 6,
};

enum {
	FPC_IRQ_STATE_ACTIVE    = 1,
	FPC_IRQ_STATE_SUSPENDED = 2,
	FPC_IRQ_STATE_EARLY_SUSPENDED = 3,
	FPC_IRQ_STATE_LATE_RESUME = 4,
};

typedef struct
{
	char device_file[sizeof("/dev/fpcXXXX")];
	char device_spi[sizeof("/sys/bus/spi/devices/spiNN.NN/")];
	char device_irq[sizeof("/sys/bus/platform/devices/fpc_interrupt.NNNNN")];
} sensor_env_t;

typedef void (*req_callback_t)(int);

extern int fpc_hal_extension_enter(sensor_env_t const *env, req_callback_t req_callback);

extern int fpc_hal_extension_run(void);

extern int fpc_hal_extension_enable_irq(void);
extern int fpc_hal_extension_irq_disable(void);

extern int fpc_hal_extension_exit(void);

extern int fpc_hal_extension_check_running(void);

extern int fpc_hal_extension_enable_supply(void);
extern int fpc_hal_extension_supply_disable(void);

extern int fpc_hal_extension_hw_reset(void);

extern int fpc_hal_extension_pm_check_state(void);

extern int fpc_hal_extension_enable_pm_notify(void);
extern int fpc_hal_extension_pm_notify_disable(void);

extern int fpc_hal_extension_suspend_ack(int type);
extern int fpc_hal_extension_resume_ack(int type);

extern int fpc_hal_extension_wakeup_req(void);

#ifdef __cplusplus
}
#endif

#endif

