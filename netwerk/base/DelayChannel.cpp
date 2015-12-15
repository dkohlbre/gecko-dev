#include "DelayChannel.h"
#include "mozilla/Services.h"

namespace mozilla{

  DelayChannel::DelayChannel(){
    //    LOG(("DelayChannel creation!\n"));
  }

  DelayChannelQueue::DelayChannelQueue(){
    //    LOG(("Creating DelayChannelQueue\n"));
    this->listening = false;
  }

  int DelayChannelQueue::FireQueue(){

    //LOG(("[FuzzyFox][AsyncOpen]: FIRING QUEUE of %i DelayChannels",delayChannelQueue.delayqueuelen))

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
    if(!this->listening){
      nsCOMPtr<nsIObserverService> os = services::GetObserverService();
      
      if (os) {
	printf("&&& DELAYCHANNELQUEUE OK\n");
	os->AddObserver(this,"fuzzyfox-fire-outbound",false);
	this->listening = true;
      }
      else{
	printf("&&& FATAL CANNOT DELAYCHANNELQUEUE\n");
      }
    }



    ((HttpBaseChannel*)channel)->AddRef();
    delayqueue[delayqueuelen] = channel;
    delayqueuelen++;
    return delayqueuelen;
  }

  NS_IMETHODIMP
  DelayChannelQueue::Observe(nsISupports* aSubject, const char* aTopic,
			     const char16_t* aData)
  {
    printf("&&&&&&&&GOT A FIRE\n");
    if (!strcmp(aTopic, "fuzzyfox-fire-outbound")) {
      this->FireQueue();
    }
    return NS_OK;
  }
 
}
