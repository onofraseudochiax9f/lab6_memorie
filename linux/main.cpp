
#include <iostream>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <random>
#include <ctime>

using namespace std;

struct Memorie {
    int numar;
    sem_t semafor;
};

void proces(Memorie* memorie) {
    mt19937 generator(time(nullptr) + getpid());
    uniform_int_distribution<int> moneda(1, 2);

    while (true) {
        sem_wait(&memorie->semafor);

        while (memorie->numar < 1000 && moneda(generator) == 2) {
            memorie->numar++;
            cout << memorie->numar << " ";
            cout.flush();
        }

        bool terminat = memorie->numar >= 1000;

        sem_post(&memorie->semafor);

        if (terminat) {
            break;
        }
    }

    _exit(0);
}

int main() {
    Memorie* memorie = (Memorie*)mmap(
        nullptr,
        sizeof(Memorie),
        PROT_READ | PROT_WRITE,
        MAP_SHARED | MAP_ANONYMOUS,
        -1,
        0
    );

    if (memorie == MAP_FAILED) {
        cerr << "Eroare la crearea memoriei partajate.\n";
        return 1;
    }

    memorie->numar = 0;

    if (sem_init(&memorie->semafor, 1, 1) == -1) {
        cerr << "Eroare la initializarea semaforului.\n";
        munmap(memorie, sizeof(Memorie));
        return 1;
    }

    pid_t procese[2];

    cout << "Numere generate:\n";

    for (int i = 0; i < 2; i++) {
        procese[i] = fork();

        if (procese[i] == -1) {
            cerr << "Eroare la crearea procesului.\n";
            return 1;
        }

        if (procese[i] == 0) {
            proces(memorie);
        }
    }

    for (int i = 0; i < 2; i++) {
        waitpid(procese[i], nullptr, 0);
    }

    cout << "\nNumar final: " << memorie->numar << endl;

    sem_destroy(&memorie->semafor);
    munmap(memorie, sizeof(Memorie));

    return 0;
}
