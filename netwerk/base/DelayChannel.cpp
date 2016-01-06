#include "DelayChannel.h"
#include "mozilla/Services.h"

namespace mozilla{

  DelayChannel::DelayChannel(){
    //    LOG(("[FuzzyFox][DC]DelayChannel creation!\n"));
  }

  DelayChannelQueue::~DelayChannelQueue(){
    //    LOG(("[FuzzyFox][DCQ]DelayChannelQueue Destroyed!\n"));
  }

  DelayChannelQueue::DelayChannelQueue():
    queueLock("DelayChannelQueue"){
    //    LOG(("[FuzzyFox][DCQ]Creating DelayChannelQueue\n"));
    this->listening = false;
    this->delayqueuelen = 0;
  }

  int DelayChannelQueue::FireQueue(){
    MutexAutoLock lock(this->queueLock);
    if(this->delayqueuelen == 0){
      return 0;
    }
    //LOG(("[FuzzyFox][DCQ]: FIRING QUEUE of %i DelayChannels for DCQ %p",delayChannelQueue.this->delayqueuelen,this));

    //TODO: get this from the DOM clock?
    TimeStamp ts = TimeStamp::Now();

    DelayChannelQueuePage* currentpage = &firstPage;
    int idx = 0;

    for(int i=0;i<this->delayqueuelen;i++){
      if(i % DelayChannelQueue::PageLen == 0 && i > 0){
	currentpage = currentpage->next;
      }

      currentpage->page[i%DelayChannelQueue::PageLen]->delayready = true;
      ((nsHttpChannel*)currentpage->page[i%DelayChannelQueue::PageLen])->AsyncOpenFinal(ts);

      //TODO: This should really be NS_RELEASE, but that isn't working when i use it this way
      ((nsHttpChannel*)currentpage->page[i%DelayChannelQueue::PageLen])->Release();
      currentpage->page[i%DelayChannelQueue::PageLen] = NULL;
    }
    int fired = this->delayqueuelen;
    this->delayqueuelen = 0;
    //LOG(("[FuzzyFox][DCQ]: Firing done for DCQ %p\n",this));
    return fired;
  }


  int DelayChannelQueue::QueueChannel(DelayChannel* channel){
    MutexAutoLock lock(this->queueLock);
    if(!this->listening){
      nsCOMPtr<nsIObserverService> os = services::GetObserverService();
      
      if (os) {
        //LOG(("[FuzzyFox][DCQ]: DCQ observing OK\n"));
	os->AddObserver(this,"fuzzyfox-fire-outbound",false);
	this->listening = true;
      }
      else{
	 //LOG(("[FuzzyFox][DCQ]: FATAL DCQ couldn't observe\n"));
      }
    }



    ((HttpBaseChannel*)channel)->AddRef();
    int pageidx=0;
    int idx=this->delayqueuelen;
    //Find the current page to queue on
    while(idx >= DelayChannelQueue::PageLen){
      pageidx++;
      idx-=DelayChannelQueue::PageLen;
    }

    // Find the new queue spot
    DelayChannelQueuePage* newestpage = &firstPage;
    while(pageidx > 0 && newestpage->next != NULL){
      pageidx--;
      newestpage = newestpage->next;
    }

    // Check if we are making a new page
    if(pageidx > 0){
      newestpage->next = new DelayChannelQueuePage();//malloc(sizeof(DelayChannelQueuePage));
      newestpage=newestpage->next;
      newestpage->next = NULL;
      //LOG(("[FuzzyFox][DCQ]: Had to make a new DCQ page!\n"));
    }
    newestpage->page[idx] = channel;
    this->delayqueuelen++;
    //LOG(("[FuzzyFox][DCQ]: Queued, new length %i for DCQ %p\n",this->delayqueuelen,this));
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
