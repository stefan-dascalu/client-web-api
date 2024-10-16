Dascalu Stefan-Nicolae
321CA

# TEMA 4 PROTOCOALE DE COMUNICATIE - CLIENT WEB. COMUNICATIE CU REST API #

Am folosit scheletul oferit in cadrul `laboratorului 9`, adaptat pentru a satisface cerintele acestei teme. Datele sunt parsate si serializate folosind biblioteca `parson`, care ofera functionalitati simple si eficiente pentru manipularea JSON. Functiile principale utilizate includ `json_object_set_string` pentru a crea payload-uri JSON si `json_serialize_to_string_pretty` pentru a serializa obiectele JSON in stringuri.

`Fisierul client.c` contine functia principala a aplicatiei. In acest fisier, programul asteapta comenzi de la utilizator, le proceseaza si apeleaza functiile corespunzatoare pentru fiecare tip de comanda. Utilizatorul poate introduce comenzi precum `register`, `login`, `enter_library`, `logout` si `exit`. Programul utilizeaza un `buffer` pentru a citi si a stoca inputul utilizatorului, apoi parseaza si gestioneaza comenzile prin functiile definite in celelalte fisiere.

## Functionalitati Implementate ##
* `Functia parse_command` este utilizata pentru a analiza comanda introdusa de utilizator si pentru a returna tipul corespunzator de comanda, reprezentat printr-un enum. 

* `Functia handle_command` primeste comanda analizata si, pe baza tipului de comanda, apeleaza functia corespunzatoare pentru a executa comanda.

* `Functia get_credentials` solicita utilizatorului sa introduca un username si o parola, verificand daca aceste campuri contin spatii. Daca username-ul sau parola contin spatii, se afiseaza un mesaj de eroare si se invalideaza inputul.

* `Functia prepare_payload` creeaza un payload JSON cu username-ul si parola introduse de utilizator. Aceasta utilizeaza biblioteca parson pentru a seta campurile corespunzatoare in obiectul JSON si pentru a serializa obiectul intr-un string.

* `Functia send_request` trimite o cerere POST catre server cu payload-ul JSON creat anterior. Aceasta deschide o conexiune cu serverul, trimite cererea si primeste raspunsul, inchizand apoi conexiunea. Raspunsul serverului este returnat pentru a fi procesat ulterior.

* `Functia register_user` primeste username-ul si parola de la utilizator, creeaza un payload JSON si trimite cererea de inregistrare catre server. Raspunsul serverului este analizat si se afiseaza un mesaj de succes sau eroare.

* `Functia login_user` functioneaza prin solicitarea username-ului si parolei, crearea unui payload JSON si trimiterea cererii de autentificare. Cookie-ul returnat de server este extras si salvat pentru utilizarile ulterioare. Se afiseaza mesajul de succes sau eroare in functie de rezultatul cererii.

* `Functia enter_library` verifica daca utilizatorul este autentificat prin prezenta unui cookie valid. Daca este autentificat, trimite cererea de acces la biblioteca si salveaza token-ul JWT returnat de server. In caz de succes, utilizatorul primeste acces la biblioteca protejata.

* `Functia logout_user` trimite cererea de delogare catre server si sterge cookie-ul utilizatorului. Aceasta previne accesul neautorizat dupa delogare si confirma utilizatorului ca sesiunea a fost inchisa.

* `Functia find_json_value` permite gasirea unei valori intr-un string JSON, dat fiind un key. Aceasta este folosita pentru a extrage informatii specifice din raspunsurile serverului.

* `Functia extract_json_list` extrage o lista JSON dintr-un string, fiind utila pentru parsarea listelor de date returnate de server.
