```
string sha256Win(const string &path) {
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
}

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
```