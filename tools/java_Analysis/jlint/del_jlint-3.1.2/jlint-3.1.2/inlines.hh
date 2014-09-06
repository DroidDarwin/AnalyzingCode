// inline functions

#ifndef INLINES_HH
#define INLINES_HH

#include <assert.h>

inline int unpack2(byte* s) { 
  return (s[0] << 8) + s[1]; 
}

inline int unpack4(byte* s) { 
  return (((((s[0] << 8) + s[1]) << 8) + s[2]) << 8) + s[3];
} 

inline int unpack2_le(byte* s) { 
  return (s[1] << 8) + s[0]; 
}

inline int unpack4_le(byte* s) { 
  return (((((s[3] << 8) + s[2]) << 8) + s[1]) << 8) + s[0];
} 

//
// Some bit support functions
//

inline int first_set_bit(int4 val)
{
  if (val == 0) { 
    return 32;
  } 
  int n = 0;
  while (!(val & 1)) { 
    n += 1;
    val >>= 1;
  }
  return n;
}

inline int last_set_bit(nat4 val)
{
  int n = 0;
  if (val == 0) {
    return -1;
  }
  while ((val >>= 1) != 0) { 
    n += 1;
  }
  return n;
}

inline int minimum(int4 x, int4 y) { return x < y ? x : y; }
inline int maximum(int4 x, int4 y) { return x > y ? x : y; }

inline int4 make_mask(int shift) { 
  return shift < 0 ? ALL_BITS : shift >= 32 ? 0 : ALL_BITS << shift;
}

inline int4 make_mask(int high_bit, int low_bit) 
{
  int4 mask = (high_bit >= 31) ? ALL_BITS : ((1 << (high_bit+1)) - 1);
  if (low_bit < 32) mask &= ~((1 << low_bit) - 1);
  return mask;
}

inline int4 make_lshift_mask(int4 mask, int min_shift, int max_shift) {  
  if (unsigned(max_shift - min_shift) >= 32) { 
    return ALL_BITS;
  }
  int4 result = 0;
  while (min_shift <= max_shift) {
    result |= mask << (min_shift & 31);
    min_shift += 1;
  }
  return result;
}

inline int4 make_rshift_mask(int4 mask, int min_shift, int max_shift) {  
  if (unsigned(max_shift - min_shift) >= 32) { 
    return ALL_BITS;
  }
  int4 result = 0;
  while (min_shift <= max_shift) {
    result |= mask >> (min_shift & 31);
    min_shift += 1;
  }
  return result;
}

inline int4 make_rushift_mask(nat4 mask, int min_shift, int max_shift) {  
  if (unsigned(max_shift - min_shift) >= 32) { 
    return ALL_BITS;
  }
  int4 result = 0;
  while (min_shift <= max_shift) {
    result |= mask >> (min_shift & 31);
    min_shift += 1;
  }
  return result;
}

inline bool calculate_multiply_range(vbm_operand& z, 
                  vbm_operand& x, vbm_operand& y)
{
  // z = x*y
  if (x.max < 0 && y.max < 0) { 
    z.min = x.max*y.max;
    z.max = x.min*y.min;
    return (z.max > 0 && z.max/x.min == y.min);// no overflow
  } else if (x.max < 0 && y.min < 0 && y.max >= 0) { 
    z.min = x.min*y.max;
    z.max = x.min*y.min;
    return (z.max > 0 && z.min/x.min == y.max && z.max/x.min == y.min);
  } else if (x.max < 0 && y.min >= 0) { 
    z.min = x.min*y.max;
    z.max = x.max*y.min;
    return (z.min/x.min == y.max && z.max/x.max == y.min); // no overflow
  } else if (x.min < 0 && x.max >= 0 && y.max < 0) { 
    z.min = x.max*y.min;
    z.max = x.min*y.min;
    return (z.max > 0 && z.min/y.min == x.max && z.max/y.min == x.min); 
  } else if (x.min < 0 && x.max >= 0 && y.min < 0 && y.max >= 0) { 
    int4 m1, m2;
    m1 = x.min*y.max; 
    m2 = x.max*y.min;
    if (m1/x.min != y.max || m2/y.min != x.max) return false;
    z.min = minimum(m1, m2);
    m1 = x.max*y.max; 
    m2 = x.min*y.min;
    if (m2 <= 0 || (x.max != 0 && m1/x.max != y.max) || m2/y.min != x.min) 
      return false;
    z.max = maximum(m1, m2);
    return true;
  } else if (x.min < 0 && x.max >= 0 && y.min >= 0) { 
    z.min = x.min*y.max;
    z.max = x.max*y.max;
    return z.min/x.min == y.max && (x.max == 0 || z.max/x.max == y.max);
  } else if (x.min >= 0 && y.max < 0) { 
    z.min = x.max*y.min;
    z.max = x.min*y.max;
    return z.min/y.min == x.max && z.max/y.max == x.min;
  } else if (x.min >= 0 && y.min < 0 && y.max >= 0) {
    z.min = x.max*y.min;
    z.max = x.max*y.max;
    return z.min/y.min == x.max && (x.max == 0 || z.max/x.max == y.max);
  } else { 
    assert(x.min >= 0 && y.min >= 0);
    z.min = x.min*y.min;
    z.max = x.max*y.max;
    return x.max == 0 || z.max/x.max == y.max;
  }
}

