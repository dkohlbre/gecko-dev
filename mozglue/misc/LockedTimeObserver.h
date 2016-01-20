#ifndef LockedTimeObserver_h
#define LockedTimeObserver_h

#include "LockedTime.h"
#include "nsIObserver.h"
#include "nsIObserverService.h"
#include "mozilla/Services.h"

namespace mozilla{

class FuzzyfoxClockObserver : public nsIObserver{
 public:
  //  NS_DECL_NSIOBSERVER
    
  /*       FuzzyfoxClockObserver(); */
  /*     ~FuzzyfoxClockObserver(); */

  /* void init(); */
  //TODO fix this
  NS_IMETHOD QueryInterface(REFNSIID aIID, void** aResult) override { return NS_OK;}
  
  NS_IMETHOD_(MozExternalRefCountType) AddRef(void) override
    {
      return 1;
    }
  NS_IMETHOD_(MozExternalRefCountType) Release(void) override
    {
      return 1;
    }

  FuzzyfoxClockObserver(){
    ready = false;
  }
  
  ~FuzzyfoxClockObserver() {
    
    nsCOMPtr<nsIObserverService> os  = services::GetObserverService();
    
    if (os) {
      os->RemoveObserver(this,"fuzzyfox-update-clocks");
    }
  }

  void initIfNeeded(LockedClock* aLockedClock){
    if(ready)
      return;

    this->ready = true;
    
    this->lockedClock = aLockedClock;

    nsCOMPtr<nsIObserverService> os = services::GetObserverService();
    
    if (os) {
      //LOG(("[FuzzyFox][FCO]: FCO observing OK\n"));
      os->AddObserver(this,"fuzzyfox-update-clocks",false);
    }
    else{
      //LOG(("[FuzzyFox][FCO]: FATAL FCO couldn't observe\n"));
      printf("&&&&&&&& FATAL FCO DIDNT SUBSCRIBE!\n");
    }
  }
  
  nsresult Observe(nsISupports* aSubject, const char* aTopic,
		   const char16_t* aData){
    if (!strcmp(aTopic, "fuzzyfox-update-clocks")) {
      lockedClock->updateLockedClockUS(*(int64_t*)aData);
    }
    return NS_OK;
  }
  
  LockedClock* lockedClock;
  bool ready;

};  
}
#endif /* LockedTimeObserver_h */
