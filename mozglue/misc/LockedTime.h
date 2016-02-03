/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef LockedTime_h
#define LockedTime_h
//borrowed from Time.h
#define PRMJ_USEC_PER_SEC       1000000L
#include <sys/time.h>
#include <stdint.h>


/* Locked clock impl*/
class LockedClock{
public:
    LockedClock(){
      this->currentClockUS = 0;
    }

    int64_t getLockedClock(){
      return this->currentClockUS;
    }

    bool clockStarted(){
      return this->currentClockUS != 0;
    }

    int64_t updateLockedClockUS(int64_t update){
      this->currentClockUS = update;
      return this->currentClockUS;
    }
#if defined(XP_UNIX)
    int64_t checkLockedClockOffset(){
      struct timeval tv;
      gettimeofday(&tv,0);
      int64_t actualTime = int64_t(tv.tv_sec) * PRMJ_USEC_PER_SEC + int64_t(tv.tv_usec);
      return actualTime-this->currentClockUS;
    }
#endif


private:
    int64_t currentClockUS = 0;
};

//extern LockedClock* lockedClock;

#endif /* LockedTime_h*/

