#pragma once

struct HealConfig {
    const char* uuid;
    const char* disease;
    const char* reactionAudio;
};

const HealConfig healConfigs[] = {
    {"UUID_FLU", "Flu", "flu_healed.mp3"},
    {"UUID_RUBELLA", "Rubella", "rubella_healed.mp3"},
    {"UUID_EARACHE", "Earache", "earache_healed.mp3"},
    {"UUID_ALLERGY", "Allergy", "allergy_healed.mp3"},
    {"UUID_EARINFECTION", "EarInfection", "earinfection_healed.mp3"},
};
const int NUM_HEAL_CONFIGS = sizeof(healConfigs) / sizeof(healConfigs[0]);
