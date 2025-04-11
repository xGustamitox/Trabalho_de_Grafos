#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <limits>
#include <queue>
#include <iomanip>
using namespace std;

const int INF = numeric_limits<int>::max();

class Aresta
{
public:
    int origem, destino, custo, demanda;
    bool requerido;
    bool orientada;

    Aresta(int o, int d, int c, int dem, bool req, bool ori)
        : origem(o), destino(d), custo(c), demanda(dem), requerido(req), orientada(ori) {}
};

class Grafo
{
private:
    int numVertices;
    vector<Aresta> arestas;
    unordered_map<int, vector<pair<int, int>>> adj;

public:
    Grafo() : numVertices(0) {}

    void adicionarAresta(int o, int d, int c, int dem, bool req, bool ori)
    {
        arestas.emplace_back(o, d, c, dem, req, ori);
        adj[o].emplace_back(d, c);
        if (!ori)
        {
            adj[d].emplace_back(o, c);
        }
    }

    void carregarDeArquivo(const string &nomeArquivo)
    {
        ifstream arq(nomeArquivo);
        if (!arq.is_open())
        {
            cerr << "Erro ao abrir o arquivo." << endl;
            exit(1);
        }

        string linha;
        string secao = "";

        while (getline(arq, linha))
        {
            if (linha.empty())
                continue;

            if (linha.find("ReN.") != string::npos)
            {
                secao = "ReN";
                continue;
            }
            else if (linha.find("ReE.") != string::npos)
            {
                secao = "ReE";
                continue;
            }
            else if (linha.find("EDGE") != string::npos && linha.find("NrE") != string::npos)
            {
                secao = "EDGE";
                continue;
            }
            else if (linha.find("ReA.") != string::npos)
            {
                secao = "ReA";
                continue;
            }
            else if (linha.find("ARC") != string::npos)
            {
                secao = "ARC";
                continue;
            }

            if (secao == "ReN")
            {
                string id;
                int demanda, custo;
                istringstream iss(linha);
                iss >> id >> demanda >> custo;
                int v = stoi(id.substr(1));
                numVertices = max(numVertices, v);
            }
            else if (secao == "ReE")
            {
                string id;
                int o, d, c, dem, sc;
                istringstream iss(linha);
                iss >> id >> o >> d >> c >> dem >> sc;
                adicionarAresta(o, d, c, dem, true, false);
                numVertices = max(numVertices, max(o, d));
            }
            else if (secao == "EDGE")
            {
                string id;
                int o, d, c;
                istringstream iss(linha);
                iss >> id >> o >> d >> c;
                adicionarAresta(o, d, c, 0, false, false);
                numVertices = max(numVertices, max(o, d));
            }
            else if (secao == "ReA")
            {
                string id;
                int o, d, c, dem, sc;
                istringstream iss(linha);
                iss >> id >> o >> d >> c >> dem >> sc;
                adicionarAresta(o, d, c, dem, true, true);
                numVertices = max(numVertices, max(o, d));
            }
            else if (secao == "ARC")
            {
                string id;
                int o, d, c;
                istringstream iss(linha);
                iss >> id >> o >> d >> c;
                adicionarAresta(o, d, c, 0, false, true);
                numVertices = max(numVertices, max(o, d));
            }
        }

        arq.close();
    }

    double calcularDensidade() const
    {
        int m = arestas.size();
        return (double)(2 * m) / (numVertices * (numVertices - 1));
    }

    int encontrarComponente(int v, unordered_set<int> &visitado) const
    {
        queue<int> fila;
        fila.push(v);
        visitado.insert(v);
        while (!fila.empty())
        {
            int u = fila.front();
            fila.pop();
            for (auto [viz, _] : adj.at(u))
            {
                if (visitado.find(viz) == visitado.end())
                {
                    visitado.insert(viz);
                    fila.push(viz);
                }
            }
        }
        return 1;
    }

    int contarComponentesConectados() const
    {
        unordered_set<int> visitado;
        int comp = 0;
        for (int v = 1; v <= numVertices; ++v)
        {
            if (adj.find(v) != adj.end() && visitado.find(v) == visitado.end())
            {
                comp += encontrarComponente(v, visitado);
            }
        }
        return comp;
    }

    pair<int, int> grauMinMax() const
    {
        unordered_map<int, unordered_set<int>> vizinhos;
        for (const auto &a : arestas)
        {
            vizinhos[a.origem].insert(a.destino);
            if (!a.orientada)
                vizinhos[a.destino].insert(a.origem);
        }

        int gmin = INF, gmax = 0;
        for (int i = 1; i <= numVertices; ++i)
        {
            int grau = vizinhos[i].size();
            if (grau > 0)
            {
                gmin = min(gmin, grau);
                gmax = max(gmax, grau);
            }
        }
        return {gmin == INF ? 0 : gmin, gmax};
    }

    void imprimirEstatisticas()
    {
        int qtdArestas = 0, qtdArcos = 0;
        int reqArestas = 0, reqArcos = 0;
        unordered_set<int> verticesRequeridos;

        for (const auto &a : arestas)
        {
            if (a.orientada)
                qtdArcos++;
            else
                qtdArestas++;

            if (a.requerido)
            {
                if (a.orientada)
                    reqArcos++;
                else
                    reqArestas++;
                verticesRequeridos.insert(a.origem);
                verticesRequeridos.insert(a.destino);
            }
        }

        auto [gmin, gmax] = grauMinMax();

        cout << fixed << setprecision(4);
        cout << "1. Quantidade de vertices: " << numVertices << endl;
        cout << "2. Quantidade de arestas (nao orientadas): " << qtdArestas << endl;
        cout << "3. Quantidade de arcos (orientadas): " << qtdArcos << endl;
        cout << "4. Vertices requeridos: " << verticesRequeridos.size() << endl;
        cout << "5. Arestas requeridas: " << reqArestas << endl;
        cout << "6. Arcos requeridos: " << reqArcos << endl;
        cout << "7. Densidade: " << calcularDensidade() << endl;
        cout << "8. Componentes conectados: " << contarComponentesConectados() << endl;
        cout << "9. Grau minimo: " << gmin << endl;
        cout << "10. Grau maximo: " << gmax << endl;
    }
};

int main()
{
    Grafo grafo;
    grafo.carregarDeArquivo("DI-NEARP-n422-Q8k.dat");
    grafo.imprimirEstatisticas();
    return 0;
}