#include "PauseTask.h"
#define __STDC_FORMAT_MACROS
#include <sys/time.h>
#include <unistd.h>
#include <inttypes.h>
using namespace mozilla;

static LazyLogModule sPauseTaskLog("PauseTask");
#ifdef LOG
#undef LOG
#endif
#define LOG(args) MOZ_LOG(sPauseTaskLog, mozilla::LogLevel::Debug, args)


pauseTask::pauseTask(uint32_t aDuration, pauseTask::tick aTickType, nsThreadPool* const aParent){
  this->mDuration = aDuration;
  this->mParent = aParent;
  this->mTickType = aTickType;

  timeval tv;
  gettimeofday(&tv,NULL);
  this->mStartTime = tv.tv_usec + (1000000*tv.tv_sec);
}

NS_IMETHODIMP
pauseTask::Run()
{
  // We need to check how long its been since we ran
  timeval tv;
  gettimeofday(&tv,NULL);
  uint64_t endTime = tv.tv_usec + (1000000*tv.tv_sec);

  uint64_t remainingUS = 0;

  // Pick the amount to sleep
  if( (endTime-this->mStartTime) > this->mDuration){
    // We ran over our budget!
    uint64_t overUS = (endTime-this->mStartTime)-this->mDuration;
    LOG(("[PauseTaskEvent] PT(%p) TP(%p) Overran budget of %" PRIu64 " by %" PRIu64 " \n", this,mParent,this->mDuration,overUS));

    uint64_t nextDuration = this->pickDuration();
    while(overUS > nextDuration){
      overUS -= nextDuration;
      nextDuration = this->pickDuration();
    }
    remainingUS = nextDuration - overUS;
  }
  else{
    // Didn't go over budget
    remainingUS = this->mDuration-(endTime-this->mStartTime);
    LOG(("[PauseTaskEvent] PT(%p) TP(%p) Finishing budget of %"PRIu64" with %"PRIu64" \n", this,mParent,this->mDuration,remainingUS));

  }

  // Sleep for now
  usleep(remainingUS);


  // Queue next event
  mParent->Dispatch(
                    new pauseTask(this->pickDuration(),
                                  (this->mTickType == this->uptick)?this->downtick:this->uptick,
                                  mParent),
                    0);

  return NS_OK;
}

uint64_t pauseTask::pickDuration(){                                                                                                                           
  // Static for now
  return 80;
}
