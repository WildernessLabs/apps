
#include "../hcom_common.h"

/*******************************************************************
* TESTS OF REGISTER ACCESS VIA THE PROXY
 *******************************************************************/
#if HCOM_INCLUDE_BBR_TESTS_IN_STARTUP > 0

void hcom_bbr_tests()
{
  #define BBR_TEST_VALUE_DB (0xdeadbeef)
  #define BBR_TEST_VALUE_0x00010000 (0x00010000)
  #define BBR_TEST_VALUE_0 (0)

  uint32_t testValue;

  // Verify that 0 won't be broken is handled correctly
  hcom_utils_bbr_write(BBR_TEST_VALUE_0);
  uint32_t returnedValue = hcom_utils_read_bbr_and_right_justify(0xffffffff);
  syslog(1, "Register Tests a: Expected 0x%08x and got 0x%08x\n", BBR_TEST_VALUE_0, returnedValue);

  // Verify that clearing bits is handled correctly
  hcom_utils_bbr_write(0xffffffff);
  hcom_utils_clear_bbr_bits(0x55555555);
  returnedValue = hcom_utils_bbr_read();
  syslog(1, "Register Tests b: Expected 0x%08x and got 0x%08x\n", 0xaaaaaaaa, returnedValue);

  // Verify that a single bit can be read from 0xffffffff 
  hcom_utils_bbr_write(0xffffffff);
  returnedValue = hcom_utils_read_bbr_and_right_justify(0x00000100);
  syslog(1, "Register Tests c: Expected 0x%08x and got 0x%08x\n", 0x00000001, returnedValue);

  // Verify that value not shifted or modified
  hcom_utils_set_bits_in_bbr(0x0000000f);
  returnedValue = hcom_utils_read_bbr_and_right_justify(0x0000000f);
  syslog(1, "Register Tests d: Expected 0x%08x and got 0x%08x\n", 0x0000000f, returnedValue);

  // Verify that value not shifted or modified
  hcom_utils_set_bits_in_bbr(0x0000f000);
  returnedValue = hcom_utils_read_bbr_and_right_justify(0x0000f000);
  syslog(1, "Register Tests e: Expected 0x%08x and got 0x%08x\n", 0x0000000f, returnedValue);

  // Verify that a single bit can be set
  hcom_utils_bbr_write(0x0000000f);
  hcom_utils_set_bits_in_bbr(0x00000100);
  returnedValue = hcom_utils_bbr_read();
  syslog(1, "Register Tests f: Expected 0x%08x and got 0x%08x\n", 0x0000010f, returnedValue);

  hcom_utils_bbr_write(0x0000010f);
  hcom_utils_clear_bits_in_bbr(0x00000100);
  returnedValue = hcom_utils_bbr_read();
  syslog(1, "Register Tests g: Expected 0x%08x and got 0x%08x\n", 0x0000000f, returnedValue);

  // Write and read the battery backed register
  hcom_utils_bbr_write(0);
  hcom_utils_bbr_write(BBR_TEST_VALUE_DB);
  testValue = hcom_utils_bbr_read();
  syslog(1, "Register Tests 1: Expected 0x%08x and got 0x%08x\n", BBR_TEST_VALUE_DB, testValue);

  // Set a bit and read value
  hcom_utils_bbr_write(0);
  hcom_utils_set_bbr_bits(BBR_TEST_VALUE_0x00010000);
  testValue = hcom_utils_bbr_read();
  syslog(1, "Register Tests 2: Expected 0x%08x and got 0x%08x\n", BBR_TEST_VALUE_0x00010000, testValue);

  // Test if expected bit is set
  hcom_utils_bbr_write(BBR_TEST_VALUE_0x00010000);
  bool bitsettest = hcom_utils_is_bbr_bit_set(BBR_TEST_VALUE_0x00010000);
  syslog(1, "Register Tests 3: Expected 1 and got %d\n", bitsettest);

  // Clear bit and insure it's clear
  hcom_utils_bbr_write(BBR_TEST_VALUE_0x00010000);
  hcom_utils_clear_bbr_bits(BBR_TEST_VALUE_0x00010000);
  testValue = hcom_utils_bbr_read();
  syslog(1, "Register Tests 4: Expected 0x%08x and got 0x%08x\n", BBR_TEST_VALUE_0, testValue);

  // Verify we can read and clear the same bit
  hcom_utils_bbr_write(0);
  hcom_utils_set_bbr_bits(BBR_TEST_VALUE_0x00010000);
  bitsettest = hcom_utils_is_bbr_bits_set_n_clear(BBR_TEST_VALUE_0x00010000);
  syslog(1, "Register Tests 5: Expected 1 and got %d\n", bitsettest);

  testValue = hcom_utils_bbr_read();
  syslog(1, "Register Tests 6: Expected 0x%08x and got 0x%08x\n", BBR_TEST_VALUE_0, testValue);

  hcom_utils_bbr_write(0);
}

#endif
