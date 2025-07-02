#include <cstdlib>
#include <iostream>
#include <string>
#include <chrono>
#include <fstream>
#include <thread>
#include "sha256/sha256.h"
//necesarios para devc++
#include <vector>
#include <sstream>
#include <iomanip>

// NECESARIO PARA COPILAR EN DEVC++ "./sha256/sha256.c -std=c++11 -O3 -L./sha256/sha256.h"
// Tools >> Compiler Options >> Add the following commands when calling the compiler:
using namespace std;
vector<long long> times(10);

string encrypt(const string &text, const int s) {
    string result;
    // traverse text
    for (int i = 0; i < text.length(); i++) {
        // apply transformation to each character
        // Encrypt Uppercase letters
        int letra = text[i];
        if ((letra >= 65)&&(letra <= 90)) {
            // cout<<"mayus";
            result += char((letra + s - 65) % 26 + 65);
        } else if ((letra >= 97)&&(letra <= 122))  {
            // cout<<"minus";
            result += char((letra + s - 97) % 26 + 97);
        }else if ((letra >= 48)&&(letra <= 57)) {
            result += char(int(text[i] + 9 - 2 * (int(text[i]) - 48) - 48) % 10 + 48);;
        }  else {
            result += text[i];
        } 
    }
    // Return the resulting string
    return result;
}


void encriptarArchivo(const string &path, const string &path2, const int offset) {
    ofstream salida(path2);
    ifstream entrada(path);
    string aux;
    // if (!entrada.is_open()) {
    //     cout << "postivo \n";
    // }
    while (getline(entrada, aux)) {
        // Output the text from the file
        salida << encrypt(aux, offset) + "\n";
    }
    entrada.close();
    salida.close();
}

/*string sha256Win(const string &path) {
    FILE *pipe = popen(("Certutil -hashfile " + path + " sha256").c_str(), "r");
    if (!pipe) {
        cerr << "No se pudo abrir el pipe." << endl;
        throw runtime_error("Error al leer el archivo ");
    }
    char buffer[256];
    int a = 2;
    string c;
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        if (3 == a++) {
            c = buffer;
        }
    }
    pclose(pipe);
    return c;
}*/

string sha256Cpp(const string& filename) {
    ifstream file(filename, ios::binary);
    if (!file) return "";

    SHA256_CTX ctx;
    sha256_init(&ctx);

    vector<unsigned char> buffer(4096);
    while (file) {
        file.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
        streamsize bytes = file.gcount();
        if (bytes > 0)
            sha256_update(&ctx, buffer.data(), bytes);
    }

    unsigned char hash[32];
    sha256_final(&ctx, hash);

    std::ostringstream result;
    for (int i = 0; i < 32; ++i) {
        result << hex << setw(2) << setfill('0') << (int)hash[i];
    }
    return result.str();
}

void procesoHilo(const int i, const string &originalPath, const string &hashOriginal) {
    const auto inicio = chrono::high_resolution_clock::now();
    const string encriptado = to_string(i)+".txt";
    encriptarArchivo(originalPath, encriptado, 3);

    const string hashEncr = sha256Cpp(encriptado);
    //9d33fcc7d3de592d985368da616c0f9696f4fbb50779e0f3c733388786720e95

    ofstream sha(to_string(i)+".sha");
    sha<< hashEncr;
    sha.close();
    //desencriptar
    const auto fin = chrono::high_resolution_clock::now();
    const auto duration = chrono::duration_cast<chrono::milliseconds>(fin - inicio);
    times[i] = duration.count();
}

int main() {
    int n = 1;
    cout << "Ingrese el numero de copias a crear:";
    cin >> n;
    times.resize(n);
    try {
        const string originalPath = "original.txt";
        const string shaOrg = sha256Cpp(originalPath);
        vector<thread> threads;
        for (int i = 1; i <= n; i++) {
            threads.emplace_back(procesoHilo, i, originalPath, shaOrg);
        }
        for (auto &t : threads) {
            t.join();
        }
    } catch (const runtime_error &e) {
        cerr << e.what() << endl;
        return 1;
    }

    long long timesAdded = 0;
    for (int i = 1; i <= n; i++) {
        timesAdded = timesAdded + times[i];
        cout << "Tiempo de ejecucion del hilo " << i << ": " << times[i] << " ms" << endl;
    }
    cout << "Tiempo promedio de ejecucion: " << timesAdded/n << " ms" << endl;

    /*string prueba = "ABC XYZ abc xyz 0123456789 ./,- áéíóú ÁÉÍÓÚ";
    string encriptado = encrypt(prueba, 3);
    string desencriptado = encrypt(encriptado, 23);
    cout<< encriptado << "\n" << desencriptado;*/
    return 0;
}
