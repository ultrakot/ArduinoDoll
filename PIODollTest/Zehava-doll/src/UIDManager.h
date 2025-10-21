#pragma once
#include <Arduino.h>

class UIDManager {
public:
    enum MedicineType {
        UNKNOWN = -1,
        AKAMOL = 0,
        OINTMENT = 1,
        ANTIBIOTICS = 2,
        EARDROPS = 3,
        EARTEST = 4
    };

    struct UIDDefinition {
        const byte* uid;
        byte size;
        MedicineType type;
        const char* name;
        uint16_t audioTrack;
    };

    // UID definitions
    static const byte UID_AKAMOL[7];
    static const byte UID_OINTMENT[7];
    static const byte UID_ANTIBIOTICS[7];
    static const byte UID_EARDROPS[7];
    static const byte UID_EARTEST[7];
    
    // Disease healing UIDs
    static const byte UID_RUBELLA[7];
    static const byte UID_ALLERGY[7];
    static const byte UID_EARPAIN[7];
    static const byte UID_EARINFECTION[7];
    static const byte UID_FEVER[7];

    // Medicine definitions
    static const UIDDefinition medicines[];
    static const int numMedicines;

    // Utility functions
    static bool compareUID(const byte* a, byte aSize, const byte* b, byte bSize);
    static MedicineType identifyMedicine(const byte* uid, byte size);
    static uint16_t getTrackForMedicine(MedicineType medicine);
    static const char* getMedicineName(MedicineType medicine);
    static bool isEarTestUID(const byte* uid, byte size);
    static bool isDiseaseHealingUID(const byte* uid, byte size, int diseaseIndex);
    
    // Reader location validation
    static bool isMedicineValidForReader(MedicineType medicine, int readerIndex);
    static const char* getReaderLocationName(int readerIndex);
};