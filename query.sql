-- 1. Query di ricerca
-- Id del pacco consegnato la cui spedizione è la più lunga.

SELECT s1.idPacco, AGE(s2.dataFineFase, s1.dataInizioFase) AS difference
FROM stato_spedizioni AS s1 JOIN stato_spedizioni AS s2 ON s1.idPacco = s2.idPacco
WHERE s2.dataFineFase IS NOT NULL AND s1.dataInizioFase IS NOT NULL AND EXTRACT(EPOCH FROM s2.dataFineFase - s1.dataInizioFase) >= ALL(
	SELECT EXTRACT(EPOCH FROM s2.dataFineFase - s1.dataInizioFase) AS difference
	FROM stato_spedizioni AS s1 JOIN stato_spedizioni AS s2 ON s1.idPacco = s2.idPacco
	WHERE s1.stato = 'PCR-SO' AND s2.stato = 'SO-PCR' AND s2.dataFineFase IS NOT NULL AND s1.dataInizioFase IS NOT NULL);

-- 2. Query di ricerca
-- Mostrare la lista delle sedi e il numero di pacchi passati per esse (partiti o arrivati). Ordinare il risultato in base al numero di pacchi in modo decrescente.

DROP VIEW IF EXISTS pacchiPassati;
CREATE VIEW pacchiPassati AS
SELECT idPacco, idSedeArrivo as idSedePassata
FROM stato_spedizioni
UNION
SELECT idPacco, idSedePartenza as idSedePassata
FROM stato_spedizioni;

SELECT idSedePassata, COUNT(DISTINCT idPacco) as numPacchiPassati
FROM pacchiPassati
GROUP BY idSedePassata
ORDER BY COUNT(DISTINCT idPacco) DESC;

-- 3. Query di ricerca
-- Per ogni regione, identificare i clienti che si sono distinti per il maggior numero di pacchi inviati nella stessa. Mostrare i risultati ordinati in modo alfabetico per regione.

DROP VIEW IF EXISTS regioniClienti;
CREATE VIEW regioniClienti AS
SELECT sedi.regione, clienti.idCliente, COUNT(*) AS numSpedizioni
FROM pacchi JOIN sedi ON pacchi.idSedeDestinazione = sedi.idSede
    JOIN clienti ON pacchi.idCliente = clienti.idCliente
GROUP BY sedi.regione, clienti.idCliente;

SELECT regione, idCliente
FROM regioniClienti as rc1
WHERE numSpedizioni >= ALL
    (SELECT MAX(numSpedizioni)
    FROM regioniClienti as rc2
    WHERE rc1.regione = rc2.regione)
ORDER BY regione;

-- 4. Query di ricerca
-- Mostrare la lista dei clienti che non hanno ancora ricevuto risposta nei messaggi di supporto.

SELECT idCliente, orarioInvio, isFromCliente
FROM supporto
WHERE isFromCliente = FALSE
EXCEPT
SELECT s1.idCliente, s1.orarioInvio, s1.isFromCliente
FROM supporto AS s1 JOIN supporto AS s2 ON s1.idCliente = s2.idCliente AND s1.idOperatore = s2.idOperatore
WHERE s1.orarioInvio < s2.orarioInvio;

-- 5. Query di vincolo
-- Gli operatori indicati nella relazione mezzi devono essere tutti trasportatori.

SELECT operatori.idOperatore
FROM mezzi JOIN operatori ON mezzi.idOperatore = operatori.idOperatore
WHERE operatori.tipoOperatore <> 'Trasportatore';

-- 6. Query di vincolo
-- La sede di arrivo indicata nell’ultimo stato di spedizione di un pacco deve essere uguale alla sede di destinazione indicata in Pacco.

SELECT pacchi.idPacco
FROM stato_spedizioni JOIN pacchi ON stato_spedizioni.idPacco = pacchi.idPacco
WHERE stato_spedizioni.stato = 'SO-PCR' AND stato_spedizioni.idSedeArrivo <> pacchi.idSedeDestinazione;

-- 7. Query di vincolo
-- Verifica la correttezza dei dati nella relazione stato_spedizioni. In particolare, controlla che, tra stati consecutivi dello stesso pacco, date e luoghi siano coerenti.

