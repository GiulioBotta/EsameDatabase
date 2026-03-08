CREATE TABLE CLIENTI (
    idCliente int NOT NULL PRIMARY KEY,
    nome varchar(30) NOT NULL,
    cognome varchar(30) NOT NULL,
    email varchar(30) NOT NULL,
    numTelefono varchar(15) NOT NULL
);

CREATE TYPE tipiSede AS ENUM ('CRS', 'SO', 'PCR');

CREATE TABLE SEDI (
    idSede int NOT NULL PRIMARY KEY,
    email varchar(30) NOT NULL,
    numTelefono varchar(15) NOT NULL,
    tipoSede tipiSede NOT NULL,
    indirizzo varchar(30) NOT NULL,
    CAP char(5) NOT NULL,
    provincia varchar(30) NOT NULL,
    regione varchar(30) NOT NULL,
    nomeAttivita varchar(30),
    dataScadenzaAffiliazione timestamp
);

CREATE TYPE tipiOperatore AS ENUM ('Magazziniere', 'Trasportatore');

CREATE TABLE OPERATORI (
    idOperatore int NOT NULL PRIMARY KEY,
    nome varchar(30) NOT NULL,
    cognome varchar(30) NOT NULL,
    email varchar(30) NOT NULL,
    numTelefono varchar(15) NOT NULL,
    tipoOperatore tipiOperatore NOT NULL,
    idSedeLavoro int NOT NULL,
    FOREIGN KEY (idSedeLavoro) REFERENCES SEDI(idSede)
);

CREATE TYPE dimPacco AS ENUM ('Piccolo', 'Medio', 'Grande');

CREATE TABLE PACCHI (
    idPacco int NOT NULL PRIMARY KEY,
    idCliente int NOT NULL,
    dataOrdine timestamp NOT NULL,
    idSedeDestinazione int NOT NULL,
    dimensioni dimPacco NOT NULL,
    peso float NOT NULL,
    isFragile boolean NOT NULL,
    FOREIGN KEY (idCliente) REFERENCES CLIENTI(idCliente),
    FOREIGN KEY (idSedeDestinazione) REFERENCES SEDI(idSede)
);

CREATE TYPE statiSpedizione AS ENUM('PCR-SO', 'SO-CRS', 'CRS-CRS', 'CRS-SO', 'SO-PCR');

CREATE TABLE STATO_SPEDIZIONI (
    idPacco int NOT NULL,
    stato statiSpedizione NOT NULL,
    idSedePartenza int NOT NULL,
    idSedeArrivo int NOT NULL,
    dataInizioFase timestamp,
    dataFineFase timestamp,
    idOperatore int NOT NULL,
    PRIMARY KEY(idPacco, stato),
    FOREIGN KEY (idSedePartenza) REFERENCES SEDI(idSede),
    FOREIGN KEY (idSedeArrivo) REFERENCES SEDI(idSede),
    FOREIGN KEY (idOperatore) REFERENCES OPERATORI(idOperatore)
);

CREATE TABLE MEZZI (
    targa varchar(10) NOT NULL PRIMARY KEY,
    idOperatore int NOT NULL,
    caricoMax float NOT NULL,
    idSedeAppartenenza int,
    FOREIGN KEY(idOperatore) REFERENCES OPERATORI(idOperatore),
    FOREIGN KEY(idSedeAppartenenza) REFERENCES SEDI(idSede)
);

CREATE TABLE SUPPORTO (
    idCliente int NOT NULL,
    idOperatore int NOT NULL,
    orarioInvio timestamp NOT NULL,
    messaggio varchar(200) NOT NULL,
    isFromCliente boolean NOT NULL,
    PRIMARY KEY(idCliente, idOperatore, orarioInvio),
    FOREIGN KEY(idCliente) REFERENCES CLIENTI(idCliente),
    FOREIGN KEY(idOperatore) REFERENCES OPERATORI(idOperatore)
)