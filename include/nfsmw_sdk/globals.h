/*
 * nfsmw_sdk/globals.h — every NFSPluginSDK-documented global address.
 *
 * Sourced from docs/sdk_addrs.json (extracted from NFSPluginSDK by berkayylmao,
 * BSD-3). Each entry is a typed pointer constant in the speed.exe address space.
 *
 * Usage:
 *   #include <nfsmw_sdk/globals.h>
 *   bool* p = nfsmw::DrawCars;
 *   *p = false;  // disable car rendering
 *
 * Or via macros:
 *   *(bool*)NFSMW_GLOBAL_DrawCars = false;
 */

#ifndef NFSMW_SDK_GLOBALS_H
#define NFSMW_SDK_GLOBALS_H

#include "platform.h"

/* Full machine-generated address table (NFSMW_GEN_G_* + NFSMW_GEN_F_*).
 * Regenerate with: python3 sdk/tools/codegen.py */
#include "_generated_addrs.h"

/* ---- Tweak / Draw / Skip flags (1-byte bools unless noted) ---- */

#define NFSMW_GLOBAL_AnimationSpeed                  0x904AECu  /* float, default 45.0 */
#define NFSMW_GLOBAL_CarScaleMatrix                  0x9B34B0u  /* Math::Matrix4 */
#define NFSMW_GLOBAL_DrawCars                        0x903320u  /* bool */
#define NFSMW_GLOBAL_DrawCarsReflections             0x903324u  /* bool */
#define NFSMW_GLOBAL_DrawCarShadow                   0x903328u  /* bool */
#define NFSMW_GLOBAL_DrawHUD                         0x57CAA8u  /* bool */
#define NFSMW_GLOBAL_DrawLightFlares                 0x8F2918u  /* bool */
#define NFSMW_GLOBAL_ForceCarLOD                     0x903384u  /* CARPART_LOD enum */
#define NFSMW_GLOBAL_ForceTireLOD                    0x903388u  /* CARPART_LOD enum */
#define NFSMW_GLOBAL_IsFadeScreenOn                  0x91CAE4u  /* bool */
#define NFSMW_GLOBAL_IsInNIS                         0x91606Cu  /* bool — Non-Interactive Sequence (cutscene) */
#define NFSMW_GLOBAL_NOSFOVWidening                  0x91112Cu  /* uint16, default 0x666 (1638) */
#define NFSMW_GLOBAL_PrecullerMode                   0x8FAE44u  /* ePrecullerMode */
#define NFSMW_GLOBAL_SkipFE                          0x926064u  /* bool */
#define NFSMW_GLOBAL_SkipFEDisableCops               0x8F86C0u  /* bool */
#define NFSMW_GLOBAL_SkipFEDisableTraffic            0x926094u  /* bool */
#define NFSMW_GLOBAL_SkipFEPlayerCar                 0x8F86A8u  /* const char* (e.g. "bmwm3gtre46") */
#define NFSMW_GLOBAL_SkipFEPOVType                   0x8F86C4u  /* ePlayerSettingsCameras */
#define NFSMW_GLOBAL_SkipFETrafficDensity            0x926090u  /* float */
#define NFSMW_GLOBAL_StopUpdatingCamera              0x911020u  /* bool */
#define NFSMW_GLOBAL_TheOneCopManager                0x90D5F4u  /* AICopManager* */
#define NFSMW_GLOBAL_TheGameFlowManager              0x925E90u  /* GameFlowState */
#define NFSMW_GLOBAL_Tweak_GameBreakerCollisionMass  0x901AECu  /* float, default 2.0 */
#define NFSMW_GLOBAL_Tweak_GameSpeed                 0x901B1Cu  /* float, default 1.0 */
#define NFSMW_GLOBAL_Tweak_InfiniteNOS               0x937804u  /* bool */
#define NFSMW_GLOBAL_Tweak_InfiniteRaceBreaker       0x988E1Cu  /* bool */
#define NFSMW_GLOBAL_Tweak_PauseCameraLock           0x92584Cu  /* bool */
#define NFSMW_GLOBAL_WindowHasLostFocus              0x982C50u  /* bool */

/* ---- Singletons (pointers to game-master objects) ---- */

#define NFSMW_SINGLETON_CFEManager                   0x91CF90u  /* cFrontEndDatabase / FE manager */
#define NFSMW_SINGLETON_GRaceStatus                  0x91E000u  /* race state object (Lua-visible) */
#define NFSMW_SINGLETON_GameMasterData               0x91CF90u  /* (alias) — game master pointer */
#define NFSMW_SINGLETON_PhysicsWorldCoordinator      0x9885C8u  /* DAT_009885c8 — physics integrator */
#define NFSMW_SINGLETON_SimulationTimeSeconds        0x9885D8u  /* float — sim time accumulator */
#define NFSMW_SINGLETON_NFSMixMaster                 0x911FA8u  /* audio mixer master */
#define NFSMW_SINGLETON_UIRootContext                0x91CADCu  /* UI root (used in PostUIEventToNamedNode) */
#define NFSMW_SINGLETON_EventBus_DAT_0091e0d0        0x91E0D0u  /* event bus for MPursuitBreaker etc. */
#define NFSMW_SINGLETON_FrontEndManager              0x91CF90u  /* FE manager; +0x18 = curr screen */

/* ---- HUD vtables + global addresses (wave-13/14) ---- */

