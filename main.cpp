// Authores: Alex Monsalve CI: 30 407 958
//           Manuel Martinez CI: 30 845 197
// NECESARIO PARA COPILAR EN DEVC++ "./sha25602/sha256.c -std=c++11 -O3 -L./sha25602/sha256.h"
// Tools >> Compiler Options >> Add the following commands when calling the compiler:

#include <cstdlib>
#include <iostream>
#include <string>
#include <chrono>
#include <fstream>
#include <thread>
#include <iterator>
#include <vector>
#include <iomanip>
#include <ctime>
#include "sha25602/sha256.h"
using namespace std;

vector<char> original;
vector<bool> hashValidation;
vector<bool> originalValidation;
vector<long long> times;

void imprimirTiempo(const long long &microsegundos) {
    using namespace std::chrono;
    const long long total_milliseconds = microsegundos / 1000;
    const int hours = static_cast<int>(total_milliseconds / 3600000);
    const int minutes = static_cast<int>((total_milliseconds % 3600000) / 60000);
    const int seconds = static_cast<int>((total_milliseconds % 60000) / 1000);
    const int millis = static_cast<int>(total_milliseconds % 1000);

    cout << setfill('0') << setw(2) << hours << ":"
              << setw(2) << minutes << ":"
              << setw(2) << seconds << "."
              << setw(3) << millis << endl;
}

void imprimirHoraActual() {
    using namespace std::chrono;
    const auto ahora = system_clock::now();
    const auto tiempo = system_clock::to_time_t(ahora);
    const auto ms = duration_cast<milliseconds>(ahora.time_since_epoch()) % 1000;
    const std::tm* local_tm = std::localtime(&tiempo);

    char buffer[16];
    std::strftime(buffer, sizeof(buffer), "%H:%M:%S", local_tm);
    std::cout << buffer << "." << std::setfill('0') << std::setw(3) << ms.count() << std::endl;
}

vector<char> leerArchivo(const string &path) {
    vector<char> vectorBuffer;
    {
        ifstream file(path, ios::binary);
        vectorBuffer = vector<char>(istreambuf_iterator<char>(file), {});
        file.close();
    }
    return vectorBuffer;
}

string encrypt(const string &text, const int s) {
    string result;
    for (int i = 0; i < text.length(); i++) {
        // Encrypt Uppercase letters
        int letra = text[i];
        if ((letra >= 65)&&(letra <= 90)) {
            // cout<<"mayus";
            result += static_cast<char>((letra + s - 65) % 26 + 65);
        } else if ((letra >= 97)&&(letra <= 122))  {
            // cout<<"minus";
            result += static_cast<char>((letra + s - 97) % 26 + 97);
        }else if ((letra >= 48)&&(letra <= 57)) {
            result += static_cast<char>(static_cast<int>(text[i] + 9 - 2 * (static_cast<int>(text[i]) - 48) - 48) % 10 + 48);;
        }  else {
            result += text[i];
        } 
    }
    return result;
}

vector<char> encryptBuffer(const std::vector<char>& buffer, const int s) {
    vector<char> result;
    result.reserve(buffer.size());
    for (char c : buffer) {
        int letra = static_cast<unsigned char>(c);
        if (letra >= 97 && letra <= 122) {
            result.push_back((letra + s - 97) % 26 + 97);
        } else if (letra >= 65 && letra <= 90) {
            result.push_back((letra + s - 65) % 26 + 65);
        } else if (letra >= 48 && letra <= 57) {
            result.push_back((static_cast<int>(c + 9 - 2 * (letra - 48) - 48) % 10 + 48));
        } else {
            result.push_back(c);
        }
    }
    return result;
}

void encriptarArchivoFile(const string &path, const string &path2, const int offset) {
    ofstream salida(path2);
    ifstream entrada(path);
    string aux;
    while (getline(entrada, aux)) {
        // Output the text from the file
        salida << encrypt(aux, offset) + "\n";
    }
    entrada.close();
    salida.close();
}

vector<char> encriptarArchivoBinario(const std::vector<char>& buffer, const std::string& path2, const int offset) {
    vector<char> encrypted = encryptBuffer(buffer, offset);
    ofstream salida(path2, ios::binary);
    salida.write(encrypted.data(), encrypted.size());
    salida.close();
    return encrypted;
}

string sha256CppFile(const string& filename) {
    FILE* file = fopen(filename.c_str(), "rb");
    char buffer[1024];
    sha256_buff buff{};
    sha256_init(&buff);
    while (!feof(file)) {
        // Hash file by 1 kb chunks, instead of loading into RAM at once
        const size_t size = fread(buffer, 1, 1024, file);
        sha256_update(&buff, buffer, size);
    }
    fclose(file);
    char hash[65] = {0};
    sha256_finalize(&buff);
    sha256_read_hex(&buff, hash);
    return hash;
}

string sha256CppLocal(const vector<char> &buffer) {
    SHA256 buff;
    buff.update(buffer.data(), buffer.size());
    return buff.hash();
}

void procesoHilo(const int i, const string &hashOriginal) {
    const auto inicio = chrono::high_resolution_clock::now();

    const string encriptado = to_string(i+1)+".txt";
    const vector<char> encriptadoI = encriptarArchivoBinario(original,encriptado,3);

    const string hashEncr = sha256CppFile(encriptado);
    // hash: 9d33fcc7d3de592d985368da616c0f9696f4fbb50779e0f3c733388786720e95
    ofstream sha(to_string(i+1)+".sha");
    sha<< hashEncr;
    sha.close();
    const string hashVerif = sha256CppLocal(encriptadoI); // sha256CppFile(encriptado); Lee del archivo, Omar querra esto?
    if (hashEncr == hashVerif) {
        hashValidation[i] = true;
    } else {
        hashValidation[i] = false;
    }

    const string desencriptado = to_string(i+1)+"2.txt";
    const vector<char> desencriptadoI = encriptarArchivoBinario(encriptadoI,desencriptado,23);
    const string hashVerif2 = sha256CppLocal(desencriptadoI);
    if (hashOriginal == hashVerif2) {
        originalValidation[i] = true;
    } else {
        originalValidation[i] = false;
    }

    const auto fin = chrono::high_resolution_clock::now();
    const auto duration = chrono::duration_cast<chrono::microseconds>(fin - inicio);
    times[i] = duration.count();
}

