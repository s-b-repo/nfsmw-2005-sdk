/*
 * nfsmw_sdk/functions.h — typedefs + raw-address constants for every
 * SDK-documented function in speed.exe.
 *
 * To call a function from a mod:
 *   auto setActive = NFSMW_FN(SetSpeedBreakerActive, void, void*, char);
 *   setActive(controller, 1);
 *
 * Or via raw constant + reinterpret_cast:
 *   ((void(__thiscall*)(void*, char))NFSMW_FN_SetSpeedBreakerActive)(ctrl, 1);
 */

#ifndef NFSMW_SDK_FUNCTIONS_H
#define NFSMW_SDK_FUNCTIONS_H

#include "platform.h"

/* ----- Hash / attribute API ----- */
#define NFSMW_FN_StringToKey                                    0x454640u  /* uint32_t __cdecl (const char*) — bChunk Jenkins mix3 */
#define NFSMW_FN_FindCollection                                 0x455FD0u  /* Collection* __cdecl (uint32_t classKey, uint32_t collKey) */

/* ----- Game state machine ----- */
#define NFSMW_FN_ProcessGameStateMachine                        0x6596A0u  /* __fastcall — function-pointer trampoline */
#define NFSMW_FN_GameFrameTick_MainLoopUpdate                   0x663D30u

/* ----- HUD subsystem (wave-13/14) ----- */
#define NFSMW_FN_CHudWidgetArray_Ctor                           0x5A6600u
#define NFSMW_FN_CHudWidgetArray_Tick                           0x58CA30u  /* vt[1], per-frame walker */
#define NFSMW_FN_CHudWidgetArray_FlipVisibilityOnGameMasterChange 0x5696C0u

/* ----- HUD widget Update functions (verified wave-11/12) ----- */
#define NFSMW_FN_UpdateHud_Speedometer                          0x57A540u
#define NFSMW_FN_UpdateHud_Tachometer                           0x57A6E0u
#define NFSMW_FN_UpdateHud_DragTachometer                       0x57D130u
#define NFSMW_FN_UpdateHud_EngineTempGauge                      0x5685E0u
#define NFSMW_FN_UpdateHud_ShiftUpdater                         0x569780u
#define NFSMW_FN_UpdateHud_Minimap                              0x59DB50u
#define NFSMW_FN_UpdateHud_HeatMeter                            0x7A5AA0u  /* FE-side heat meter */
#define NFSMW_FN_UpdateHud_NitrousGauge                         0x568330u
#define NFSMW_FN_UpdateHud_RaceOverMessage                      0x57A4B0u
#define NFSMW_FN_UpdateHud_GenericMessage                       0x567F80u
#define NFSMW_FN_UpdateHud_TurboMeter                           0x57BB40u
#define NFSMW_FN_UpdateHud_Countdown                            0x5679B0u
#define NFSMW_FN_UpdateHud_TimeExtension                        0x57B780u  /* TOLLBOOTH-mode only */
#define NFSMW_FN_UpdateHud_Reputation                           0x5669B0u
#define NFSMW_FN_UpdateHud_HeatMeterInRace                      0x5666A0u
#define NFSMW_FN_UpdateHud_CostToState                          0x5668F0u
#define NFSMW_FN_UpdateHud_PursuitBoardInRace                   0x57AEE0u
#define NFSMW_FN_UpdateHud_BustedMeter                          0x568EB0u  /* no-op */
#define NFSMW_FN_UpdateHud_MenuZoneTrigger                      0x57BBC0u
#define NFSMW_FN_UpdateHud_Infractions                          0x569A50u
#define NFSMW_FN_UpdateHud_RadarDetector                        0x566170u

