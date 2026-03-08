#include <string>
#include <iostream>
#include <iomanip>
#include <vector>
#include "./dependencies/include/libpq-fe.h"

using namespace std;

#define PG_HOST "127.0.0.1"
#define PG_USER "postgres"
#define PG_PASS "password" // CAMBIARE CON PASSWORD DI PGADMIN
#define PG_PORT 5432
#define PG_DB "quickpack"

class Query {

private:
    string name;
    string query;
    int nParams;
    char** params;
    char** defParams;
    PGresult* result;
    PGresult* stmt;

    void getParams() {
        if (nParams == 0) return;

        params = (char**)::operator new(nParams);

        cout << "Inserisci " << nParams << " parametro/i: \n";
        for (int i = 0; i < nParams; ++i) {
            params[i] = new char[30];
            cin >> params[i];
        }
    }

public:
    Query(string name, string query, int nParams, PGconn* conn, char** defParams = 0)
        : name(name), query(query), nParams(nParams), params(0), result(0), stmt(0), defParams(defParams) {
        stmt = PQprepare(conn, name.c_str(), query.c_str(), nParams, 0);
        if (PQresultStatus(stmt) != 1) {
            cout << "Risultati inconsistenti in preparazione! " << PQerrorMessage(conn) << endl;
            PQclear(stmt);
            stmt = 0;
            exit(1);
        }
    }

    ~Query() {
        PQclear(stmt);
        PQclear(result);
        if (params != 0) delete[] params;
    }

    void printName() {
        cout << name;
    }

    bool execute(PGconn* conn, bool use_defParams = false) {
        if (!(use_defParams && defParams)) {
            cout << "\nEseguendo: \n" << name << "\n\n";
            getParams();
        } else {
            cout << "\nEseguendo: \n" << name << "\n\ncon parametro/i:  ";
            for (int i = 0; i < nParams; ++i) {
                cout << defParams[i] << "; ";
            }
            cout << '\n';
        }

        if (result != 0) {
            PQclear(result);
            result = 0;
        }

        result = PQexecPrepared(conn, name.c_str(), nParams, use_defParams && defParams ? defParams : params, 0, 0, 0);

        if (PQresultStatus(result) != PGRES_TUPLES_OK) {
            cout << "\nRisultati inconsistenti in esecuzione! " << PQerrorMessage(conn) << endl;
            PQclear(result);
            result = 0;
            return false;
        } else return true;
    }

    void print() {
        if (result == 0) return;

        int tuple = PQntuples(result);
        int campi = PQnfields(result);

        cout << "\nRisultato: \n\n";

        for (int i = 0; i < campi; ++i) {
            cout << left << setw(25) << PQfname(result, i);
        }
        cout << "\n\n";
        for (int i = 0; i < tuple; ++i) {
            for (int n = 0; n < campi; ++n) {
                cout << left << setw(25) << PQgetvalue(result, i, n);
            }
            cout << '\n';
        }
        cout << '\n' << endl;
    }
};

int selectQuery() {
    int a = 0;
    while (a > 10 || a == 0 || a < -1) {
        cout << "Inserire il numero della query desiderata (-1 per uscire): ";
        if (!(cin >> a)) {
            cin.clear();
            cin.ignore(10000, '\n');
        }
    }
    return a;
}

