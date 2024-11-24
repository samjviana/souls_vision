//
// Created by PC-SAMUEL on 22/11/2024.
//

#include "game_handler.h"
#include "memory.h"
#include "logger.h"

namespace souls_vision {

std::string GameHandler::CS_MENU_MAN_IMP_SIGNATURE = "48 8B 0D ?? ?? ?? ?? 48 8B 49 08 E8 ?? ?? ?? ?? 48 8B D0 48 8B CE E8";
std::string GameHandler::WORLD_CHR_MAN_IMP_SIGNATURE = "48 8B 05 ?? ?? ?? ?? 48 85 C0 74 0F 48 39 88";
std::string GameHandler::GET_CHR_INS_FROM_HANDLE_SIGNATURE = "48 83 EC 28 E8 17 FF FF FF 48 85 C0 74 08 48 8B 00 48 83 C4 28 C3";
std::string GameHandler::MODULE_NAME = "eldenring.exe";

uintptr_t GameHandler::csMenuManImp_;
uintptr_t GameHandler::worldChrManImp_;
uintptr_t GameHandler::getChrInsFromHandle_;

void GameHandler::Initialize() {
    LoadPointers();
}

void GameHandler::LoadPointers() {
    csMenuManImp_ = Memory::SignatureScan(CS_MENU_MAN_IMP_SIGNATURE, MODULE_NAME);
    Logger::Info("CSMenuManImp Pointer: 0x" + Memory::ToString(csMenuManImp_));

    worldChrManImp_ = Memory::SignatureScan(WORLD_CHR_MAN_IMP_SIGNATURE, MODULE_NAME);
    Logger::Info("WorldChrManImp Pointer: 0x" + Memory::ToString(worldChrManImp_));

    getChrInsFromHandle_ = Memory::SignatureScan(GET_CHR_INS_FROM_HANDLE_SIGNATURE, MODULE_NAME);
    Logger::Info("GetChrInsFromHandle Address: 0x" + Memory::ToString(getChrInsFromHandle_));

    Logger::Info("Game Pointers loaded");
}

structs::CSMenuManImp* GameHandler::CSMenuManImp() {
    uintptr_t ptr = Memory::DereferenceAddress(csMenuManImp_);
    if (!ptr) {
        return nullptr;
    }

    return reinterpret_cast<structs::CSMenuManImp*>(ptr);
}

structs::WorldChrManImp* GameHandler::WorldChrManImp() {
    uintptr_t ptr = Memory::DereferenceAddress(worldChrManImp_);
    if (!ptr) {
        return nullptr;
    }

    return reinterpret_cast<structs::WorldChrManImp*>(ptr);
}

structs::ChrIns* GameHandler::GetChrInsFromHandle(uint64_t* handle) {
    if (!getChrInsFromHandle_) {
        Logger::Error("getChrInsFromHandle function address is not loaded.");
        return nullptr;
    }

    typedef structs::ChrIns* (*GetChrInsFromHandleFunc)(structs::WorldChrManImp* worldChrMan, uint64_t* chrInsHandlePtr);
    auto getChrInsFromHandleFunc = reinterpret_cast<GetChrInsFromHandleFunc>(getChrInsFromHandle_);

    structs::WorldChrManImp* worldChrMan = WorldChrManImp();
    if (!worldChrMan) {
        Logger::Error("WorldChrManImp instance is null.");
        return nullptr;
    }

    return getChrInsFromHandleFunc(worldChrMan, handle);
}

structs::ChrIns *GameHandler::GetLocalPlayer() {
    structs::WorldChrManImp* worldChrManImp = GameHandler::WorldChrManImp();
    if (!worldChrManImp) {
        return nullptr;
    }

    structs::Entry* playerEntry = worldChrManImp->playerArray[0];
    if (!playerEntry) {
        return nullptr;
    }

    structs::ChrIns* player = playerEntry->chrIns;
    if (!player) {
        return nullptr;
    }

    return player;
}

} // souls_vision