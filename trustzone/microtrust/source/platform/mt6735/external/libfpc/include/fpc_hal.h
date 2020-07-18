#ifndef __FPC_HAL_H_DEFINED__
#define __FPC_HAL_H_DEFINED__


fpc_def_return_t fpc_hal_wait_for_finger_present(uint32_t limit);
fpc_def_return_t fpc_hal_wait_for_finger_lost(uint32_t limit);
fpc_def_return_t fpc_hal_standby();
fpc_def_return_t fpc_hal_stop_standby();
fpc_def_return_t fpc_hal_init();
fpc_def_return_t fpc_hal_deinit();
fpc_def_return_t fpc_hal_suspend();
fpc_def_return_t fpc_hal_abort();
fpc_def_return_t fpc_hal_stop_standby();
fpc_def_return_t fpc_hal_standby();

#endif /* __FPC_HAL_H_DEFINED__ */
