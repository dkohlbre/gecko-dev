#include "DelayChannel.h"

namespace mozilla{

  DelayChannel::DelayChannel(){
    //    LOG(("DelayChannel creation!\n"));
  }

  DelayChannelQueue::DelayChannelQueue(){
    //    LOG(("Creating DelayChannelQueue\n"));
  }

  int DelayChannelQueue::FireQueue(){

    //TODO: get this from the DOM clock?
    TimeStamp ts = TimeStamp::Now();

    for(int i=0;i<delayqueuelen;i++){
      delayqueue[i]->delayready = true;
      ((nsHttpChannel*)delayqueue[i])->AsyncOpenFinal(ts);

      //TODO: This should really be NS_RELEASE, but that isn't working when i use it this way
      ((nsHttpChannel*)delayqueue[i])->Release();
      delayqueue[i] = NULL;
    }
    int fired = delayqueuelen;
    delayqueuelen = 0;
    return fired;
  }


  int DelayChannelQueue::QueueChannel(DelayChannel* channel){
    ((HttpBaseChannel*)channel)->AddRef();
    delayqueue[delayqueuelen] = channel;
    delayqueuelen++;
    return delayqueuelen;
  }

}
