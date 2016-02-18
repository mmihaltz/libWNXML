# libWNXML

C++ API and command-line executable for querying Hungarian WordNet (HuWN) XML files.

Author: M�rton Mih�ltz <mmihaltz@gmail.com>

**Contents:**

- `WNXMLConsole.exe`: standalone command-line Windows application binary for querying HuWN
- `LibWNXML`: C++ API for querying HuWN XML files, used by WNXMLConsole.

## Using WNXMLConsole

*Requirements*: you will need the latest HuWN XML files (download from https://github.com/dlt-rilmta/huwn).

Open a command-line prompt in Windows, change to the directory containing WNXMLConsole.exe and type:

`wnxmlconsole.exe <huwn_xml_file>`

Once `WNXMLConsole` has started, type `.h` to get help on the available commands.

The following example queries hyponyms for all senses of the noun *kutya*:

```
>.rl kutya n hyponym
ENG20-02001223-n  {Canis familiaris:1, h�zikutya:1, kutya:1, eb:1}  (H�z- �s ny�j�rz�sre, vad�szatra haszn�lt vagy kedvtel�sb�l tartott h�zi�llat.)
  ENG20-02001977-n  {korcs kutya:1, korcs:1, kever�k kutya:1}  (Nem fajtatiszta kutya.)
  ENG20-02002490-n  {�leb:1}  (Kedvtel�sb�l tartott kis test� kutya.)
  ENG20-02004217-n  {vad�szkutya:1}  (Vad�szatra idom�tott kutyafajta.)
  ENG20-02020367-n  {munkakutya:1}  (Hasznos szolg�lat��rt teny�sztett, tartott kutya.)
  ENG20-02027313-n  {mopszli:1, mopsz:1}  (T�mpe orr�, kerek fej� �leb, sz�re sima, farka h�t�ra g�rb�l; �zsiai eredet�.)
  ENG20-02027929-n  {spicc:1}  (Hegyes orr�, �ll� f�l�, hossz� sz�r�, z�m�k kutyafajta; �szaki eredet�.)
  ENG20-02029627-n  {uszk�r:1, pudli:1}  (Hossz�, selymes feh�r, fekete stb. sz�r�, rendszerint k�l�nlegesen ny�rt kisebb �si kutyafajta, intelligens, gyakran vad�szatra vagy el�ad�sra idom�tott�k.)

ENG20-09256536-n  {kutya:2}  (Jelzett tulajdons�ga miatt megvet�st �rdeml� szem�ly.)
```

## About LibWNXML

Source code and .vcproj project files for MS Visual Studio are included.
**Please note** that you will not be able to build the sources since they 
still depend on proprietary 3rd party libraries ((C) MorphoLogic). Contact 
me if you need the code to be upgraded to be buildable, or feel free to do it 
yourself, pull requests are welcome. Until then this library is condidered abandonware, I guess.
