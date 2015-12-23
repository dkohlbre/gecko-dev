#include "DelayChannel.h"
#include "mozilla/Services.h"

namespace mozilla{

  DelayChannel::DelayChannel(){
    //    LOG(("DelayChannel creation!\n"));
  }

  DelayChannelQueue::DelayChannelQueue(){
    //    LOG(("Creating DelayChannelQueue\n"));
    this->listening = false;
    this->delayqueuelen = 0;
  }

  int DelayChannelQueue::FireQueue(){

    //LOG(("[FuzzyFox][AsyncOpen]: FIRING QUEUE of %i DelayChannels",delayChannelQueue.this->delayqueuelen))

    //TODO: get this from the DOM clock?
    TimeStamp ts = TimeStamp::Now();

    for(int i=0;i<this->delayqueuelen;i++){
      this->delayqueue[i]->delayready = true;
      ((nsHttpChannel*)this->delayqueue[i])->AsyncOpenFinal(ts);

      //TODO: This should really be NS_RELEASE, but that isn't working when i use it this way
      ((nsHttpChannel*)this->delayqueue[i])->Release();
      this->delayqueue[i] = NULL;
    }
    int fired = this->delayqueuelen;
    this->delayqueuelen = 0;

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
    this->delayqueue[this->delayqueuelen] = channel;
    this->delayqueuelen++;
    printf("&&&& DELAY LEN %i for %p\n",this->delayqueuelen,this);
    return this->delayqueuelen;
  }

  NS_IMETHODIMP
  DelayChannelQueue::Observe(nsISupports* aSubject, const char* aTopic,
			     const char16_t* aData)
  {
    if (!strcmp(aTopic, "fuzzyfox-fire-outbound")) {
      this->FireQueue();
    }
    return NS_OK;
  }
 
}
