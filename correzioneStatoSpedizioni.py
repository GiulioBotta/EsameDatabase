import random as r

file = open("stato_spedizioni_aggiunta_2.txt", "r")

# Legge tutte le righe del file e le salva in una lista
righe = file.readlines()

for i in range(0, (len(righe) - 5) // 5):
    nCanc = r.randint(0, 4)
    for j in range(nCanc):
        righe[5 * i + 4 - j] = ""
    if nCanc == 0:
        scelta = r.randint(0, 1)
        if (scelta) == 1:
            righe[5 * i + 4 - nCanc] = (
                righe[5 * i + 4 - nCanc][
                    : righe[5 * i + 4 - nCanc].find("), TO_TIMESTAMP(") + 3
                ]
                + "NULL"
                + righe[5 * i + 4 - nCanc][
                    righe[5 * i + 4 - nCanc].find("), TO_TIMESTAMP(") + 63 :
                ]
            )
    else:
        righe[5 * i + 4 - nCanc] = (
            righe[5 * i + 4 - nCanc][
                : righe[5 * i + 4 - nCanc].find("), TO_TIMESTAMP(") + 3
            ]
            + "NULL"
            + righe[5 * i + 4 - nCanc][
                righe[5 * i + 4 - nCanc].find("), TO_TIMESTAMP(") + 63 :
            ]
        )

# Chiude il file
file.close()


file = open("stato_spedizioni_c.txt", "w")

for r in righe:
    if r != "":
        file.write("(" + r)

file.close()
