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

#define PT_STATIC_DURATION  80
#define PT_STATIC_CLOCK_GRAIN 80

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


NS_IMETHODIMP
pauseTask::Run()
{
  // We need to check how long its been since we ran
  uint64_t endTime_us = actualTime_us();

  uint64_t remaining_us = 0;
  uint64_t durationCount = 1;

  // Pick the amount to sleep
  if( (endTime_us-this->mStartTime_us) > this->mDuration_us){
    // We ran over our budget!
    uint64_t over_us = (endTime_us-this->mStartTime_us)-this->mDuration_us;
    LOG(("[FuzzyFox][PauseTaskEvent] PT(%p) TP(%p) Overran budget of %" PRIu64 " by %" PRIu64 " \n", this,mParent,this->mDuration_us,over_us));

    uint64_t nextDuration_us = this->pickDuration_us();
    while(over_us > nextDuration_us){
      durationCount++;
      over_us -= nextDuration_us;
      nextDuration_us = this->pickDuration_us();
    }
    
    remaining_us = nextDuration_us - over_us;
  }
  else{
    // Didn't go over budget
    remaining_us = this->mDuration_us-(endTime_us-this->mStartTime_us);
    LOG(("[FuzzyFox][PauseTaskEvent] PT(%p) TP(%p) Finishing budget of %" PRIu64 " with %" PRIu64 " \n", this,mParent,this->mDuration_us,remaining_us));

  }

  // Sleep for now
  usleep(remaining_us);

  // Update clocks (and fire pending events etc)
  updateClocks();


  // Queue next event
  NS_DispatchToMainThread(
                    new pauseTask(this->pickDuration_us(),
                                  (this->mTickType == this->uptick)?this->downtick:this->uptick,
                                  mParent,mBootTimeStamp),
                    0);

  return NS_OK;
}

void pauseTask::updateClocks(){

  int64_t timeus = actualTime_us();
  TimeStamp ts = TimeStamp::Now();
  timespec tv1,tv2;
  int rv = clock_gettime(CLOCK_MONOTONIC, &tv1);
  rv = clock_gettime(CLOCK_REALTIME, &tv2);

  if(mBootTimeStamp == 0){
    int64_t v1 = tv1.tv_nsec + (1000000000*tv1.tv_sec);
    int64_t v2 = tv2.tv_nsec + (1000000000*tv2.tv_sec);
    int64_t boot_delta = v2-v1;
    LOG(("[FuzzyFox][PauseTask][Time] Boot delta %jd Monotonic %jd Realtime %jd\n", boot_delta, v1,v2));
    mBootTimeStamp= boot_delta;
  }

  int64_t grainus = getClockGrain_us();
  int64_t newTime_us = timeus - (timeus % grainus);
  int64_t newTimeStamp_ns = newTime_us*1000 - mBootTimeStamp;
  // newTime_us is the new canonical time for this scope!

  nsCOMPtr<nsIObserverService> os = services::GetObserverService();
  os->NotifyObservers(nullptr,"fuzzyfox-fire-outbound",(char16_t*)&newTimeStamp_ns);
  os->NotifyObservers(nullptr,"fuzzyfox-update-clocks",(char16_t*)&newTime_us);
}

uint64_t pauseTask::pickDuration_us(){
  // Static for now
  // Some normal distribution centered on a configurable value
  return PT_STATIC_DURATION;
}

int64_t pauseTask::getClockGrain_us(){
  // Static for now
  // Should be reading from some config probably
  return PT_STATIC_CLOCK_GRAIN;
}
