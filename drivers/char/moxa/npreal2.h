
#ifndef _NPREAL2_H
#define _NPREAL2_H

#define DRV_VAR		(npvar_sdriver)
#define DRV_VAR_P(x)	npvar_sdriver->x

#ifndef INIT_WORK
#define INIT_WORK(_work, _func){	\
	(_work)->routine = _func;\
	}
#endif

#ifndef set_current_state
#define	set_current_state(x) 		current->state = x
#endif


#define IRQ_RET irqreturn_t


#if 0
#define	MXQ_TASK()	schedule_work(&info->tqueue)
#else
#define	MXQ_TASK(queue)	schedule_work(queue)
#endif

#define MX_MOD_INC	try_module_get(THIS_MODULE)
#define MX_MOD_DEC	module_put(THIS_MODULE)

#ifndef ASYNC_CALLOUT_ACTIVE
#define ASYNC_CALLOUT_ACTIVE 0
#endif


#define MX_TTY_DRV(x)	tty->driver->x

#define MX_SESSION()    task_session(current)

#define MX_CGRP()       task_pgrp(current)

#define GET_FPAGE	__get_free_page

#define DOWN(tx_lock, flags)    spin_lock_irqsave(&tx_lock, flags);
#define UP(tx_lock, flags)      spin_unlock_irqrestore(&tx_lock, flags);

#ifndef atomic_read
#define atomic_read(v)	v
#endif


#ifndef UCHAR
typedef unsigned char	UCHAR;
#endif

// Scott: 2005-09-13 begin
// Added the debug print management
#define MX_DEBUG_OFF        0
#define MX_DEBUG_ERROR      1		// 1~19 for ERROR level
#define MX_DEBUG_WARN       20		// 20~39 for WARN level
#define MX_DEBUG_TRACE      40		// 40~59 for TRACE level
#define MX_DEBUG_INFO       60		// 60~79 for INFO level
#define MX_DEBUG_LOUD       80		// 80~ for LOUD level

#define MX_DBG 1
#ifdef MX_DBG
extern int	MXDebugLevel;
#define DBGPRINT(level, fmt, args...)			\
{							\
    if ((level) <= MXDebugLevel)				\
    {							\
	printk(KERN_DEBUG "MX(%d,%s) " fmt, __LINE__, __FUNCTION__, ## args);		\
    }							\
}
#else
#define DBGPRINT(level, fmt, args...)	while (0) ;
#endif
// Scott: 2005-09-13 end

#endif
