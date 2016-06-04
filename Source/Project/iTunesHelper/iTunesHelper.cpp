#include "iTunesHelper.h"

#pragma warning(disable:4715)

#if ML_X86
    #define ITUNES_DLL_PATH     L"iTunesDLL"
#else
    #define ITUNES_DLL_PATH L"iTunesDLL64"
#endif

/************************************************************************
  init
************************************************************************/

#define CORE_FP_KEY_HANDLE  ((HANDLE)-0x1000)

API_POINTER(NtOpenKeyEx)        StubNtOpenKeyEx;
API_POINTER(NtQueryValueKey)    StubNtQueryValueKey;

API_POINTER(RegOpenKeyExA)      StubRegOpenKeyExA;
API_POINTER(RegQueryValueExA)   StubRegQueryValueExA;

LSTATUS
NTAPI
ItRegOpenKeyExA(
    HKEY    hKey,
    PCSTR   lpSubKey,
    DWORD   ulOptions,
    REGSAM  samDesired,
    PHKEY   phkResult
)
{
    LSTATUS r;

    r = StubRegOpenKeyExA(hKey, lpSubKey, ulOptions, samDesired, phkResult);
    if (r == NO_ERROR)
        return r;

    if (hKey != HKEY_LOCAL_MACHINE)
        return r;

    if (StrICompareA(lpSubKey, "Software\\Apple Inc.\\CoreFP") != 0)
        return r;

    *phkResult = (HKEY)CORE_FP_KEY_HANDLE;

    return NO_ERROR;
}

