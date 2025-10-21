#include "UIDManager.h"

// UID definitions
const byte UIDManager::UID_AKAMOL[7] = {0x04, 0xA1, 0x35, 0xB2, 0x35, 0x5E, 0x80};;
const byte UIDManager::UID_OINTMENT[7] = {0x04, 0xA2, 0x34, 0xB2, 0x35, 0x5E, 0x80};
const byte UIDManager::UID_ANTIBIOTICS[7] = {0x04, 0xBF, 0x36, 0xB2, 0x35, 0x5E, 0x80};
const byte UIDManager::UID_EARDROPS[7] = {0x04, 0x49, 0xBB, 0x32, 0x59, 0x13, 0x90};
const byte UIDManager::UID_EARTEST[7] = {0x04, 0x9E, 0x34, 0xB2, 0x35, 0x5E, 0x80};

// Disease healing UIDs
const byte UIDManager::UID_RUBELLA[7] = {0x04, 0xBF, 0x36, 0xB2, 0x35, 0x5E, 0x80}; // antibiotics
const byte UIDManager::UID_ALLERGY[7] = {0x04, 0xA2, 0x34, 0xB2, 0x35, 0x5E, 0x80}; // ointment
const byte UIDManager::UID_EARPAIN[7] = {0x04, 0x49, 0xBB, 0x32, 0x59, 0x13, 0x90}; // eardrops
const byte UIDManager::UID_EARINFECTION[7] = {0x04, 0x49, 0xBB, 0x32, 0x59, 0x13, 0x90}; // eardrops
const byte UIDManager::UID_FEVER[7] = {0x04, 0xA1, 0x35, 0xB2, 0x35, 0x5E, 0x80};; // akamol

// Medicine definitions with their audio tracks
const UIDManager::UIDDefinition UIDManager::medicines[] = {
    {UID_ANTIBIOTICS, 7, ANTIBIOTICS, "Antibiotics", 5},
    {UID_EARDROPS, 7, EARDROPS, "Ear Drops", 2},
    {UID_OINTMENT, 7, OINTMENT, "Ointment", 3},
    {UID_AKAMOL, 7, AKAMOL, "Akamol", 4}
};

const int UIDManager::numMedicines = sizeof(medicines) / sizeof(medicines[0]);

bool UIDManager::compareUID(const byte* a, byte aSize, const byte* b, byte bSize) {
    if (aSize != bSize) return false;
    for (byte i = 0; i < aSize; i++) {
        if (a[i] != b[i]) return false;
    }
    return true;
}

UIDManager::MedicineType UIDManager::identifyMedicine(const byte* uid, byte size) {
    for (int i = 0; i < numMedicines; i++) {
        if (compareUID(uid, size, medicines[i].uid, medicines[i].size)) {
            return medicines[i].type;
        }
    }
    return UNKNOWN;
}

uint16_t UIDManager::getTrackForMedicine(MedicineType medicine) {
    for (int i = 0; i < numMedicines; i++) {
        if (medicines[i].type == medicine) {
            return medicines[i].audioTrack;
        }
    }
    return 1; // Default to track 1 (crying)
}

const char* UIDManager::getMedicineName(MedicineType medicine) {
    for (int i = 0; i < numMedicines; i++) {
        if (medicines[i].type == medicine) {
            return medicines[i].name;
        }
    }
    return "Unknown";
}

bool UIDManager::isEarTestUID(const byte* uid, byte size) {
    return compareUID(uid, size, UID_EARTEST, 7);
}

bool UIDManager::isDiseaseHealingUID(const byte* uid, byte size, int diseaseIndex) {
    const byte* diseaseUIDs[] = {UID_RUBELLA, UID_ALLERGY, UID_EARPAIN, UID_EARINFECTION, UID_FEVER};
    const byte diseaseSizes[] = {7, 7, 7, 7, 4};
    
    if (diseaseIndex < 0 || diseaseIndex >= 5) return false;
    
    return compareUID(uid, size, diseaseUIDs[diseaseIndex], diseaseSizes[diseaseIndex]);
}

bool UIDManager::isMedicineValidForReader(MedicineType medicine, int readerIndex) {
    // Reader mapping (readerIndex is 0-based from poll, but represents readers 1-5):
    // Reader 0 (Reader 1) = Body
    // Reader 1 (Reader 2) = Right Ear
    // Reader 2 (Reader 3) = Mouth
    // Reader 3 (Reader 4) = Left Ear
    // Reader 4 (Reader 5) = Body
    
    switch (medicine) {
        case AKAMOL:
        case ANTIBIOTICS:
            // Must be placed in mouth (Reader 3)
            return (readerIndex == 2);
            
        case EARTEST:
        case EARDROPS:
            // Must be placed on ears (Readers 2 or 4)
            return (readerIndex == 1 || readerIndex == 3);
            
        case OINTMENT:
            // Must be applied to body (Readers 1 or 5)
            return (readerIndex == 0 || readerIndex == 4);
            
        default:
            // Unknown medicine - allow on any reader (backward compatible)
            return true;
    }
}

const char* UIDManager::getReaderLocationName(int readerIndex) {
    // Reader location names for user feedback
    switch (readerIndex) {
        case 0: return "Body (Reader 1)";
        case 1: return "Right Ear (Reader 2)";
        case 2: return "Mouth (Reader 3)";
        case 3: return "Left Ear (Reader 4)";
        case 4: return "Body (Reader 5)";
        default: return "Unknown Reader";
    }
}