class base_class {
    public int x;
    public void foo(int y) 
    {
	int x; //+ shadow class component
	x = y & 3;
	if ((x & 8) != 0) { //+ always evaluates to false
	    x >>>= 1;
	    if (x < y && y <= 0) { //+ always evaluates to false
		y = x;
	    }
	    if (x <= 0) //+ can be replaced with == 0
		if (x < y)
		    x ++;
	    else x--; //+ wrong assumption about else branch association
	} 
    }
    public int refer(derived_class dc) {
	/* start of nested comments
	commented_statement; /* nested comments */
	if (dc == null) { 
	    base_class bc = null;
	    return dc.refer(bc); //+ variable 'dc' may be null
	}
	return 0;
    }
    public int longop(int x) {
	if ((x & 4) == 0) { 
	    if ((x & 4) != 0) { //+ always false
		x &= ~4;
	    }
	}
        long y = x & 255;
	if (y + 1 > 1000) { //+ always false 
	    y += 1;
	    if ((y & 1) == (y & 1024)) { //+ disjoint mask
	        if (y - 1 < 0) { //+ always false
		    y *= 1024*1024;
		    short s = (short)y; //* no intersection
		    return s;
		}
	    }
	} else { 
	    return (int)(y << 32); //+ overflow
	}
	x &= 31;
	if ((++x & 1024) == 0) { // aways true
	    x <<= ++x; // shift count can be greater than 31
	} 
	return 0;
    }	
    static public int increment(base_class bc) { 
	return ++bc.x; //+ variable bc may be null 
    }
}

class derived_class extends base_class { 
    public int x; //+ shadow base_class::x
  
    //+ do not override method foo(int) of base class
    public void foo(int i, short s, char c)
    {  
	if (s == c) { //+ signed/unsigned mix: short is compared with char
	    if ((c-1)*3 < (c % 3) - 5) //+ always false
		if (String.valueOf(1) == "1") //+ using of == for Strings
		    return;
	    if (c >= 0) { //+ always true
		i <<= 32 - (i & 31); //+ shift counter can be greater than 31
		i = (i & 8) >> 5; //+ result of operation is always 0
	    }
	    i = s & 3;
	    switch (i) { 
	      case 0:
		s *= i; //+ zero operand
		break;
	      case 1:
		if (i < 1) //+ condition is always false
		    i++;
		    i++; //+ wrong assumption about if body 
		//+ BREAK is missed
	      case 4: //+ constant is out of range
		while (i != 0); //+ condition is always false
		    i--; //+ wrong assumption about while body
		i = 1<<i - 1; //+ wrong assumption about operators priorities
	    }    
	} else if (s <= 0) { //+ Can be replaced with == 0
	    long l = i * 2; //+ possible overflow
	    s = (short)(((i&1)+1)<<16); //+ expression is out of range
	    l = 0x11111l; //+ 'l' used instead of '1'
	    s = 0;
	    x = x * s; //+ operand is 0
	}
    }
    static public boolean equal(boolean a, boolean b) { 
	if (a = b) { //+ using of '=" instead of "=="
	  return true;
	}
	return false;
    }
    public boolean cmp(String a, String b) { 
	return a == b; //+ compare strings by == 
    } 
    public void finalize(){} //+ finalize method doesn't call super.finalize()

     //+ do not override method refer(derived_class) of base class
    public int refer(base_class bc) { 
	bc.refer(this);  //+ variable 'bc' may be null
	notify(); //+ notify is called from non-synchronized method
	return base_class.increment(bc);
    }
    synchronized public void monitor(derived_class dc) {
	dc.monitor(this); //+ can be source of deadlock
	equal(true, false); //+ can be source of race condition
	
	foo(1, (short)2, '3'); 
    } 	
}

interface Executable extends Runnable { 
    void foo(); 
    void f();
}
 
class Thread1 implements Executable { 
    public int x;
    Thread2 t;
    synchronized public void foo() { 
        t.x = 0; //+ non-synchronized access to data
	t.f(); //+ can be a source of deadlock
    }
    public void reset() {
	x = 0; 
    }
    public void run() { //+ Method Run not synchronized 
        foo();
	t.g(); 
    }        
    synchronized public void f() { 
        try {
	    wait(); 
        } catch(Exception e) {}
	x = 0; 
    }
    synchronized public static int h() { 
        Thread2.h(new Thread1_derived()); //+ possible deadlock
	return Thread2.zero;
    }
}

class Thread1_derived extends Thread1 { 
    public void f() {  //+ synchronized attribute lost
        h(); //+ possible deadlock
    }
}

class Thread2 { 
    public int x;
    public final static int zero = 0;
    Executable t;
    synchronized public void f() { 
        t.f(); //+ possible source of deadlock
    }
    synchronized public void g() {
        t.foo(); //+ possible source of deadlock
	x = zero;
    }
    synchronized static public void h() {
	h();
    }
    synchronized static public int h(Thread1_derived td) { 
        td.f(); //+ possible source of deadlock
	return zero;
    }
    synchronized public void sync(Thread1 t) //+ t shadows calss component
    {
	t.f(); //+ synchronized method Thread1.f() can call wait
    }
}
