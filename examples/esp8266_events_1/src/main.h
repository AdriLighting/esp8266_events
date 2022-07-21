#ifndef MAIN_H
  #define MAIN_H
  
  #include <Arduino.h>

  class TestEvent
  {
  public:
    TestEvent();
    ~TestEvent(){};
    
  };
  class TestEventItem
  {
    uint8_t * _event;
    const char * _name = "";
  public:
    TestEventItem(uint8_t eP1, uint8_t eP2, const char * name);
    ~TestEventItem(){};
    void event_1();
    void event_2();
    
  };

#endif // MAIN_H