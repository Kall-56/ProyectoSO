#include <cstdlib>
#include <iostream>
#include <filesystem>
#include <string>
#include <chrono>
#include <fstream>
#include <thread>

using namespace std;
namespace fs = std::filesystem;

string encrypt(const string &text, const int s) {
    string result;
    // traverse text
    for (int i = 0; i < text.length(); i++) {
        // apply transformation to each character
        // Encrypt Uppercase letters
        if (isupper(text[i])) {
            // cout<<"mayus";
            result += char(int(text[i] + s - 65) % 26 + 65);
        } else if (islower(text[i])) {
            // cout<<"minus";
            result += char(int(text[i] + s - 97) % 26 + 97);
        } else if (ispunct(text[i]) || isspace(text[i])) {
            result += text[i];
        } else {
            result += char(int(text[i] + 9 - 2 * (int(text[i]) - 48) - 48) % 10 + 48);
        }
    }
    // Return the resulting string
    return result;
}

void encriptarArchivo(const filesystem::path &path, const filesystem::path &path2) {
    ofstream salida(path2);
    ifstream entrada(path);
    string aux;
    // if (!entrada.is_open()) {
    //     cout << "postivo \n";
    // }
    while (getline(entrada, aux)) {
        // Output the text from the file
        salida << encrypt(aux, 3) + "\n";
    }
    entrada.close();
    salida.close();
}

string sha256Win(const filesystem::path &path) {
    FILE *pipe = popen(("Certutil -hashfile " + path.generic_string() + " sha256").c_str(), "r"); // _popen en Windows
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
}

void procesoHilo(const int i, const fs::path &originalPath, const string &hashOriginal) {
    const fs::path encriptado = to_string(i)+".txt";
    encriptarArchivo(originalPath, encriptado);
    const string hashEncr = sha256Win(encriptado);
    ofstream sha(to_string(i)+".sha");
    sha<< hashEncr;
    sha.close();
    //desencriptar
}

int main() {
    cout << fs::current_path();
    const auto inicio = chrono::high_resolution_clock::now();
    try {
        const fs::path originalPath = "original.txt";
        const string shaOrg = sha256Win(originalPath);
        for (int i = 1; i <= 5; i++) {
            thread t(procesoHilo, i, originalPath, shaOrg);
            t.join();
        }
    } catch (const runtime_error &e) {
        cerr << e.what() << endl;
        return 1;
    }
    const auto fin = chrono::high_resolution_clock::now();
    const auto duration = chrono::duration_cast<chrono::milliseconds>(fin - inicio);
    cout << "Tiempo de ejecucion: " << duration.count() << " ms" << endl;
    string a;
    cin>>a;
    return 0;
}
