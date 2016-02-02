#include "DelayChannel.h"
#include "mozilla/Services.h"

namespace mozilla{

#define LOG printf

  DelayChannelQueue::~DelayChannelQueue(){
    //    LOG(("[FuzzyFox][DCQ]DelayChannelQueue Destroyed!\n"));
    if(this->listening){
      nsCOMPtr<nsIObserverService> os = services::GetObserverService();
      if(os) {
	os->RemoveObserver(this,"fuzzyfox-fire-outbound");
      }
    }
  } 
  DelayChannelQueue::DelayChannelQueue(){
    //    LOG(("[FuzzyFox][DCQ]Creating DelayChannelQueue\n"));
    this->listening = false;
    this->delayqueuelen = 0;

    nsCOMPtr<nsIObserverService> os = services::GetObserverService();
    
    if (os) {
      //      LOG(("[FuzzyFox][DCQ]: DCQ observing OK\n"));
      os->AddObserver(this,"fuzzyfox-fire-outbound",false);
      this->listening = true;
    }
    else{
      //      LOG(("[FuzzyFox][DCQ]: FATAL DCQ couldn't observe\n"));
    }

    memset(&(this->firstPage),'\0',sizeof(DelayChannelQueuePage));

  }

  int DelayChannelQueue::FireQueue(){

    MOZ_ASSERT(NS_IsMainThread(), "dcq firing from wrong thread");
    if(this->delayqueuelen == 0){
      return 0;
    }
    //LOG("[FuzzyFox][DCQ]: FIRING QUEUE of %i DelayChannels for DCQ %p\n",this->delayqueuelen,this);

    //TODO: get this from the DOM clock?
    TimeStamp ts = TimeStamp::Now();


    DelayChannelQueuePage* currentpage = &firstPage;

    for(int i=0;i<this->delayqueuelen;i++){
      if(i % DelayChannelQueue::PageLen == 0 && i > 0){
	currentpage = currentpage->next;
      }

      //      LOG("[FuzzyFox][DCQ]: FIRING Element %p for DCQ %p\n",currentpage->page[i%DelayChannelQueue::PageLen],this);
      ((nsHttpChannel*)currentpage->page[i%DelayChannelQueue::PageLen])->AsyncOpenFinal(ts);
      //      LOG("[FuzzyFox][DCQ]: Finished Element %p for DCQ %p\n",currentpage->page[i%DelayChannelQueue::PageLen],this);

      //TODO: This should really be NS_RELEASE, but that isn't working when i use it this way
      ((nsHttpChannel*)currentpage->page[i%DelayChannelQueue::PageLen])->Release();
      currentpage->page[i%DelayChannelQueue::PageLen] = NULL;
    }
    int fired = this->delayqueuelen;
    this->delayqueuelen = 0;
    //    LOG("[FuzzyFox][DCQ]: Firing done for DCQ %p\n",this);
    return fired;
  }


  int DelayChannelQueue::QueueChannel(DelayChannel* channel){

    MOZ_ASSERT(NS_IsMainThread(), "dcq from wrong thread");
    //LOG("[FuzzyFox][DCQ]: Queuing Channel %p for DCQ %p\n",channel,this);
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
      newestpage->next = new DelayChannelQueuePage();
      memset(&(newestpage),'\0',sizeof(DelayChannelQueuePage));      
      newestpage=newestpage->next;
      newestpage->next = NULL;
      //LOG(("[FuzzyFox][DCQ]: Had to make a new DCQ page!\n"));
    }
    newestpage->page[idx] = channel;
    this->delayqueuelen++;
    //LOG("[FuzzyFox][DCQ]: Queued, new length %i for DCQ %p\n",this->delayqueuelen,this);
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
