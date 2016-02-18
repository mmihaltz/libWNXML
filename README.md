# libWNXML

C++ API and command-line executable for querying Hungarian WordNet (HuWN) XML files.

Author: Márton Miháltz <mmihaltz@gmail.com>

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
ENG20-02001223-n  {Canis familiaris:1, házikutya:1, kutya:1, eb:1}  (Ház- és nyájõrzésre, vadászatra használt vagy kedvtelésbõl tartott háziállat.)
  ENG20-02001977-n  {korcs kutya:1, korcs:1, keverék kutya:1}  (Nem fajtatiszta kutya.)
  ENG20-02002490-n  {öleb:1}  (Kedvtelésbõl tartott kis testû kutya.)
  ENG20-02004217-n  {vadászkutya:1}  (Vadászatra idomított kutyafajta.)
  ENG20-02020367-n  {munkakutya:1}  (Hasznos szolgálatáért tenyésztett, tartott kutya.)
  ENG20-02027313-n  {mopszli:1, mopsz:1}  (Tömpe orrú, kerek fejû öleb, szõre sima, farka hátára görbül; ázsiai eredetû.)
  ENG20-02027929-n  {spicc:1}  (Hegyes orrú, álló fülû, hosszú szõrû, zömök kutyafajta; északi eredetû.)
  ENG20-02029627-n  {uszkár:1, pudli:1}  (Hosszú, selymes fehér, fekete stb. szõrû, rendszerint különlegesen nyírt kisebb õsi kutyafajta, intelligens, gyakran vadászatra vagy elõadásra idomították.)

ENG20-09256536-n  {kutya:2}  (Jelzett tulajdonsága miatt megvetést érdemlõ személy.)
```

## About LibWNXML

Source code and .vcproj project files for MS Visual Studio are included.
**Please note** that you will not be able to build the sources since they 
still depend on proprietary 3rd party libraries ((C) MorphoLogic). Contact 
me if you need the code to be upgraded to be buildable, or feel free to do it 
yourself, pull requests are welcome. Until then this library is condidered abandonware, I guess.