/* ----- FNG event bus (wave-16/17) ----- */
#define NFSMW_FN_PostUIEventToNamedNode                         0x516C90u
#define NFSMW_FN_DispatchUIEventToNamedNode                     0x516BE0u
#define NFSMW_FN_ScheduleUIDeferredEvent                        0x5B7780u
#define NFSMW_FN_DrainUIDeferredEventQueue_PerFrame             0x5C1460u  /* THE drainer */
#define NFSMW_FN_FE_PerFrameTick_DrainQueueAndUpdateChildren    0x5C53C0u  /* THE tick caller */
#define NFSMW_FN_DispatchUIEvent_ToSubscriberHandler            0x5BBC00u
#define NFSMW_FN_DispatchUIEvent_ToTypeRegistrySubscribers      0x5BEAA0u
/* NOTE: FindSceneNodeByName / RemoveSceneNodeByName are not yet resolved
 * in the RE database. They are intentionally NOT defined so that any use
 * fails to compile rather than calling address 0. Define them yourself
 * once the addresses are known. */

/* ----- Audio event bus (DAT_0091e0d0) ----- */
#define NFSMW_FN_BusBroadcastEventByHash                        0x61FC10u
#define NFSMW_FN_BusSubscribeListenerByHash                     0x61FD00u
#define NFSMW_FN_BusDispatchEventToListenerFilters              0x5F9DA0u
#define NFSMW_FN_BusFindOrCreateBucketByHash                    0x61FB40u
#define NFSMW_FN_ConstructEventBus                              0x61F8A0u
#define NFSMW_FN_CreateGlobalEventBus_DAT0091e0d0               0x61F940u

/* ----- SpeedBreaker / GameBreaker (wave-11) ----- */
#define NFSMW_FN_HandleSpeedBreakerToggleRequest                0x6EDD10u
#define NFSMW_FN_SetSpeedBreakerActive                          0x6E9AA0u
#define NFSMW_FN_UpdateSpeedBreakerEnergyTick                   0x6EDD60u
#define NFSMW_FN_CheckSpeedBreakerAvailable                     0x6EDC90u
#define NFSMW_FN_WorldTickScaleSimulationDt                     0x6F6CF0u
#define NFSMW_FN_DispatchWorldSimulationTick                    0x6F6EC0u

/* ----- PursuitBreaker (environmental) ----- */
#define NFSMW_FN_GetMPursuitBreakerEventHash_Cached             0x4B5880u
#define NFSMW_FN_GetMBreakerStopCopsEventHash_Cached            0x405B70u
#define NFSMW_FN_ConstructPursuitBreakerSoundFader              0x4F72A0u
#define NFSMW_FN_HandlePursuitBreakerSoundEvent                 0x4F0FB0u
#define NFSMW_FN_DispatchStartBreakerToScript                   0x626510u
#define NFSMW_FN_RegisterMPursuitBreakerEventListener           0x6332A0u

/* ----- Cop AI ----- */
#define NFSMW_FN_PushAIGoalByHash                               0x422480u
#define NFSMW_FN_SetAICopPursuitGoal                            0x42AB80u
#define NFSMW_FN_CreateAIActionRace                             0x43B800u
#define NFSMW_FN_CreateAIActionRam                              0x43B9B0u
#define NFSMW_FN_UpdateAIActionRam                              0x43BB28u
#define NFSMW_FN_CreateAIActionStaticRoadBlock                  0x43BE50u
#define NFSMW_FN_CreateAIGoalPit                                0x43FE90u
#define NFSMW_FN_SetAIHeliExitGoal                              0x423510u
#define NFSMW_FN_AwardPlayerBounty_Impl                         0x612220u
#define NFSMW_FN_RegisterPursuitBountyEventListeners            0x648590u

/* ----- AI vehicle vtables - Update methods (wave-16) ----- */
#define NFSMW_FN_UpdateAIVehicleTraffic_OnDriving               0x42AB40u  /* heli vt[10] */
#define NFSMW_FN_UpdateAIVehicleHelicopter_OnDriving            0x42ADB0u  /* heli vt[10] */
#define NFSMW_FN_FilterHeliAltitudeVector                       0x417A20u  /* heli vt[17] */
#define NFSMW_FN_DestructAIVehicleTraffic                       0x433680u
#define NFSMW_FN_DestructAIVehicleHelicopter                    0x4336E0u
#define NFSMW_FN_AIVehicle_BaseUpdate_vt1                       0x406600u

