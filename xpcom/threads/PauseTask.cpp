/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "PauseTask.h"
#define __STDC_FORMAT_MACROS
#include <sys/time.h>
#include <unistd.h>
#include <inttypes.h>
#include "nsIObserverService.h"
#include "mozilla/Services.h"
#include <time.h>
#include "prrng.h"
using namespace mozilla;

static LazyLogModule sPauseTaskLog("PauseTask");
#ifdef LOG
#undef LOG
#endif
#define LOG(args) MOZ_LOG(sPauseTaskLog, mozilla::LogLevel::Debug, args)

#define US_TO_NS(x) (x*1000)
#define NS_TO_US(x) (x/1000)
#define PT_DURATION_CENTER_US 100000
#define PT_STATIC_CLOCK_GRAIN_US PT_DURATION_CENTER_US


// This is for creating a next-scheduled pausetask
pauseTask::pauseTask(pauseTask::tick aTickType, PauseTaskState* aState){
  // ALWAYS set state up first
  this->mState = aState;

  this->mTickType = aTickType;
  this->mStartTime_us = actualTime_us();
  this->mDuration_us = this->pickDuration_us();
}

// This should only be called by the nsThreadPool
pauseTask::pauseTask(nsThreadPool* const aParent ){
  // ALWAYS set state up first
  initState(aParent);

  this->mTickType = this->uptick;
  this->mStartTime_us = actualTime_us();
  this->mDuration_us = this->pickDuration_us();
}

//TODO: this gets passed around but never freed when everything dies, should fix that somehow
void pauseTask::initState(nsThreadPool* const aParent){
  this->mState = new PauseTaskState(aParent);

  //Calculate boot timestamp
  timespec tv1,tv2;
  clock_gettime(CLOCK_MONOTONIC, &tv1);
  clock_gettime(CLOCK_REALTIME, &tv2);
  int64_t v1 = tv1.tv_nsec + (1000000000*tv1.tv_sec);
  int64_t v2 = tv2.tv_nsec + (1000000000*tv2.tv_sec);
  int64_t boot_delta = v2-v1;
  LOG(("[FuzzyFox][PauseTask][Time] Boot delta %" PRIu64 " Monotonic %" PRIu64 " Realtime %" PRIu64 "\n", boot_delta, v1,v2));
  this->mState->mBootTimeStamp= US_TO_NS(roundToGrain_us(NS_TO_US(boot_delta)));

  long int seedv;
  PR_GetRandomNoise(&seedv,sizeof(long int));
  srand48_r(seedv,&(this->mState->mRandState));
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
    LOG(("[FuzzyFox][PauseTaskEvent] PT(%p) TP(%p) Overran budget of %" PRIu64 " by %" PRIu64 " \n", this,mState->mParent,this->mDuration_us,over_us));

    //TODO: this should probably influence uptick/downtick stuff, and it doesn't
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
    LOG(("[FuzzyFox][PauseTaskEvent] PT(%p) TP(%p) Finishing budget of %" PRIu64 " with %" PRIu64 " \n", this,mState->mParent,this->mDuration_us,remaining_us));

  }

  // Sleep for now
  usleep(remaining_us);

  //TODO we slept extra long almost certainly, what do we do about that?

  // Update clocks (and fire pending events etc)
  updateClocks();


  // Queue next event
  while(NS_FAILED(NS_DispatchToMainThread(
					  new pauseTask((this->mTickType == this->uptick)?this->downtick:this->uptick,mState),
					  0))){
    LOG(("[FuzzyFox][PauseTaskEvent] FATAL Unable to schedule next PauseTaskEvent! Retrying!\n"));
  }

  return NS_OK;
}

int64_t pauseTask::roundToGrain_us(int64_t aValue){
  int64_t grain = getClockGrain_us();
  return aValue-(aValue%grain);
}

void pauseTask::updateClocks(){

  int64_t timeus = actualTime_us();


  int64_t newTime_us = roundToGrain_us(timeus);
  int64_t newTimeStamp_ns = US_TO_NS(newTime_us) - mState->mBootTimeStamp;
  // newTime_us is the new canonical time for this scope!
  LOG(("[FuzzyFox][PauseTask][Time] New time is %" PRIu64 "\n", newTimeStamp_ns));

  // Fire notifications
  nsCOMPtr<nsIObserverService> os = services::GetObserverService();
  // Event firings on occur on downticks and have no data
  if(this->mTickType == this->downtick){
    os->NotifyObservers(nullptr,"fuzzyfox-fire-outbound",nullptr);
  }
  // Clocks get the official 'realtime' time
  // This happens on all ticks
  os->NotifyObservers(nullptr,"fuzzyfox-update-clocks",(char16_t*)&newTime_us);
  // Update the timestamp canonicaltime
  TimeStamp::UpdateFuzzyTimeStamp(newTimeStamp_ns);

}

uint64_t pauseTask::pickDuration_us(){
  
  // Get the next stateful random value
  // uniform random number from 0->2**31
  long int rval;
  lrand48_r(&(this->mState->mRandState),&rval);
  
  // We want uniform distribution from 1->PT_DURATION_CENTER*2
  // so that the mean is PT_DURATION_CENTER
  return 1+(rval%(PT_DURATION_CENTER_US*2));
}

int64_t pauseTask::getClockGrain_us(){
  // Static for now
  // Should be reading from some config probably
  return PT_STATIC_CLOCK_GRAIN_US;
}