SELECT ss1.idPacco
FROM stato_spedizioni as ss1 JOIN stato_spedizioni as ss2 ON ss1.idPacco = ss2.idPacco
WHERE ss1.stato = 'PCR-SO' AND ss2.stato = 'SO-CSR' AND (ss1.idSedeArrivo <> ss2.idSedePartenza OR ss1.dataFineFase > ss2.dataInizioFase)
UNION
SELECT ss1.idPacco
FROM stato_spedizioni as ss1 JOIN stato_spedizioni as ss2 ON ss1.idPacco = ss2.idPacco
WHERE ss1.stato = 'SO-CSR' AND ss2.stato = 'CSR-CSR' AND (ss1.idSedeArrivo <> ss2.idSedePartenza OR ss1.dataFineFase > ss2.dataInizioFase)
UNION
SELECT ss1.idPacco
FROM stato_spedizioni as ss1 JOIN stato_spedizioni as ss2 ON ss1.idPacco = ss2.idPacco
WHERE ss1.stato = 'CSR-CSR' AND ss2.stato = 'CSR-SO' AND (ss1.idSedeArrivo <> ss2.idSedePartenza OR ss1.dataFineFase > ss2.dataInizioFase)
UNION
SELECT ss1.idPacco
FROM stato_spedizioni as ss1 JOIN stato_spedizioni as ss2 ON ss1.idPacco = ss2.idPacco
WHERE ss1.stato = 'CSR-SO' AND ss2.stato = 'SO-PCR' AND (ss1.idSedeArrivo <> ss2.idSedePartenza OR ss1.dataFineFase > ss2.dataInizioFase);

-- 8. Query parametrica
-- Mostrare la lista dei pacchi partiti (e non arrivati) dalla sede: *Inserire idSede*.
-- Per risultati interessanti, provare con: 31.

SELECT idPacco
FROM stato_spedizioni
WHERE dataInizioFase IS NOT NULL AND dataFinefase IS NULL AND idSedePartenza = *idSedeInserita*;

-- 9. Query parametrica
-- Mostrare la lista dei pacchi trasportati in qualsiasi fase dal mezzo con targa: *Inserire targa*.
-- Per risultati interessanti, provare con: EF456GH.

SELECT DISTINCT stato_spedizioni.idPacco
FROM mezzi JOIN operatori ON mezzi.idOperatore = operatori.idOperatore
    JOIN stato_spedizioni ON operatori.idOperatore = stato_spedizioni.idOperatore
WHERE targa = *targaInserita*;

-- 10. Query parametrica
-- Mostrare il nome e il cognome del cliente/i che ha concluso più spedizioni nell'anno: *Inserire anno*.
-- Per risultati interessanti, provare con: 2021.

DROP VIEW IF EXISTS clienti_spedizioni_concluse;
CREATE VIEW clienti_spedizioni_concluse AS
SELECT pacchi.idCliente, stato_spedizioni.idPacco, stato_spedizioni.dataFineFase
    FROM stato_spedizioni JOIN pacchi ON stato_spedizioni.idPacco = pacchi.idPacco
WHERE stato_spedizioni.stato = 'SO-PCR' AND stato_spedizioni.dataFineFase IS NOT NULL;

SELECT clienti.nome, clienti.cognome, clienti_migliori.numSpedizioni
FROM
(SELECT csc.idCliente, COUNT(DISTINCT csc.idPacco) as numSpedizioni
FROM clienti_spedizioni_concluse as csc
WHERE EXTRACT(YEAR FROM csc.dataFineFase) = *annoInserito*
GROUP BY csc.idCliente
HAVING COUNT(DISTINCT csc.idPacco) >= ALL(
    SELECT COUNT(DISTINCT csc2.idPacco)
    FROM clienti_spedizioni_concluse as csc2
    WHERE EXTRACT(YEAR FROM csc2.dataFineFase) = *annoInserito*
    GROUP BY csc2.idCliente
)) as clienti_migliori JOIN clienti ON clienti_migliori.idCliente = clienti.idCliente;

