/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/*
 * Implementation of the OS-independent methods of the TimeStamp class
 */

#include "mozilla/TimeStamp.h"
#include <string.h>
#include <pk11pub.h>

namespace mozilla {

/**
 * Wrapper class used to initialize static data used by the TimeStamp class
 */
struct TimeStampInitialization
{
  /**
   * First timestamp taken when the class static initializers are run. This
   * timestamp is used to sanitize timestamps coming from different sources.
   */
  TimeStamp mFirstTimeStamp;

  /**
   * Timestamp representing the time when the process was created. This field
   * is populated lazily the first time this information is required and is
   * replaced every time the process is restarted.
   */
  TimeStamp mProcessCreation;

  TimeStampInitialization()
  {
    TimeStamp::Startup();
    mFirstTimeStamp = TimeStamp::Now();
  };

  ~TimeStampInitialization()
  {
    TimeStamp::Shutdown();
  };
};

  
static TimeStampInitialization sInitOnce;

MFBT_API TimeStamp
TimeStamp::ProcessCreation(bool& aIsInconsistent)
{
  aIsInconsistent = false;

  if (sInitOnce.mProcessCreation.IsNull()) {
    char* mozAppRestart = getenv("MOZ_APP_RESTART");
    TimeStamp ts;

    /* When calling PR_SetEnv() with an empty value the existing variable may
     * be unset or set to the empty string depending on the underlying platform
     * thus we have to check if the variable is present and not empty. */
    if (mozAppRestart && (strcmp(mozAppRestart, "") != 0)) {
      /* Firefox was restarted, use the first time-stamp we've taken as the new
       * process startup time. */
      ts = sInitOnce.mFirstTimeStamp;
    } else {
      TimeStamp now = Now();
      uint64_t uptime = ComputeProcessUptime();

      ts = now - TimeDuration::FromMicroseconds(uptime);

      if ((ts > sInitOnce.mFirstTimeStamp) || (uptime == 0)) {
        /* If the process creation timestamp was inconsistent replace it with
         * the first one instead and notify that a telemetry error was
         * detected. */
        aIsInconsistent = true;
        ts = sInitOnce.mFirstTimeStamp;
      }
    }

    sInitOnce.mProcessCreation = ts;
  }

  return sInitOnce.mProcessCreation;
}

  /** Start Fuzzy Time stuff **/
static bool needsFuzzyInit = true; 
static TimeStamp nextUpdate,canonicalTime;

  
  
  // Configured via about::config prefs, set in init
  static uint64_t timeGranularity_ns = 5000; // Defaults to 5us
  
bool
TimeStamp::realNeedsInit(){
  return needsFuzzyInit;
}

#ifdef XP_WIN
  TimeStamp TimeStamp::roundTime(TimeStamp time){
    return TimeStamp(time.mValue.roundWithNs(timeGranularity_ns));
}
#endif
 
#ifndef XP_WIN
  TimeStamp TimeStamp::roundTime(TimeStamp time_ns){
  return TimeStamp(floor(time_ns.mValue/timeGranularity_ns)*timeGranularity_ns);
}
#endif

static TimeDuration pickDuration(){
  unsigned char randomData[8];
  SECStatus rv = PK11_GenerateRandom(randomData,8);
  if (rv != SECSuccess){
    //TODO: Log this failure somewhere
    // This is bad and we should stop trying to get random values
    needsFuzzyInit = true;
  }
 return TimeDuration::FromMicroseconds(timeGranularity_ns/1000.0);
}

void
TimeStamp::realInitFuzzyTime(unsigned int granularity_ns){
  needsFuzzyInit = false;
  timeGranularity_ns = granularity_ns;
}



  
MFBT_API TimeStamp
TimeStamp::Now_fuzzy(TimeStamp currentTime){
  if(needsFuzzyInit){
    //    printf("[TIMESTAMP] Returning UNFUZZY\n");
    return TimeStamp(currentTime);

  }
  //  printf("[TIMESTAMP] Grain %i\n",timeGranularity_ns);
  // Are we overdue to (maybe) update the time?
  if(nextUpdate < currentTime){
    
    // Next update check occurs at a random time in the future
    nextUpdate = currentTime+pickDuration();

    // This may NOT result in a change to canonicalTime
    canonicalTime = roundTime(currentTime);
  }

  return TimeStamp(canonicalTime);
}

  /** Stop Fuzzy Time stuff **/
  
void
TimeStamp::RecordProcessRestart()
{
  sInitOnce.mProcessCreation = TimeStamp();
}

} // namespace mozilla
