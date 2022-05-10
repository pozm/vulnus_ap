//#pragma comment(lib, "MinHook.x64.lib")


#include "pch.h"
#include "il2cpp.h"
#include "cheat.h"
#include <cstdio>
#include <iostream>
#include "./MinHook.h"
#include <format>
#define create_hook(current,mnew,old) { \
    if (MH_CreateHook((void*)current, mnew, reinterpret_cast<LPVOID*>(old)) != MH_OK) \
    { \
        fclose(f); \
        FreeConsole(); \
        FreeLibraryAndExitThread(hmod, 0); \
        return; \
    } \
    if (MH_EnableHook((void*)current) != MH_OK) \
    { \
        fclose(f); \
        FreeConsole(); \
        FreeLibraryAndExitThread(hmod, 0); \
        return; \
    } \
}
typedef void (__stdcall *MapPlayer__Update)(MapPlayer_o* khis, const MethodInfo* method);
typedef void(__stdcall* MapPlayer__OnNoteHit)(MapPlayer_o* kthis, UnityEngine_GameObject_o* note, int32_t baseScore, const MethodInfo* method);
typedef void(__stdcall* MapPlayer__OnNoteMiss)(MapPlayer_o* kthis, UnityEngine_GameObject_o* note, const MethodInfo* method);

MapPlayer__Update oldUpdate;
MapPlayer__Update oldStart;
MapPlayer__OnNoteHit oldNH;
MapPlayer__OnNoteMiss oldNM;

Map_Note_o* Notes[65535];
uint32_t NotesSize = 0;
uint32_t NotesIndex = 0;

void __stdcall MapPlayer__Start__hook(MapPlayer_o* khis, const MethodInfo* method) {
    try {
        auto current_time = khis->klass->static_fields->RealTime;
        auto notes = khis->klass->static_fields->CurrentDiff->fields._notes;
        auto arr = *notes->fields._items;
        NotesSize = notes->fields._size;
        for (int idx = 0x0; idx < NotesSize; idx ++) {
            Notes[idx] = *(arr.m_Items + idx);
        }

        std::cout << "@0x" << (void*)notes->fields._items->m_Items << std::endl;
        std::cout << "Start up" << std::endl;;
    }
    catch (const std::exception& ex) {

    }
    oldStart(khis, method);


}
void __stdcall MapPlayer__OnNoteHit__hook(MapPlayer_o* kthis, UnityEngine_GameObject_o* note, int32_t baseScore, const MethodInfo* method) {
    std::cout << "hit note" << std::endl;
    NotesIndex++;
    oldNH(kthis, note, baseScore, method);
}
void __stdcall MapPlayer__OnNoteMiss__hook(MapPlayer_o* kthis, UnityEngine_GameObject_o* note, const MethodInfo* method) {
    std::cout << "miss note" << std::endl;
    NotesIndex++;
    oldNM(kthis, note, method);
}
void __stdcall MapPlayer__Update__hook(MapPlayer_o* khis, const MethodInfo* method) {
    if (NotesIndex >= NotesSize) {
        oldUpdate(khis, method);
        return;
    }
    try {
        
        std::cout << NotesIndex << "/"<< NotesSize << " real time:" << khis->klass->static_fields->RealTime << " next note: " << Notes[NotesIndex]->fields._time << std::endl;
    }
    catch (const std::exception& ex) {

    }
        oldUpdate(khis,method);
    

}


void hijackGameLoop(HMODULE hmod) {
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    std::cout << "pog\n"; // \x48\x8B\xC4\x48\x89\x48\x08\x53\x56\x57\x41\x56, xxxxxxxxxxxx
    if (MH_Initialize() != MH_OK)
    {
        fclose(f);
        FreeConsole();
        FreeLibraryAndExitThread(hmod, 0);
        return;
    }
    std::cout << "minhook successfully initalized\n";

    uintptr_t gameAssem = (uintptr_t)GetModuleHandle(L"GameAssembly.dll");

    auto mapPlayerUpdateAddr = (MapPlayer__Update)(gameAssem+0x2B7360);
    auto mapPlayerStartAddr = (MapPlayer__Update)(gameAssem+0x2B5B20);
    auto mapPlayerNHAddr = (MapPlayer__OnNoteHit)(gameAssem+0x2B5200);
    auto mapPlayerNMAddr = (MapPlayer__OnNoteHit)(gameAssem+ 2840176);
    std::cout << "@0x" << (void*)mapPlayerUpdateAddr << std::endl;

    create_hook(mapPlayerUpdateAddr, &MapPlayer__Update__hook, &oldUpdate);
    create_hook(mapPlayerStartAddr, &MapPlayer__Start__hook, &oldStart);
    create_hook(mapPlayerNHAddr, &MapPlayer__OnNoteHit__hook, &oldNH);
    create_hook(mapPlayerNMAddr, &MapPlayer__OnNoteMiss__hook, &oldNM);
    //if (MH_CreateHook((void*)mapPlayerUpdateAddr,&MapPlayer__Update__hook, reinterpret_cast<LPVOID*>(&oldUpdate)) != MH_OK)
    //{
    //    fclose(f);
    //    FreeConsole();
    //    FreeLibraryAndExitThread(hmod, 0);
    //    return;
    //}
    //
    //if (MH_EnableHook((void*)mapPlayerUpdateAddr) != MH_OK)
    //{
    //    fclose(f);
    //    FreeConsole();
    //    FreeLibraryAndExitThread(hmod, 0);
    //    return;
    //}

    std::cout << "pogging\n";

}