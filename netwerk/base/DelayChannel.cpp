#include "DelayChannel.h"
#include "mozilla/Services.h"

namespace mozilla{

#define LOG printf

  DelayChannelQueue::~DelayChannelQueue(){
  } 
  DelayChannelQueue::DelayChannelQueue(){
  }

  int DelayChannelQueue::FireQueue(){
    return 0;
  }


  int DelayChannelQueue::QueueChannel(DelayChannel* channel){
    return this->delayqueuelen;
  }

  NS_IMETHODIMP
  DelayChannelQueue::Observe(nsISupports* aSubject, const char* aTopic,
			     const char16_t* aData)
  {
  }
}