LSTATUS
NTAPI
ItRegQueryValueExA(
    HKEY    hKey,
    PCSTR   lpValueName,
    LPDWORD lpReserved,
    LPDWORD lpType,
    LPBYTE  lpData,
    LPDWORD lpcbData
)
{
    LSTATUS status;

    LOOP_ONCE
    {
        if (StrICompareA(lpValueName, "LibraryPath") != 0)
            continue;

        if (hKey != (HKEY)CORE_FP_KEY_HANDLE)
        {
            OBJECT_NAME_INFORMATION2 name;

            status = NtQueryObject(hKey, ObjectNameInformation, &name, sizeof(name), nullptr);
            if (NT_FAILED(status))
                break;

            if (RtlEqualUnicodeString(&name.Name, PUSTR(L"\\REGISTRY\\MACHINE\\SOFTWARE\\Wow6432Node\\Apple Inc.\\CoreFP"), TRUE) == FALSE &&
                RtlEqualUnicodeString(&name.Name, PUSTR(L"\\REGISTRY\\MACHINE\\SOFTWARE\\Apple Inc.\\CoreFP"), TRUE) == FALSE)
            {
                break;
            }
        }

        String DllPath;
        Rtl::GetModuleDirectory(DllPath, nullptr);

        //DllPath += ITUNES_DLL_PATH;
        //DllPath += L"\\CoreFP.dll";
        DllPath = L"CoreFP.dll";

        auto CoreFPPath = DllPath.Encode(CP_ACP);

        if (lpData == nullptr)
        {
            if (lpcbData == nullptr)
                return ERROR_INVALID_PARAMETER;

            *lpcbData = CoreFPPath.GetSize();
            return NO_ERROR;
        }

        if (lpcbData == nullptr)
            return ERROR_INVALID_PARAMETER;

        if (*lpcbData < CoreFPPath.GetSize())
        {
            *lpcbData = CoreFPPath.GetSize();
            return ERROR_MORE_DATA;
        }

        CopyMemory(lpData, CoreFPPath.GetData(), CoreFPPath.GetSize());
        *lpcbData = CoreFPPath.GetSize();

        return NO_ERROR;
    }

    return StubRegQueryValueExA(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
}

iTunesHelper::iTunesHelper()
{
    this->iTunesBase = nullptr;
    this->Initialized = FALSE;

    RtlInitializeCriticalSectionAndSpinCount(&this->DeviceCallbacksLock, 4000);
    RtlInitializeCriticalSectionAndSpinCount(&this->SapLock, 4000);
}

NTSTATUS iTunesHelper::iTunesInitialize()
{
    using namespace Mp;

    NTSTATUS    status;
    PVOID       DllBase;
    PCSTR       DllName;

    status = STATUS_SUCCESS;

    if (this->Initialized)
        return status;

    status = iTunesApi::Initialize();
    DebugLog(L"iTunesApi::Initialize return %p", status);
    FAIL_RETURN(status);

    String ExePath;

    Rtl::GetModuleDirectory(ExePath, &__ImageBase);

    ExePath += ITUNES_DLL_PATH;

#if 0

#if ML_X86
    Rtl::EnvironmentAppend(PUSTR(L"Path"), PUSTR(L"C:\\Program Files (x86)\\iTunes"));
#else
    Rtl::EnvironmentAppend(PUSTR(L"Path"), PUSTR(L"C:\\Program Files\\iTunes"));
#endif

    this->iTunesBase = Ldr::LoadDll(L"iTunesCore.dll");
    LdrDisableThreadCalloutsForDll(this->iTunesBase);

    DebugLog(L"load %s\\iTunesCore.dll at %p", ExePath, this->iTunesBase);

#else

    status = Rtl::EnvironmentAppend(PUSTR(L"Path"), ExePath + L";");

#if ML_X86
    status = Ldr::LoadPeImage(ExePath + L"\\iTunesCore.dll", &this->iTunesBase, nullptr, LOAD_PE_DLL_NOT_FOUND_CONTINUE);
#elif ML_AMD64
    status = LdrLoadDll(nullptr, nullptr, ExePath + L"\\iTunesCore.dll", &this->iTunesBase);
#endif

    DebugLog(L"load %s\\iTunesCore.dll at %p: %p", ExePath, this->iTunesBase, status);
    FAIL_RETURN(status);

#if ML_X86

    ((PIMAGE_TLS_CALLBACK)PtrAdd(this->iTunesBase, ImageNtHeaders(this->iTunesBase)->OptionalHeader.AddressOfEntryPoint))(this->iTunesBase, DLL_PROCESS_ATTACH, nullptr);

    *(PVOID *)&StubRegOpenKeyExA = _InterlockedExchangePointer((PVOID *)LookupImportTable(iTunesBase, "ADVAPI32.dll", KERNEL32_RegOpenKeyExA), ItRegOpenKeyExA);
    *(PVOID *)&StubRegQueryValueExA = _InterlockedExchangePointer((PVOID *)LookupImportTable(iTunesBase, "ADVAPI32.dll", KERNEL32_RegQueryValueExA), ItRegQueryValueExA);

#elif ML_AMD64
    PVOID RegOpenKeyExA, RegQueryValueExA, ptr;

    RegOpenKeyExA = LookupImportTable(iTunesBase, "ADVAPI32.dll", KERNEL32_RegOpenKeyExA);
    RegQueryValueExA = LookupImportTable(iTunesBase, "ADVAPI32.dll", KERNEL32_RegQueryValueExA);

    *(PVOID *)&StubRegOpenKeyExA    = *(PVOID *)RegOpenKeyExA;
    *(PVOID *)&StubRegQueryValueExA = *(PVOID *)RegQueryValueExA;

    ptr = ItRegOpenKeyExA;
    Mm::WriteProtectMemory(CurrentProcess, RegOpenKeyExA, &ptr, sizeof(ptr));

    ptr = ItRegQueryValueExA;
    Mm::WriteProtectMemory(CurrentProcess, RegQueryValueExA, &ptr, sizeof(ptr));

#endif // arch

#endif // load test

    PVOID Address;
    PLDR_MODULE AirTrafficHost = FindLdrModuleByName(PUSTR(L"AirTrafficHost.dll"));

    DebugLog(L"fuck AirTrafficHost: %p", AirTrafficHost);

    if (AirTrafficHost != nullptr)
    {
        using namespace Mp;

        PVOID ATAddAppleSearchPathsToEnvironmentFromReg;

        static BYTE ReturnStub[] =
        {
            // xor eax, eax
            // ret
            0x33, 0xC0, 0xC3,
        };

        ATAddAppleSearchPathsToEnvironmentFromReg = GetRoutineAddress(AirTrafficHost->DllBase, "ATAddAppleSearchPathsToEnvironmentFromReg");
        Mm::WriteProtectMemory(CurrentProcess, ATAddAppleSearchPathsToEnvironmentFromReg, ReturnStub, sizeof(ReturnStub));

        PATCH_MEMORY_DATA p[] =
        {
#if 0

            FunctionJumpVa(
                GetRoutineAddress(AirTrafficHost->DllBase, "ATAddAppleSearchPathsToEnvironmentFromReg"),
                (API_POINTER(NtTestAlert))[]() -> NTSTATUS { return 0; }
            ),

#endif

            MemoryPatchVa((ULONG64)ItRegOpenKeyExA, sizeof(PVOID), LookupImportTable(AirTrafficHost->DllBase, "ADVAPI32.dll", KERNEL32_RegOpenKeyExA)),
            MemoryPatchVa((ULONG64)ItRegQueryValueExA, sizeof(PVOID), LookupImportTable(AirTrafficHost->DllBase, "ADVAPI32.dll", KERNEL32_RegQueryValueExA)),
        };

        PatchMemory(p, countof(p), nullptr);
    }

    this->LoadiTunesRoutines();

    this->Initialized = TRUE;

    return status;
}

/************************************************************************
    device methods
************************************************************************/

NTSTATUS
iTunesHelper::
DeviceNotificationSubscribe(
    DeviceNotificationCallback Callback,
    PVOID Context
)
{
    PROTECT_SECTION(&this->DeviceCallbacksLock)
    {
        this->DeviceCallbacks.Add(CALLBACK_ENTRY{Callback, Context});

        if (this->DeviceCallbacks.GetSize() != 1)
            return STATUS_SUCCESS;
    }

    return this->monitor.NotificationSubscribe(
        [](PDEVICE_CONNECTION_INFO Info, PVOID UserData)
        {
            iTunesHelper* thiz = (iTunesHelper *)UserData;

            PROTECT_SECTION(&thiz->DeviceCallbacksLock)
            {
                for (auto &entry : thiz->DeviceCallbacks)
                {
                    entry.Callback(Info->Device, Info->State, entry.Context);
                }
            }
        },
        this
    );
}

/************************************************************************
    itunes wrapper
************************************************************************/

NTSTATUS iTunesHelper::InitScInfo(PKBSYNC_SESSION *kbsync)
{
    PKBSYNC_SESSION session;

    AllocStack(16); // restore esp for initScInfo

    //return iTunes.initScInfo(FALSE, kbsync == nullptr ? &session : kbsync, 0);
}

NTSTATUS iTunesHelper::LoadiTunesRoutines()
{
    PVOID *func;
    ULONG_PTR *p, rva[] =
    {
/*
        0x000052E0,     // freeSessionData
        0x004B8DA0,     // getDeviceId
        0x004B8C70,     // getDeviceId2

        0x000ABA50,     // kbsyncCreateSession
        0x00090760,     // kbsyncValidate
        0x00093D50,     // kbsyncInitSomething
        0x0001CF80,     // kbsyncGetData
        0x0005F650,     // kbsyncImport
        0x0000A880,     // KbsyncAuthorizeDsid
        0x00072730,     // KbsyncDsidBindMachine
        0x00075D40,     // kbsyncCloseSession
        0x00056240,     // kbsyncAuthorizeDsid2
        0x0003F4C0,     // kbsyncAuthorizeDsid3

        0x000D84B0,     // machineDataStartProvisioning
        0x000D7970,     // machineDataFinishProvisioning
        0x000D8A00,     // machineDataFree
        0x000D7880,     // machineDataClose
        0x000D8A80,     // machineDataGetData

        0x00065FA0,     // sapCreateSession
        0x000321A0,     // sapCloseSession
        0x000A8920,     // sapExchangeData
        0x00032F40,     // sapCreatePrimeSignature
        0x0007D620,     // sapVerifyPrimeSignature
        0x00082FE0,     // sapSignData

        0x00043CB0,     // airFairVerifyRequest
        0x00042AC0,     // airFairSyncCreateSession
        0x00054170,     // airFairSyncSetRequest
        0x0007BC20,     // airFairSyncAddAccount
        0x0000EE80,     // airFairSyncGetAuthorizedAccount
        0x00005350,     // airFairSyncGetResponse
        0x0000ACD0,     // airFairSyncSignData
*/
#if ML_X86
        0xFFFFFFFF,     // freeSessionData
        0xFFFFFFFF,     // getDeviceId
        0xFFFFFFFF,     // getDeviceId2

        0xFFFFFFFF,     // kbsyncCreateSession
        0xFFFFFFFF,     // kbsyncValidate
        0xFFFFFFFF,     // kbsyncInitSomething
        0xFFFFFFFF,     // kbsyncGetData
        0xFFFFFFFF,     // kbsyncImport
        0xFFFFFFFF,     // KbsyncAuthorizeDsid
        0xFFFFFFFF,     // KbsyncDsidBindMachine
        0xFFFFFFFF,     // kbsyncCloseSession
        0xFFFFFFFF,     // kbsyncAuthorizeDsid2
        0xFFFFFFFF,     // kbsyncAuthorizeDsid3

        0xFFFFFFFF,     // machineDataStartProvisioning
        0xFFFFFFFF,     // machineDataFinishProvisioning
        0xFFFFFFFF,     // machineDataFree
        0xFFFFFFFF,     // machineDataClose
        0xFFFFFFFF,     // machineDataGetData

        0xFFFFFFFF,     // sapCreateSession
        0xFFFFFFFF,     // sapCloseSession
        0xFFFFFFFF,     // sapExchangeData
        0xFFFFFFFF,     // sapCreatePrimeSignature
        0x00047600,     // sapVerifyPrimeSignature
        0x000B0EF0,     // sapSignData

        0xFFFFFFFF,     // airFairVerifyRequest
        0xFFFFFFFF,     // airFairSyncCreateSession
        0xFFFFFFFF,     // airFairSyncSetRequest
        0xFFFFFFFF,     // airFairSyncAddAccount
        0xFFFFFFFF,     // airFairSyncGetAuthorizedAccount
        0xFFFFFFFF,     // airFairSyncGetResponse
        0xFFFFFFFF,     // airFairSyncSignData

        0x000E5AD0,     // encryptJsSpToken @ signStorePlatformRequestData
#else
        0x64D00,    // freeSessionData
        ~0u,        // getDeviceId
        ~0u,        // getDeviceId2

        ~0u,        // kbsyncCreateSession
        ~0u,        // kbsyncValidate
        ~0u,        // kbsyncInitSomething
        ~0u,        // kbsyncGetData
        ~0u,        // kbsyncImport
        ~0u,        // KbsyncAuthorizeDsid
        ~0u,        // KbsyncDsidBindMachine
        ~0u,        // kbsyncCloseSession
        ~0u,        // kbsyncAuthorizeDsid2
        ~0u,        // kbsyncAuthorizeDsid3

        ~0u,        // machineDataStartProvisioning
        ~0u,        // machineDataFinishProvisioning
        ~0u,        // machineDataFree
        ~0u,        // machineDataClose
        ~0u,        // machineDataGetData

        0x34800,    // sapCreateSession
        0x32390,    // sapCloseSession
        0x46D10,    // sapExchangeData
        0x95930,    // sapCreatePrimeSignature
        0x4B9D0,    // sapVerifyPrimeSignature  @ X_Apple_ActionSignature
        0x864C0,    // sapSignData

        ~0u,        // airFairVerifyRequest
        ~0u,        // airFairSyncCreateSession
        ~0u,        // airFairSyncSetRequest
        ~0u,        // airFairSyncAddAccount
        ~0u,        // airFairSyncGetAuthorizedAccount
        ~0u,        // airFairSyncGetResponse
        ~0u,        // airFairSyncSignData

        0xD5DD0,    // encryptJsSpToken @ signStorePlatformRequestData
#endif
    };

    func = (PVOID *)&this->iTunes;
    FOR_EACH(p, rva, countof(rva))
    {
        *func++ = PtrAdd(*p == ~0u ? nullptr : this->iTunesBase, *p);
    }

#if ML_X86
    this->iTunes.searchRoutines(this->iTunesBase);
#endif

    return STATUS_SUCCESS;
}

/*++

    itunes wrapper

--*/

NTSTATUS iTunesHelper::FreeSessionData(PVOID data)
{
    return data != nullptr ? this->iTunes.freeSessionData(data) : STATUS_SUCCESS;
}

BOOL iTunesHelper::GetDeviceId(PFAIR_PLAY_HW_INFO deviceId, PFAIR_PLAY_HW_INFO deviceId2)
{
    if (deviceId != nullptr)
        this->iTunes.getDeviceId(deviceId);

    if (deviceId2 != nullptr)
        this->iTunes.getDeviceId2(deviceId2);

    return TRUE;
}

/************************************************************************
  kbsync
************************************************************************/

NTSTATUS iTunesHelper::KbsyncCreateSession(PHANDLE kbsyncSession, PFAIR_PLAY_HW_INFO machineId, PFAIR_PLAY_HW_INFO machineId2, PCSTR ScInfoPath)
{
    *kbsyncSession = nullptr;

    NTSTATUS st;

#if 0
    PKBSYNC_SESSION session;

    st = this->InitScInfo(&session);

    //this->iTunes.kbsyncInitSomething(session->session, 0, 0);
    *kbsyncSession = session->session;

#else

    st = this->iTunes.kbsyncCreateSession(machineId2, machineId, ScInfoPath, kbsyncSession);

    if (st == 0)
    {
        //st = this->iTunes.kbsyncInitSomething(*kbsyncSession, 0, 0);
    }

#endif

    return st;
}

NTSTATUS
iTunesHelper::
KbsyncValidate(
    HANDLE kbsyncSession
)
{
    return this->iTunes.kbsyncValidate(kbsyncSession);
}

NTSTATUS
iTunesHelper::
KbsyncGetData(
    HANDLE      kbsyncSession,
    ULONG64     dsid,
    ULONG       quickTimeVersion,
    ULONG       syncType,
    PVOID*      output,
    PULONG_PTR  outputSize
)
{
    return this->iTunes.kbsyncGetData(kbsyncSession, dsid, quickTimeVersion, syncType, output, outputSize);
}

NTSTATUS
iTunesHelper::
KbsyncSaveDsid(
    HANDLE      kbsyncSession,
    ULONG64     dsid
)
{
    BYTE buf[0x1000];
    this->iTunes.kbsyncAuthorizeDsid2(kbsyncSession, dsid, 0, buf);
    return this->iTunes.kbsyncAuthorizeDsid3(kbsyncSession, dsid, 0);
}

NTSTATUS
iTunesHelper::
KbsyncImport(
    HANDLE      kbsyncSession,
    PVOID       keybag,
    ULONG_PTR   size
)
{
    return this->iTunes.kbsyncImport(kbsyncSession, keybag, size);
}

NTSTATUS iTunesHelper::KbsyncCloseSession(HANDLE session)
{
    if (session == nullptr)
        return STATUS_INVALID_PARAMETER;

    return this->iTunes.kbsyncCloseSession(session);
}

/************************************************************************
  machine data
************************************************************************/

NTSTATUS
iTunesHelper::
MachineDataStartProvisioning(
    ULONG64     dsid,
    PVOID       data,
    ULONG_PTR   dataSize,
    PVOID*      clientData,
    PULONG_PTR  clientDataSize,
    PHANDLE     sessionId
)
{
    return this->iTunes.machineDataStartProvisioning(dsid, data, dataSize, clientData, clientDataSize, sessionId);
}

NTSTATUS
iTunesHelper::
MachineDataFinishProvisioning(
    HANDLE      sessionId,
    PVOID       settingInfo,
    ULONG_PTR   settingInfoSize,
    PVOID       transportKey,
    ULONG_PTR   transportKeySize
)
{
    return this->iTunes.machineDataFinishProvisioning(sessionId, settingInfo, settingInfoSize, transportKey, transportKeySize);
}

NTSTATUS iTunesHelper::MachineDataFree(PVOID data)
{
    return this->iTunes.machineDataFree(data);
}

NTSTATUS iTunesHelper::MachineDataClose(HANDLE sessionId)
{
    return this->iTunes.machineDataClose(sessionId);
}

NTSTATUS
iTunesHelper::
MachineDataGetData(
    ULONG64     dsid,
    PVOID*      data,
    PULONG_PTR  dataSize,
    PVOID*      signature,
    PULONG_PTR  signatureSize
)
{
    return this->iTunes.machineDataGetData(dsid, data, dataSize, signature, signatureSize);
}

/************************************************************************
  sap
************************************************************************/

NTSTATUS iTunesHelper::SapCreateSession(PHANDLE sapSession, PFAIR_PLAY_HW_INFO deviceId)
{
    PROTECT_SECTION(&this->SapLock)
    {
#if 1
        return this->iTunes.sapCreateSession(sapSession, deviceId);
#else
        NTSTATUS Status;
        HANDLE Thread;

        Thread = nullptr;

        Ps::CreateThreadT(
            [&](PVOID) -> ULONG
            {
                PrintConsole(L"new thread\n");
                Status = this->iTunes.sapCreateSession(sapSession, deviceId);
                return 0;
            },
            nullptr,
            FALSE,
            Ps::CurrentProcess,
            &Thread
        );

        if (Thread == nullptr)
        {
            return STATUS_UNSUCCESSFUL;
        }

        NtWaitForSingleObject(Thread, FALSE, nullptr);
        NtClose(Thread);

        return Status;
#endif
    }
}

NTSTATUS iTunesHelper::SapCloseSession(HANDLE sapSession)
{
    PROTECT_SECTION(&this->SapLock)
    {
        return this->iTunes.sapCloseSession(sapSession);
    }
}

NTSTATUS iTunesHelper::SapCreatePrimeSignature(HANDLE sapSession, PVOID* output, PULONG_PTR outputSize)
{
    PROTECT_SECTION(&this->SapLock)
    {
        return this->iTunes.sapCreatePrimeSignature(sapSession, 0x64, 0, 0, output, outputSize);
    }
}

NTSTATUS iTunesHelper::SapVerifyPrimeSignature(HANDLE sapSession, PVOID signature, ULONG_PTR signatureSize)
{
    PROTECT_SECTION(&this->SapLock)
    {
        return this->iTunes.sapVerifyPrimeSignature(sapSession, signature, signatureSize, nullptr, nullptr);
    }
}

NTSTATUS
iTunesHelper::
SapExchangeData(
    ULONG_PTR           certType,
    PFAIR_PLAY_HW_INFO  deviceId,
    HANDLE              sapSession,
    PVOID               certData,
    ULONG_PTR           certSize,
    PVOID*              output,
    PULONG_PTR          outputSize
)
{
    PROTECT_SECTION(&this->SapLock)
    {
        BOOLEAN continueSync = TRUE;
        return this->iTunes.sapExchangeData(certType, deviceId, sapSession, certData, certSize, output, outputSize, &continueSync);
    }
}

NTSTATUS
iTunesHelper::
SapSignData(
    HANDLE      sapSession,
    PVOID       data,
    ULONG_PTR   dataSize,
    PVOID*      signature,
    PULONG_PTR  signatureSize
)
{
    PROTECT_SECTION(&this->SapLock)
    {
        return this->iTunes.sapSignData(sapSession, data, dataSize, signature, signatureSize);
    }
}

/************************************************************************
  afsync
************************************************************************/

NTSTATUS
iTunesHelper::
AirFairVerifyRequest(
    ULONG_PTR   grappaSessionId,
    PVOID       afsyncRequest,
    ULONG_PTR   requestSize,
    PVOID       afsyncRequestSignature,
    ULONG_PTR   signatureSize
)
{
    return this->iTunes.airFairVerifyRequest((ULONG)grappaSessionId, afsyncRequest, requestSize, afsyncRequestSignature, signatureSize);
}

NTSTATUS
iTunesHelper::
AirFairSyncCreateSession(
    PHANDLE                 afsyncSession,
    HANDLE                  kbsyncSession,
    PVOID                   fairPlayCertificate,
    ULONG_PTR               certificateSize,
    PFAIR_PLAY_HW_INFO              fairPlayGuid,
    PAIR_FAIR_DEVICE_INFO   deviceInfo,
    ULONG                   flags
)
{
    return this->iTunes.airFairSyncCreateSession(
                kbsyncSession,
                fairPlayCertificate,
                certificateSize,
                fairPlayGuid,
                deviceInfo,
                flags,
                afsyncSession
            );
}

NTSTATUS
iTunesHelper::
AirFairSyncSetRequest(
    HANDLE      afsyncSession,
    PVOID       afsyncRequest,
    ULONG_PTR   requestSize,
    PVOID       ICInfo,
    ULONG_PTR   ICInfoSize
)
{
    return this->iTunes.airFairSyncSetRequest(afsyncSession, afsyncRequest, requestSize, ICInfo, ICInfoSize);
}

NTSTATUS
iTunesHelper::
AirFairSyncAddAccount(
    HANDLE      kbsyncSession,
    PFAIR_PLAY_HW_INFO  machineId2,
    HANDLE      afsyncSession,
    ULONG64     dsPersonId,
    ULONG       what1,
    ULONG       what2
)
{
    //((NTSTATUS(CDECL*)(...))(PtrAdd(this->iTunesBase, 0x2E9C0)))(kbsyncSession, machineId2, dsPersonId);
    //this->iTunes.KbsyncAuthorizeDsid(kbsyncSession, dsPersonId, 0);
    //this->iTunes.KbsyncDsidBindMachine(afsyncSession, machineId2, dsPersonId);
    return this->iTunes.airFairSyncAddAccount(afsyncSession, dsPersonId, what1, what2);
}

NTSTATUS
iTunesHelper::
AirFairSyncGetAuthorizedAccount(
    HANDLE                      afsyncSession,
    PFAIR_PLAY_HW_INFO          fairPlayGuid,
    ULONG_PTR                   deviceType,
    PVOID                       afsyncRequest,
    ULONG_PTR                   requestSize,
    PAIR_FAIR_AUTHORIZED_DSID*  dsid,
    PULONG_PTR                  count
)
{
    *dsid = nullptr;
    *count = 0;
    return this->iTunes.airFairSyncGetAuthorizedAccount(afsyncSession, fairPlayGuid, (USHORT)deviceType, afsyncRequest, requestSize, (PVOID *)dsid, (PULONG)count);
}

NTSTATUS
iTunesHelper::
AirFairSyncGetResponse(
    HANDLE      afsyncSession,
    PVOID*      afsyncResponse,
    PULONG      responseSize
)
{
    PVOID someData = nullptr;
    ULONG dataSize = 0;

    *afsyncResponse = nullptr;
    *responseSize = 0;
    someData = nullptr;

    NTSTATUS st = this->iTunes.airFairSyncGetResponse(afsyncSession, afsyncResponse, responseSize, &someData, &dataSize);

    if (st == STATUS_SUCCESS)
    {
        FreeSessionData(someData);
    }

    return st;
}

NTSTATUS
iTunesHelper::
AirFairSyncReset(
    HANDLE afsyncSession
)
{
    return this->iTunes.airFairSyncGetResponse(afsyncSession, nullptr, nullptr, nullptr, nullptr);
}

NTSTATUS
iTunesHelper::
AirFairSyncSignData(
    ULONG       grappaSessionId,
    PVOID       data,
    ULONG_PTR   dataLength,
    PVOID*      signature,
    PULONG      signatureLength
)
{
    return this->iTunes.airFairSyncSignData(grappaSessionId, data, dataLength, signature, signatureLength);
}

/************************************************************************
  misc
************************************************************************/
NTSTATUS
iTunesHelper::
EncryptJsSpToken(
    ULONG_PTR   method,
    PVOID       sha1
)
{
    return this->iTunes.encryptJsSpToken(method, sha1);
}
