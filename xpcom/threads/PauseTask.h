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

  pauseTask(uint32_t aDuration, pauseTask::tick aTickType, nsThreadPool * const aParent);
  NS_IMETHOD Run() override;



 private:
  tick mTickType;
  uint32_t mDuration;
  uint64_t mStartTime;
  nsThreadPool* mParent;
  uint64_t pickDuration();
};

#endif /* PauseTask_h__ */
