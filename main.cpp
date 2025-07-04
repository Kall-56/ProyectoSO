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
#include <sstream>
#include <iomanip>
using namespace std;

// NECESARIO PARA COPILAR EN DEVC++ "./sha25602/sha256.c -std=c++11 -O3 -L./sha25602/sha256.h"
// Tools >> Compiler Options >> Add the following commands when calling the compiler:

vector<long long> times;
vector<char> original;

string encrypt(const string &text, const int s) {
    string result;
    for (int i = 0; i < text.length(); i++) {
        // apply transformation to each character
        // Encrypt Uppercase letters
        if (int letra = text[i]; (letra >= 65)&&(letra <= 90)) {
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

void encriptarArchivo(const string &path, const string &path2, const int offset) {
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

vector<char> encryptBuffer(const std::vector<char>& buffer, int s) {
    vector<char> result;
    result.reserve(buffer.size());
    for (char c : buffer) {
        int letra = static_cast<unsigned char>(c);
        if (letra >= 65 && letra <= 90) {
            result.push_back((letra + s - 65) % 26 + 65);
        } else if (letra >= 97 && letra <= 122) {
            result.push_back((letra + s - 97) % 26 + 97);
        } else if (letra >= 48 && letra <= 57) {
            result.push_back((static_cast<int>(c + 9 - 2 * (letra - 48) - 48) % 10 + 48));
        } else {
            result.push_back(c);
        }
    }
    return result;
}

void encriptarArchivoBinario(const std::vector<char>& buffer, const std::string& path2, int offset) {
    vector<char> encrypted = encryptBuffer(buffer, offset);
    ofstream salida(path2, ios::binary);
    salida.write(encrypted.data(), encrypted.size());
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

/*string sha256Cpp(const string& filename) {
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
}*/

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
    char hash[65] = {0};
    sha256_finalize(&buff);
    sha256_read_hex(&buff, hash);
    return hash;
}

string sha256CppLocal(vector<char> &buffer, const string& path) {
    {
        ifstream file(path, ios::binary);
        buffer = vector<char>(istreambuf_iterator<char>(file), {});
    }
    SHA256 buff;
    buff.update(buffer.data(), buffer.size());
    return buff.hash();
}

void procesoHilo(const int i, const string &originalPath, const string &hashOriginal) {
    const auto inicio = chrono::high_resolution_clock::now();
    const string encriptado = to_string(i)+".txt";
    //encriptarArchivo(originalPath, encriptado, 3);
    encriptarArchivoBinario(original, encriptado, 3);

    const string hashEncr = sha256CppFile(encriptado);
    // hash: 9d33fcc7d3de592d985368da616c0f9696f4fbb50779e0f3c733388786720e95

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
        const string shaOrg = sha256CppLocal(original,"original.txt");

        vector<thread> threads;
        for (int i = 1; i <= n; i++) {
            threads.emplace_back(procesoHilo, i, "original.txt", shaOrg);
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
