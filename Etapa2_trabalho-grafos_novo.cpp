#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>

const int INFINITO = 1000000000;

struct Tarefa {
    int id, origem, destino, custo, carga;
    bool precisaAtendimento, ehDirecionada, jaAtendida;

    Tarefa(int ident, int ori, int dest, int peso, int dem,
           bool precisa, bool direcionada)
        : id(ident), origem(ori), destino(dest), custo(peso),
          carga(dem), precisaAtendimento(precisa),
          ehDirecionada(direcionada), jaAtendida(false) {}
};

struct Veiculo {
    std::vector<int> tarefasIds;
    int custoTotal = 0, cargaTotal = 0;
};

void limparEspacos(std::string &texto) {
    for (char &caractere : texto)
        if (caractere == '\t') caractere = ' ';

    std::string ajustado;
    bool espacoAnterior = false;

    for (char c : texto) {
        if (isspace(c)) {
            if (!espacoAnterior) ajustado += ' ';
            espacoAnterior = true;
        } else {
            ajustado += c;
            espacoAnterior = false;
        }
    }
    texto = ajustado;
}

void carregarArquivo(const std::string &arquivo, int &capMaxima, int &nodoInicial,
                     std::vector<Tarefa> &listaTarefas, int &totalVertices) {
    std::ifstream entrada(arquivo);
    if (!entrada) {
        std::cerr << "Erro ao abrir o arquivo\n";
        exit(1);
    }

    std::string linhaLida, secaoAtual;
    int contadorId = 1;

    while (std::getline(entrada, linhaLida)) {
        limparEspacos(linhaLida);
        if (linhaLida.empty()) continue;

        if (linhaLida.rfind("Capacity:", 0) == 0) {
            capMaxima = std::stoi(linhaLida.substr(9));
            continue;
        }
        if (linhaLida.rfind("Depot Node:", 0) == 0) {
            nodoInicial = std::stoi(linhaLida.substr(11));
            continue;
        }

        if (linhaLida.find("ReN.") != std::string::npos) {
            secaoAtual = "ReN"; continue;
        }
        if (linhaLida.find("ReE.") != std::string::npos) {
            secaoAtual = "ReE"; continue;
        }
        if (linhaLida.find("ReA.") != std::string::npos) {
            secaoAtual = "ReA"; continue;
        }
        if (linhaLida.find("EDGE") != std::string::npos) {
            secaoAtual = "EDGE"; continue;
        }
        if (linhaLida.find("ARC") != std::string::npos) {
            secaoAtual = "ARC"; continue;
        }

        std::istringstream dados(linhaLida);
        std::string tag;
        int ori, dest, custo, dem = 0, dummy = 0;

        if (secaoAtual == "ReN") {
            if (!(dados >> tag >> dem >> dummy)) continue;
            if (tag[0] != 'N') continue;
            ori = std::stoi(tag.substr(1));
            listaTarefas.emplace_back(contadorId++, ori, ori, dummy, dem, true, false);
            totalVertices = std::max(totalVertices, ori);
            continue;
        }

        bool direcionado = (secaoAtual == "ReA" || secaoAtual == "ARC");
        bool necessario = (secaoAtual == "ReE" || secaoAtual == "ReA");

        if (necessario)
            dados >> tag >> ori >> dest >> custo >> dem >> dummy;
        else
            dados >> tag >> ori >> dest >> custo;

        totalVertices = std::max(totalVertices, std::max(ori, dest));
        listaTarefas.emplace_back(contadorId++, ori, dest, custo, dem, necessario, direcionado);
    }
}

std::vector<Veiculo> construirRotas(int capacidade, std::vector<Tarefa> &tarefas) {
    std::vector<Veiculo> frota;

    while (true) {
        Veiculo atual;
        bool adicionou = false;

        std::vector<int> pendentes;
        for (int i = 0; i < (int)tarefas.size(); ++i)
            if (!tarefas[i].jaAtendida && tarefas[i].precisaAtendimento)
                pendentes.push_back(i);

        if (pendentes.empty()) break;

        std::sort(pendentes.begin(), pendentes.end(), [&](int a, int b) {
            return tarefas[a].custo < tarefas[b].custo;
        });

        for (int i : pendentes) {
            Tarefa &t = tarefas[i];
            if (t.jaAtendida || atual.cargaTotal + t.carga > capacidade) continue;

            atual.tarefasIds.push_back(t.id);
            atual.cargaTotal += t.carga;
            atual.custoTotal += t.custo;
            t.jaAtendida = true;
            adicionou = true;
        }

        if (!adicionou) break;
        frota.push_back(atual);
    }

    return frota;
}

void salvarResultado(const std::string &saida,
                     const std::vector<Veiculo> &rotas,
                     const std::vector<Tarefa> &tarefas,
                     int vertices) {
    std::ofstream out(saida);
    int somaCusto = 0, somaCarga = 0;

    for (const auto &v : rotas) {
        somaCusto += v.custoTotal;
        somaCarga += v.cargaTotal;
    }

    out << vertices << "\n" << rotas.size() << "\n"
        << somaCusto << "\n" << somaCarga << "\n";

    int id = 1;
    for (const auto &v : rotas) {
        out << " 0 1 " << id++ << " " << v.tarefasIds.size()
            << " " << v.custoTotal << " " << v.cargaTotal << " (D 0,1,1)";
        for (int tid : v.tarefasIds) {
            const Tarefa &t = tarefas[tid - 1];
            out << " (S " << t.id << "," << t.origem << "," << t.destino << ")";
        }
        out << " (D 0,1,1)\n";
    }
}

void mostrarResumo(const std::vector<Veiculo> &rotas,
                   const std::vector<Tarefa> &tarefas, int vertices) {
    int totalCusto = 0, totalCarga = 0;

    for (const auto &r : rotas) {
        totalCusto += r.custoTotal;
        totalCarga += r.cargaTotal;
    }

    std::cout << vertices << "\n" << rotas.size() << "\n"
              << totalCusto << "\n" << totalCarga << "\n";

    int id = 1;
    for (const auto &r : rotas) {
        std::cout << " 0 1 " << id++ << " " << r.tarefasIds.size()
                  << " " << r.custoTotal << " " << r.cargaTotal << " (D 0,1,1)";
        for (int tid : r.tarefasIds) {
            const Tarefa &t = tarefas[tid - 1];
            std::cout << " (S " << t.id << "," << t.origem << "," << t.destino << ")";
        }
        std::cout << " (D 0,1,1)\n";
    }
}

int main() {
    int capacidadeVeiculo = 0, pontoInicial = 0, quantidadeVertices = 0;
    std::vector<Tarefa> tarefasInstancia;

    carregarArquivo("mggdb_0.25_10.dat", capacidadeVeiculo,
                    pontoInicial, tarefasInstancia, quantidadeVertices);

    auto resultado = construirRotas(capacidadeVeiculo, tarefasInstancia);

    salvarResultado("sol-mggdb_0.25_10.dat", resultado,
                    tarefasInstancia, quantidadeVertices);

    mostrarResumo(resultado, tarefasInstancia, quantidadeVertices);
    return 0;
}
