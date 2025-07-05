#include <cstdlib>
#include <iostream>
#include <string>
#include <chrono>
#include <fstream>
#include <thread>
#include <iterator>
#include "sha25602/sha256.h"
//NECESARIOS para DevC++
#include <vector>
#include <iomanip>
using namespace std;

// NECESARIO PARA COPILAR EN DEVC++ "./sha25602/sha256.c -std=c++11 -O3 -L./sha25602/sha256.h"
// Tools >> Compiler Options >> Add the following commands when calling the compiler:

vector<char> original;
vector<bool> hashValidation;
vector<bool> originalValidation;
vector<long long> times;

vector<char> leerArchivo(const string &path) {
    vector<char> vectorBuffer;
    {
        ifstream file(path, ios::binary);
        vectorBuffer = vector<char>(istreambuf_iterator<char>(file), {});
        file.close();
    }
    return vectorBuffer;
}

/*string encrypt(const string &text, const int s) {
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
}*/

vector<char> encryptBuffer(const std::vector<char>& buffer, int s) {
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

/*void encriptarArchivoFile(const string &path, const string &path2, const int offset) {
    ofstream salida(path2);
    ifstream entrada(path);
    string aux;
    while (getline(entrada, aux)) {
        // Output the text from the file
        salida << encrypt(aux, offset) + "\n";
    }
    entrada.close();
    salida.close();
}*/

vector<char> encriptarArchivoBinario(const std::vector<char>& buffer, const std::string& path2, int offset) {
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
    string hashVerif = sha256CppLocal(encriptadoI); // sha256CppFile(encriptado); Lee del archivo, no se si Omar solo quiere eso
    if (hashEncr == hashVerif) {
        hashValidation[i] = true;
    } else {
        hashValidation[i] = false;
    }

    const string desencriptado = to_string(i+1)+"2.txt";
    encriptarArchivoBinario(encriptadoI,desencriptado,23); // Aqui tambien podria sacar el vector pero again no se si quiere eso
    const string hashVerif2 = sha256CppFile(desencriptado);
    if (hashOriginal == hashVerif2) {
        originalValidation[i] = true;
    } else {
        originalValidation[i] = false;
    }

    const auto fin = chrono::high_resolution_clock::now();
    const auto duration = chrono::duration_cast<chrono::microseconds>(fin - inicio);
    times[i] = duration.count();
}

int main() {
    int n = 1;
    cout << "Ingrese el numero de copias a crear:";
    cin >> n;
    cout << "----------------------------------------" << endl;
    times.resize(n);
    hashValidation.resize(n);
    originalValidation.resize(n);
    try {
        original = leerArchivo("original.txt");
        const string shaOrg = sha256CppLocal(original);
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
    long long timesAdded = 0;
    for (int i = 0; i < n; i++) {
        timesAdded = timesAdded + times[i];
        cout << "Tiempo de ejecucion del hilo " << (i+1) << ": " << times[i]/1000 << " ms" << endl;
        cout << "Hash encriptados iguales?: " << boolalpha << hashValidation[i] << endl;
        cout << "Hash desencriptado igual a hash original?: " << boolalpha << originalValidation[i] << endl;
        cout << "----------------------------------------" << endl;
    }
    cout << "Tiempo promedio de ejecucion: " << (timesAdded/n)/1000 << " ms" << endl;
    return 0;
}