long long mainParalelo(const int n) {
    cout << "---|   PROCESO OPTIMIZADO   |---" << endl;
    times.resize(n);
    hashValidation.resize(n);
    originalValidation.resize(n);
    const string shaOrg = sha256CppLocal(original);
    const auto inicio = chrono::high_resolution_clock::now();
    cout << "TI: ";
    imprimirHoraActual();
    try {
        vector<thread> threads;
        for (int i = 0; i < n; i++) {
            threads.emplace_back(procesoHilo,i,shaOrg);
        }
        for (auto &t : threads) {
            t.join();
        }
    } catch (const runtime_error &e) {
        cerr << e.what() << endl;
        return 1;
    }
    const auto fin = chrono::high_resolution_clock::now();
    const auto duration = chrono::duration_cast<chrono::microseconds>(fin - inicio);
    long long timesAdded = 0;
    for (int i = 0; i < n; i++) {
        timesAdded = timesAdded + times[i];
        cout << "Tiempo " << (i+1) << ":    ";
        imprimirTiempo(times[i]);
        cout << "Hash encriptados iguales?: " << boolalpha << hashValidation[i] << endl;
        cout << "Hash desencriptado igual a hash original?: " << boolalpha << originalValidation[i] << endl;
        cout << "----------------------------------------" << endl;
    }
    cout << "TFIN: ";
    imprimirHoraActual();
    cout << "TPPA: ";
    imprimirTiempo(timesAdded/n);
    cout << "TT: ";
    imprimirTiempo(duration.count());
    return duration.count();
}

void procesoHiloSecuencial(const int i, const string &hashOriginal) {
    const auto inicio = chrono::high_resolution_clock::now();

    const string encriptado = to_string(i+1)+".txt";
    encriptarArchivoFile("original.txt",encriptado,3);

    const string hashEncr = sha256CppFile(encriptado);
    // hash: 9d33fcc7d3de592d985368da616c0f9696f4fbb50779e0f3c733388786720e95

    ofstream sha(to_string(i+1)+".sha");
    sha<< hashEncr;
    sha.close();
    const string hashVerif = sha256CppFile(encriptado);
    if (hashEncr == hashVerif) {
        cout << "Hash encriptados iguales?: " << boolalpha << true << endl;
    } else {
        cout << "Hash encriptados iguales?: " << boolalpha << false << endl;
    }

    const string desencriptado = to_string(i+1)+"2.txt";
    encriptarArchivoFile(encriptado,desencriptado,23);
    const string hashVerif2 = sha256CppFile(desencriptado);
    if (hashOriginal == hashVerif2) {
        cout << "Hash desencriptado igual a hash original?: " << boolalpha << true << endl;
    } else {
        cout << "Hash desencriptado igual a hash original?: " << boolalpha << false << endl;
    }

    const auto fin = chrono::high_resolution_clock::now();
    const auto duration = chrono::duration_cast<chrono::microseconds>(fin - inicio);
    cout << "Tiempo " << (i+1) << ":    ";
    imprimirTiempo(duration.count());
    cout << "----------------------------------------" << endl;
    times[i] = duration.count();
}

long long mainSecuencial(const int n) {
    cout << "---|   PROCESO BASE  |---" << endl;
    times.resize(n);
    const string shaOrg = sha256CppLocal(original);
    const auto inicio = chrono::high_resolution_clock::now();
    cout << "TI: ";
    imprimirHoraActual();
    try {
        for (int i = 0; i < n; i++) {
            thread t(procesoHiloSecuencial, i, shaOrg);
            t.join();
        }
    } catch (const runtime_error &e) {
        cerr << e.what() << endl;
        return 1;
    }
    const auto fin = chrono::high_resolution_clock::now();
    const auto duration = chrono::duration_cast<chrono::microseconds>(fin - inicio);
    cout << "TF: ";
    imprimirHoraActual();
    long long timesAdded = 0;
    for (int i = 0; i < n; i++) {
        timesAdded = timesAdded + times[i];
    }
    cout << "TPPA: ";
    imprimirTiempo(timesAdded/n);
    cout << "TT: ";
    imprimirTiempo(duration.count());
    return duration.count();
}

int main() {
    original = leerArchivo("original.txt");
    int n = 0;
    while (n <= 0) {
        cout << " [Ingrese el numero de copias a crear]: ";
        cin >> n;
        if (cin.fail()) {
            cout << "Entrada invalida. Intente de nuevo." << endl;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    }
    cout << "----------------------------------------" << endl;
    const auto inicio = chrono::high_resolution_clock::now();

    const auto TS = mainSecuencial(n);
    const auto TP = mainParalelo(n);

    const auto fin = chrono::high_resolution_clock::now();
    const auto duration = chrono::duration_cast<chrono::microseconds>(fin - inicio);
    cout << "----------------------------------------" << endl;
    cout << "DF: ";
    imprimirTiempo(duration.count());
    const double mejora = (static_cast<double>(TS - TP) / TS) * 100.0;
    cout << "PM: " << mejora << "%" << endl;
    return 0;
}