/* ----- AI traffic ----- */
#define NFSMW_FN_SetAITrafficGoal                               0x423190u
#define NFSMW_FN_DestroyAITrafficVehicleEntry                   0x4352F0u
#define NFSMW_FN_SetTrafficSignVtable                           0x50DF00u
#define NFSMW_FN_ReleaseAITrafficSlot                           0x7831E0u
#define NFSMW_FN_DestroyTrafficSpawnSlotEntry                   0x78FBC0u
#define NFSMW_FN_GetTrafficSpawnSystemSingleton                 0x799F50u
#define NFSMW_FN_DestroyTrafficAIChainList                      0x7B3F00u

/* ----- AI racer ----- */
#define NFSMW_FN_ComputeRacerRubberBandTargetSpeed              0x5FF990u
#define NFSMW_FN_GetRacerDifficultyBucketFromAttribs            0x5FAC10u

/* ----- Camera ----- */
#define NFSMW_FN_UpdateWorldCamerasAndViewport                  0x72AA70u
#define NFSMW_FN_ShakeCamera                                    0x62B110u

/* ----- Save/Load (MD5-validated) ----- */
#define NFSMW_FN_WriteProfileToFile                             0x5188F0u
#define NFSMW_FN_BuildProfileSaveBuffer                         0x58FD10u
#define NFSMW_FN_LoadProfileFromFile                            0x5268B0u
#define NFSMW_FN_ComputeMd5OfSaveBuffer                         0x57F920u
#define NFSMW_FN_CheckShouldAutosaveNow                         0x5184D0u
#define NFSMW_FN_EnsureProfileDirectoryExists                   0x6CBF00u

/* ----- Movies / Cutscenes (On2 VP6 + MAD) ----- */
#define NFSMW_FN_ProcessMovieOpenAndBegin                       0x542A40u
#define NFSMW_FN_ProcessMoviePlayerPerFrameTick                 0x525F70u
#define NFSMW_FN_MoviePlayer_GetOrCreateSingleton               0x525DF0u

/* ----- Lua VM ----- */
#define NFSMW_FN_ExecuteLuaVMOpcodes                            0x60E9D0u  /* luaV_execute */
#define NFSMW_FN_CallLuaScriptFunctionD                         0x6126A0u
#define NFSMW_FN_PrecallLuaFunctionFrame                        0x60E7D0u
#define NFSMW_FN_ResumeLuaCoroutineState                        0x6150E0u
#define NFSMW_FN_SuspendLuaCoroutineYield                       0x6138B0u
#define NFSMW_FN_RegisterScriptNativesGameplay                  0x61E750u

/* ----- Input bindings (wave-8) ----- */
#define NFSMW_FN_PollAllGameActionBindingsPerFrame              0x6349B0u
#define NFSMW_FN_ReadGameActionBindingAxisFromDevice            0x628940u
#define NFSMW_FN_CopyGameActionBindingTemplateToRuntime         0x628410u
#define NFSMW_FN_InstallGameActionBindingIntoSlot               0x6285F0u

/* ----- Race rules ----- */
#define NFSMW_FN_ProcessStartRace                               0x60DBD0u
#define NFSMW_FN_ProcessNotifyRacePlacement                     0x60AA00u
#define NFSMW_FN_GetEventRaceModeId                             0x5FAA20u

/* ----- C++ helper to make callable function pointers ----- */
#ifdef __cplusplus
namespace nfsmw {
template <typename Sig>
inline Sig as_fn(uintptr_t addr) { return reinterpret_cast<Sig>(addr); }
}  /* namespace nfsmw */

/* Example:
 *   auto setBreaker = nfsmw::as_fn<void(NFSMW_THISCALL*)(void*, char)>(NFSMW_FN_SetSpeedBreakerActive);
 *   setBreaker(my_breaker_obj, 1);
 */
#endif

#endif /* NFSMW_SDK_FUNCTIONS_H */
