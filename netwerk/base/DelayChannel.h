#ifndef _DelayChannel_h
#define _DelayChannel_h

#include "nsIObserverService.h"
#include "nsIObserver.h"

namespace mozilla{
  class DelayChannel{
  public:
    DelayChannel() {};
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

    DelayChannelQueuePage firstPage;
    bool listening;
    int delayqueuelen;
    DelayChannelQueue();
    ~DelayChannelQueue();
    int FireQueue();
    int QueueChannel(DelayChannel* channel);


    //TODO fix this
    NS_IMETHOD QueryInterface(REFNSIID aIID, void** aResult) override {return NS_ERROR_NO_INTERFACE;}

    NS_IMETHOD_(MozExternalRefCountType) AddRef(void) override
      {
	return 2;
      }
    NS_IMETHOD_(MozExternalRefCountType) Release(void) override
      {
	return 1;
      }

  };
  static DelayChannelQueue* delayChannelQueue;


  static int AttemptQueueChannel(DelayChannel* channel){
    if(delayChannelQueue == NULL){
      delayChannelQueue = new DelayChannelQueue();
      delayChannelQueue->AddRef();
    }
    return delayChannelQueue->QueueChannel(channel);
  }

}
#endif /* _DelayChannel_h */
