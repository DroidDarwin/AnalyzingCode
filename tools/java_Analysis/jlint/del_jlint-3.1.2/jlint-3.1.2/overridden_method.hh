#ifndef OVERRIDDEN_METHOD_HH
#define OVERRIDDEN_METHOD_HH

class method_desc;

class overridden_method { 
  public:
    overridden_method* next;
    method_desc*       method;

    overridden_method(method_desc* mth, overridden_method* chain) { 
      method = mth;
      next = chain;
    }
};

#endif
