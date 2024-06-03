#include <windows.h>
#include <stdio.h>
#include <netlistmgr.h>
#include <stdbool.h>
#include <windows.h>
#include <time.h>

#pragma comment(lib, "Ole32.lib")

#define PROGRAM_VERSION "1.0.0"

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"

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
    if (connectivity == 0) {
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


int main(int argc, char *argv[]) {
    
    HRESULT hr = S_OK;
    bool loop = false;
    int wait = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--loop") == 0) {
            loop = true;
        }
        if (strcmp(argv[i], "-w") == 0 || strcmp(argv[i], "--wait") == 0) {
            if (argv[i+1] == NULL){
                wait = 500;
            } else {
                wait = atoi(argv[i+1]);
            }
        }
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "/?") == 0 || strcmp(argv[i], "-?") == 0) {
            displayHelp();
            return 0;
        }
        if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
            display_version();
            return 0;
        }
        
    }

    // Initialize COM (if not already done)
    CoInitializeEx(NULL, COINIT_MULTITHREADED);

    // Create an instance of the Network List Manager
    INetworkListManager* pNetworkListManager = NULL;
    hr = CoCreateInstance(&CLSID_NetworkListManager, NULL, CLSCTX_ALL, &IID_INetworkListManager, (LPVOID*)&pNetworkListManager);

    if (SUCCEEDED(hr)) {

        if (!loop) {
            // Get connectivity level
            NLM_CONNECTIVITY connectivity = NLM_CONNECTIVITY_DISCONNECTED;
            hr = pNetworkListManager->lpVtbl->GetConnectivity(pNetworkListManager, &connectivity);
            if (SUCCEEDED(hr)) {
                resolveConnectivityStates(connectivity);
            } else {
                printf("Error getting Connectivity: %ld\n", hr);
            }
        } else {
            while (true) {
                // Get connectivity level
                NLM_CONNECTIVITY connectivity = NLM_CONNECTIVITY_DISCONNECTED;
                hr = pNetworkListManager->lpVtbl->GetConnectivity(pNetworkListManager, &connectivity);
                if (SUCCEEDED(hr)) {
                    resolveConnectivityStates(connectivity);
                } else {
                    printf("Error getting Connectivity: %ld\n", hr);
                }

                Sleep(wait); // Pause for waitTime milliseconds before next iteration
            }
        }
    }
    else {
        printf("Error creating Network List Manager instance: %ld\n", hr);
    }
        // Release the Network List Manager
        pNetworkListManager->lpVtbl->Release(pNetworkListManager);

    // Clean up COM
    CoUninitialize();
    return 0;

}
