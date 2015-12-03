#ifndef PauseTask_h__
#define PauseTask_h__

#include "nsThreadPool.h"
#include "nsIRunnable.h"
#include "nsEventQueue.h"
#include "nsThreadUtils.h"
#include "PauseTask.h"
#include "mozilla/Attributes.h"
#include "mozilla/AlreadyAddRefed.h"
#include "mozilla/Mutex.h"
#include "mozilla/Monitor.h"


class pauseTask :
  public nsRunnable
{
 public:
  enum tick {uptick,downtick};

  pauseTask(uint32_t aDuration_us, pauseTask::tick aTickType, nsThreadPool * const aParent);
  NS_IMETHOD Run() override;


  void updateClocks();
  int64_t getClockGrain_us();

 private:
  tick mTickType;
  uint32_t mDuration_us;
  uint64_t mStartTime_us;
  nsThreadPool* mParent;
  uint64_t pickDuration_us();
  static int64_t actualTime_us();
};

#endif /* PauseTask_h__ */