#ifdef INT8_DEFINED
inline int first_set_bit(int8 val)
{
  if (val == 0) { 
    return 64;
  } 
  int n = 0;
  while (!(val & 1)) { 
    n += 1;
    val >>= 1;
  }
  return n;
}

inline int8 make_int8_mask(int shift) { 
  return shift < 0 ? INT8_ALL_BITS : shift >= 64 
      ? INT8_ZERO : INT8_ALL_BITS << shift;
}

inline int8 make_int8_lshift_mask(int8 mask, int min_shift, int max_shift) {  
  if (unsigned(max_shift - min_shift) >= 64) { 
    return INT8_ALL_BITS;
  }
  int8 result = 0;
  while (min_shift <= max_shift) {
    result |= mask << (min_shift & 63);
    min_shift += 1;
  }
  return result;
}

inline int8 make_int8_rshift_mask(int8 mask, int min_shift, int max_shift) {  
  if (unsigned(max_shift - min_shift) >= 64) { 
    return INT8_ALL_BITS;
  }
  int8 result = 0;
  while (min_shift <= max_shift) {
    result |= mask >> (min_shift & 63);
    min_shift += 1;
  }
  return result;
}

inline int8 make_int8_rushift_mask(nat8 mask, int min_shift, int max_shift) {  
  if (unsigned(max_shift - min_shift) >= 64) { 
    return INT8_ALL_BITS;
  }
  int8 result = 0;
  while (min_shift <= max_shift) {
    result |= mask >> (min_shift & 63);
    min_shift += 1;
  }
  return result;
}

inline bool calculate_int8_multiply_range(vbm_operand* x, vbm_operand* y)
{
  // x *= y
  int8 x_min = LOAD_INT8(x, min);
  int8 x_max = LOAD_INT8(x, max);
  int8 y_min = LOAD_INT8(y, min);
  int8 y_max = LOAD_INT8(y, max);
  int8 z_min, z_max;
  if (x_max < 0 && y_max < 0) { 
  z_min = x_max*y_max;
    z_max = x_min*y_min;
    STORE_INT8(x, min, z_min);
    STORE_INT8(x, max, z_max);
    return (z_min > 0 && z_max/x_min == y_min); // no overflow
  } else if (x_max < 0 && y_min < 0 && y_max >= 0) { 
    z_min = x_min*y_max;
    z_max = x_min*y_min;
    STORE_INT8(x, min, z_min);
    STORE_INT8(x, max, z_max);
    return (z_max > 0 && z_min/x_min == y_max && z_max/x_min == y_min);
  } else if (x_max < 0 && y_min >= 0) { 
    z_min = x_min*y_max;
    z_max = x_max*y_min;
    STORE_INT8(x, min, z_min);
    STORE_INT8(x, max, z_max);
    return (z_min/x_min == y_max && z_max/x_max == y_min); // no overflow
  } else if (x_min < 0 && x_max >= 0 && y_max < 0) { 
    z_min = x_max*y_min;
    z_max = x_min*y_min;
    STORE_INT8(x, min, z_min);
    STORE_INT8(x, max, z_max);
    return (z_max > 0 && z_min/y_min == x_max && z_max/y_min == x_min); 
  } else if (x_min < 0 && x_max >= 0 && y_min < 0 && y_max >= 0) { 
    int8 m1, m2;
    m1 = x_min*y_max; 
    m2 = x_max*y_min;
    if (m1/x_min != y_max || m2/y_min != x_max) return false;
    z_min = m1 < m2 ? m1 : m2;
    m1 = x_max*y_max; 
    m2 = x_min*y_min;
    if (m2 <= 0 || (x_max != 0 && m1/x_max != y_max) || m2/y_min != x_min) 
      return false;
    z_max = m1 > m2 ? m1 : m2;
    STORE_INT8(x, min, z_min);
    STORE_INT8(x, max, z_max);
    return true;
  } else if (x_min < 0 && x_max >= 0 && y_min >= 0) { 
    z_min = x_min*y_max;
    z_max = x_max*y_max;
    STORE_INT8(x, min, z_min);
    STORE_INT8(x, max, z_max);
    return z_min/x_min == y_max && (x_max == 0 || z_max/x_max == y_max);
  } else if (x_min >= 0 && y_max < 0) { 
    z_min = x_max*y_min;
    z_max = x_min*y_max;
    STORE_INT8(x, min, z_min);
    STORE_INT8(x, max, z_max);
    return z_min/y_min == x_max && z_max/y_max == x_min;
  } else if (x_min >= 0 && y_min < 0 && y_max >= 0) {
    z_min = x_max*y_min;
    z_max = x_max*y_max;
    STORE_INT8(x, min, z_min);
    STORE_INT8(x, max, z_max);
    return z_min/y_min == x_max && (x_max == 0 || z_max/x_max == y_max);
  } else { 
    assert(x_min >= 0 && y_min >= 0);
    z_min = x_min*y_min;
    z_max = x_max*y_max;
    STORE_INT8(x, min, z_min);
    STORE_INT8(x, max, z_max);
    return x_max == 0 || z_max/x_max == y_max;
  }
}
#endif

#endif
