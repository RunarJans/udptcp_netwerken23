# udptcp_netwerken23
netwerken taak udptcp herkansing




De code is verdeeld in verschillende secties:

Inclusies en Preprocessor Directives: Deze sectie bevat preprocessordirectives om de benodigde bibliotheken in te sluiten op basis van het besturingssysteem. Voor Windows wordt Winsock gebruikt, en voor Linux worden de standaard netwerk- en systeembibliotheken gebruikt.

Functie Definities voor Initialisatie en Opschoning: Hier zijn functies gedefinieerd om de initiële setup en opschoning van het netwerk uit te voeren. De functies variëren afhankelijk van het besturingssysteem.

UDP Functies: Deze functies zijn specifiek voor UDP-communicatie. Er zijn functies voor de initiële opzet, uitvoering en opschoning van UDP-client en -server.

TCP Functies: Deze functies zijn specifiek voor TCP-communicatie. Er zijn functies voor de initiële opzet, uitvoering en opschoning van de TCP-client en -server.

main-functie: De main-functie is de instappunt voor het programma. Hier worden de verschillende delen van het programma gecoördineerd en aangeroepen. De functies worden achtereenvolgens opgeroepen voor zowel UDP als TCP.

De belangrijkste stappen in het programma omvatten:

Initialisatie van het netwerk en het opzetten van sockets.
UDP: Verzenden en ontvangen van willekeurige getallen tussen client en server, berekenen van het hoogste getal, en resultaten naar de client sturen.
TCP: Verbinding maken tussen de client en de server, verzenden en ontvangen van wiskundige bewerkingen en resultaten, en opschonen van de verbindingen.
Algemeen gezien implementeert deze code een eenvoudige client-server-interactie waarbij de server willekeurige getallen genereert en bewerkingen uitvoert, en de resultaten naar de client stuurt via zowel UDP- als TCP-verbindingen.
