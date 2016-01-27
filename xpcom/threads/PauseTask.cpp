#include "PauseTask.h"
#define __STDC_FORMAT_MACROS
#include <sys/time.h>
#include <unistd.h>
#include <inttypes.h>
#include "nsIObserverService.h"
#include "mozilla/Services.h"
#include <time.h>

using namespace mozilla;

static LazyLogModule sPauseTaskLog("PauseTask");
#ifdef LOG
#undef LOG
#endif
#define LOG(args) MOZ_LOG(sPauseTaskLog, mozilla::LogLevel::Debug, args)

#define PT_DURATION_CENTER  150
#define PT_STATIC_CLOCK_GRAIN PT_DURATION_CENTER

pauseTask::pauseTask(uint32_t aDuration_us, pauseTask::tick aTickType, nsThreadPool* const aParent, int64_t aBootTimeStamp){
  this->mDuration_us = aDuration_us;
  this->mParent = aParent;
  this->mTickType = aTickType;
  this->mBootTimeStamp = aBootTimeStamp;
  timeval tv;
  gettimeofday(&tv,NULL);
  this->mStartTime_us = actualTime_us();

}

int64_t pauseTask::actualTime_us(){
  timeval tv;
  gettimeofday(&tv,NULL);
  return tv.tv_usec + (1000000*tv.tv_sec);
}


#define US_TO_NS(x) (x*1000)
#define NS_TO_US(x) (x/1000)

NS_IMETHODIMP
pauseTask::Run()
{
  // We need to check how long its been since we ran
  uint64_t endTime_us = actualTime_us();
  LOG(("[FuzzyFox][PauseTaskEvent] Duration of %" PRIu64 " \n", endTime_us-this->mStartTime_us));

  // Queue next event
  while(NS_FAILED(NS_DispatchToMainThread(
					  new pauseTask(0,
                                  (this->mTickType == this->uptick)?this->downtick:this->uptick,
                                  mParent,mBootTimeStamp),
                    0))){
    LOG(("[FuzzyFox][PauseTaskEvent] FATAL Unable to schedule next PauseTaskEvent! Retrying!\n"));
  }

  return NS_OK;
}

