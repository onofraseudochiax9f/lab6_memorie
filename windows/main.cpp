
#include <iostream>
#include <windows.h>
#include <random>
#include <ctime>
#include <string>

using namespace std;

struct Memorie {
    int numar;
};

const char* NUME_MEMORIE = "Local\\MemorieNumere1000";
const char* NUME_SEMAFOR = "Local\\SemaforNumere1000";

int procesCopil() {
    HANDLE hMemorie = OpenFileMappingA(
        FILE_MAP_ALL_ACCESS, FALSE, NUME_MEMORIE
    );

    HANDLE hSemafor = OpenSemaphoreA(
        SEMAPHORE_ALL_ACCESS, FALSE, NUME_SEMAFOR
    );

    if (!hMemorie || !hSemafor) {
        return 1;
    }

    Memorie* memorie = (Memorie*)MapViewOfFile(
        hMemorie,
        FILE_MAP_ALL_ACCESS,
        0,
        0,
        sizeof(Memorie)
    );

    if (!memorie) {
        CloseHandle(hMemorie);
        CloseHandle(hSemafor);
        return 1;
    }

    mt19937 generator(time(nullptr) + GetCurrentProcessId());
    uniform_int_distribution<int> moneda(1, 2);

    while (true) {
        WaitForSingleObject(hSemafor, INFINITE);

        while (memorie->numar < 1000 && moneda(generator) == 2) {
            memorie->numar++;
            cout << memorie->numar << " ";
            cout.flush();
        }

        bool terminat = memorie->numar >= 1000;

        ReleaseSemaphore(hSemafor, 1, NULL);

        if (terminat) {
            break;
        }
    }

    UnmapViewOfFile(memorie);
    CloseHandle(hMemorie);
    CloseHandle(hSemafor);

    return 0;
}

int main(int argc, char* argv[]) {
    if (argc == 2 && string(argv[1]) == "copil") {
        return procesCopil();
    }

    HANDLE hMemorie = CreateFileMappingA(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        sizeof(Memorie),
        NUME_MEMORIE
    );

    HANDLE hSemafor = CreateSemaphoreA(
        NULL,
        1,
        1,
        NUME_SEMAFOR
    );

    if (!hMemorie || !hSemafor) {
        cerr << "Eroare la crearea memoriei sau a semaforului.\n";
        return 1;
    }

    Memorie* memorie = (Memorie*)MapViewOfFile(
        hMemorie,
        FILE_MAP_ALL_ACCESS,
        0,
        0,
        sizeof(Memorie)
    );

    if (!memorie) {
        cerr << "Eroare la maparea memoriei.\n";
        CloseHandle(hMemorie);
        CloseHandle(hSemafor);
        return 1;
    }

    memorie->numar = 0;

    char cale[MAX_PATH];

    if (GetModuleFileNameA(NULL, cale, MAX_PATH) == 0) {
        cerr << "Eroare la obtinerea caii programului.\n";
        return 1;
    }

    PROCESS_INFORMATION procese[2] = {};

    cout << "Numere generate:\n";

    for (int i = 0; i < 2; i++) {
        STARTUPINFOA si = {};
        si.cb = sizeof(si);

        string comanda = "\"" + string(cale) + "\" copil";

        if (!CreateProcessA(
            NULL,
            &comanda[0],
            NULL,
            NULL,
            FALSE,
            0,
            NULL,
            NULL,
            &si,
            &procese[i]
        )) {
            cerr << "Eroare la crearea procesului.\n";
            return 1;
        }
    }

    HANDLE proceseHandle[2] = {
        procese[0].hProcess,
        procese[1].hProcess
    };

    WaitForMultipleObjects(2, proceseHandle, TRUE, INFINITE);

    cout << "\nNumar final: " << memorie->numar << endl;

    for (int i = 0; i < 2; i++) {
        CloseHandle(procese[i].hThread);
        CloseHandle(procese[i].hProcess);
    }

    UnmapViewOfFile(memorie);
    CloseHandle(hMemorie);
    CloseHandle(hSemafor);

    return 0;
}
