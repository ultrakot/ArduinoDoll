
#include <Arduino.h>
#include <unity.h>
#include "NFCManager.h"

NFCManager nfcManager;

void setUp(void) {
    // Optional: runs before each test
}

void tearDown(void) {
    // Optional: runs after each test
}

void test_nfc_readers(void) {
    for (int i = 0; i < 3; ++i) {
        byte uid[10];
        byte length = 0;
        bool found = nfcManager.getCurrentUid(i, uid, &length);
        if (found) {
            printf("Reader %d detected tag UID: ", i);
            for (byte j = 0; j < length; j++) {
                printf("%02X", uid[j]);
            }
            printf("\n");
        } else {
            printf("Reader %d: No tag detected\n", i);
        }
        // Optionally, add assertions here if you know expected UIDs
        // TEST_ASSERT_TRUE(found);
    }
}

void setup() {
    Serial.begin(115200);
    nfcManager.init();
    UNITY_BEGIN();
    RUN_TEST(test_nfc_readers);
    UNITY_END();
}

void loop() {
    // Not used by Unity tests
}