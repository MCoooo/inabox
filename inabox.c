#include <windows.h>
#include <stdio.h>
#include <netlistmgr.h>
#include <stdbool.h>
#include <windows.h>
#include <time.h>
#include <winerror.h>
#include <ocidl.h>
#include <olectl.h>

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "uuid.lib")

// #pragma comment(lib, "Ole32.lib")

#define PROGRAM_VERSION "1.0.0"

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"

INetworkListManager* g_pNetworkListManager = NULL;

void resolveConnectivityStates(NLM_CONNECTIVITY connectivity) {

    time_t rawtime;
    struct tm *timeinfo;
    char buffer[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", timeinfo);

    printf("\n");
    printf(GREEN "[%s] Current network state:\n" RESET, buffer);

    // printf(BLUE "Connecivity is %d\n", connectivity);
    if (connectivity & NLM_CONNECTIVITY_DISCONNECTED) {
        printf(RED "No connectivity" RESET);
    }

    if (connectivity & NLM_CONNECTIVITY_DISCONNECTED) {
        printf(YELLOW "  Disconnected\n" RESET);
    }
    if (connectivity & NLM_CONNECTIVITY_IPV4_NOTRAFFIC) {
        printf(YELLOW "  IPv4 No Traffic\n" RESET);
    }
    if (connectivity & NLM_CONNECTIVITY_IPV6_NOTRAFFIC) {
        printf(YELLOW "  IPv6 No Traffic\n" RESET);
    }
    if (connectivity & NLM_CONNECTIVITY_IPV4_SUBNET) {
        printf(YELLOW "  IPv4 Subnet\n" RESET);
    }
    if (connectivity & NLM_CONNECTIVITY_IPV4_LOCALNETWORK) {
        printf(YELLOW "  IPv4 Local Network\n" RESET);
    }
    if (connectivity & NLM_CONNECTIVITY_IPV4_INTERNET) {
        printf(YELLOW "  IPv4 Internet\n" RESET);
    }
    if (connectivity & NLM_CONNECTIVITY_IPV6_SUBNET) {
        printf(YELLOW "  IPv6 Subnet\n" RESET);
    }
    if (connectivity & NLM_CONNECTIVITY_IPV6_LOCALNETWORK) {
        printf(YELLOW "  IPv6 Local Network\n" RESET);
    }
    if (connectivity & NLM_CONNECTIVITY_IPV6_INTERNET) {
        printf(YELLOW "  IPv6 Internet\n" RESET);
    }
    printf("\n");
}

void displayHelp() {
    printf("\n");
    printf("Usage: inabox [options]\n");
    printf("Description : Get NLM connectivity state\n");
    printf("Options:\n");
    printf("  -l, --loop     : Enable indefinite check loop\n");
    printf("  -w, --wait     : Loop wait time in ms (defaults to 500)\n");
    printf("  -v, --version  : Get version information\n");
    printf("\n");
}

void display_version() {
    printf("\n");
    printf(" Welcome to ip v%s\n", PROGRAM_VERSION);
    printf("\n");

}

void get_nlm_connectivity(){

    HRESULT hr = S_OK;
    NLM_CONNECTIVITY connectivity = NLM_CONNECTIVITY_DISCONNECTED;
    hr = g_pNetworkListManager->lpVtbl->GetConnectivity(g_pNetworkListManager, &connectivity);
    if (SUCCEEDED(hr)) {
        resolveConnectivityStates(connectivity);
    } else {
        printf("Error getting Connectivity: %ld\n", hr);
    }

}

void cleanup(INetworkListManager *pNetworkListManager, INetworkConnectionEvents *pNetworkConnectionEvents, IConnectionPointContainer *pConnectionPointContainer, IConnectionPoint *pConnectionPoint) {
    if (pNetworkConnectionEvents != NULL) {
        pNetworkConnectionEvents->lpVtbl->Release(pNetworkConnectionEvents);
    }

    if (pConnectionPoint != NULL) {
        pConnectionPoint->lpVtbl->Release(pConnectionPoint);
    }

    if (pConnectionPointContainer != NULL) {
        pConnectionPointContainer->lpVtbl->Release(pConnectionPointContainer);
    }

    if (pNetworkListManager != NULL) {
        pNetworkListManager->lpVtbl->Release(pNetworkListManager);
    }

    CoUninitialize();
}


// Define a class that implements INetworkConnectionEvents in C
typedef struct CNetworkConnectionEvents {
    INetworkConnectionEventsVtbl *lpVtbl;
    ULONG refCount;
} CNetworkConnectionEvents;

HRESULT STDMETHODCALLTYPE QueryInterface(
    INetworkConnectionEvents *This,
    REFIID riid,
    void **ppvObject){
    if (IsEqualIID(riid, &IID_IUnknown) || IsEqualIID(riid, &IID_INetworkConnectionEvents)) {
        *ppvObject = This;
        This->lpVtbl->AddRef(This);
        return S_OK;
    }
    *ppvObject = NULL;
    return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE AddRef(INetworkConnectionEvents *This){
    CNetworkConnectionEvents *pThis = (CNetworkConnectionEvents *)This;
    return InterlockedIncrement(&pThis->refCount);
}

ULONG STDMETHODCALLTYPE Release(INetworkConnectionEvents *This){
    CNetworkConnectionEvents *pThis = (CNetworkConnectionEvents *)This;
    ULONG ulRefCount = InterlockedDecrement(&pThis->refCount);

    if (ulRefCount == 0) {
        free(pThis);
    }
    return ulRefCount;
}


HRESULT STDMETHODCALLTYPE NetworkConnectionConnectivityChanged(
    INetworkConnectionEvents *This,
    GUID connectionId,
    NLM_CONNECTIVITY newConnectivity){
    wprintf(L"Network connection changed\n");
    get_nlm_connectivity();
    return S_OK;
}

INetworkConnectionEventsVtbl vtbl = {
    QueryInterface,
    AddRef,
    Release,
    NetworkConnectionConnectivityChanged,
};

CNetworkConnectionEvents* CreateNetworkConnectionEventsInstance(){
    CNetworkConnectionEvents *pInstance = (CNetworkConnectionEvents *)malloc(sizeof(CNetworkConnectionEvents));
    if (pInstance) {
        pInstance->lpVtbl = &vtbl;
        pInstance->refCount = 1;
    }
    return pInstance;
}

// Handler for Ctrl+C event
BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType){
    if (dwCtrlType == CTRL_C_EVENT) {
        // Cleanup and exit
        exit(0);
    }
    return FALSE;
}


int main(int argc, char *argv[]) {
    
    HRESULT hr = S_OK;


    // Create an instance of the Network List Manager
    // INetworkListManager* pNetworkListManager = NULL;
    INetworkConnectionEvents *pNetworkConnectionEvents = NULL;
    IConnectionPointContainer *pConnectionPointContainer = NULL;
    IConnectionPoint *pConnectionPoint = NULL;
    DWORD dwCookie = 0;


    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr)) {
        printf(RED "Error creating NLM instance\n" RESET);
        return -1;
    }


    hr = CoCreateInstance(&CLSID_NetworkListManager, NULL, CLSCTX_ALL, &IID_INetworkListManager, (LPVOID*)&g_pNetworkListManager);

       if (FAILED(hr)) {
        printf("CoCreateInstance failed: 0x%08X\n", hr);
        cleanup(g_pNetworkListManager, pNetworkConnectionEvents, pConnectionPointContainer, pConnectionPoint);
        return -1;
    }

      // Get the IConnectionPointContainer interface
    hr = g_pNetworkListManager->lpVtbl->QueryInterface(g_pNetworkListManager, &IID_IConnectionPointContainer, (void**)&pConnectionPointContainer);

    if (FAILED(hr)) {
        printf("QueryInterface for IConnectionPointContainer failed: 0x%08X\n", hr);
        cleanup(g_pNetworkListManager, pNetworkConnectionEvents, pConnectionPointContainer, pConnectionPoint);
        return -1;
    }

    // Find the connection point for INetworkConnectionEvents
    hr = pConnectionPointContainer->lpVtbl->FindConnectionPoint(pConnectionPointContainer, &IID_INetworkConnectionEvents, &pConnectionPoint);

    if (FAILED(hr)) {
        printf("FindConnectionPoint failed: 0x%08X\n", hr);
        cleanup(g_pNetworkListManager, pNetworkConnectionEvents, pConnectionPointContainer, pConnectionPoint);
        return -1;
    }

    // Create an instance of the Network Connection Events object
    pNetworkConnectionEvents = (INetworkConnectionEvents*)CreateNetworkConnectionEventsInstance();

    if (pNetworkConnectionEvents == NULL) {
        hr = E_OUTOFMEMORY;
        printf("CreateNetworkConnectionEventsInstance failed\n");
        cleanup(g_pNetworkListManager, pNetworkConnectionEvents, pConnectionPointContainer, pConnectionPoint);
        return -1;
    }

    // Advise the connection point
    hr = pConnectionPoint->lpVtbl->Advise(pConnectionPoint, (IUnknown*)pNetworkConnectionEvents, &dwCookie);

    if (FAILED(hr)) {
        printf("Advise failed: 0x%08X\n", hr);
        cleanup(g_pNetworkListManager, pNetworkConnectionEvents, pConnectionPointContainer, pConnectionPoint);
        return -1;
    }

    // Set up Ctrl+C handler
    if (!SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE)) {
        printf("SetConsoleCtrlHandler failed\n");
        cleanup(g_pNetworkListManager, pNetworkConnectionEvents, pConnectionPointContainer, pConnectionPoint);
        return -1;
    }

    // Wait for network events (in a real application, you would have a message loop or similar mechanism)
    printf("Listening for network events...\n");

    while (1) {
        Sleep(1000);
    }

    // Unadvise the connection point
    hr = pConnectionPoint->lpVtbl->Unadvise(pConnectionPoint, dwCookie);
    if (FAILED(hr)) {
        printf("Unadvise failed: 0x%08X\n", hr);
    }

    cleanup(g_pNetworkListManager, pNetworkConnectionEvents, pConnectionPointContainer, pConnectionPoint);

    return 0;

}
