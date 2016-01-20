#ifndef _DelayChannel_h
#define _DelayChannel_h

#include "nsIObserverService.h"
#include "nsIObserver.h"
#include "mozilla/Mutex.h"

namespace mozilla{
  class DelayChannel{
  public:
    DelayChannel();
    bool delayready = false;
    //NS_IMETHOD AsyncOpenFinal();
  };

  class DelayChannelQueue : public nsIObserver{

  private:
    static const int PageLen = 128;
    struct DelayChannelQueuePage{
      DelayChannel* page[PageLen];
      DelayChannelQueuePage* next;
    };


  public:
    NS_DECL_NSIOBSERVER

    Mutex queueLock;
    DelayChannel* delayqueue[100];
    DelayChannelQueuePage firstPage;
    bool listening;
    int delayqueuelen;
    DelayChannelQueue();
    ~DelayChannelQueue();
    int FireQueue();
    int QueueChannel(DelayChannel* channel);


    //TODO fix this
    NS_IMETHOD QueryInterface(REFNSIID aIID, void** aResult) override {}

    NS_IMETHOD_(MozExternalRefCountType) AddRef(void) override
      {
	return 1;
      }
    NS_IMETHOD_(MozExternalRefCountType) Release(void) override
      {
	return 1;
      }

  };
  static DelayChannelQueue delayChannelQueue;


  static int AttemptQueueChannel(DelayChannel* channel){
    return delayChannelQueue.QueueChannel(channel);
  }

}
#endif /* _DelayChannel_h */
