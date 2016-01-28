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
#include "mozilla/TimeStamp.h"

class pauseTask :
  public nsRunnable
{
 private:
  class PauseTaskState{
  public:
    PauseTaskState(nsThreadPool* const aParent) {mParent = aParent;}
    nsThreadPool* mParent;
    int64_t mBootTimeStamp;
    struct drand48_data mRandState;

  };


 public:
  enum tick {uptick,downtick};

  pauseTask(nsThreadPool * const aParent);

  NS_IMETHOD Run() override;


  void updateClocks();
  int64_t getClockGrain_us();

 private:
  pauseTask(pauseTask::tick aTickType, PauseTaskState* aState);
  friend class mozilla::TimeStamp;

  PauseTaskState* mState;
  tick mTickType;
  uint32_t mDuration_us;
  uint64_t mStartTime_us;
  uint64_t pickDuration_us();
  static int64_t actualTime_us();
  int64_t roundToGrain_us(int64_t aValue);
  void initState(nsThreadPool* const aParent);
};



#endif /* PauseTask_h__ */
