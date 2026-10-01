//Includes required to compile under AROS

#ifndef AROS_INC_H
#define AROS_INC_H

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/cybergraphics.h>
#include <proto/asl.h>
#include <proto/gadtools.h>

#include <proto/timer.h>
#include <devices/timer.h>

#include <libraries/asl.h>

#include <string.h>

#include <aros/macros.h>

//extern void GetSysTime(struct Library *, struct timeval * );
//extern void SubTime(struct Library *, struct timeval *, struct timeval *);

// AMP uses PPC defines, so patch them with ours

#if(0)
extern struct Task *CreateTask( struct TagItem *);
#endif

#ifndef GOT_TASKTAGS
#define GOT_TASKTAGS
#include <exec/tasks.h>
/* Map the WarpUP/PPC CreateTask tags onto AROS NewCreateTaskA tags */
#define TASKATTR_CODE			TASKTAG_PC
#define TASKATTR_NAME			TASKTAG_NAME
#define TASKATTR_STACKSIZE		TASKTAG_STACKSIZE
#define TASKATTR_INHERITR2		TASKTAG_USERDATA
#endif /* GOT_TASKTAGS */

#define AllocVecPPC(x,y,z) AllocVec(x,y)
#define FreeVecPPC FreeVec

#define PutMsgPPC PutMsg
#define WaitPortPPC WaitPort
#define GetMsgPPC GetMsg
#define ReplyMsgPPC ReplyMsg
#define FindPortPPC FindPort
#define CreateMsgPortPPC CreateMsgPort
#define AddPortPPC AddPort
#define RemPortPPC RemPort
#define DeleteMsgPortPPC DeleteMsgPort

#define MsgPortPPC MsgPort

#define InitSemaphorePPC InitSemaphore
#define SignalSemaphorePPC SignalSemaphore
#define ObtainSemaphorePPC ObtainSemaphore
#define ReleaseSemaphorePPC ReleaseSemaphore

#define FreeSemaphorePPC(sem)
#define WaitTime(x,y) { int tmp = y ; while (tmp >= 0) tmp--; }
#define SSPPC_SUCCESS TRUE

/* WarpUP's AllocSignal returned a signal MASK, and AMP stores/uses the value
 * directly with SetSignal/Signal/Wait. AROS AllocSignal returns a signal BIT
 * NUMBER, so convert to/from a mask to preserve AMP's intent. */
static inline LONG AllocSignalPPC(LONG num)
{
  LONG bit = AllocSignal(num);
  return (bit >= 0) ? (1L << bit) : -1;
}
static inline void FreeSignalPPC(LONG mask)
{
  LONG bit = 0;
  while (mask > 1) { mask >>= 1; bit++; }
  if (mask == 1) {
    FreeSignal(bit);
  }
}
#define SetSignalPPC SetSignal
#define SignalPPC Signal
#define WaitPPC Wait
#define GetSysTimePPC GetSysTime
#define SubTimePPC SubTime
#define CreateTaskPPC NewCreateTaskA
#define FindTaskPPC FindTask


#endif /* AROS_INC_H */