int main(int argc, char** argv) {
    char conninfo[250];
    sprintf(conninfo, "user=%s password=%s dbname=%s hostaddr=%s port=%d", PG_USER, PG_PASS, PG_DB, PG_HOST, PG_PORT);

    PGconn* conn;
    conn = PQconnectdb(conninfo);

    cout << '\n';
    if (PQstatus(conn) != CONNECTION_OK) {
        cout << "Errore di connessione \n\n" << PQerrorMessage(conn);
        PQfinish(conn);
        exit(1);
    } else {
        cout << "Connessione avvenuta correttamente \n\n";
    }

    cout << "+--------------------------------------------------------------------+\n";
    cout << "|                             QUICK PACK                             |\n";
    cout << "|                     Spedizioni veloci e sicure                     |\n";
    cout << "+--------------------------------------------------------------------+\n\n\n";

    vector<Query*> q;

    q.push_back(new Query("1 - Id del pacco consegnato la cui spedizione e' la piu' lunga.",
        "SELECT s1.idPacco, AGE(s2.dataFineFase, s1.dataInizioFase) AS difference \
        FROM stato_spedizioni AS s1 JOIN stato_spedizioni AS s2 ON s1.idPacco = s2.idPacco \
        WHERE s2.dataFineFase IS NOT NULL AND s1.dataInizioFase IS NOT NULL AND EXTRACT(EPOCH FROM s2.dataFineFase - s1.dataInizioFase) >= ALL( \
            SELECT EXTRACT(EPOCH FROM s2.dataFineFase - s1.dataInizioFase) AS difference \
            FROM stato_spedizioni AS s1 JOIN stato_spedizioni AS s2 ON s1.idPacco = s2.idPacco \
            WHERE s1.stato = 'PCR-SO' AND s2.stato = 'SO-PCR' AND s2.dataFineFase IS NOT NULL AND s1.dataInizioFase IS NOT NULL)", 0, conn));

    q.push_back(new Query("2 - Mostrare la lista delle sedi e il numero di pacchi passati per esse (partiti o arrivati). Ordinare il risultato in base al numero di pacchi in modo decrescente.",
        "SELECT idSedePassata, COUNT(DISTINCT idPacco) as numPacchiPassati \
        FROM pacchiPassati \
        GROUP BY idSedePassata \
        ORDER BY COUNT(DISTINCT idPacco) DESC;", 0, conn));

    q.push_back(new Query("3 - Per ogni regione, identificare i clienti che si sono distinti per il maggior numero di pacchi inviati nella stessa. Mostrare i risultati ordinati in modo alfabetico per regione.",
        "SELECT regione, idCliente \
        FROM regioniClienti as rc1 \
        WHERE numSpedizioni >= ALL \
            (SELECT MAX(numSpedizioni) \
        FROM regioniClienti as rc2 \
            WHERE rc1.regione = rc2.regione) \
        ORDER BY regione;", 0, conn));

    q.push_back(new Query("4 - Mostrare la lista dei clienti che non hanno ancora ricevuto risposta nei messaggi di supporto.",
        "SELECT idCliente, orarioInvio, isFromCliente \
        FROM supporto \
        WHERE isFromCliente = FALSE \
        EXCEPT \
        SELECT s1.idCliente, s1.orarioInvio, s1.isFromCliente \
        FROM supporto AS s1 JOIN supporto AS s2 ON s1.idCliente = s2.idCliente AND s1.idOperatore = s2.idOperatore \
        WHERE s1.orarioInvio < s2.orarioInvio;", 0, conn));

    q.push_back(new Query("5 - Gli operatori indicati nella relazione mezzi devono essere tutti trasportatori. (Vuoto se rispettato)",
        "SELECT operatori.idOperatore \
        FROM mezzi JOIN operatori ON mezzi.idOperatore = operatori.idOperatore \
        WHERE operatori.tipoOperatore <> 'Trasportatore';", 0, conn));

    q.push_back(new Query("6 - La sede di arrivo indicata nell'ultimo stato di spedizione di un pacco deve essere uguale alla sede di destinazione indicata in Pacco. (Vuoto se rispettato)",
        "SELECT pacchi.idPacco \
        FROM stato_spedizioni JOIN pacchi ON stato_spedizioni.idPacco = pacchi.idPacco \
        WHERE stato_spedizioni.stato = 'SO-PCR' AND stato_spedizioni.idSedeArrivo <> pacchi.idSedeDestinazione;", 0, conn));

    q.push_back(new Query("7 - Verifica la correttezza dei dati nella relazione stato_spedizioni. In particolare, controlla che, tra stati consecutivi dello stesso pacco, date e luoghi siano coerenti. (Vuoto se rispettato)",
        "SELECT ss1.idPacco \
        FROM stato_spedizioni as ss1 JOIN stato_spedizioni as ss2 ON ss1.idPacco = ss2.idPacco \
        WHERE ss1.stato = 'PCR-SO' AND ss2.stato = 'SO-CSR' AND(ss1.idSedeArrivo <> ss2.idSedePartenza OR ss1.dataFineFase > ss2.dataInizioFase) \
        UNION \
        SELECT ss1.idPacco \
        FROM stato_spedizioni as ss1 JOIN stato_spedizioni as ss2 ON ss1.idPacco = ss2.idPacco \
        WHERE ss1.stato = 'SO-CSR' AND ss2.stato = 'CSR-CSR' AND(ss1.idSedeArrivo <> ss2.idSedePartenza OR ss1.dataFineFase > ss2.dataInizioFase) \
        UNION \
        SELECT ss1.idPacco \
        FROM stato_spedizioni as ss1 JOIN stato_spedizioni as ss2 ON ss1.idPacco = ss2.idPacco \
        WHERE ss1.stato = 'CSR-CSR' AND ss2.stato = 'CSR-SO' AND(ss1.idSedeArrivo <> ss2.idSedePartenza OR ss1.dataFineFase > ss2.dataInizioFase) \
        UNION \
        SELECT ss1.idPacco \
        FROM stato_spedizioni as ss1 JOIN stato_spedizioni as ss2 ON ss1.idPacco = ss2.idPacco \
        WHERE ss1.stato = 'CSR-SO' AND ss2.stato = 'SO-PCR' AND(ss1.idSedeArrivo <> ss2.idSedePartenza OR ss1.dataFineFase > ss2.dataInizioFase);", 0, conn));

    char* p1 = (char*)"31";
    q.push_back(new Query("8 - Mostrare la lista dei pacchi partiti (e non arrivati) dalla sede: *Inserire idSede*.",
        "SELECT idPacco \
        FROM stato_spedizioni \
        WHERE dataInizioFase IS NOT NULL AND dataFinefase IS NULL AND idSedePartenza = $1::int;", 1, conn, &p1));

    char* p2 = (char*)"EF456GH";
    q.push_back(new Query("9 - Mostrare la lista dei pacchi trasportati in qualsiasi fase dal mezzo con targa: *Inserire targa*.",
        "SELECT DISTINCT stato_spedizioni.idPacco \
        FROM mezzi JOIN operatori ON mezzi.idOperatore = operatori.idOperatore \
        JOIN stato_spedizioni ON operatori.idOperatore = stato_spedizioni.idOperatore \
        WHERE targa = $1::varchar(10);", 1, conn, &p2));

    char* p3 = (char*)"2021";
    q.push_back(new Query("10 - Mostrare il nome e il cognome del cliente/i che ha concluso piu' spedizioni nell'anno: *Inserire anno*.",
        "SELECT clienti.nome, clienti.cognome, clienti_migliori.numSpedizioni \
        FROM \
            (SELECT csc.idCliente, COUNT(DISTINCT csc.idPacco) as numSpedizioni \
            FROM clienti_spedizioni_concluse as csc \
            WHERE EXTRACT(YEAR FROM csc.dataFineFase) = $1::int \
            GROUP BY csc.idCliente \
            HAVING COUNT(DISTINCT csc.idPacco) >= ALL( \
                SELECT COUNT(DISTINCT csc2.idPacco) \
                FROM clienti_spedizioni_concluse as csc2 \
                WHERE EXTRACT(YEAR FROM csc2.dataFineFase) = $1::int \
                GROUP BY csc2.idCliente \
            )) as clienti_migliori JOIN clienti ON clienti_migliori.idCliente = clienti.idCliente;", 1, conn, &p3));

    bool end = false;
    while (end != true) {
        cout << "Lista delle Query:\n";
        for (auto it = q.begin(); it != q.end(); ++it) {
            (*it)->printName();
            cout << "\n";
        }
        int choice = selectQuery();
        if (choice == -1) {
            end = true;
            break;
        } else if (choice < (q.size() + 1) && choice > 0) {
            q[choice - 1]->execute(conn);
            q[choice - 1]->print();
        } else cout << "Input invalido\n";
    }

    PQfinish(conn);
    return 0;
}