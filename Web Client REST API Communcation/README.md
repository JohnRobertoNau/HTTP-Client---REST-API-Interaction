

-------------------------------------------------------------------------------
Pentru acest proiect am folosit fisierele buffer.c, buffer.h, helpers.c,
helpers.h din scheletul laboratorului 9, iar fisierele parson.c si parson.h
le-am clonat de pe girhubul prezentat in enuntul temei.
Rezolvarea propriu-zisa este in fisierul client.c

Pentru inceput, am setat HOSTUL la 34.246.184.49, PORTUL la 8080 si alte
lungimi de care m-am folosit pe parcursul implementarii temei. Apoi am definit
variabile globale in care stochez bearer-tokenul si session cookie-ul.

In main am citit fiecare instructiune cu ajutorul charului command.

- Daca de la tastatura se citeste 'register', in main se va apela functia
register. In functia register se creaza o cerere HTTP de tip POST pentru a
inregistra utilizatorul in server. Cererea este trimisa folosind functia
send_to_server. Se analizeaza raspunsul primit de la server, iar daca in
raspuns se gaseste mesajul "200 OK" sau "201 Created" se afiseaza succes,
altfel se afiseaza ERROR.

- Daca de la tastatura se citeste login_user, se apeleaza functia respectiva.
In functia login_user se cere sa se introduca credentialele. Se creaza o cerere
de tip POST pentru autentificare, iar raspunsul este analizat. Daca raspunsul
contine "200 OK" se afiseaza SUCCESS, altfel se afiseaza ERROR. Daca
autentificarea este un SUCCESS, se stocheaza cookie-ul de sesiune din raspuns.

- Daca de la tastatura se citeste enter_library, se va apela functia aferenta.
Aici se verifica daca exista un cookie valid. Se trimite o cerere de tip GET
pentru a accesa biblioteca si se verifica raspunsul. Daca este un SUCCESS
(contine 200 OK) se stocheaza tokenul JWT pe care il vom folosi pentru a accesa
anumite functii din biblioteca.

- Daca se citeste get_books, se va apela functia respectiva. In functia
get_books ne asiguram ca suntem autentificati, daca nu suntem se afiseaza un
mesaj de eroare. Se trimite o cerere de tip GET pentru a obtine lista de carti
cu id-ul aferent. Raspunsul este analizat si se extrage pentru a afisa ID-ul
si titlul din JSON-ul primit.
    * Functia display_book_details primeste ca parametru un sir de caractere,
    response, care contine raspunsul primit de la server. Folosim functia
    basic_extract_json_response pentru a separa continutul JSON.
      Dupa extragerea JSONului, functia json_parse_string din biblioteca Parson
    este folosita pentru a converti textul JSON in JSON_Value. Se verifica daca
    valoarea returnata este un JSON valid, cu ajutorul functiei
    json_value_get_type.
      Daca parsarea este reusita, se continua extragerea obiectului din JSON,
    cu json_value_get_object. De aici, functia extrage fiecare camp al cartii
    id, title, author, publisher, genre si page_count. Folosim functiile
    json_object_get_number si json_object_get_string. La final se elibereaza
    memoria alocata pentru JSON_Value cu ajutorul functiei json_value_free.

- Daca de la tastatura se citeste get_book, se apeleaza functia cu acelasi nume
In aceasta functie se verifica daca avem acces la biblioteca. Se solicita ID-ul
unei carti si se trimite o cerere GET pentru a obtine detalii despre cartea
respectiva. Detaliile cartii sunt afisate folosind functia display_book_details

- Daca de la tastatura se citeste add_book, se apeleaza instructiunea
respectiva. Se verifica daca utilizatorul are acces la biblioteca iar
utilizatorul este intrebat ce detalii va avea cartea adaugata de el (title, 
author, genre, page_count, publisher). Se trimtie o cerere de tip POST pentru
a adauga cartea in biblioteca si se analizeaza raspunsul. Pentru citirea
datelor am folosit functia fgets pentru a nu avea probleme cand se introduc
spatii. Pentru page_count am citit cu fgets apoi am folosit functia atoi pentru
a stoca numarul.

- Daca de la tastatura se citeste functia delete_book, se apeleaza functia.
Se verifica accesul la biblioteca si utilizatorul este intrebat id-ul cartii
care trebuie sterse. Se creaza o cerere de tip DELETE pentru a sterge cartea.
Raspunsul este analizat si in functie de acesta se afiseaza ERROR sau SUCCESS.

- Daca de la tastatura se citeste logout, se va apela functia log_out.
Aici se verifica daca utilizatorul este autentificat. Se trimite o cerere de
tip GET pentru a ne deconecta, iar cookie-ul si tokenul JWT sunt curatati.

- Daca de la tastatura nu se citeste niciuna din aceste instructiuni se va
afisa "ERROR , unknown command"
-------------------------------------------------------------------------------
