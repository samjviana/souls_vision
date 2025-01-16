//
// Created by PC-SAMUEL on 22/11/2024.
//

#include "game_handler.h"
#include "memory.h"
#include "logger.h"

namespace souls_vision {

std::string GameHandler::CS_MENU_MAN_IMP_SIGNATURE = "48 8B 0D ?? ?? ?? ?? 48 8B 49 08 E8 ?? ?? ?? ?? 48 8B D0 48 8B CE E8";
std::string GameHandler::SOLO_PARAM_REPOSITORY_IMP_SIGNATURE = "48 8B 0D ?? ?? ?? ?? 48 85 C9 0F 84 ?? ?? ?? ?? 45 33 C0 BA 8E 00 00 00";
std::string GameHandler::WORLD_CHR_MAN_IMP_SIGNATURE = "48 8B 05 ?? ?? ?? ?? 48 85 C0 74 0F 48 39 88";
std::string GameHandler::GET_CHR_INS_FROM_HANDLE_SIGNATURE = "48 83 EC 28 E8 17 FF FF FF 48 85 C0 74 08 48 8B 00 48 83 C4 28 C3";
std::string GameHandler::GET_PARAM_RES_CAP_SIGNATURE = "81 FA C2 00 00 00 7D 24 48 63 D2 48 8D 04 D2 44 3B 84 C1 80 00 00 00 73 13 41 8B C0 48 8D 14 D2 48 03 D0 48 8B 84 D1 88";
std::string GameHandler::MODULE_NAME = "eldenring.exe";

int GameHandler::NPC_PARAM_INDEX = 6;

uintptr_t GameHandler::csMenuManImp_;
uintptr_t GameHandler::soloParamRepositoryImp_;
uintptr_t GameHandler::worldChrManImp_;
uintptr_t GameHandler::getChrInsFromHandle_;
uintptr_t GameHandler::getParamResCap_;

void GameHandler::Initialize() { // GetParamResCap // GetChrInsFromHandle
    LoadPointers();
}

void GameHandler::LoadPointers() {
    csMenuManImp_ = Memory::SignatureScan(CS_MENU_MAN_IMP_SIGNATURE, MODULE_NAME);
    Logger::Info("CSMenuManImp Pointer: 0x" + Memory::ToString(csMenuManImp_));

    soloParamRepositoryImp_ = Memory::SignatureScan(SOLO_PARAM_REPOSITORY_IMP_SIGNATURE, MODULE_NAME);
    Logger::Info("CSSoloParamRepositoryImp Pointer: 0x" + Memory::ToString(soloParamRepositoryImp_));

    worldChrManImp_ = Memory::SignatureScan(WORLD_CHR_MAN_IMP_SIGNATURE, MODULE_NAME);
    Logger::Info("WorldChrManImp Pointer: 0x" + Memory::ToString(worldChrManImp_));

    getChrInsFromHandle_ = Memory::SignatureScan(GET_CHR_INS_FROM_HANDLE_SIGNATURE, MODULE_NAME);
    Logger::Info("GetChrInsFromHandle Address: 0x" + Memory::ToString(getChrInsFromHandle_));

    getParamResCap_ = Memory::SignatureScan(GET_PARAM_RES_CAP_SIGNATURE, MODULE_NAME);
    Logger::Info("GetParamResCap Address: 0x" + Memory::ToString(getParamResCap_));

    Logger::Info("Game Pointers loaded");
}

structs::CSMenuManImp* GameHandler::CSMenuManImp() {
    if (!csMenuManImp_) {
        return nullptr;
    }
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

structs::SoloParamRepositoryImp* GameHandler::SoloParamRepositoryImp() {
    uintptr_t ptr = Memory::DereferenceAddress(soloParamRepositoryImp_);
    if (!ptr) {
        return nullptr;
    }

    return reinterpret_cast<structs::SoloParamRepositoryImp*>(ptr);
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

// It seems that paramIndex needs to be 0 for the function to work
structs::ParamResCap *GameHandler::GetParamResCap(uint32_t paramType, uint32_t paramIndex) {
    if (!getParamResCap_) {
        Logger::Error("getParamResCap function address is not loaded.");
        return nullptr;
    }

    typedef structs::ParamResCap* (*GetParamResCapFunc)(structs::SoloParamRepositoryImp* soloParamRepositoryImp, uint32_t paramType, uint32_t paramIndex);
    auto getParamResCapFunc = reinterpret_cast<GetParamResCapFunc>(getParamResCap_);

    structs::SoloParamRepositoryImp* soloParamRepositoryImp = SoloParamRepositoryImp();
    if (!soloParamRepositoryImp) {
        Logger::Error("SoloParamRepositoryImp instance is null.");
        return nullptr;
    }

    return getParamResCapFunc(soloParamRepositoryImp, paramType, paramIndex);
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

structs::ParamRow *GameHandler::GetParamRow(uint32_t paramType, uint32_t paramId) {
    structs::ParamResCap* paramResCap = GetParamResCap(paramType);
    if (!paramResCap) {
        return nullptr;
    }

    if (!paramResCap->paramData->paramHeader) {
        return nullptr;
    }

    structs::ParamRow* paramRows = &paramResCap->paramData->paramHeader->paramRows;
    for (int i = 0; i < paramResCap->paramData->paramHeader->rowCount; i++) {
        structs::ParamRow& row = paramRows[i];
        if (row.rowId == paramId) {
            return &row;
        }
    }

    return nullptr;
}

structs::NpcParam *GameHandler::GetNpcParam(uint32_t paramId) {
    structs::ParamResCap* paramResCap = GetParamResCap(NPC_PARAM_INDEX);
    if (!paramResCap) {
        return nullptr;
    }

    structs::ParamHeader* paramHeader = paramResCap->paramData->paramHeader;
    if (!paramHeader) {
        return nullptr;
    }

    structs::ParamRow* paramRows = &paramHeader->paramRows;
    structs::ParamRow* paramRow = nullptr;
    for (int i = 0; i < paramHeader->rowCount; i++) {
        structs::ParamRow& row = paramRows[i];
        if (row.rowId == paramId) {
            paramRow = &row;
            break;
        }
    }
    if (!paramRow) {
        return nullptr;
    }

    uintptr_t npcParamAddress = reinterpret_cast<uintptr_t>(paramHeader) + paramRow->paramOffset;
    return reinterpret_cast<structs::NpcParam*>(npcParamAddress);
}

} // souls_vision