#define NFSMW_VTBL_CHudWidgetArray                   0x008A2538u  /* 12 slots; vt[1] = CHudWidgetArray_Tick */
#define NFSMW_VTBL_PVehicle_AICar                    0x008AC0FCu  /* AI car pvehicle vtable */
#define NFSMW_VTBL_PVehicle_PlayerCar                0x008AC06Cu  /* player pvehicle vtable */
#define NFSMW_VTBL_PVehicle_SubPhysicsObject         0x008AB6A0u  /* shared sub-physics */

/* ---- AI vtables (wave-16) ---- */

#define NFSMW_VTBL_AIVehicleTraffic                  0x00891CF8u  /* 24 slots */
#define NFSMW_VTBL_AIVehicleHelicopter               0x008920D8u  /* 30 slots */
#define NFSMW_VTBL_AIVehiclePursuit                  0x00891EC0u  /* shared parent */
#define NFSMW_VTBL_AIVehicleHuman                    0x00892AD0u  /* used by racer */
#define NFSMW_VTBL_AIGoalRacer                       0x00892720u

/* ---- HUD widget storage offsets within CHudWidgetArray (wave-14) ---- */

#define NFSMW_HUD_SLOT_Speedometer          0x2C0
#define NFSMW_HUD_SLOT_Tachometer           0x2C4
#define NFSMW_HUD_SLOT_DragTachometer       0x2C8
#define NFSMW_HUD_SLOT_ShiftUpdater         0x2CC
#define NFSMW_HUD_SLOT_CostToState          0x2D0
#define NFSMW_HUD_SLOT_Reputation           0x2D4
#define NFSMW_HUD_SLOT_HeatMeterInRace      0x2D8
#define NFSMW_HUD_SLOT_TurboMeter           0x2DC  /* walker-ticked */
#define NFSMW_HUD_SLOT_EngineTempGauge      0x2E0  /* walker-ticked */
#define NFSMW_HUD_SLOT_NitrousGauge         0x2E4
#define NFSMW_HUD_SLOT_SpeedBreakerMeter    0x2E8  /* walker-ticked */
#define NFSMW_HUD_SLOT_RaceOverMessage      0x2EC  /* walker-ticked */
#define NFSMW_HUD_SLOT_GenericMessage       0x2F0  /* walker-ticked */
#define NFSMW_HUD_SLOT_LeaderBoard          0x2FC
#define NFSMW_HUD_SLOT_PursuitBoardInRace   0x300
#define NFSMW_HUD_SLOT_MilestoneBoard       0x304
#define NFSMW_HUD_SLOT_BustedMeter          0x308  /* passive — no-op Update */
#define NFSMW_HUD_SLOT_TimeExtension        0x30C  /* TOLLBOOTH-only */
#define NFSMW_HUD_SLOT_WrongWayIndicator    0x310  /* walker-ticked */
#define NFSMW_HUD_SLOT_AlwaysOn             0x314  /* walker calls unconditionally; empty in retail */
#define NFSMW_HUD_SLOT_Countdown            0x318  /* walker-ticked */
#define NFSMW_HUD_SLOT_RadarDetector        0x31C  /* walker-ticked */
#define NFSMW_HUD_SLOT_GetAwayMeter         0x324  /* stub Update */
#define NFSMW_HUD_SLOT_MenuZoneTrigger      0x328  /* walker-ticked */
#define NFSMW_HUD_SLOT_Infractions          0x32C  /* walker-ticked */

/* ---- C++ accessor wrappers ---- */
#ifdef __cplusplus
namespace nfsmw {

template <typename T>
inline T* global(uintptr_t addr) { return reinterpret_cast<T*>(addr); }

/* Typed shortcuts for the most-used globals */
inline bool*   DrawCars                  = reinterpret_cast<bool*>(NFSMW_GLOBAL_DrawCars);
inline bool*   DrawCarShadow             = reinterpret_cast<bool*>(NFSMW_GLOBAL_DrawCarShadow);
inline bool*   DrawHUD                   = reinterpret_cast<bool*>(NFSMW_GLOBAL_DrawHUD);
inline bool*   IsInNIS                   = reinterpret_cast<bool*>(NFSMW_GLOBAL_IsInNIS);
inline bool*   SkipFE                    = reinterpret_cast<bool*>(NFSMW_GLOBAL_SkipFE);
inline bool*   SkipFEDisableCops         = reinterpret_cast<bool*>(NFSMW_GLOBAL_SkipFEDisableCops);
inline bool*   Tweak_InfiniteNOS         = reinterpret_cast<bool*>(NFSMW_GLOBAL_Tweak_InfiniteNOS);
inline bool*   Tweak_InfiniteRaceBreaker = reinterpret_cast<bool*>(NFSMW_GLOBAL_Tweak_InfiniteRaceBreaker);
inline float*  Tweak_GameSpeed           = reinterpret_cast<float*>(NFSMW_GLOBAL_Tweak_GameSpeed);
inline float*  AnimationSpeed            = reinterpret_cast<float*>(NFSMW_GLOBAL_AnimationSpeed);
inline uint16_t* NOSFOVWidening          = reinterpret_cast<uint16_t*>(NFSMW_GLOBAL_NOSFOVWidening);

} // namespace nfsmw
#endif /* __cplusplus */

#endif /* NFSMW_SDK_GLOBALS_H */
