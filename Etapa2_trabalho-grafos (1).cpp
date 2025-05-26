#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>

const int INF = 1000000000;

struct Servico
{
    int id, origem, destino, custo, demanda;
    bool requerido, orientado, atendido;
    Servico(int i, int o, int d, int c, int dem, bool req, bool ori)
        : id(i), origem(o), destino(d), custo(c), demanda(dem), requerido(req), orientado(ori), atendido(false) {}
};

struct Rota
{
    std::vector<int> ids;
    int custo = 0, demanda = 0;
};

void limpar(std::string &linha)
{
    for (char &c : linha)
        if (c == '\t')
            c = ' ';
    std::string out;
    bool espaco = false;
    for (char c : linha)
    {
        if (isspace(c))
        {
            if (!espaco)
                out += ' ';
            espaco = true;
        }
        else
        {
            out += c;
            espaco = false;
        }
    }
    linha = out;
}

void lerInstancia(const std::string &nome, int &capacidade, int &deposito,
                  std::vector<Servico> &servicos, int &numVertices)
{
    std::ifstream in(nome);
    if (!in)
    {
        std::cerr << "Erro ao abrir arquivo\n";
        exit(1);
    }

    std::string linha, secao;
    int idS = 1;
    while (std::getline(in, linha))
    {
        limpar(linha);
        if (linha.empty())
            continue;

        if (linha.rfind("Capacity:", 0) == 0)
        {
            capacidade = std::stoi(linha.substr(9));
            continue;
        }
        if (linha.rfind("Depot Node:", 0) == 0)
        {
            deposito = std::stoi(linha.substr(11));
            continue;
        }

        if (linha.find("ReN.") != std::string::npos)
        {
            secao = "ReN";
            continue;
        }
        if (linha.find("ReE.") != std::string::npos)
        {
            secao = "ReE";
            continue;
        }
        if (linha.find("ReA.") != std::string::npos)
        {
            secao = "ReA";
            continue;
        }
        if (linha.find("EDGE") != std::string::npos)
        {
            secao = "EDGE";
            continue;
        }
        if (linha.find("ARC") != std::string::npos)
        {
            secao = "ARC";
            continue;
        }

        std::istringstream iss(linha);
        std::string tag;
        int o, d, c, dem = 0, sc = 0;

        if (secao == "ReN")
        {
            if (!(iss >> tag >> dem >> sc))
                continue;
            if (tag[0] != 'N')
                continue;
            o = std::stoi(tag.substr(1));
            servicos.emplace_back(idS++, o, o, sc, dem, true, false);
            numVertices = std::max(numVertices, o);
            continue;
        }

        bool orientado = (secao == "ReA" || secao == "ARC");
        bool requerido = (secao == "ReE" || secao == "ReA");

        if (requerido)
            iss >> tag >> o >> d >> c >> dem >> sc;
        else
            iss >> tag >> o >> d >> c;

        numVertices = std::max(numVertices, std::max(o, d));
        servicos.emplace_back(idS++, o, d, c, dem, requerido, orientado);
    }
}

std::vector<Rota> pathScanning(int capacidade, std::vector<Servico> &servicos)
{
    std::vector<Rota> rotas;

    while (true)
    {
        Rota rota;
        bool inseriu = false;

        std::vector<int> indices;
        for (int i = 0; i < (int)servicos.size(); ++i)
            if (!servicos[i].atendido && servicos[i].requerido)
                indices.push_back(i);

        if (indices.empty())
            break;

        std::sort(indices.begin(), indices.end(), [&](int a, int b)
                  { return servicos[a].custo < servicos[b].custo; });

        for (int i : indices)
        {
            Servico &s = servicos[i];
            if (s.atendido || rota.demanda + s.demanda > capacidade)
                continue;

            rota.ids.push_back(s.id);
            rota.demanda += s.demanda;
            rota.custo += s.custo;
            s.atendido = true;
            inseriu = true;
        }

        if (!inseriu)
            break;
        rotas.push_back(rota);
    }

    return rotas;
}

void escrever(const std::string &arq, const std::vector<Rota> &rotas,
              const std::vector<Servico> &servicos, int n)
{
    std::ofstream out(arq);
    int custoTotal = 0, demandaTotal = 0;
    for (const auto &r : rotas)
    {
        custoTotal += r.custo;
        demandaTotal += r.demanda;
    }

    out << n << "\n"
        << rotas.size() << "\n"
        << custoTotal << "\n"
        << demandaTotal << "\n";
    int id = 1;
    for (const auto &r : rotas)
    {
        out << " 0 1 " << id++ << " " << r.ids.size() << " " << r.custo << " " << r.demanda << " (D 0,1,1)";
        for (int sid : r.ids)
        {
            const Servico &s = servicos[sid - 1];
            out << " (S " << s.id << "," << s.origem << "," << s.destino << ")";
        }
        out << " (D 0,1,1)\n";
    }
}

void imprimir(const std::vector<Rota> &rotas, const std::vector<Servico> &servicos, int n)
{
    int custo = 0, dem = 0;
    for (auto &r : rotas)
    {
        custo += r.custo;
        dem += r.demanda;
    }

    std::cout << n << "\n"
              << rotas.size() << "\n"
              << custo << "\n"
              << dem << "\n";
    int id = 1;
    for (const auto &r : rotas)
    {
        std::cout << " 0 1 " << id++ << " " << r.ids.size() << " " << r.custo << " " << r.demanda << " (D 0,1,1)";
        for (int sid : r.ids)
        {
            const Servico &s = servicos[sid - 1];
            std::cout << " (S " << s.id << "," << s.origem << "," << s.destino << ")";
        }
        std::cout << " (D 0,1,1)\n";
    }
}

int main()
{
    int capacidade = 0, deposito = 0, numVertices = 0;
    std::vector<Servico> servicos;

    lerInstancia("mggdb_0.25_10.dat", capacidade, deposito, servicos, numVertices);
    auto rotas = pathScanning(capacidade, servicos);
    escrever("sol-mggdb_0.25_10.dat", rotas, servicos, numVertices);
    imprimir(rotas, servicos, numVertices);
    return 0;
}
