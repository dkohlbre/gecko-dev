#ifndef _DelayChannel_h
#define _DelayChannel_h

namespace mozilla{
  class DelayChannel{
  public:
    DelayChannel();
    bool delayready = false;
    //NS_IMETHOD AsyncOpenFinal();
  };

  class DelayChannelQueue{
  public:
    DelayChannel* delayqueue[10];
    int delayqueuelen;
  };
  static DelayChannelQueue delayChannelQueue;
}
#endif /* _DelayChannel_h */
